#include "ContentsEditorSelectionBridge.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/note/WhatSonLocalNoteFileStore.hpp"
#include "file/note/WhatSonNoteFileStatSupport.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QMetaObject>
#include <QMetaProperty>
#include <QThreadPool>

#include <algorithm>

namespace
{
    constexpr auto kApplyPersistedBodyStateForNoteSignature =
        "applyPersistedBodyStateForNote(QString,QString,QString,QString)";
    constexpr auto kRequestTrackedStatisticsRefreshForNoteSignature =
        "requestTrackedStatisticsRefreshForNote(QString,bool)";
    constexpr auto kSaveBodyTextForNoteSignature = "saveBodyTextForNote(QString,QString)";
    constexpr auto kSaveCurrentBodyTextSignature = "saveCurrentBodyText(QString)";
    constexpr auto kNoteDirectoryPathForNoteIdSignature = "noteDirectoryPathForNoteId(QString)";
    constexpr auto kReloadNoteMetadataForNoteIdSignature = "reloadNoteMetadataForNoteId(QString)";
}

ContentsEditorSelectionBridge::ContentsEditorSelectionBridge(QObject* parent)
    : QObject(parent)
{
}

ContentsEditorSelectionBridge::~ContentsEditorSelectionBridge() = default;

QObject* ContentsEditorSelectionBridge::noteListModel() const noexcept
{
    return m_noteListModel;
}

void ContentsEditorSelectionBridge::setNoteListModel(QObject* model)
{
    if (m_noteListModel == model)
    {
        return;
    }

    disconnectNoteListModel();
    resetBoundNotePersistenceSession();
    m_noteListModel = model;

    if (m_noteListModel != nullptr)
    {
        m_noteListDestroyedConnection = connect(
            m_noteListModel,
            &QObject::destroyed,
            this,
            &ContentsEditorSelectionBridge::handleNoteListDestroyed);
        m_currentNoteIdChangedConnection = connect(
            m_noteListModel,
            SIGNAL(currentNoteIdChanged()),
            this,
            SLOT(handleNoteListSelectionChanged()));
        m_currentBodyTextChangedConnection = connect(
            m_noteListModel,
            SIGNAL(currentBodyTextChanged()),
            this,
            SLOT(handleNoteListSelectionChanged()));
        m_itemCountChangedConnection = connect(
            m_noteListModel,
            SIGNAL(itemCountChanged()),
            this,
            SLOT(handleNoteListCountChanged()));
    }

    emit noteListModelChanged();
    refreshNoteSelectionState();
    refreshNoteCountState();
}

QObject* ContentsEditorSelectionBridge::contentViewModel() const noexcept
{
    return m_contentViewModel;
}

void ContentsEditorSelectionBridge::setContentViewModel(QObject* model)
{
    if (m_contentViewModel == model)
    {
        return;
    }

    disconnectContentViewModel();
    resetBoundNotePersistenceSession();
    m_contentViewModel = model;

    if (m_contentViewModel != nullptr)
    {
        m_contentViewModelDestroyedConnection = connect(
            m_contentViewModel,
            &QObject::destroyed,
            this,
            &ContentsEditorSelectionBridge::handleContentViewModelDestroyed);
    }

    emit contentViewModelChanged();
    refreshContentPersistenceState();
}

bool ContentsEditorSelectionBridge::noteSelectionContractAvailable() const noexcept
{
    return m_noteSelectionContractAvailable;
}

bool ContentsEditorSelectionBridge::noteCountContractAvailable() const noexcept
{
    return m_noteCountContractAvailable;
}

bool ContentsEditorSelectionBridge::contentPersistenceContractAvailable() const noexcept
{
    return m_contentPersistenceContractAvailable;
}

QString ContentsEditorSelectionBridge::selectedNoteId() const
{
    return m_selectedNoteId;
}

QString ContentsEditorSelectionBridge::selectedNoteBodyText() const
{
    return m_selectedNoteBodyText;
}

int ContentsEditorSelectionBridge::visibleNoteCount() const noexcept
{
    return m_visibleNoteCount;
}

bool ContentsEditorSelectionBridge::persistEditorTextForNote(const QString& noteId, const QString& text)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty() || m_contentViewModel == nullptr || !m_contentPersistenceContractAvailable)
    {
        return false;
    }

    if (directPersistenceAvailable())
    {
        return enqueueDirectPersistenceRequest(normalizedNoteId, text);
    }

    bool saved = false;
    if (hasInvokableMethod(m_contentViewModel, kSaveBodyTextForNoteSignature))
    {
        return QMetaObject::invokeMethod(
                m_contentViewModel,
                "saveBodyTextForNote",
                Qt::DirectConnection,
                Q_RETURN_ARG(bool, saved),
                Q_ARG(QString, normalizedNoteId),
                Q_ARG(QString, text))
            && saved;
    }

    if (hasInvokableMethod(m_contentViewModel, kSaveCurrentBodyTextSignature))
    {
        return QMetaObject::invokeMethod(
            m_contentViewModel,
            "saveCurrentBodyText",
                Qt::DirectConnection,
                Q_RETURN_ARG(bool, saved),
                Q_ARG(QString, text))
            && saved;
    }

    return false;
}

bool ContentsEditorSelectionBridge::directPersistenceAvailable() const noexcept
{
    return m_directPersistenceContractAvailable;
}

bool ContentsEditorSelectionBridge::refreshSelectedNoteSnapshot()
{
    const QString currentNoteId = readStringProperty(m_noteListModel, "currentNoteId").trimmed();
    bool reloaded = false;
    if (!currentNoteId.isEmpty() && hasInvokableMethod(m_contentViewModel, kReloadNoteMetadataForNoteIdSignature))
    {
        bool reloadSucceeded = false;
        reloaded = QMetaObject::invokeMethod(
                       m_contentViewModel,
                       "reloadNoteMetadataForNoteId",
                       Qt::DirectConnection,
                       Q_RETURN_ARG(bool, reloadSucceeded),
                       Q_ARG(QString, currentNoteId))
            && reloadSucceeded;
    }

    if (reloaded)
    {
        resetBoundNotePersistenceSession();
    }

    refreshNoteSelectionState();
    refreshNoteCountState();
    return reloaded;
}

void ContentsEditorSelectionBridge::handleNoteListSelectionChanged()
{
    refreshNoteSelectionState();
}

void ContentsEditorSelectionBridge::handleNoteListCountChanged()
{
    refreshNoteCountState();
}

void ContentsEditorSelectionBridge::handleNoteListDestroyed()
{
    disconnectNoteListModel();
    resetBoundNotePersistenceSession();
    m_noteListModel = nullptr;
    emit noteListModelChanged();
    refreshNoteSelectionState();
    refreshNoteCountState();
}

void ContentsEditorSelectionBridge::handleContentViewModelDestroyed()
{
    disconnectContentViewModel();
    resetBoundNotePersistenceSession();
    m_contentViewModel = nullptr;
    emit contentViewModelChanged();
    refreshContentPersistenceState();
}

bool ContentsEditorSelectionBridge::hasReadableProperty(const QObject* object, const char* propertyName)
{
    if (object == nullptr || propertyName == nullptr)
    {
        return false;
    }

    const int propertyIndex = object->metaObject()->indexOfProperty(propertyName);
    if (propertyIndex < 0)
    {
        return false;
    }

    return object->metaObject()->property(propertyIndex).isReadable();
}

bool ContentsEditorSelectionBridge::hasInvokableMethod(const QObject* object, const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    return object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodSignature)) >= 0;
}

QString ContentsEditorSelectionBridge::readStringProperty(const QObject* object, const char* propertyName)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return {};
    }

    return object->property(propertyName).toString();
}

int ContentsEditorSelectionBridge::readIntProperty(const QObject* object, const char* propertyName)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return 0;
    }

    return std::max(0, object->property(propertyName).toInt());
}

QString ContentsEditorSelectionBridge::resolveNoteDirectoryPathForNote(const QString& noteId) const
{
    if (m_contentViewModel == nullptr || !hasInvokableMethod(m_contentViewModel, kNoteDirectoryPathForNoteIdSignature))
    {
        return {};
    }

    QString noteDirectoryPath;
    if (!QMetaObject::invokeMethod(
            m_contentViewModel,
            "noteDirectoryPathForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(QString, noteDirectoryPath),
            Q_ARG(QString, noteId.trimmed())))
    {
        return {};
    }

    return noteDirectoryPath.trimmed();
}

bool ContentsEditorSelectionBridge::ensureBoundNotePersistenceSession(const QString& noteId, QString* errorMessage)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteId must not be empty.");
        }
        return false;
    }

    const QString resolvedDirectoryPath = resolveNoteDirectoryPathForNote(normalizedNoteId);
    if (resolvedDirectoryPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve note directory path.");
        }
        return false;
    }

    if (m_boundNoteId != normalizedNoteId
        || QDir::cleanPath(m_boundNoteDirectoryPath) != QDir::cleanPath(resolvedDirectoryPath))
    {
        resetBoundNotePersistenceSession();
        m_boundNoteId = normalizedNoteId;
        m_boundNoteDirectoryPath = resolvedDirectoryPath;
    }

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

bool ContentsEditorSelectionBridge::enqueueDirectPersistenceRequest(const QString& noteId, const QString& text)
{
    if (!directPersistenceAvailable())
    {
        return false;
    }

    QString sessionError;
    if (!ensureBoundNotePersistenceSession(noteId, &sessionError))
    {
        return false;
    }

    DirectPersistenceRequest request;
    request.sequence = m_nextDirectPersistenceSequence++;
    request.noteId = noteId.trimmed();
    request.noteDirectoryPath = m_boundNoteDirectoryPath;
    request.text = text;

    if (m_directPersistenceInFlight
        && m_activeDirectPersistenceRequest.noteId == request.noteId
        && m_activeDirectPersistenceRequest.text == request.text)
    {
        return true;
    }

    const int pendingRequestIndex = findPendingDirectPersistenceRequestIndex(request.noteId);
    if (pendingRequestIndex >= 0)
    {
        if (m_pendingDirectPersistenceRequests.at(pendingRequestIndex).text == request.text)
        {
            return true;
        }
        m_pendingDirectPersistenceRequests[pendingRequestIndex] = request;
    }
    else if (m_directPersistenceInFlight)
    {
        m_pendingDirectPersistenceRequests.push_back(request);
    }
    else
    {
        m_activeDirectPersistenceRequest = request;
        m_directPersistenceInFlight = true;
        dispatchNextDirectPersistenceRequest();
    }

    return true;
}

void ContentsEditorSelectionBridge::dispatchNextDirectPersistenceRequest()
{
    if (!m_directPersistenceInFlight)
    {
        if (m_pendingDirectPersistenceRequests.isEmpty())
        {
            return;
        }

        m_activeDirectPersistenceRequest = m_pendingDirectPersistenceRequests.takeFirst();
        m_directPersistenceInFlight = true;
    }

    const DirectPersistenceRequest request = m_activeDirectPersistenceRequest;
    QPointer<ContentsEditorSelectionBridge> bridgeGuard(this);
    QThreadPool::globalInstance()->start([bridgeGuard, request]()
    {
        const DirectPersistenceResult result = ContentsEditorSelectionBridge::performDirectPersistence(request);

        if (QCoreApplication* app = QCoreApplication::instance())
        {
            QMetaObject::invokeMethod(
                app,
                [bridgeGuard, result]()
                {
                    if (bridgeGuard != nullptr)
                    {
                        bridgeGuard->handleDirectPersistenceFinished(result);
                    }
                },
                Qt::QueuedConnection);
        }
    });
}

ContentsEditorSelectionBridge::DirectPersistenceResult
ContentsEditorSelectionBridge::performDirectPersistence(DirectPersistenceRequest request)
{
    DirectPersistenceResult result;
    result.sequence = request.sequence;
    result.noteId = request.noteId;
    result.text = request.text;

    WhatSonLocalNoteFileStore noteFileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = result.noteId;
    readRequest.noteDirectoryPath = request.noteDirectoryPath;
    if (!noteFileStore.readNote(readRequest, &document, &result.errorMessage))
    {
        result.success = false;
        return result;
    }

    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = std::move(document);
    updateRequest.document.bodyPlainText = result.text;
    updateRequest.document.bodySourceText = result.text;
    updateRequest.persistHeader = false;
    updateRequest.persistBody = true;
    updateRequest.touchLastModified = true;
    updateRequest.incrementModifiedCount = false;
    updateRequest.refreshIncomingBacklinkStatistics = false;
    updateRequest.refreshAffectedBacklinkTargets = false;

    if (!noteFileStore.updateNote(updateRequest, &result.persistedDocument, &result.errorMessage))
    {
        result.success = false;
        return result;
    }

    result.success = true;
    return result;
}

void ContentsEditorSelectionBridge::handleDirectPersistenceFinished(DirectPersistenceResult result)
{
    if (result.success)
    {
        if (m_boundNoteId == result.noteId
            && QDir::cleanPath(m_boundNoteDirectoryPath)
                == QDir::cleanPath(result.persistedDocument.noteDirectoryPath))
        {
            m_boundNoteDirectoryPath = result.persistedDocument.noteDirectoryPath.trimmed();
        }

        if (!applyPersistedBodyStateToContentViewModel(result.noteId, result.persistedDocument)
            && hasInvokableMethod(m_contentViewModel, kReloadNoteMetadataForNoteIdSignature))
        {
            QMetaObject::invokeMethod(
                m_contentViewModel,
                "reloadNoteMetadataForNoteId",
                Qt::QueuedConnection,
                Q_ARG(QString, result.noteId));
        }
        requestTrackedStatisticsRefresh(result.noteId, false);
    }

    emit editorTextPersistenceFinished(
        result.noteId,
        result.text,
        result.success,
        result.errorMessage);

    if (!m_pendingDirectPersistenceRequests.isEmpty())
    {
        m_activeDirectPersistenceRequest = m_pendingDirectPersistenceRequests.takeFirst();
        m_directPersistenceInFlight = true;
        dispatchNextDirectPersistenceRequest();
        return;
    }

    m_activeDirectPersistenceRequest = DirectPersistenceRequest();
    m_directPersistenceInFlight = false;
}

int ContentsEditorSelectionBridge::findPendingDirectPersistenceRequestIndex(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    for (int index = m_pendingDirectPersistenceRequests.size() - 1; index >= 0; --index)
    {
        if (m_pendingDirectPersistenceRequests.at(index).noteId == normalizedNoteId)
        {
            return index;
        }
    }
    return -1;
}

bool ContentsEditorSelectionBridge::applyPersistedBodyStateToContentViewModel(
    const QString& noteId,
    const WhatSonLocalNoteDocument& document) const
{
    if (m_contentViewModel == nullptr
        || !hasInvokableMethod(m_contentViewModel, kApplyPersistedBodyStateForNoteSignature))
    {
        return false;
    }

    bool applied = false;
    return QMetaObject::invokeMethod(
               m_contentViewModel,
               "applyPersistedBodyStateForNote",
               Qt::DirectConnection,
               Q_RETURN_ARG(bool, applied),
               Q_ARG(QString, noteId.trimmed()),
               Q_ARG(QString, document.bodyPlainText),
               Q_ARG(QString, document.bodySourceText),
               Q_ARG(QString, document.headerStore.lastModifiedAt()))
        && applied;
}

void ContentsEditorSelectionBridge::requestTrackedStatisticsRefresh(
    const QString& noteId,
    const bool incrementOpenCount) const
{
    if (m_contentViewModel == nullptr
        || !hasInvokableMethod(m_contentViewModel, kRequestTrackedStatisticsRefreshForNoteSignature))
    {
        return;
    }

    QMetaObject::invokeMethod(
        m_contentViewModel,
        "requestTrackedStatisticsRefreshForNote",
        Qt::QueuedConnection,
        Q_ARG(QString, noteId.trimmed()),
        Q_ARG(bool, incrementOpenCount));
}

void ContentsEditorSelectionBridge::resetBoundNotePersistenceSession()
{
    m_boundNoteId.clear();
    m_boundNoteDirectoryPath.clear();
}

void ContentsEditorSelectionBridge::refreshNoteSelectionState()
{
    const bool nextContractAvailable = hasReadableProperty(m_noteListModel, "currentNoteId")
        && hasReadableProperty(m_noteListModel, "currentBodyText");
    const QString nextNoteId = nextContractAvailable ? readStringProperty(m_noteListModel, "currentNoteId") : QString();
    const QString nextBodyText = nextContractAvailable
                                     ? readStringProperty(m_noteListModel, "currentBodyText")
                                     : QString();

    if (m_noteSelectionContractAvailable != nextContractAvailable)
    {
        m_noteSelectionContractAvailable = nextContractAvailable;
        emit noteSelectionContractAvailableChanged();
    }
    if (m_selectedNoteId != nextNoteId)
    {
        resetBoundNotePersistenceSession();
        if (!nextNoteId.trimmed().isEmpty())
        {
            m_boundNoteId = nextNoteId.trimmed();
            m_boundNoteDirectoryPath = resolveNoteDirectoryPathForNote(nextNoteId);
            if (!m_boundNoteDirectoryPath.isEmpty())
            {
                QString openCountError;
                if (!WhatSon::NoteFileStatSupport::incrementOpenCountForNoteHeader(
                        m_boundNoteId,
                        m_boundNoteDirectoryPath,
                        &openCountError))
                {
                    WhatSon::Debug::traceSelf(
                        this,
                        QStringLiteral("content.selection.bridge"),
                        QStringLiteral("refreshNoteSelectionState.incrementOpenCount.failed"),
                        QStringLiteral("noteId=%1 error=%2").arg(m_boundNoteId, openCountError));
                }
            }
        }
        m_selectedNoteId = nextNoteId;
        emit selectedNoteIdChanged();
    }
    if (m_selectedNoteBodyText != nextBodyText)
    {
        m_selectedNoteBodyText = nextBodyText;
        emit selectedNoteBodyTextChanged();
    }
}

void ContentsEditorSelectionBridge::refreshNoteCountState()
{
    const bool nextContractAvailable = hasReadableProperty(m_noteListModel, "itemCount");
    const int nextVisibleNoteCount = nextContractAvailable ? readIntProperty(m_noteListModel, "itemCount") : 0;

    if (m_noteCountContractAvailable != nextContractAvailable)
    {
        m_noteCountContractAvailable = nextContractAvailable;
        emit noteCountContractAvailableChanged();
    }
    if (m_visibleNoteCount != nextVisibleNoteCount)
    {
        m_visibleNoteCount = nextVisibleNoteCount;
        emit visibleNoteCountChanged();
    }
}

void ContentsEditorSelectionBridge::refreshContentPersistenceState()
{
    const bool nextDirectPersistenceAvailable = hasInvokableMethod(m_contentViewModel, kNoteDirectoryPathForNoteIdSignature)
        && hasInvokableMethod(m_contentViewModel, kApplyPersistedBodyStateForNoteSignature);
    const bool nextContractAvailable = nextDirectPersistenceAvailable
        || hasInvokableMethod(m_contentViewModel, kSaveBodyTextForNoteSignature)
        || hasInvokableMethod(m_contentViewModel, kSaveCurrentBodyTextSignature);

    const bool directPersistenceChanged =
        m_directPersistenceContractAvailable != nextDirectPersistenceAvailable;
    if (directPersistenceChanged)
    {
        m_directPersistenceContractAvailable = nextDirectPersistenceAvailable;
    }

    if (m_contentPersistenceContractAvailable == nextContractAvailable)
    {
        if (directPersistenceChanged)
        {
            emit contentPersistenceContractAvailableChanged();
        }
        return;
    }

    m_contentPersistenceContractAvailable = nextContractAvailable;
    emit contentPersistenceContractAvailableChanged();
}

void ContentsEditorSelectionBridge::disconnectNoteListModel()
{
    if (m_noteListDestroyedConnection)
    {
        disconnect(m_noteListDestroyedConnection);
        m_noteListDestroyedConnection = QMetaObject::Connection();
    }
    if (m_currentNoteIdChangedConnection)
    {
        disconnect(m_currentNoteIdChangedConnection);
        m_currentNoteIdChangedConnection = QMetaObject::Connection();
    }
    if (m_currentBodyTextChangedConnection)
    {
        disconnect(m_currentBodyTextChangedConnection);
        m_currentBodyTextChangedConnection = QMetaObject::Connection();
    }
    if (m_itemCountChangedConnection)
    {
        disconnect(m_itemCountChangedConnection);
        m_itemCountChangedConnection = QMetaObject::Connection();
    }
}

void ContentsEditorSelectionBridge::disconnectContentViewModel()
{
    if (m_contentViewModelDestroyedConnection)
    {
        disconnect(m_contentViewModelDestroyedConnection);
        m_contentViewModelDestroyedConnection = QMetaObject::Connection();
    }
}
