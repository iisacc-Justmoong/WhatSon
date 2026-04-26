#include "app/models/editor/persistence/ContentsEditorPersistenceController.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/note/ContentsNoteManagementCoordinator.hpp"

namespace
{
    constexpr int kEditorPersistenceFetchIntervalMs = 1000;
}

ContentsEditorPersistenceController::ContentsEditorPersistenceController(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("editorPersistence"), QStringLiteral("ctor"));
    m_noteManagementCoordinator = new ContentsNoteManagementCoordinator(this);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::contentPersistenceContractAvailableChanged,
        this,
        &ContentsEditorPersistenceController::contentPersistenceContractAvailableChanged);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::contentPersistenceContractAvailableChanged,
        this,
        &ContentsEditorPersistenceController::handleContentPersistenceContractAvailabilityChanged);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::editorTextPersistenceFinished,
        this,
        &ContentsEditorPersistenceController::handlePersistenceFinished);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::noteBodyTextLoaded,
        this,
        &ContentsEditorPersistenceController::noteBodyTextLoaded);
    connect(
        m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::viewSessionSnapshotReconciled,
        this,
        &ContentsEditorPersistenceController::viewSessionSnapshotReconciled);

    m_fetchTimer.setInterval(kEditorPersistenceFetchIntervalMs);
    m_fetchTimer.setSingleShot(false);
    connect(
        &m_fetchTimer,
        &QTimer::timeout,
        this,
        &ContentsEditorPersistenceController::handleFetchTimerTimeout);
}

ContentsEditorPersistenceController::~ContentsEditorPersistenceController()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("dtor"),
        QStringLiteral("dirtyCount=%1 inFlight=%2 currentNoteId=%3")
            .arg(m_dirtyNoteOrder.size())
            .arg(m_persistenceInFlight)
            .arg(m_inFlightNoteId));
}

QObject* ContentsEditorPersistenceController::contentViewModel() const noexcept
{
    return m_noteManagementCoordinator != nullptr
        ? m_noteManagementCoordinator->contentViewModel()
        : nullptr;
}

void ContentsEditorPersistenceController::setContentViewModel(QObject* model)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("setContentViewModel"),
        QStringLiteral("next=0x%1").arg(QString::number(reinterpret_cast<quintptr>(model), 16)));
    if (m_noteManagementCoordinator != nullptr)
    {
        m_noteManagementCoordinator->setContentViewModel(model);
    }
}

bool ContentsEditorPersistenceController::contentPersistenceContractAvailable() const noexcept
{
    return m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->contentPersistenceContractAvailable();
}

bool ContentsEditorPersistenceController::directPersistenceAvailable() const noexcept
{
    return m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->directPersistenceAvailable();
}

bool ContentsEditorPersistenceController::persistEditorTextForNote(const QString& noteId, const QString& text)
{
    return stageEditorTextForPersistence(noteId, text);
}

bool ContentsEditorPersistenceController::stageEditorTextForPersistence(const QString& noteId, const QString& text)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("stageEditorTextForPersistence"),
        QStringLiteral("noteId=%1 %2").arg(noteId.trimmed()).arg(WhatSon::Debug::summarizeText(text)));
    return stageEditorSnapshot(noteId, QString(), text, false);
}

bool ContentsEditorPersistenceController::stageEditorTextForPersistenceAtPath(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& text)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("stageEditorTextForPersistenceAtPath"),
        QStringLiteral("noteId=%1 noteDirectoryPath=%2 %3")
            .arg(noteId.trimmed())
            .arg(noteDirectoryPath.trimmed())
            .arg(WhatSon::Debug::summarizeText(text)));
    return stageEditorSnapshot(noteId, noteDirectoryPath, text, false);
}

bool ContentsEditorPersistenceController::stageEditorTextForIdleSync(const QString& noteId, const QString& text)
{
    return stageEditorTextForPersistence(noteId, text);
}

bool ContentsEditorPersistenceController::stageEditorTextForIdleSyncAtPath(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& text)
{
    return stageEditorTextForPersistenceAtPath(noteId, noteDirectoryPath, text);
}

bool ContentsEditorPersistenceController::flushEditorTextForNote(const QString& noteId, const QString& text)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("flushEditorTextForNote"),
        QStringLiteral("noteId=%1 %2").arg(noteId.trimmed()).arg(WhatSon::Debug::summarizeText(text)));
    return stageEditorSnapshot(noteId, QString(), text, true);
}

bool ContentsEditorPersistenceController::flushEditorTextForNoteAtPath(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& text)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("flushEditorTextForNoteAtPath"),
        QStringLiteral("noteId=%1 noteDirectoryPath=%2 %3")
            .arg(noteId.trimmed())
            .arg(noteDirectoryPath.trimmed())
            .arg(WhatSon::Debug::summarizeText(text)));
    return stageEditorSnapshot(noteId, noteDirectoryPath, text, true);
}

QString ContentsEditorPersistenceController::noteDirectoryPathForNote(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    const auto bufferedSnapshotIt = m_bufferedSnapshotsByNote.constFind(normalizedNoteId);
    if (bufferedSnapshotIt != m_bufferedSnapshotsByNote.cend())
    {
        const QString bufferedPath = bufferedSnapshotIt.value().noteDirectoryPath.trimmed();
        if (!bufferedPath.isEmpty())
        {
            return bufferedPath;
        }
    }

    if (m_noteManagementCoordinator == nullptr)
    {
        return {};
    }

    return m_noteManagementCoordinator->noteDirectoryPathForNote(normalizedNoteId).trimmed();
}

bool ContentsEditorPersistenceController::pendingEditorTextForNote(
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

quint64 ContentsEditorPersistenceController::loadNoteBodyTextForNote(const QString& noteId)
{
    if (m_noteManagementCoordinator == nullptr)
    {
        return 0;
    }
    const quint64 sequence = m_noteManagementCoordinator->loadNoteBodyTextForNote(noteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("loadNoteBodyTextForNote"),
        QStringLiteral("noteId=%1 sequence=%2").arg(noteId.trimmed()).arg(sequence));
    return sequence;
}

quint64 ContentsEditorPersistenceController::loadNoteBodyTextForNoteAtPath(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    if (m_noteManagementCoordinator == nullptr)
    {
        return 0;
    }

    const quint64 sequence = m_noteManagementCoordinator->loadNoteBodyTextForNote(noteId, noteDirectoryPath);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("loadNoteBodyTextForNoteAtPath"),
        QStringLiteral("noteId=%1 noteDirectoryPath=%2 sequence=%3")
            .arg(noteId.trimmed())
            .arg(noteDirectoryPath.trimmed())
            .arg(sequence));
    return sequence;
}

bool ContentsEditorPersistenceController::reconcileViewSessionAndRefreshSnapshotForNote(
    const QString& noteId,
    const QString& viewSessionText,
    const bool preferViewSessionOnMismatch)
{
    const bool accepted = m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->reconcileViewSessionAndRefreshSnapshotForNote(
            noteId,
            viewSessionText,
            preferViewSessionOnMismatch);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("reconcileViewSessionAndRefreshSnapshotForNote"),
        QStringLiteral("accepted=%1 preferViewSession=%2 noteId=%3 %4")
            .arg(accepted)
            .arg(preferViewSessionOnMismatch)
            .arg(noteId.trimmed())
            .arg(WhatSon::Debug::summarizeText(viewSessionText)));
    return accepted;
}

bool ContentsEditorPersistenceController::reconcileViewSessionAndRefreshSnapshotForNoteAtPath(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& viewSessionText,
    const bool preferViewSessionOnMismatch)
{
    const bool accepted = m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->reconcileViewSessionAndRefreshSnapshotForNote(
            noteId,
            viewSessionText,
            preferViewSessionOnMismatch,
            noteDirectoryPath);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("reconcileViewSessionAndRefreshSnapshotForNoteAtPath"),
        QStringLiteral("accepted=%1 preferViewSession=%2 noteId=%3 noteDirectoryPath=%4 %5")
            .arg(accepted)
            .arg(preferViewSessionOnMismatch)
            .arg(noteId.trimmed())
            .arg(noteDirectoryPath.trimmed())
            .arg(WhatSon::Debug::summarizeText(viewSessionText)));
    return accepted;
}

bool ContentsEditorPersistenceController::refreshNoteSnapshotForNote(const QString& noteId)
{
    const bool refreshed = m_noteManagementCoordinator != nullptr
        && m_noteManagementCoordinator->refreshNoteSnapshotForNote(noteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("refreshNoteSnapshotForNote"),
        QStringLiteral("refreshed=%1 noteId=%2").arg(refreshed).arg(noteId.trimmed()));
    return refreshed;
}

void ContentsEditorPersistenceController::bindSelectedNote(const QString& noteId)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("bindSelectedNote"),
        QStringLiteral("noteId=%1").arg(noteId.trimmed()));
    if (m_noteManagementCoordinator != nullptr)
    {
        m_noteManagementCoordinator->bindSelectedNote(noteId);
    }
}

void ContentsEditorPersistenceController::bindSelectedNoteAtPath(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("bindSelectedNoteAtPath"),
        QStringLiteral("noteId=%1 noteDirectoryPath=%2")
            .arg(noteId.trimmed())
            .arg(noteDirectoryPath.trimmed()));
    if (m_noteManagementCoordinator != nullptr)
    {
        m_noteManagementCoordinator->bindSelectedNote(noteId, noteDirectoryPath);
    }
}

void ContentsEditorPersistenceController::clearSelectedNote()
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("editorPersistence"), QStringLiteral("clearSelectedNote"));
    if (m_noteManagementCoordinator != nullptr)
    {
        m_noteManagementCoordinator->clearSelectedNote();
    }
}

void ContentsEditorPersistenceController::handleFetchTimerTimeout()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("handleFetchTimerTimeout"),
        QStringLiteral("dirtyCount=%1 inFlight=%2").arg(m_dirtyNoteOrder.size()).arg(m_persistenceInFlight));
    enqueueNextBufferedPersistenceIfNeeded();
}

void ContentsEditorPersistenceController::handleContentPersistenceContractAvailabilityChanged()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("handleContentPersistenceContractAvailabilityChanged"),
        QStringLiteral("available=%1").arg(contentPersistenceContractAvailable()));
    if (!contentPersistenceContractAvailable())
    {
        return;
    }

    enqueueNextBufferedPersistenceIfNeeded();
}

void ContentsEditorPersistenceController::handlePersistenceFinished(
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString completedText = text;

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("handlePersistenceFinished"),
        QStringLiteral("success=%1 noteId=%2 dirtyCount=%3 %4")
            .arg(success)
            .arg(normalizedNoteId)
            .arg(m_dirtyNoteOrder.size())
            .arg(WhatSon::Debug::summarizeText(text)));

    m_persistenceInFlight = false;
    m_inFlightNoteId.clear();
    const QString completedNoteDirectoryPath = m_inFlightNoteDirectoryPath;
    m_inFlightNoteDirectoryPath.clear();
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
                ->reconcileViewSessionAndRefreshSnapshotForNote(
                    normalizedNoteId,
                    completedText,
                    true,
                    completedNoteDirectoryPath);
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

bool ContentsEditorPersistenceController::stageEditorSnapshot(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& text,
    const bool requestImmediateFetch)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString normalizedNoteDirectoryPath = noteDirectoryPath.trimmed();
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("stageEditorSnapshot"),
        QStringLiteral("noteId=%1 noteDirectoryPath=%2 immediate=%3 %4")
            .arg(normalizedNoteId)
            .arg(normalizedNoteDirectoryPath)
            .arg(requestImmediateFetch)
            .arg(WhatSon::Debug::summarizeText(text)));
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const QString normalizedText = text;
    BufferedSnapshot bufferedSnapshot;
    bufferedSnapshot.text = normalizedText;
    if (!normalizedNoteDirectoryPath.isEmpty())
    {
        bufferedSnapshot.noteDirectoryPath = normalizedNoteDirectoryPath;
        bufferedSnapshot.directPersistenceReady = true;
    }
    else if (m_noteManagementCoordinator != nullptr)
    {
        QString resolvedNoteDirectoryPath;
        if (m_noteManagementCoordinator->captureDirectPersistenceContextForNote(
                normalizedNoteId,
                &resolvedNoteDirectoryPath))
        {
            bufferedSnapshot.noteDirectoryPath = resolvedNoteDirectoryPath.trimmed();
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
        return enqueueNextBufferedPersistenceIfNeeded();
    }

    return true;
}

bool ContentsEditorPersistenceController::enqueueNextBufferedPersistenceIfNeeded()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("enqueueNextBufferedPersistenceIfNeeded"),
        QStringLiteral("dirtyCount=%1 inFlight=%2").arg(m_dirtyNoteOrder.size()).arg(m_persistenceInFlight));
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
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("editorPersistence"),
            QStringLiteral("enqueueNextBufferedPersistenceIfNeeded.empty"));
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
    m_inFlightNoteDirectoryPath = candidateSnapshot.noteDirectoryPath;
    m_inFlightText = candidateText;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("enqueuePersist.accepted"),
        QStringLiteral("noteId=%1 direct=%2 %3")
            .arg(candidateNoteId)
            .arg(candidateSnapshot.directPersistenceReady)
            .arg(WhatSon::Debug::summarizeText(candidateText)));
    emit editorTextPersistenceQueued(candidateNoteId, candidateText);
    return true;
}

void ContentsEditorPersistenceController::ensureFetchTimerRunning()
{
    if (!m_dirtyNoteOrder.isEmpty() && !m_fetchTimer.isActive())
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("editorPersistence"),
            QStringLiteral("ensureFetchTimerRunning"),
            QStringLiteral("dirtyCount=%1").arg(m_dirtyNoteOrder.size()));
        m_fetchTimer.start();
    }
}

void ContentsEditorPersistenceController::markNoteDirty(const QString& noteId)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("markNoteDirty"),
        QStringLiteral("noteId=%1").arg(noteId.trimmed()));
    removeNoteFromDirtyOrder(noteId);
    m_dirtyNoteOrder.append(noteId);
}

void ContentsEditorPersistenceController::removeNoteFromDirtyOrder(const QString& noteId)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPersistence"),
        QStringLiteral("removeNoteFromDirtyOrder"),
        QStringLiteral("noteId=%1").arg(noteId.trimmed()));
    m_dirtyNoteOrder.removeAll(noteId);
}
