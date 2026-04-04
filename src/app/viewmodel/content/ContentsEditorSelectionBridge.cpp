#include "ContentsEditorSelectionBridge.hpp"

#include "file/note/WhatSonNoteFileStatSupport.hpp"

#include <QMetaObject>
#include <QMetaProperty>

#include <algorithm>

namespace
{
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

bool ContentsEditorSelectionBridge::persistEditorTextForNote(const QString& noteId, const QString& text) const
{
    if (m_contentViewModel == nullptr || !m_contentPersistenceContractAvailable)
    {
        return false;
    }

    bool saved = false;
    if (hasInvokableMethod(m_contentViewModel, kSaveBodyTextForNoteSignature))
    {
        return QMetaObject::invokeMethod(
                m_contentViewModel,
                "saveBodyTextForNote",
                Qt::DirectConnection,
                Q_RETURN_ARG(bool, saved),
                Q_ARG(QString, noteId),
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
    m_noteListModel = nullptr;
    emit noteListModelChanged();
    refreshNoteSelectionState();
    refreshNoteCountState();
}

void ContentsEditorSelectionBridge::handleContentViewModelDestroyed()
{
    disconnectContentViewModel();
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
    if (m_selectedNoteId != nextNoteId && !nextNoteId.trimmed().isEmpty()
        && hasInvokableMethod(m_contentViewModel, kNoteDirectoryPathForNoteIdSignature))
    {
        QString noteDirectoryPath;
        if (QMetaObject::invokeMethod(
                m_contentViewModel,
                "noteDirectoryPathForNoteId",
                Qt::DirectConnection,
                Q_RETURN_ARG(QString, noteDirectoryPath),
                Q_ARG(QString, nextNoteId)))
        {
            QString statRefreshError;
            WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNote(
                nextNoteId,
                noteDirectoryPath,
                true,
                &statRefreshError);
            if (hasInvokableMethod(m_contentViewModel, kReloadNoteMetadataForNoteIdSignature))
            {
                QMetaObject::invokeMethod(
                    m_contentViewModel,
                    "reloadNoteMetadataForNoteId",
                    Qt::QueuedConnection,
                    Q_ARG(QString, nextNoteId));
            }
        }
    }
    if (m_selectedNoteId != nextNoteId)
    {
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
    const bool nextContractAvailable = hasInvokableMethod(m_contentViewModel, kSaveBodyTextForNoteSignature)
        || hasInvokableMethod(m_contentViewModel, kSaveCurrentBodyTextSignature);
    if (m_contentPersistenceContractAvailable == nextContractAvailable)
    {
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
