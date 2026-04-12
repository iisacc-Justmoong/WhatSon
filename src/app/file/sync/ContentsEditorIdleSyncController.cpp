#include "ContentsEditorIdleSyncController.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/note/ContentsNoteManagementCoordinator.hpp"

namespace
{
    constexpr int kEditorPersistenceFetchIntervalMs = 1000;
}

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
        &ContentsNoteManagementCoordinator::contentPersistenceContractAvailableChanged,
        this,
        &ContentsEditorIdleSyncController::handleContentPersistenceContractAvailabilityChanged);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::editorTextPersistenceFinished,
        this,
        &ContentsEditorIdleSyncController::handlePersistenceFinished);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::noteBodyTextLoaded,
        this,
        &ContentsEditorIdleSyncController::noteBodyTextLoaded);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::viewSessionSnapshotReconciled,
        this,
        &ContentsEditorIdleSyncController::viewSessionSnapshotReconciled);

    m_fetchTimer.setInterval(kEditorPersistenceFetchIntervalMs);
    m_fetchTimer.setSingleShot(false);
    connect(
        &m_fetchTimer,
        &QTimer::timeout,
        this,
        &ContentsEditorIdleSyncController::handleFetchTimerTimeout);
}

ContentsEditorIdleSyncController::~ContentsEditorIdleSyncController() = default;

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

bool ContentsEditorIdleSyncController::pendingEditorTextForNote(
    const QString& noteId,
    QString* text) const
{
    if (text != nullptr)
    {
        text->clear();
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    if (m_persistenceInFlight && m_inFlightNoteId == normalizedNoteId)
    {
        if (text != nullptr)
        {
            *text = m_inFlightText;
        }
        return true;
    }

    const auto bufferedSnapshotIt = m_bufferedSnapshotsByNote.constFind(normalizedNoteId);
    if (bufferedSnapshotIt == m_bufferedSnapshotsByNote.cend())
    {
        return false;
    }

    const BufferedSnapshot& bufferedSnapshot = bufferedSnapshotIt.value();
    const bool pendingPersistence =
        m_dirtyNoteOrder.contains(normalizedNoteId)
        || !m_lastPersistedTextByNote.contains(normalizedNoteId)
        || m_lastPersistedTextByNote.value(normalizedNoteId) != bufferedSnapshot.text;
    if (!pendingPersistence)
    {
        return false;
    }

    if (text != nullptr)
    {
        *text = bufferedSnapshot.text;
    }
    return true;
}

quint64 ContentsEditorIdleSyncController::loadNoteBodyTextForNote(const QString& noteId)
{
    if (m_noteManagementCoordinator == nullptr)
    {
        return 0;
    }
    return m_noteManagementCoordinator->loadNoteBodyTextForNote(noteId);
}

bool ContentsEditorIdleSyncController::reconcileViewSessionAndRefreshSnapshotForNote(
    const QString& noteId,
    const QString& viewSessionText)
{
    return m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->reconcileViewSessionAndRefreshSnapshotForNote(
            noteId,
            viewSessionText);
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

void ContentsEditorIdleSyncController::handleFetchTimerTimeout()
{
    enqueueNextBufferedPersistenceIfNeeded();
}

void ContentsEditorIdleSyncController::handleContentPersistenceContractAvailabilityChanged()
{
    if (!contentPersistenceContractAvailable())
    {
        return;
    }

    enqueueNextBufferedPersistenceIfNeeded();
}

void ContentsEditorIdleSyncController::handlePersistenceFinished(
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString completedText = text;

    m_persistenceInFlight = false;
    m_inFlightNoteId.clear();
    m_inFlightText.clear();

    if (success)
    {
        m_lastPersistedTextByNote.insert(normalizedNoteId, completedText);
        if (m_bufferedSnapshotsByNote.value(normalizedNoteId).text == completedText)
        {
            removeNoteFromDirtyOrder(normalizedNoteId);
        }
        else
        {
            markNoteDirty(normalizedNoteId);
        }
        if (!normalizedNoteId.isEmpty() && m_noteManagementCoordinator != nullptr)
        {
            const bool reconciled = m_noteManagementCoordinator
                ->reconcileViewSessionAndRefreshSnapshotForNote(normalizedNoteId, completedText);
            if (!reconciled)
            {
                WhatSon::Debug::traceSelf(
                    this,
                    QStringLiteral("content.idleSync"),
                    QStringLiteral("postPersist.reconcile.failed"),
                    QStringLiteral("noteId=%1").arg(normalizedNoteId));
            }
        }
    }
    else
    {
        markNoteDirty(normalizedNoteId);
    }

    emit editorTextPersistenceFinished(noteId, text, success, errorMessage);

    if (m_dirtyNoteOrder.isEmpty())
    {
        m_fetchTimer.stop();
        return;
    }

    ensureFetchTimerRunning();
}

bool ContentsEditorIdleSyncController::stageEditorSnapshot(
    const QString& noteId,
    const QString& text,
    const bool requestImmediateFetch)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const QString normalizedText = text;
    BufferedSnapshot bufferedSnapshot;
    bufferedSnapshot.text = normalizedText;
    if (m_noteManagementCoordinator != nullptr)
    {
        QString noteDirectoryPath;
        if (m_noteManagementCoordinator->captureDirectPersistenceContextForNote(
                normalizedNoteId,
                &noteDirectoryPath))
        {
            bufferedSnapshot.noteDirectoryPath = noteDirectoryPath.trimmed();
            bufferedSnapshot.directPersistenceReady = !bufferedSnapshot.noteDirectoryPath.isEmpty();
        }
    }
    m_bufferedSnapshotsByNote.insert(normalizedNoteId, bufferedSnapshot);

    const bool matchesInFlight =
        m_persistenceInFlight
        && m_inFlightNoteId == normalizedNoteId
        && m_inFlightText == normalizedText;
    const bool matchesLastPersisted =
        m_lastPersistedTextByNote.contains(normalizedNoteId)
        && m_lastPersistedTextByNote.value(normalizedNoteId) == normalizedText;

    if (matchesLastPersisted && !matchesInFlight)
    {
        removeNoteFromDirtyOrder(normalizedNoteId);
        if (m_dirtyNoteOrder.isEmpty() && !m_persistenceInFlight)
        {
            m_fetchTimer.stop();
        }
    }
    else
    {
        markNoteDirty(normalizedNoteId);
        ensureFetchTimerRunning();
    }

    if (requestImmediateFetch)
    {
        enqueueNextBufferedPersistenceIfNeeded();
    }

    return true;
}

bool ContentsEditorIdleSyncController::enqueueNextBufferedPersistenceIfNeeded()
{
    if (m_noteManagementCoordinator == nullptr || m_persistenceInFlight)
    {
        return true;
    }

    if (m_dirtyNoteOrder.isEmpty())
    {
        m_fetchTimer.stop();
        return true;
    }

    QString candidateNoteId;
    QString candidateText;
    BufferedSnapshot candidateSnapshot;
    const int dirtyNoteCount = m_dirtyNoteOrder.size();
    for (int index = 0; index < dirtyNoteCount; ++index)
    {
        const QString currentNoteId = m_dirtyNoteOrder.takeFirst();
        const BufferedSnapshot currentSnapshot = m_bufferedSnapshotsByNote.value(currentNoteId);
        const QString currentText = currentSnapshot.text;
        const bool alreadyPersisted =
            m_lastPersistedTextByNote.contains(currentNoteId)
            && m_lastPersistedTextByNote.value(currentNoteId) == currentText;
        if (alreadyPersisted)
        {
            continue;
        }

        candidateNoteId = currentNoteId;
        candidateText = currentText;
        candidateSnapshot = currentSnapshot;
        m_dirtyNoteOrder.append(currentNoteId);
        break;
    }

    if (candidateNoteId.isEmpty())
    {
        m_fetchTimer.stop();
        return true;
    }

    if (!candidateSnapshot.directPersistenceReady
        && !contentPersistenceContractAvailable())
    {
        ensureFetchTimerRunning();
        return true;
    }

    bool accepted = false;
    if (candidateSnapshot.directPersistenceReady
        && !candidateSnapshot.noteDirectoryPath.isEmpty())
    {
        accepted = m_noteManagementCoordinator->persistEditorTextForNoteAtPath(
            candidateNoteId,
            candidateSnapshot.noteDirectoryPath,
            candidateText);
    }
    if (!accepted)
    {
        accepted = m_noteManagementCoordinator->persistEditorTextForNote(
            candidateNoteId,
            candidateText);
    }

    if (!accepted)
    {
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("content.idleSync"),
            QStringLiteral("enqueuePersist.failed"),
            QStringLiteral("noteId=%1").arg(candidateNoteId));
        ensureFetchTimerRunning();
        return false;
    }

    m_persistenceInFlight = true;
    m_inFlightNoteId = candidateNoteId;
    m_inFlightText = candidateText;
    emit editorTextPersistenceQueued(candidateNoteId, candidateText);
    return true;
}

void ContentsEditorIdleSyncController::ensureFetchTimerRunning()
{
    if (!m_dirtyNoteOrder.isEmpty() && !m_fetchTimer.isActive())
    {
        m_fetchTimer.start();
    }
}

void ContentsEditorIdleSyncController::markNoteDirty(const QString& noteId)
{
    removeNoteFromDirtyOrder(noteId);
    m_dirtyNoteOrder.append(noteId);
}

void ContentsEditorIdleSyncController::removeNoteFromDirtyOrder(const QString& noteId)
{
    m_dirtyNoteOrder.removeAll(noteId);
}
