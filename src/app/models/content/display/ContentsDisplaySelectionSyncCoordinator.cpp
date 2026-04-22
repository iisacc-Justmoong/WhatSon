#include "app/models/content/display/ContentsDisplaySelectionSyncCoordinator.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

namespace
{
    QString normalizedOptionalNoteId(const QString& value)
    {
        return value.trimmed();
    }

    QString summarizeSelectionSyncPlan(const QVariantMap& plan)
    {
        return QStringLiteral(
                   "reason=%1 noteId=%2 selectedNoteId=%3 bodyNoteId=%4 bodyResolved=%5 allowSnapshotRefresh=%6 attemptReconcile=%7 attemptSelectionSync=%8 resetSelectionCache=%9 scheduleSnapshotReconcile=%10 focusEditor=%11 flushPendingEditorText=%12 fallbackRefresh=%13 forceVisualRefresh=%14")
            .arg(plan.value(QStringLiteral("reason")).toString())
            .arg(plan.value(QStringLiteral("noteId")).toString())
            .arg(plan.value(QStringLiteral("selectedNoteId")).toString())
            .arg(plan.value(QStringLiteral("selectedNoteBodyNoteId")).toString())
            .arg(plan.value(QStringLiteral("selectedNoteBodyResolved")).toBool())
            .arg(plan.value(QStringLiteral("allowSnapshotRefresh")).toBool())
            .arg(plan.value(QStringLiteral("attemptReconcile")).toBool())
            .arg(plan.value(QStringLiteral("attemptSelectionSync")).toBool())
            .arg(plan.value(QStringLiteral("resetSelectionCache")).toBool())
            .arg(plan.value(QStringLiteral("scheduleSnapshotReconcile")).toBool())
            .arg(plan.value(QStringLiteral("focusEditorForSelectedNote")).toBool())
            .arg(plan.value(QStringLiteral("flushPendingEditorText")).toBool())
            .arg(plan.value(QStringLiteral("fallbackRefreshIfSyncSkipped")).toBool())
            .arg(plan.value(QStringLiteral("forceVisualRefresh")).toBool());
    }
}

ContentsDisplaySelectionSyncCoordinator::ContentsDisplaySelectionSyncCoordinator(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("selectionSyncCoordinator"), QStringLiteral("ctor"));
}

ContentsDisplaySelectionSyncCoordinator::~ContentsDisplaySelectionSyncCoordinator()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("dtor"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 bodyResolved=%3 editorBoundNoteId=%4 pendingSnapshot=%5 comparedSnapshot=%6")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_editorBoundNoteId)
            .arg(m_pendingSnapshotNoteId)
            .arg(m_comparedSnapshotNoteId));
}

bool ContentsDisplaySelectionSyncCoordinator::visible() const noexcept
{
    return m_visible;
}

void ContentsDisplaySelectionSyncCoordinator::setVisible(const bool visible)
{
    if (m_visible == visible)
    {
        return;
    }

    m_visible = visible;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setVisible"),
        QStringLiteral("visible=%1").arg(visible));
    emit visibleChanged();
}

QString ContentsDisplaySelectionSyncCoordinator::selectedNoteId() const
{
    return m_selectedNoteId;
}

void ContentsDisplaySelectionSyncCoordinator::setSelectedNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_selectedNoteId == normalized)
    {
        return;
    }

    m_selectedNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setSelectedNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit selectedNoteIdChanged();
}

QString ContentsDisplaySelectionSyncCoordinator::selectedNoteBodyNoteId() const
{
    return m_selectedNoteBodyNoteId;
}

void ContentsDisplaySelectionSyncCoordinator::setSelectedNoteBodyNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_selectedNoteBodyNoteId == normalized)
    {
        return;
    }

    m_selectedNoteBodyNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setSelectedNoteBodyNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit selectedNoteBodyNoteIdChanged();
}

QString ContentsDisplaySelectionSyncCoordinator::selectedNoteBodyText() const
{
    return m_selectedNoteBodyText;
}

void ContentsDisplaySelectionSyncCoordinator::setSelectedNoteBodyText(const QString& text)
{
    const QString normalized = text;
    if (m_selectedNoteBodyText == normalized)
    {
        return;
    }

    m_selectedNoteBodyText = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setSelectedNoteBodyText"),
        WhatSon::Debug::summarizeText(normalized));
    emit selectedNoteBodyTextChanged();
}

bool ContentsDisplaySelectionSyncCoordinator::selectedNoteBodyResolved() const noexcept
{
    return m_selectedNoteBodyResolved;
}

void ContentsDisplaySelectionSyncCoordinator::setSelectedNoteBodyResolved(const bool resolved)
{
    if (m_selectedNoteBodyResolved == resolved)
    {
        return;
    }

    m_selectedNoteBodyResolved = resolved;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setSelectedNoteBodyResolved"),
        QStringLiteral("resolved=%1").arg(resolved));
    emit selectedNoteBodyResolvedChanged();
}

bool ContentsDisplaySelectionSyncCoordinator::selectedNoteBodyLoading() const noexcept
{
    return m_selectedNoteBodyLoading;
}

void ContentsDisplaySelectionSyncCoordinator::setSelectedNoteBodyLoading(const bool loading)
{
    if (m_selectedNoteBodyLoading == loading)
    {
        return;
    }

    m_selectedNoteBodyLoading = loading;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setSelectedNoteBodyLoading"),
        QStringLiteral("loading=%1").arg(loading));
    emit selectedNoteBodyLoadingChanged();
}

QString ContentsDisplaySelectionSyncCoordinator::editorBoundNoteId() const
{
    return m_editorBoundNoteId;
}

void ContentsDisplaySelectionSyncCoordinator::setEditorBoundNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_editorBoundNoteId == normalized)
    {
        return;
    }

    m_editorBoundNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setEditorBoundNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit editorBoundNoteIdChanged();
}

bool ContentsDisplaySelectionSyncCoordinator::editorSessionBoundToSelectedNote() const noexcept
{
    return m_editorSessionBoundToSelectedNote;
}

void ContentsDisplaySelectionSyncCoordinator::setEditorSessionBoundToSelectedNote(const bool bound)
{
    if (m_editorSessionBoundToSelectedNote == bound)
    {
        return;
    }

    m_editorSessionBoundToSelectedNote = bound;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setEditorSessionBoundToSelectedNote"),
        QStringLiteral("bound=%1").arg(bound));
    emit editorSessionBoundToSelectedNoteChanged();
}

bool ContentsDisplaySelectionSyncCoordinator::editorInputFocused() const noexcept
{
    return m_editorInputFocused;
}

void ContentsDisplaySelectionSyncCoordinator::setEditorInputFocused(const bool focused)
{
    if (m_editorInputFocused == focused)
    {
        return;
    }

    m_editorInputFocused = focused;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setEditorInputFocused"),
        QStringLiteral("focused=%1").arg(focused));
    emit editorInputFocusedChanged();
}

bool ContentsDisplaySelectionSyncCoordinator::pendingBodySave() const noexcept
{
    return m_pendingBodySave;
}

void ContentsDisplaySelectionSyncCoordinator::setPendingBodySave(const bool pending)
{
    if (m_pendingBodySave == pending)
    {
        return;
    }

    m_pendingBodySave = pending;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setPendingBodySave"),
        QStringLiteral("pending=%1").arg(pending));
    emit pendingBodySaveChanged();
}

bool ContentsDisplaySelectionSyncCoordinator::typingSessionSyncProtected() const noexcept
{
    return m_typingSessionSyncProtected;
}

void ContentsDisplaySelectionSyncCoordinator::setTypingSessionSyncProtected(const bool protectedSync)
{
    if (m_typingSessionSyncProtected == protectedSync)
    {
        return;
    }

    m_typingSessionSyncProtected = protectedSync;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setTypingSessionSyncProtected"),
        QStringLiteral("protected=%1").arg(protectedSync));
    emit typingSessionSyncProtectedChanged();
}

QString ContentsDisplaySelectionSyncCoordinator::comparedSnapshotNoteId() const
{
    return m_comparedSnapshotNoteId;
}

QString ContentsDisplaySelectionSyncCoordinator::pendingSnapshotNoteId() const
{
    return m_pendingSnapshotNoteId;
}

QString ContentsDisplaySelectionSyncCoordinator::pendingEditorFocusNoteId() const
{
    return m_pendingEditorFocusNoteId;
}

void ContentsDisplaySelectionSyncCoordinator::scheduleSelectionSync(const QVariantMap& options)
{
    const bool resetSnapshot = options.value(QStringLiteral("resetSnapshot")).toBool();
    const bool scheduleReconcile = options.value(QStringLiteral("scheduleReconcile")).toBool();
    const bool focusEditor = options.value(QStringLiteral("focusEditor")).toBool();
    const bool fallbackRefresh = options.value(QStringLiteral("fallbackRefresh")).toBool();
    const bool forceVisualRefresh = options.value(QStringLiteral("forceVisualRefresh")).toBool();

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("scheduleSelectionSync"),
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
        m_selectionSyncResetSnapshotPending = true;
    }
    if (scheduleReconcile)
    {
        m_selectionSyncReconcilePending = true;
    }
    if (focusEditor)
    {
        m_selectionSyncFocusEditorPending = true;
    }
    if (fallbackRefresh)
    {
        m_selectionSyncFallbackRefreshPending = true;
    }
    if (forceVisualRefresh)
    {
        m_selectionSyncForceVisualRefreshPending = true;
    }

    if (m_selectionSyncQueued)
    {
        return;
    }

    if (canFlushSelectionSyncImmediately())
    {
        flushSelectionSync();
        return;
    }

    m_selectionSyncQueued = true;
    QTimer::singleShot(0, this, [this]()
    {
        flushSelectionSync();
    });
}

void ContentsDisplaySelectionSyncCoordinator::scheduleSnapshotReconcile()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("scheduleSnapshotReconcile"),
        QStringLiteral("alreadyQueued=%1").arg(m_snapshotReconcileQueued));
    if (m_snapshotReconcileQueued)
    {
        return;
    }

    m_snapshotReconcileQueued = true;
    QTimer::singleShot(0, this, [this]()
    {
        m_snapshotReconcileQueued = false;
        emit snapshotReconcileRequested();
    });
}

QVariantMap ContentsDisplaySelectionSyncCoordinator::snapshotPollPlan() const
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    const auto tracedPlan = [this](const QVariantMap& plan)
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionSyncCoordinator"),
            QStringLiteral("selectionFlow.snapshotPollPlan"),
            summarizeSelectionSyncPlan(plan));
        return plan;
    };
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("snapshotPollPlan"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 bodyResolved=%3 editorBoundToSelected=%4 focused=%5 pendingBodySave=%6 typingProtected=%7 pendingSnapshot=%8")
            .arg(normalizedSelectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_editorSessionBoundToSelectedNote)
            .arg(m_editorInputFocused)
            .arg(m_pendingBodySave)
            .arg(m_typingSessionSyncProtected)
            .arg(m_pendingSnapshotNoteId));

    if (m_typingSessionSyncProtected || m_pendingBodySave)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("blocked-by-local-edit")));
    }
    if (!m_editorSessionBoundToSelectedNote)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("session-not-bound")));
    }
    if (normalizedSelectedNoteId.isEmpty())
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("no-selected-note")));
    }
    if (!normalizedSelectedNoteId.isEmpty() && m_selectedNoteBodyNoteId != normalizedSelectedNoteId)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("body-note-mismatch")));
    }
    if (!normalizedSelectedNoteId.isEmpty() && !m_selectedNoteBodyResolved)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("body-unresolved")));
    }
    if (m_pendingSnapshotNoteId == normalizedSelectedNoteId)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("reconcile-in-flight")));
    }

    QVariantMap plan = buildSnapshotPlan(QStringLiteral("poll"));
    plan.insert(QStringLiteral("attemptReconcile"), !normalizedSelectedNoteId.isEmpty());
    plan.insert(QStringLiteral("allowSnapshotRefresh"), true);
    plan.insert(QStringLiteral("noteId"), normalizedSelectedNoteId);
    return tracedPlan(plan);
}

QVariantMap ContentsDisplaySelectionSyncCoordinator::snapshotReconcilePlan() const
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    const auto tracedPlan = [this](const QVariantMap& plan)
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionSyncCoordinator"),
            QStringLiteral("selectionFlow.snapshotReconcilePlan"),
            summarizeSelectionSyncPlan(plan));
        return plan;
    };
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("snapshotReconcilePlan"),
        QStringLiteral("visible=%1 selectedNoteId=%2 bodyNoteId=%3 bodyResolved=%4 sessionBound=%5 compared=%6 pending=%7 focused=%8 pendingBodySave=%9 typingProtected=%10")
            .arg(m_visible)
            .arg(normalizedSelectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_editorSessionBoundToSelectedNote)
            .arg(m_comparedSnapshotNoteId)
            .arg(m_pendingSnapshotNoteId)
            .arg(m_editorInputFocused)
            .arg(m_pendingBodySave)
            .arg(m_typingSessionSyncProtected));

    if (!m_visible || normalizedSelectedNoteId.isEmpty())
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("inactive-host")));
    }
    if (!m_editorSessionBoundToSelectedNote)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("session-not-bound")));
    }
    if (m_selectedNoteBodyNoteId != normalizedSelectedNoteId)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("body-note-mismatch")));
    }
    if (!m_selectedNoteBodyResolved)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("body-unresolved")));
    }
    if (m_comparedSnapshotNoteId == normalizedSelectedNoteId)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("already-compared")));
    }
    if (m_pendingSnapshotNoteId == normalizedSelectedNoteId)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("reconcile-in-flight")));
    }
    if (m_editorInputFocused || m_pendingBodySave || m_typingSessionSyncProtected)
    {
        return tracedPlan(buildSnapshotPlan(QStringLiteral("blocked-by-input")));
    }

    QVariantMap plan = buildSnapshotPlan(QStringLiteral("reconcile"));
    plan.insert(QStringLiteral("attemptReconcile"), true);
    plan.insert(QStringLiteral("noteId"), normalizedSelectedNoteId);
    return tracedPlan(plan);
}

void ContentsDisplaySelectionSyncCoordinator::markSnapshotReconcileStarted(const QString& noteId)
{
    const QString normalizedNoteId = normalizeNoteId(noteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("markSnapshotReconcileStarted"),
        QStringLiteral("noteId=%1").arg(normalizedNoteId));
    setPendingSnapshotNoteId(normalizedNoteId);
}

void ContentsDisplaySelectionSyncCoordinator::handleSnapshotReconcileFinished(const QString& noteId, const bool success)
{
    const QString normalizedNoteId = normalizeNoteId(noteId);
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("handleSnapshotReconcileFinished"),
        QStringLiteral("noteId=%1 success=%2 selectedNoteId=%3").arg(normalizedNoteId).arg(success).arg(normalizedSelectedNoteId));

    if (m_pendingSnapshotNoteId == normalizedNoteId)
    {
        setPendingSnapshotNoteId(QString());
    }

    if (normalizedNoteId.isEmpty() || normalizedNoteId != normalizedSelectedNoteId)
    {
        return;
    }

    if (success)
    {
        setComparedSnapshotNoteId(normalizedNoteId);
    }
}

void ContentsDisplaySelectionSyncCoordinator::invalidateComparedSnapshot()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("invalidateComparedSnapshot"),
        QStringLiteral("previous=%1").arg(m_comparedSnapshotNoteId));
    setComparedSnapshotNoteId(QString());
}

void ContentsDisplaySelectionSyncCoordinator::scheduleEditorFocusForNote(const QString& noteId)
{
    const QString normalizedNoteId = normalizeNoteId(noteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("scheduleEditorFocusForNote"),
        QStringLiteral("noteId=%1").arg(normalizedNoteId));
    if (normalizedNoteId.isEmpty())
    {
        return;
    }

    setPendingEditorFocusNoteId(normalizedNoteId);
    emit editorFocusRequested();
}

QString ContentsDisplaySelectionSyncCoordinator::takePendingEditorFocusNoteId(const QString& selectedNoteId)
{
    const QString normalizedSelectedNoteId = normalizeNoteId(selectedNoteId);
    const QString pendingNoteId = m_pendingEditorFocusNoteId;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("takePendingEditorFocusNoteId"),
        QStringLiteral("pending=%1 selected=%2").arg(pendingNoteId).arg(normalizedSelectedNoteId));
    if (pendingNoteId.isEmpty() || pendingNoteId != normalizedSelectedNoteId)
    {
        return QString();
    }

    setPendingEditorFocusNoteId(QString());
    return pendingNoteId;
}

QString ContentsDisplaySelectionSyncCoordinator::normalizeNoteId(const QString& value)
{
    return normalizedOptionalNoteId(value);
}

bool ContentsDisplaySelectionSyncCoordinator::canFlushSelectionSyncImmediately() const
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    if (normalizedSelectedNoteId.isEmpty())
    {
        return true;
    }
    if (m_selectedNoteBodyLoading)
    {
        return false;
    }
    return m_selectedNoteBodyResolved
        && m_selectedNoteBodyNoteId == normalizedSelectedNoteId;
}

QVariantMap ContentsDisplaySelectionSyncCoordinator::buildSnapshotPlan(const QString& reason) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("allowSnapshotRefresh"), false);
    plan.insert(QStringLiteral("attemptReconcile"), false);
    plan.insert(QStringLiteral("noteId"), QString());
    plan.insert(QStringLiteral("reason"), reason);
    return plan;
}

void ContentsDisplaySelectionSyncCoordinator::flushSelectionSync()
{
    const bool shouldResetSnapshot = m_selectionSyncResetSnapshotPending;
    const bool shouldScheduleReconcile = m_selectionSyncReconcilePending;
    const bool shouldFocusEditor = m_selectionSyncFocusEditorPending;
    const bool shouldFallbackRefresh = m_selectionSyncFallbackRefreshPending;
    const bool shouldForceVisualRefresh = m_selectionSyncForceVisualRefreshPending;

    m_selectionSyncQueued = false;
    m_selectionSyncResetSnapshotPending = false;
    m_selectionSyncReconcilePending = false;
    m_selectionSyncFocusEditorPending = false;
    m_selectionSyncFallbackRefreshPending = false;
    m_selectionSyncForceVisualRefreshPending = false;

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("flushSelectionSync"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 bodyResolved=%3 loading=%4 resetSnapshot=%5 scheduleReconcile=%6 focusEditor=%7 fallbackRefresh=%8 forceVisualRefresh=%9")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_selectedNoteBodyLoading)
            .arg(shouldResetSnapshot)
            .arg(shouldScheduleReconcile)
            .arg(shouldFocusEditor)
            .arg(shouldFallbackRefresh)
            .arg(shouldForceVisualRefresh));

    QVariantMap plan;
    plan.insert(QStringLiteral("attemptSelectionSync"), false);
    plan.insert(QStringLiteral("fallbackRefreshIfSyncSkipped"), shouldFallbackRefresh);
    plan.insert(QStringLiteral("flushPendingEditorText"), false);
    plan.insert(QStringLiteral("focusEditorForSelectedNote"), false);
    plan.insert(QStringLiteral("forceVisualRefresh"), shouldForceVisualRefresh);
    plan.insert(QStringLiteral("reason"), QStringLiteral("selection-sync"));
    plan.insert(QStringLiteral("resetSelectionCache"), shouldResetSnapshot);
    plan.insert(QStringLiteral("scheduleSnapshotReconcile"), shouldScheduleReconcile);
    plan.insert(QStringLiteral("selectedNoteBodyNoteId"), m_selectedNoteBodyNoteId);
    plan.insert(QStringLiteral("selectedNoteBodyResolved"), m_selectedNoteBodyResolved);
    plan.insert(QStringLiteral("selectedNoteBodyText"), m_selectedNoteBodyText);
    plan.insert(QStringLiteral("selectedNoteId"), m_selectedNoteId);

    if (shouldResetSnapshot)
    {
        setComparedSnapshotNoteId(QString());
        setPendingSnapshotNoteId(QString());
    }

    if (shouldFocusEditor)
    {
        setPendingEditorFocusNoteId(m_selectedNoteId);
    }

    if (m_selectedNoteBodyLoading && !m_selectedNoteId.isEmpty())
    {
        const bool shouldFlushPendingEditorText =
            m_editorBoundNoteId != m_selectedNoteId && m_pendingBodySave;
        plan.insert(QStringLiteral("flushPendingEditorText"), shouldFlushPendingEditorText);
        plan.insert(QStringLiteral("reason"), QStringLiteral("awaiting-body-load"));
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionSyncCoordinator"),
            QStringLiteral("selectionFlow.flushPlan"),
            summarizeSelectionSyncPlan(plan));
        emit selectionSyncFlushRequested(plan);
        return;
    }

    plan.insert(QStringLiteral("attemptSelectionSync"), !m_selectedNoteId.isEmpty());
    if (!m_pendingEditorFocusNoteId.isEmpty() && m_pendingEditorFocusNoteId == m_selectedNoteId)
    {
        plan.insert(QStringLiteral("focusEditorForSelectedNote"), true);
    }
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("selectionFlow.flushPlan"),
        summarizeSelectionSyncPlan(plan));
    emit selectionSyncFlushRequested(plan);
}

void ContentsDisplaySelectionSyncCoordinator::setComparedSnapshotNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_comparedSnapshotNoteId == normalized)
    {
        return;
    }

    m_comparedSnapshotNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setComparedSnapshotNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit comparedSnapshotNoteIdChanged();
}

void ContentsDisplaySelectionSyncCoordinator::setPendingSnapshotNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_pendingSnapshotNoteId == normalized)
    {
        return;
    }

    m_pendingSnapshotNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setPendingSnapshotNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit pendingSnapshotNoteIdChanged();
}

void ContentsDisplaySelectionSyncCoordinator::setPendingEditorFocusNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_pendingEditorFocusNoteId == normalized)
    {
        return;
    }

    m_pendingEditorFocusNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionSyncCoordinator"),
        QStringLiteral("setPendingEditorFocusNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit pendingEditorFocusNoteIdChanged();
}
