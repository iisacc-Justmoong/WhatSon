#include "ContentsNoteManagementCoordinator.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "WhatSonLocalNoteFileStore.hpp"
#include "file/statistic/WhatSonNoteFileStatSupport.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QMetaObject>
#include <QThreadPool>

namespace
{
    constexpr auto kApplyPersistedBodyStateForNoteSignature =
        "applyPersistedBodyStateForNote(QString,QString,QString,QString)";
    constexpr auto kNoteDirectoryPathForNoteIdSignature = "noteDirectoryPathForNoteId(QString)";
    constexpr auto kReloadNoteMetadataForNoteIdSignature = "reloadNoteMetadataForNoteId(QString)";
    constexpr auto kSaveBodyTextForNoteSignature = "saveBodyTextForNote(QString,QString)";
    constexpr auto kSaveCurrentBodyTextSignature = "saveCurrentBodyText(QString)";
} // namespace

ContentsNoteManagementCoordinator::ContentsNoteManagementCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsNoteManagementCoordinator::~ContentsNoteManagementCoordinator() = default;

QObject* ContentsNoteManagementCoordinator::contentViewModel() const noexcept
{
    return m_contentViewModel;
}

void ContentsNoteManagementCoordinator::setContentViewModel(QObject* model)
{
    if (m_contentViewModel == model)
    {
        return;
    }

    if (m_contentViewModelDestroyedConnection)
    {
        disconnect(m_contentViewModelDestroyedConnection);
        m_contentViewModelDestroyedConnection = QMetaObject::Connection();
    }

    resetBoundNotePersistenceSession();
    m_contentViewModel = model;

    if (m_contentViewModel != nullptr)
    {
        m_contentViewModelDestroyedConnection = connect(
            m_contentViewModel,
            &QObject::destroyed,
            this,
            &ContentsNoteManagementCoordinator::handleContentViewModelDestroyed);
    }

    refreshContentPersistenceState();
}

bool ContentsNoteManagementCoordinator::contentPersistenceContractAvailable() const noexcept
{
    return m_contentPersistenceContractAvailable;
}

bool ContentsNoteManagementCoordinator::directPersistenceAvailable() const noexcept
{
    return m_directPersistenceContractAvailable;
}

bool ContentsNoteManagementCoordinator::persistEditorTextForNote(const QString& noteId, const QString& text)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty() || !m_contentPersistenceContractAvailable)
    {
        return false;
    }

    return enqueuePersistenceRequest(normalizedNoteId, text);
}

bool ContentsNoteManagementCoordinator::refreshNoteSnapshotForNote(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty() || !hasInvokableMethod(m_contentViewModel, kReloadNoteMetadataForNoteIdSignature))
    {
        return false;
    }

    bool reloaded = false;
    if (!QMetaObject::invokeMethod(
            m_contentViewModel,
            "reloadNoteMetadataForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(bool, reloaded),
            Q_ARG(QString, normalizedNoteId))
        || !reloaded)
    {
        return false;
    }

    if (m_boundNoteId == normalizedNoteId)
    {
        const QString reboundNoteId = m_boundNoteId;
        resetBoundNotePersistenceSession();
        ensureBoundNotePersistenceSession(reboundNoteId, nullptr);
    }
    return true;
}

void ContentsNoteManagementCoordinator::bindSelectedNote(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        resetBoundNotePersistenceSession();
        return;
    }

    QString sessionError;
    if (!ensureBoundNotePersistenceSession(normalizedNoteId, &sessionError))
    {
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("content.note.management"),
            QStringLiteral("bindSelectedNote.failed"),
            QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, sessionError));
        return;
    }

    enqueueOpenCountIncrement(normalizedNoteId, m_boundNoteDirectoryPath);
}

void ContentsNoteManagementCoordinator::clearSelectedNote()
{
    resetBoundNotePersistenceSession();
}

void ContentsNoteManagementCoordinator::handleContentViewModelDestroyed()
{
    if (m_contentViewModelDestroyedConnection)
    {
        disconnect(m_contentViewModelDestroyedConnection);
        m_contentViewModelDestroyedConnection = QMetaObject::Connection();
    }

    resetBoundNotePersistenceSession();
    m_contentViewModel = nullptr;
    refreshContentPersistenceState();
}

bool ContentsNoteManagementCoordinator::hasInvokableMethod(
    const QObject* object,
    const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    return object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodSignature)) >= 0;
}

QString ContentsNoteManagementCoordinator::resolveNoteDirectoryPathForNote(const QString& noteId) const
{
    if (m_contentViewModel == nullptr
        || !hasInvokableMethod(m_contentViewModel, kNoteDirectoryPathForNoteIdSignature))
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

bool ContentsNoteManagementCoordinator::ensureBoundNotePersistenceSession(
    const QString& noteId,
    QString* errorMessage)
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

    m_boundNoteId = normalizedNoteId;
    m_boundNoteDirectoryPath = resolvedDirectoryPath;
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

void ContentsNoteManagementCoordinator::resetBoundNotePersistenceSession()
{
    m_boundNoteId.clear();
    m_boundNoteDirectoryPath.clear();
}

void ContentsNoteManagementCoordinator::refreshContentPersistenceState()
{
    const bool nextDirectPersistenceAvailable =
        hasInvokableMethod(m_contentViewModel, kNoteDirectoryPathForNoteIdSignature)
        && hasInvokableMethod(m_contentViewModel, kApplyPersistedBodyStateForNoteSignature);
    const bool nextFallbackPersistenceAvailable =
        hasInvokableMethod(m_contentViewModel, kSaveBodyTextForNoteSignature)
        || hasInvokableMethod(m_contentViewModel, kSaveCurrentBodyTextSignature);
    const bool nextContractAvailable =
        nextDirectPersistenceAvailable || nextFallbackPersistenceAvailable;

    const bool changed =
        m_contentPersistenceContractAvailable != nextContractAvailable
        || m_directPersistenceContractAvailable != nextDirectPersistenceAvailable;

    m_contentPersistenceContractAvailable = nextContractAvailable;
    m_directPersistenceContractAvailable = nextDirectPersistenceAvailable;

    if (changed)
    {
        emit contentPersistenceContractAvailableChanged();
    }
}

bool ContentsNoteManagementCoordinator::enqueuePersistenceRequest(
    const QString& noteId,
    const QString& text)
{
    Request request;
    request.sequence = m_nextRequestSequence++;
    request.noteId = noteId.trimmed();
    request.text = text;

    if (m_directPersistenceContractAvailable)
    {
        QString sessionError;
        if (!ensureBoundNotePersistenceSession(request.noteId, &sessionError))
        {
            WhatSon::Debug::traceSelf(
                this,
                QStringLiteral("content.note.management"),
                QStringLiteral("enqueuePersistenceRequest.failed"),
                QStringLiteral("noteId=%1 error=%2").arg(request.noteId, sessionError));
            return false;
        }
        request.kind = RequestKind::DirectPersistBody;
        request.noteDirectoryPath = m_boundNoteDirectoryPath;
        return enqueueRequest(std::move(request));
    }

    request.kind = RequestKind::ViewModelPersistBody;
    request.noteDirectoryPath = resolveNoteDirectoryPathForNote(request.noteId);
    return enqueueRequest(std::move(request));
}

bool ContentsNoteManagementCoordinator::enqueueOpenCountIncrement(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString normalizedDirectoryPath = noteDirectoryPath.trimmed();
    if (normalizedNoteId.isEmpty() || normalizedDirectoryPath.isEmpty())
    {
        return false;
    }

    Request request;
    request.kind = RequestKind::IncrementOpenCount;
    request.sequence = m_nextRequestSequence++;
    request.noteId = normalizedNoteId;
    request.noteDirectoryPath = normalizedDirectoryPath;
    return enqueueRequest(std::move(request));
}

bool ContentsNoteManagementCoordinator::enqueueTrackedStatisticsRefresh(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const bool incrementOpenCount)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString normalizedDirectoryPath = noteDirectoryPath.trimmed();
    if (normalizedNoteId.isEmpty() || normalizedDirectoryPath.isEmpty())
    {
        return false;
    }

    Request request;
    request.kind = RequestKind::RefreshTrackedStatistics;
    request.sequence = m_nextRequestSequence++;
    request.noteId = normalizedNoteId;
    request.noteDirectoryPath = normalizedDirectoryPath;
    request.incrementOpenCount = incrementOpenCount;
    return enqueueRequest(std::move(request));
}

bool ContentsNoteManagementCoordinator::enqueueRequest(Request request)
{
    if (request.noteId.isEmpty())
    {
        return false;
    }

    if (m_requestInFlight
        && m_activeRequest.kind == request.kind
        && m_activeRequest.noteId == request.noteId)
    {
        if ((request.kind == RequestKind::DirectPersistBody
             || request.kind == RequestKind::ViewModelPersistBody)
            && m_activeRequest.text == request.text)
        {
            return true;
        }
        if (request.kind == RequestKind::IncrementOpenCount
            || request.kind == RequestKind::RefreshTrackedStatistics)
        {
            return true;
        }
    }

    const int pendingIndex = findPendingRequestIndex(request.kind, request.noteId);
    if (pendingIndex >= 0)
    {
        Request& pendingRequest = m_pendingRequests[pendingIndex];
        if (request.kind == RequestKind::DirectPersistBody
            || request.kind == RequestKind::ViewModelPersistBody)
        {
            if (pendingRequest.text == request.text)
            {
                return true;
            }
            pendingRequest = std::move(request);
            return true;
        }
        if (request.kind == RequestKind::RefreshTrackedStatistics)
        {
            pendingRequest.incrementOpenCount =
                pendingRequest.incrementOpenCount || request.incrementOpenCount;
            return true;
        }
        return true;
    }

    if (m_requestInFlight)
    {
        m_pendingRequests.push_back(std::move(request));
        return true;
    }

    m_activeRequest = std::move(request);
    m_requestInFlight = true;
    dispatchNextRequest();
    return true;
}

int ContentsNoteManagementCoordinator::findPendingRequestIndex(
    const RequestKind kind,
    const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    for (int index = m_pendingRequests.size() - 1; index >= 0; --index)
    {
        const Request& pendingRequest = m_pendingRequests.at(index);
        if (pendingRequest.kind == kind && pendingRequest.noteId == normalizedNoteId)
        {
            return index;
        }
    }
    return -1;
}

void ContentsNoteManagementCoordinator::dispatchNextRequest()
{
    if (!m_requestInFlight)
    {
        if (m_pendingRequests.isEmpty())
        {
            return;
        }
        m_activeRequest = m_pendingRequests.takeFirst();
        m_requestInFlight = true;
    }

    const Request request = m_activeRequest;
    if (request.kind == RequestKind::ViewModelPersistBody)
    {
        dispatchViewModelPersistenceRequest(request);
        return;
    }

    QPointer<ContentsNoteManagementCoordinator> coordinatorGuard(this);
    QThreadPool::globalInstance()->start([coordinatorGuard, request]()
    {
        const Result result = ContentsNoteManagementCoordinator::performWorkerRequest(request);
        if (coordinatorGuard != nullptr)
        {
            QMetaObject::invokeMethod(
                coordinatorGuard,
                [coordinatorGuard, result]()
                {
                    if (coordinatorGuard != nullptr)
                    {
                        coordinatorGuard->handleRequestFinished(result);
                    }
                },
                Qt::QueuedConnection);
        }
    });
}

void ContentsNoteManagementCoordinator::dispatchViewModelPersistenceRequest(const Request& request)
{
    QPointer<ContentsNoteManagementCoordinator> coordinatorGuard(this);
    QMetaObject::invokeMethod(
        this,
        [coordinatorGuard, request]()
        {
            if (coordinatorGuard == nullptr)
            {
                return;
            }
            const Result result = coordinatorGuard->performViewModelPersistence(request);
            coordinatorGuard->handleRequestFinished(result);
        },
        Qt::QueuedConnection);
}

ContentsNoteManagementCoordinator::Result
ContentsNoteManagementCoordinator::performWorkerRequest(const Request& request)
{
    Result result;
    result.kind = request.kind;
    result.sequence = request.sequence;
    result.noteId = request.noteId;
    result.noteDirectoryPath = request.noteDirectoryPath;
    result.text = request.text;
    result.incrementOpenCount = request.incrementOpenCount;

    if (request.kind == RequestKind::DirectPersistBody)
    {
        WhatSonLocalNoteFileStore noteFileStore;
        WhatSonLocalNoteDocument document;
        WhatSonLocalNoteFileStore::ReadRequest readRequest;
        readRequest.noteId = request.noteId;
        readRequest.noteDirectoryPath = request.noteDirectoryPath;
        if (!noteFileStore.readNote(readRequest, &document, &result.errorMessage))
        {
            return result;
        }

        WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
        updateRequest.document = std::move(document);
        updateRequest.document.bodyPlainText = request.text;
        updateRequest.document.bodySourceText = request.text;
        updateRequest.persistHeader = false;
        updateRequest.persistBody = true;
        updateRequest.touchLastModified = true;
        updateRequest.incrementModifiedCount = false;
        updateRequest.refreshIncomingBacklinkStatistics = false;
        updateRequest.refreshAffectedBacklinkTargets = false;

        if (!noteFileStore.updateNote(updateRequest, &result.persistedDocument, &result.errorMessage))
        {
            return result;
        }

        result.success = true;
        return result;
    }

    if (request.kind == RequestKind::IncrementOpenCount)
    {
        result.success = WhatSon::NoteFileStatSupport::incrementOpenCountForNoteHeader(
            request.noteId,
            request.noteDirectoryPath,
            &result.errorMessage);
        return result;
    }

    if (request.kind == RequestKind::RefreshTrackedStatistics)
    {
        result.success = WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNote(
            request.noteId,
            request.noteDirectoryPath,
            request.incrementOpenCount,
            &result.errorMessage);
        return result;
    }

    result.errorMessage = QStringLiteral("Unsupported worker request.");
    return result;
}

ContentsNoteManagementCoordinator::Result
ContentsNoteManagementCoordinator::performViewModelPersistence(const Request& request) const
{
    Result result;
    result.kind = request.kind;
    result.sequence = request.sequence;
    result.noteId = request.noteId;
    result.noteDirectoryPath = request.noteDirectoryPath;
    result.text = request.text;

    if (m_contentViewModel == nullptr)
    {
        result.errorMessage = QStringLiteral("contentViewModel is not available.");
        return result;
    }

    bool saved = false;
    bool invoked = false;
    if (hasInvokableMethod(m_contentViewModel, kSaveBodyTextForNoteSignature))
    {
        invoked = QMetaObject::invokeMethod(
            m_contentViewModel,
            "saveBodyTextForNote",
            Qt::DirectConnection,
            Q_RETURN_ARG(bool, saved),
            Q_ARG(QString, result.noteId),
            Q_ARG(QString, result.text));
    }
    else if (hasInvokableMethod(m_contentViewModel, kSaveCurrentBodyTextSignature))
    {
        invoked = QMetaObject::invokeMethod(
            m_contentViewModel,
            "saveCurrentBodyText",
            Qt::DirectConnection,
            Q_RETURN_ARG(bool, saved),
            Q_ARG(QString, result.text));
    }

    result.success = invoked && saved;
    if (!result.success)
    {
        result.errorMessage = QStringLiteral("Fallback content persistence contract rejected the request.");
        return result;
    }

    if (result.noteDirectoryPath.isEmpty())
    {
        result.noteDirectoryPath = resolveNoteDirectoryPathForNote(result.noteId);
    }
    return result;
}

void ContentsNoteManagementCoordinator::handleRequestFinished(const Result& result)
{
    if (result.kind == RequestKind::DirectPersistBody)
    {
        if (result.success)
        {
            const QString persistedDirectoryPath = result.persistedDocument.noteDirectoryPath.trimmed();
            if (m_boundNoteId == result.noteId
                && !persistedDirectoryPath.isEmpty())
            {
                m_boundNoteDirectoryPath = persistedDirectoryPath;
            }

            if (!applyPersistedBodyStateToContentViewModel(result.noteId, result.persistedDocument))
            {
                reloadNoteMetadataForNote(result.noteId);
            }

            const QString statsDirectoryPath =
                !persistedDirectoryPath.isEmpty() ? persistedDirectoryPath : resolveNoteDirectoryPathForNote(result.noteId);
            enqueueTrackedStatisticsRefresh(result.noteId, statsDirectoryPath, false);
        }
        else
        {
            WhatSon::Debug::traceSelf(
                this,
                QStringLiteral("content.note.management"),
                QStringLiteral("persistBody.failed"),
                QStringLiteral("noteId=%1 error=%2").arg(result.noteId, result.errorMessage));
        }

        emit editorTextPersistenceFinished(
            result.noteId,
            result.text,
            result.success,
            result.errorMessage);
    }
    else if (result.kind == RequestKind::ViewModelPersistBody)
    {
        if (result.success)
        {
            const QString statsDirectoryPath =
                !result.noteDirectoryPath.isEmpty()
                    ? result.noteDirectoryPath
                    : resolveNoteDirectoryPathForNote(result.noteId);
            enqueueTrackedStatisticsRefresh(result.noteId, statsDirectoryPath, false);
        }
        else
        {
            WhatSon::Debug::traceSelf(
                this,
                QStringLiteral("content.note.management"),
                QStringLiteral("fallbackPersistBody.failed"),
                QStringLiteral("noteId=%1 error=%2").arg(result.noteId, result.errorMessage));
        }

        emit editorTextPersistenceFinished(
            result.noteId,
            result.text,
            result.success,
            result.errorMessage);
    }
    else if (result.kind == RequestKind::IncrementOpenCount)
    {
        if (!result.success)
        {
            WhatSon::Debug::traceSelf(
                this,
                QStringLiteral("content.note.management"),
                QStringLiteral("incrementOpenCount.failed"),
                QStringLiteral("noteId=%1 error=%2").arg(result.noteId, result.errorMessage));
        }
    }
    else if (result.kind == RequestKind::RefreshTrackedStatistics)
    {
        if (result.success)
        {
            reloadNoteMetadataForNote(result.noteId);
        }
        else
        {
            WhatSon::Debug::traceSelf(
                this,
                QStringLiteral("content.note.management"),
                QStringLiteral("refreshTrackedStatistics.failed"),
                QStringLiteral("noteId=%1 error=%2").arg(result.noteId, result.errorMessage));
        }
    }

    if (!m_pendingRequests.isEmpty())
    {
        m_activeRequest = m_pendingRequests.takeFirst();
        m_requestInFlight = true;
        dispatchNextRequest();
        return;
    }

    m_activeRequest = Request();
    m_requestInFlight = false;
}

bool ContentsNoteManagementCoordinator::applyPersistedBodyStateToContentViewModel(
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

bool ContentsNoteManagementCoordinator::reloadNoteMetadataForNote(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty()
        || m_contentViewModel == nullptr
        || !hasInvokableMethod(m_contentViewModel, kReloadNoteMetadataForNoteIdSignature))
    {
        return false;
    }

    bool reloaded = false;
    if (!QMetaObject::invokeMethod(
            m_contentViewModel,
            "reloadNoteMetadataForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(bool, reloaded),
            Q_ARG(QString, normalizedNoteId))
        || !reloaded)
    {
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("content.note.management"),
            QStringLiteral("reloadNoteMetadata.failed"),
            QStringLiteral("noteId=%1").arg(normalizedNoteId));
        return false;
    }

    return true;
}
