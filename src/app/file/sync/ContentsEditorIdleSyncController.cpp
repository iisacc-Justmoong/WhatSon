#include "ContentsEditorIdleSyncController.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/note/ContentsNoteManagementCoordinator.hpp"

#include <QPointer>
#include <QTimer>

#include <algorithm>

namespace
{
    constexpr int kEditorIdleThresholdMs = 1000;
}

class ContentsEditorIdleSyncWorker final : public QObject
{
    Q_OBJECT

public:
    explicit ContentsEditorIdleSyncWorker(QObject* parent = nullptr)
        : QObject(parent)
        , m_idleTimer(this)
    {
        m_idleTimer.setSingleShot(true);
        m_idleTimer.setInterval(kEditorIdleThresholdMs);
        connect(&m_idleTimer, &QTimer::timeout, this, &ContentsEditorIdleSyncWorker::handleIdleTimeout);
    }

public slots:
    void clearPendingRevision()
    {
        m_pendingRevision = 0;
        m_idleTimer.stop();
    }

    void stagePendingRevision(const quint64 revision)
    {
        if (revision == 0)
        {
            clearPendingRevision();
            return;
        }

        m_pendingRevision = revision;
        m_idleTimer.start();
    }

signals:
    void idleRevisionReached(quint64 revision);

private slots:
    void handleIdleTimeout()
    {
        if (m_pendingRevision == 0)
        {
            return;
        }

        emit idleRevisionReached(m_pendingRevision);
    }

private:
    QTimer m_idleTimer;
    quint64 m_pendingRevision = 0;
};

ContentsEditorIdleSyncController::ContentsEditorIdleSyncController(QObject* parent)
    : QObject(parent)
{
    m_noteManagementCoordinator = new ContentsNoteManagementCoordinator(this);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::contentPersistenceContractAvailableChanged,
        this,
        &ContentsEditorIdleSyncController::contentPersistenceContractAvailableChanged);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::editorTextPersistenceFinished,
        this,
        &ContentsEditorIdleSyncController::handlePersistenceFinished);

    m_idleMonitorWorker = new ContentsEditorIdleSyncWorker();
    m_idleMonitorWorker->moveToThread(&m_idleMonitorThread);
    connect(
        &m_idleMonitorThread,
        &QThread::finished,
        m_idleMonitorWorker,
        &QObject::deleteLater);
    connect(
        this,
        &ContentsEditorIdleSyncController::idleMonitorStageRequested,
        m_idleMonitorWorker,
        &ContentsEditorIdleSyncWorker::stagePendingRevision,
        Qt::QueuedConnection);
    connect(
        this,
        &ContentsEditorIdleSyncController::idleMonitorClearRequested,
        m_idleMonitorWorker,
        &ContentsEditorIdleSyncWorker::clearPendingRevision,
        Qt::QueuedConnection);
    connect(
        m_idleMonitorWorker,
        &ContentsEditorIdleSyncWorker::idleRevisionReached,
        this,
        &ContentsEditorIdleSyncController::handleIdleRevisionReached,
        Qt::QueuedConnection);
    m_idleMonitorThread.setObjectName(QStringLiteral("ContentsEditorIdleSyncMonitor"));
    m_idleMonitorThread.start();
}

ContentsEditorIdleSyncController::~ContentsEditorIdleSyncController()
{
    emit idleMonitorClearRequested();
    m_idleMonitorThread.quit();
    m_idleMonitorThread.wait();
}

QObject* ContentsEditorIdleSyncController::contentViewModel() const noexcept
{
    return m_noteManagementCoordinator != nullptr
        ? m_noteManagementCoordinator->contentViewModel()
        : nullptr;
}

void ContentsEditorIdleSyncController::setContentViewModel(QObject* model)
{
    if (m_noteManagementCoordinator != nullptr)
    {
        m_noteManagementCoordinator->setContentViewModel(model);
    }
}

bool ContentsEditorIdleSyncController::contentPersistenceContractAvailable() const noexcept
{
    return m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->contentPersistenceContractAvailable();
}

bool ContentsEditorIdleSyncController::directPersistenceAvailable() const noexcept
{
    return m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->directPersistenceAvailable();
}

bool ContentsEditorIdleSyncController::persistEditorTextForNote(const QString& noteId, const QString& text)
{
    return stageEditorTextForIdleSync(noteId, text);
}

bool ContentsEditorIdleSyncController::stageEditorTextForIdleSync(const QString& noteId, const QString& text)
{
    return stageEditorSnapshot(noteId, text, false);
}

bool ContentsEditorIdleSyncController::flushEditorTextForNote(const QString& noteId, const QString& text)
{
    return stageEditorSnapshot(noteId, text, true);
}

bool ContentsEditorIdleSyncController::refreshNoteSnapshotForNote(const QString& noteId)
{
    return m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->refreshNoteSnapshotForNote(noteId);
}

void ContentsEditorIdleSyncController::bindSelectedNote(const QString& noteId)
{
    if (m_noteManagementCoordinator != nullptr)
    {
        m_noteManagementCoordinator->bindSelectedNote(noteId);
    }
}

void ContentsEditorIdleSyncController::clearSelectedNote()
{
    if (m_noteManagementCoordinator != nullptr)
    {
        m_noteManagementCoordinator->clearSelectedNote();
    }
}

void ContentsEditorIdleSyncController::handleIdleRevisionReached(const quint64 revision)
{
    m_latestIdleRevision = std::max(m_latestIdleRevision, revision);
    enqueueStagedPersistenceIfNeeded();
}

void ContentsEditorIdleSyncController::handlePersistenceFinished(
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    m_persistenceInFlight = false;
    if (success)
    {
        m_lastCompletedRevision = std::max(m_lastCompletedRevision, m_lastQueuedRevision);
        if (m_forceFlushRevision <= m_lastCompletedRevision)
        {
            m_forceFlushRevision = 0;
        }
    }
    else if (m_latestStagedRevision > m_lastCompletedRevision)
    {
        emit idleMonitorStageRequested(m_latestStagedRevision);
    }

    emit editorTextPersistenceFinished(noteId, text, success, errorMessage);
    enqueueStagedPersistenceIfNeeded();
}

bool ContentsEditorIdleSyncController::stageEditorSnapshot(
    const QString& noteId,
    const QString& text,
    const bool flushImmediately)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty() || !contentPersistenceContractAvailable())
    {
        return false;
    }

    const QString normalizedText = text;
    if (m_stagedNoteId != normalizedNoteId || m_stagedText != normalizedText)
    {
        m_stagedNoteId = normalizedNoteId;
        m_stagedText = normalizedText;
        m_latestStagedRevision += 1;
    }

    if (m_latestStagedRevision == 0)
    {
        return false;
    }

    if (flushImmediately)
    {
        m_forceFlushRevision = std::max(m_forceFlushRevision, m_latestStagedRevision);
        emit idleMonitorClearRequested();
        return enqueueStagedPersistenceIfNeeded();
    }

    emit idleMonitorStageRequested(m_latestStagedRevision);
    return true;
}

bool ContentsEditorIdleSyncController::enqueueStagedPersistenceIfNeeded()
{
    if (m_noteManagementCoordinator == nullptr
        || m_stagedNoteId.isEmpty()
        || !contentPersistenceContractAvailable())
    {
        return false;
    }

    const quint64 requestedRevision =
        std::max(m_latestIdleRevision, m_forceFlushRevision);
    if (requestedRevision == 0)
    {
        return true;
    }

    if (requestedRevision <= m_lastCompletedRevision)
    {
        return true;
    }

    if (m_persistenceInFlight)
    {
        return true;
    }

    if (requestedRevision <= m_lastQueuedRevision)
    {
        return true;
    }

    if (!m_noteManagementCoordinator->persistEditorTextForNote(m_stagedNoteId, m_stagedText))
    {
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("content.idleSync"),
            QStringLiteral("enqueuePersist.failed"),
            QStringLiteral("noteId=%1 revision=%2").arg(m_stagedNoteId).arg(requestedRevision));
        return false;
    }

    m_persistenceInFlight = true;
    m_lastQueuedRevision = requestedRevision;
    emit editorTextPersistenceQueued(m_stagedNoteId, m_stagedText);
    return true;
}

#include "ContentsEditorIdleSyncController.moc"
