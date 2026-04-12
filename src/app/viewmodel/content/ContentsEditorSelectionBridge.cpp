#include "ContentsEditorSelectionBridge.hpp"

#include "file/sync/ContentsEditorIdleSyncController.hpp"

#include <QMetaProperty>

#include <algorithm>
#include <utility>

ContentsEditorSelectionBridge::ContentsEditorSelectionBridge(QObject* parent)
    : QObject(parent)
{
    m_idleSyncController = new ContentsEditorIdleSyncController(this);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::contentPersistenceContractAvailableChanged,
        this,
        &ContentsEditorSelectionBridge::contentPersistenceContractAvailableChanged);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::editorTextPersistenceQueued,
        this,
        &ContentsEditorSelectionBridge::editorTextPersistenceQueued);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::editorTextPersistenceFinished,
        this,
        &ContentsEditorSelectionBridge::editorTextPersistenceFinished);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::noteBodyTextLoaded,
        this,
        &ContentsEditorSelectionBridge::handleNoteBodyTextLoaded);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::viewSessionSnapshotReconciled,
        this,
        &ContentsEditorSelectionBridge::viewSessionSnapshotReconciled);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::viewSessionSnapshotReconciled,
        this,
        &ContentsEditorSelectionBridge::handleViewSessionSnapshotReconciledInternal);
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

    if (m_idleSyncController != nullptr)
    {
        m_idleSyncController->setContentViewModel(m_contentViewModel);
        if (!m_selectedNoteId.trimmed().isEmpty())
        {
            m_idleSyncController->bindSelectedNote(m_selectedNoteId);
            if (m_selectedNoteBodySnapshotNoteId != m_selectedNoteId && !m_selectedNoteBodyLoading)
            {
                startSelectedNoteBodyLoad(m_selectedNoteId, m_selectedNoteBodyText.isEmpty());
            }
        }
        else
        {
            m_idleSyncController->clearSelectedNote();
        }
    }

    emit contentViewModelChanged();
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
    return m_idleSyncController != nullptr
        && m_idleSyncController->contentPersistenceContractAvailable();
}

QString ContentsEditorSelectionBridge::selectedNoteId() const
{
    return m_selectedNoteId;
}

QString ContentsEditorSelectionBridge::selectedNoteBodyText() const
{
    return m_selectedNoteBodyText;
}

bool ContentsEditorSelectionBridge::selectedNoteBodyLoading() const noexcept
{
    return m_selectedNoteBodyLoading;
}

int ContentsEditorSelectionBridge::visibleNoteCount() const noexcept
{
    return m_visibleNoteCount;
}

bool ContentsEditorSelectionBridge::persistEditorTextForNote(const QString& noteId, const QString& text)
{
    return stageEditorTextForIdleSync(noteId, text);
}

bool ContentsEditorSelectionBridge::stageEditorTextForIdleSync(const QString& noteId, const QString& text)
{
    return m_idleSyncController != nullptr
        && m_idleSyncController->stageEditorTextForIdleSync(noteId, text);
}

bool ContentsEditorSelectionBridge::flushEditorTextForNote(const QString& noteId, const QString& text)
{
    return m_idleSyncController != nullptr
        && m_idleSyncController->flushEditorTextForNote(noteId, text);
}

bool ContentsEditorSelectionBridge::reconcileViewSessionAndRefreshSnapshotForNote(
    const QString& noteId,
    const QString& viewSessionText)
{
    const QString normalizedNoteId = noteId.trimmed().isEmpty()
        ? m_selectedNoteId
        : noteId.trimmed();
    return m_idleSyncController != nullptr
        && m_idleSyncController->reconcileViewSessionAndRefreshSnapshotForNote(
            normalizedNoteId,
            viewSessionText);
}

bool ContentsEditorSelectionBridge::directPersistenceAvailable() const noexcept
{
    return m_idleSyncController != nullptr
        && m_idleSyncController->directPersistenceAvailable();
}

bool ContentsEditorSelectionBridge::refreshSelectedNoteSnapshot()
{
    const bool reloaded = m_idleSyncController != nullptr
        && m_idleSyncController->refreshNoteSnapshotForNote(m_selectedNoteId);
    if (reloaded)
    {
        startSelectedNoteBodyLoad(m_selectedNoteId, false);
    }
    scheduleNoteSelectionRefresh();
    refreshNoteCountState();
    return reloaded;
}

void ContentsEditorSelectionBridge::handleNoteListSelectionChanged()
{
    scheduleNoteSelectionRefresh();
}

void ContentsEditorSelectionBridge::handleNoteBodyTextLoaded(
    const quint64 sequence,
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    Q_UNUSED(errorMessage)

    const QString normalizedNoteId = noteId.trimmed();
    if (sequence == 0
        || normalizedNoteId.isEmpty()
        || normalizedNoteId != m_selectedNoteId.trimmed()
        || sequence != m_selectedNoteBodyRequestSequence)
    {
        return;
    }

    m_selectedNoteBodyRequestSequence = 0;
    m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
    setSelectedNoteBodyState(success ? text : QString(), false);
}

void ContentsEditorSelectionBridge::handleViewSessionSnapshotReconciledInternal(
    const QString& noteId,
    const bool refreshed,
    const bool success,
    const QString& errorMessage)
{
    Q_UNUSED(errorMessage)

    const QString normalizedNoteId = noteId.trimmed();
    if (!success || !refreshed || normalizedNoteId.isEmpty()
        || normalizedNoteId != m_selectedNoteId.trimmed())
    {
        return;
    }

    startSelectedNoteBodyLoad(normalizedNoteId, false);
}

void ContentsEditorSelectionBridge::flushPendingNoteSelectionRefresh()
{
    m_noteSelectionRefreshQueued = false;
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
    m_noteSelectionRefreshQueued = false;
    m_selectedNoteBodySnapshotNoteId.clear();
    m_selectedNoteBodyRequestSequence = 0;
    refreshNoteSelectionState();
    refreshNoteCountState();
}

void ContentsEditorSelectionBridge::handleContentViewModelDestroyed()
{
    disconnectContentViewModel();
    m_contentViewModel = nullptr;
    if (m_idleSyncController != nullptr)
    {
        m_idleSyncController->setContentViewModel(nullptr);
    }
    m_selectedNoteBodyRequestSequence = 0;
    emit contentViewModelChanged();
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

void ContentsEditorSelectionBridge::setSelectedNoteBodyState(QString bodyText, const bool loading)
{
    const bool loadingChanged = m_selectedNoteBodyLoading != loading;
    const bool bodyTextChanged = m_selectedNoteBodyText != bodyText;

    m_selectedNoteBodyLoading = loading;
    m_selectedNoteBodyText = std::move(bodyText);

    if (loadingChanged)
    {
        emit selectedNoteBodyLoadingChanged();
    }
    if (bodyTextChanged)
    {
        emit selectedNoteBodyTextChanged();
    }
}

void ContentsEditorSelectionBridge::startSelectedNoteBodyLoad(
    const QString& noteId,
    const bool clearCachedBody)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
        setSelectedNoteBodyState(QString(), false);
        return;
    }

    if (clearCachedBody)
    {
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
    }

    const bool shouldShowLoading = clearCachedBody
        || (m_selectedNoteBodyText.isEmpty() && m_selectedNoteBodySnapshotNoteId != normalizedNoteId);
    setSelectedNoteBodyState(clearCachedBody ? QString() : m_selectedNoteBodyText, shouldShowLoading);

    const quint64 requestSequence = m_idleSyncController != nullptr
        ? m_idleSyncController->loadNoteBodyTextForNote(normalizedNoteId)
        : 0;
    if (requestSequence != 0)
    {
        m_selectedNoteBodyRequestSequence = requestSequence;
        return;
    }

    m_selectedNoteBodyRequestSequence = 0;
    m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
    setSelectedNoteBodyState(clearCachedBody ? QString() : m_selectedNoteBodyText, false);
}

void ContentsEditorSelectionBridge::scheduleNoteSelectionRefresh()
{
    if (m_noteSelectionRefreshQueued)
    {
        return;
    }

    m_noteSelectionRefreshQueued = true;
    QMetaObject::invokeMethod(
        this,
        &ContentsEditorSelectionBridge::flushPendingNoteSelectionRefresh,
        Qt::QueuedConnection);
}

void ContentsEditorSelectionBridge::refreshNoteSelectionState()
{
    const bool nextContractAvailable = hasReadableProperty(m_noteListModel, "currentNoteId");
    const QString nextNoteId = nextContractAvailable ? readStringProperty(m_noteListModel, "currentNoteId") : QString();

    if (m_noteSelectionContractAvailable != nextContractAvailable)
    {
        m_noteSelectionContractAvailable = nextContractAvailable;
        emit noteSelectionContractAvailableChanged();
    }
    if (m_selectedNoteId != nextNoteId)
    {
        if (m_idleSyncController != nullptr)
        {
            if (nextNoteId.trimmed().isEmpty())
            {
                m_idleSyncController->clearSelectedNote();
            }
            else
            {
                m_idleSyncController->bindSelectedNote(nextNoteId);
            }
        }
    }

    const bool noteIdChanged = m_selectedNoteId != nextNoteId;
    if (!noteIdChanged)
    {
        if (!m_selectedNoteId.trimmed().isEmpty()
            && m_selectedNoteBodySnapshotNoteId != m_selectedNoteId
            && !m_selectedNoteBodyLoading)
        {
            startSelectedNoteBodyLoad(m_selectedNoteId, m_selectedNoteBodyText.isEmpty());
        }
        return;
    }

    m_selectedNoteId = nextNoteId;
    emit selectedNoteIdChanged();

    if (nextNoteId.trimmed().isEmpty())
    {
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
        setSelectedNoteBodyState(QString(), false);
        return;
    }

    startSelectedNoteBodyLoad(nextNoteId, true);
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
