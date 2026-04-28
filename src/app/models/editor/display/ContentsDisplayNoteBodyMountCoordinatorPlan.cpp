#include "app/models/editor/display/ContentsDisplayNoteBodyMountCoordinator.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QTimer>

namespace
{
    QString summarizeMountPlan(const QVariantMap& plan)
    {
        return QStringLiteral(
                   "reason=%1 selectedNoteId=%2 bodyNoteId=%3 bodyResolved=%4 attemptSnapshotRefresh=%5 attemptEditorSessionMount=%6 resetSelectionCache=%7 scheduleSnapshotReconcile=%8 focusEditor=%9 flushPendingEditorText=%10 fallbackRefresh=%11 forceVisualRefresh=%12")
            .arg(plan.value(QStringLiteral("reason")).toString())
            .arg(plan.value(QStringLiteral("selectedNoteId")).toString())
            .arg(plan.value(QStringLiteral("selectedNoteBodyNoteId")).toString())
            .arg(plan.value(QStringLiteral("selectedNoteBodyResolved")).toBool())
            .arg(plan.value(QStringLiteral("attemptSnapshotRefresh")).toBool())
            .arg(plan.value(QStringLiteral("attemptEditorSessionMount")).toBool())
            .arg(plan.value(QStringLiteral("resetSelectionCache")).toBool())
            .arg(plan.value(QStringLiteral("scheduleSnapshotReconcile")).toBool())
            .arg(plan.value(QStringLiteral("focusEditorForSelectedNote")).toBool())
            .arg(plan.value(QStringLiteral("flushPendingEditorText")).toBool())
            .arg(plan.value(QStringLiteral("fallbackRefreshIfMountSkipped")).toBool())
            .arg(plan.value(QStringLiteral("forceVisualRefresh")).toBool());
    }
}

void ContentsDisplayNoteBodyMountCoordinator::scheduleMount(const QVariantMap& options)
{
    const bool resetSnapshot = options.value(QStringLiteral("resetSnapshot")).toBool();
    const bool scheduleReconcile = options.value(QStringLiteral("scheduleReconcile")).toBool();
    const bool focusEditor = options.value(QStringLiteral("focusEditor")).toBool();
    const bool fallbackRefresh = options.value(QStringLiteral("fallbackRefresh")).toBool();
    const bool forceVisualRefresh = options.value(QStringLiteral("forceVisualRefresh")).toBool();

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("scheduleMount"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 resetSnapshot=%3 scheduleReconcile=%4 focusEditor=%5 fallbackRefresh=%6 forceVisualRefresh=%7")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(resetSnapshot)
            .arg(scheduleReconcile)
            .arg(focusEditor)
            .arg(fallbackRefresh)
            .arg(forceVisualRefresh));

    if (resetSnapshot)
    {
        m_mountResetSnapshotPending = true;
    }
    if (scheduleReconcile)
    {
        m_mountScheduleReconcilePending = true;
    }
    if (focusEditor)
    {
        m_mountFocusEditorPending = true;
    }
    if (fallbackRefresh)
    {
        m_mountFallbackRefreshPending = true;
    }
    if (forceVisualRefresh)
    {
        m_mountForceVisualRefreshPending = true;
    }

    if (m_mountQueued)
    {
        return;
    }

    if (canFlushMountImmediately())
    {
        flushMount();
        return;
    }

    setMountDecisionClean(false);
    m_mountQueued = true;
    QTimer::singleShot(0, this, [this]()
    {
        flushMount();
    });
}

void ContentsDisplayNoteBodyMountCoordinator::handleSnapshotRefreshFinished(
    const QString& noteId,
    const bool success)
{
    const QString normalizedNoteId = normalizeNoteId(noteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("handleSnapshotRefreshFinished"),
        QStringLiteral("noteId=%1 success=%2").arg(normalizedNoteId).arg(success));
    if (normalizedNoteId.isEmpty() || normalizedNoteId != normalizeNoteId(m_selectedNoteId))
    {
        return;
    }
    if (!success)
    {
        setPendingMountNoteId(QString());
        return;
    }

    setPendingMountNoteId(normalizedNoteId);
}

void ContentsDisplayNoteBodyMountCoordinator::flushMount()
{
    const bool shouldResetSnapshot = m_mountResetSnapshotPending;
    const bool shouldScheduleReconcile = m_mountScheduleReconcilePending;
    const bool shouldFocusEditor = m_mountFocusEditorPending;
    const bool shouldFallbackRefresh = m_mountFallbackRefreshPending;
    const bool shouldForceVisualRefresh = m_mountForceVisualRefreshPending;

    m_mountQueued = false;
    m_mountResetSnapshotPending = false;
    m_mountScheduleReconcilePending = false;
    m_mountFocusEditorPending = false;
    m_mountFallbackRefreshPending = false;
    m_mountForceVisualRefreshPending = false;
    setMountDecisionClean(true);

    if (shouldResetSnapshot)
    {
        m_snapshotRefreshAttemptedNoteId.clear();
        setPendingMountNoteId(QString());
    }

    if (shouldFocusEditor)
    {
        m_focusEditorOnMountNoteId = normalizeNoteId(m_selectedNoteId);
    }

    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("flushMount"),
        QStringLiteral("visible=%1 selectedNoteId=%2 bodyNoteId=%3 bodyResolved=%4 bodyLoading=%5 editorBound=%6 scheduleReconcile=%7 fallbackRefresh=%8 forceVisualRefresh=%9")
            .arg(m_visible)
            .arg(normalizedSelectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_selectedNoteBodyLoading)
            .arg(m_editorBoundNoteId)
            .arg(shouldScheduleReconcile)
            .arg(shouldFallbackRefresh)
            .arg(shouldForceVisualRefresh));

    QVariantMap plan;
    plan.insert(QStringLiteral("attemptEditorSessionMount"), false);
    plan.insert(QStringLiteral("attemptSnapshotRefresh"), false);
    plan.insert(QStringLiteral("fallbackRefreshIfMountSkipped"), shouldFallbackRefresh);
    plan.insert(QStringLiteral("flushPendingEditorText"), false);
    plan.insert(QStringLiteral("focusEditorForSelectedNote"), false);
    plan.insert(QStringLiteral("forceVisualRefresh"), shouldForceVisualRefresh);
    plan.insert(QStringLiteral("reason"), QStringLiteral("mount"));
    plan.insert(QStringLiteral("resetSelectionCache"), shouldResetSnapshot);
    plan.insert(QStringLiteral("scheduleSnapshotReconcile"), shouldScheduleReconcile);
    plan.insert(QStringLiteral("selectedNoteBodyNoteId"), m_selectedNoteBodyNoteId);
    plan.insert(QStringLiteral("selectedNoteBodyResolved"), m_selectedNoteBodyResolved);
    plan.insert(QStringLiteral("selectedNoteBodyText"), m_selectedNoteBodyText);
    plan.insert(QStringLiteral("selectedNoteId"), normalizedSelectedNoteId);

    if (!m_visible)
    {
        plan.insert(QStringLiteral("reason"), QStringLiteral("inactive-host"));
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("noteBodyMountCoordinator"),
            QStringLiteral("selectionFlow.mountPlan"),
            summarizeMountPlan(plan));
        emit mountFlushRequested(plan);
        return;
    }
    if (normalizedSelectedNoteId.isEmpty())
    {
        plan.insert(QStringLiteral("reason"), QStringLiteral("no-selected-note"));
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("noteBodyMountCoordinator"),
            QStringLiteral("selectionFlow.mountPlan"),
            summarizeMountPlan(plan));
        emit mountFlushRequested(plan);
        return;
    }
    if (!documentSurfaceRequested())
    {
        plan.insert(QStringLiteral("reason"), QStringLiteral("document-surface-not-requested"));
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("noteBodyMountCoordinator"),
            QStringLiteral("selectionFlow.mountPlan"),
            summarizeMountPlan(plan));
        emit mountFlushRequested(plan);
        return;
    }
    if (!documentSourceReady() && m_selectedNoteBodyLoading)
    {
        setPendingMountNoteId(normalizedSelectedNoteId);
        if (m_editorBoundNoteId != normalizedSelectedNoteId && m_pendingBodySave)
        {
            plan.insert(QStringLiteral("flushPendingEditorText"), true);
        }
        plan.insert(QStringLiteral("reason"), QStringLiteral("awaiting-body-load"));
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("noteBodyMountCoordinator"),
            QStringLiteral("selectionFlow.mountPlan"),
            summarizeMountPlan(plan));
        emit mountFlushRequested(plan);
        return;
    }

    const bool bodyNoteMatchesSelection = m_selectedNoteBodyNoteId.isEmpty()
        || m_selectedNoteBodyNoteId == normalizedSelectedNoteId;
    if (!documentSourceReady())
    {
        if (!refreshAttemptedForSelectedNote())
        {
            m_snapshotRefreshAttemptedNoteId = normalizedSelectedNoteId;
            setPendingMountNoteId(normalizedSelectedNoteId);
            emit mountStateChanged();
            plan.insert(QStringLiteral("attemptSnapshotRefresh"), true);
            plan.insert(
                QStringLiteral("reason"),
                bodyNoteMatchesSelection
                    ? QStringLiteral("refresh-after-body-source-pending")
                    : QStringLiteral("refresh-after-body-note-mismatch"));
            WhatSon::Debug::traceEditorSelf(
                this,
                QStringLiteral("noteBodyMountCoordinator"),
                QStringLiteral("selectionFlow.mountPlan"),
                summarizeMountPlan(plan));
            emit mountFlushRequested(plan);
            return;
        }

        setPendingMountNoteId(QString());
        plan.insert(
            QStringLiteral("reason"),
            bodyNoteMatchesSelection
                ? QStringLiteral("body-source-unavailable")
                : QStringLiteral("body-note-mismatch"));
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("noteBodyMountCoordinator"),
            QStringLiteral("selectionFlow.mountPlan"),
            summarizeMountPlan(plan));
        emit mountFlushRequested(plan);
        return;
    }

    setPendingMountNoteId(QString());
    if (editorSessionRequiresSelectionMount())
    {
        plan.insert(QStringLiteral("attemptEditorSessionMount"), true);
        plan.insert(QStringLiteral("reason"), QStringLiteral("mount-editor-session"));
    }
    else
    {
        plan.insert(QStringLiteral("reason"), QStringLiteral("mounted"));
    }

    if (m_focusEditorOnMountNoteId == normalizedSelectedNoteId && parseMounted())
    {
        plan.insert(QStringLiteral("focusEditorForSelectedNote"), true);
        m_focusEditorOnMountNoteId.clear();
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("selectionFlow.mountPlan"),
        summarizeMountPlan(plan));
    emit mountFlushRequested(plan);
}

void ContentsDisplayNoteBodyMountCoordinator::resetCurrentSelectionMountTracking()
{
    setPendingMountNoteId(QString());
    m_snapshotRefreshAttemptedNoteId.clear();
    m_focusEditorOnMountNoteId.clear();
}

void ContentsDisplayNoteBodyMountCoordinator::setMountDecisionClean(const bool clean)
{
    if (m_mountDecisionClean == clean)
    {
        return;
    }

    m_mountDecisionClean = clean;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setMountDecisionClean"),
        QStringLiteral("clean=%1").arg(clean));
    emit mountStateChanged();
}

void ContentsDisplayNoteBodyMountCoordinator::setPendingMountNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_pendingMountNoteId == normalized)
    {
        return;
    }

    m_pendingMountNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setPendingMountNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit mountStateChanged();
}
