#include "app/models/content/display/ContentsDisplayNoteBodyMountCoordinator.hpp"

#include "app/models/editor/parser/ContentsWsnBodyBlockParser.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QTimer>

namespace
{
    QString normalizedOptionalNoteId(const QString& value)
    {
        return value.trimmed();
    }

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

    QString mountFailureMessageForReason(const QString& reason)
    {
        if (reason == QStringLiteral("no-selected-note"))
        {
            return QStringLiteral("No document opened");
        }
        if (reason == QStringLiteral("body-note-mismatch"))
        {
            return QStringLiteral("The selected note body no longer matches the current selection.");
        }
        if (reason == QStringLiteral("body-unresolved"))
        {
            return QStringLiteral("The selected note body could not be resolved after refreshing its snapshot.");
        }
        if (reason == QStringLiteral("structured-document-surface-unavailable"))
        {
            return QStringLiteral("The structured document surface did not become ready for the selected note.");
        }
        if (reason == QStringLiteral("inline-document-surface-unavailable"))
        {
            return QStringLiteral("The inline editor surface did not become ready for the selected note.");
        }
        if (reason == QStringLiteral("document-surface-not-requested"))
        {
            return QStringLiteral("The current selection did not request a note document surface.");
        }
        if (reason == QStringLiteral("document-surface-unavailable"))
        {
            return QStringLiteral("The note document surface did not become ready for the selected note.");
        }
        return QStringLiteral("The selected note document could not be mounted.");
    }
}

ContentsDisplayNoteBodyMountCoordinator::ContentsDisplayNoteBodyMountCoordinator(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("noteBodyMountCoordinator"), QStringLiteral("ctor"));
}

ContentsDisplayNoteBodyMountCoordinator::~ContentsDisplayNoteBodyMountCoordinator()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("dtor"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 bodyResolved=%3 pendingMount=%4 refreshAttempted=%5")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_pendingMountNoteId)
            .arg(m_snapshotRefreshAttemptedNoteId));
}

bool ContentsDisplayNoteBodyMountCoordinator::visible() const noexcept
{
    return m_visible;
}

void ContentsDisplayNoteBodyMountCoordinator::setVisible(const bool visible)
{
    if (m_visible == visible)
    {
        return;
    }

    m_visible = visible;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setVisible"),
        QStringLiteral("visible=%1").arg(visible));
    emit visibleChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::selectedNoteId() const
{
    return m_selectedNoteId;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_selectedNoteId == normalized)
    {
        return;
    }

    m_selectedNoteId = normalized;
    resetCurrentSelectionMountTracking();
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit selectedNoteIdChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::selectedNoteBodyNoteId() const
{
    return m_selectedNoteBodyNoteId;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteBodyNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_selectedNoteBodyNoteId == normalized)
    {
        return;
    }

    m_selectedNoteBodyNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteBodyNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit selectedNoteBodyNoteIdChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::selectedNoteBodyText() const
{
    return m_selectedNoteBodyText;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteBodyText(const QString& text)
{
    if (m_selectedNoteBodyText == text)
    {
        return;
    }

    m_selectedNoteBodyText = text;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteBodyText"),
        WhatSon::Debug::summarizeText(text));
    emit selectedNoteBodyTextChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::selectedNoteBodyResolved() const noexcept
{
    return m_selectedNoteBodyResolved;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteBodyResolved(const bool resolved)
{
    if (m_selectedNoteBodyResolved == resolved)
    {
        return;
    }

    m_selectedNoteBodyResolved = resolved;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteBodyResolved"),
        QStringLiteral("resolved=%1").arg(resolved));
    emit selectedNoteBodyResolvedChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::selectedNoteBodyLoading() const noexcept
{
    return m_selectedNoteBodyLoading;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteBodyLoading(const bool loading)
{
    if (m_selectedNoteBodyLoading == loading)
    {
        return;
    }

    m_selectedNoteBodyLoading = loading;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteBodyLoading"),
        QStringLiteral("loading=%1").arg(loading));
    emit selectedNoteBodyLoadingChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::editorBoundNoteId() const
{
    return m_editorBoundNoteId;
}

void ContentsDisplayNoteBodyMountCoordinator::setEditorBoundNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_editorBoundNoteId == normalized)
    {
        return;
    }

    m_editorBoundNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setEditorBoundNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit editorBoundNoteIdChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::editorSessionBoundToSelectedNote() const noexcept
{
    return m_editorSessionBoundToSelectedNote;
}

void ContentsDisplayNoteBodyMountCoordinator::setEditorSessionBoundToSelectedNote(const bool bound)
{
    if (m_editorSessionBoundToSelectedNote == bound)
    {
        return;
    }

    m_editorSessionBoundToSelectedNote = bound;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setEditorSessionBoundToSelectedNote"),
        QStringLiteral("bound=%1").arg(bound));
    emit editorSessionBoundToSelectedNoteChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::pendingBodySave() const noexcept
{
    return m_pendingBodySave;
}

void ContentsDisplayNoteBodyMountCoordinator::setPendingBodySave(const bool pending)
{
    if (m_pendingBodySave == pending)
    {
        return;
    }

    m_pendingBodySave = pending;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setPendingBodySave"),
        QStringLiteral("pending=%1").arg(pending));
    emit pendingBodySaveChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::editorText() const
{
    return m_editorText;
}

void ContentsDisplayNoteBodyMountCoordinator::setEditorText(const QString& text)
{
    if (m_editorText == text)
    {
        return;
    }

    m_editorText = text;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setEditorText"),
        WhatSon::Debug::summarizeText(text));
    emit editorTextChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::inlineDocumentSurfaceRequested() const noexcept
{
    return m_inlineDocumentSurfaceRequested;
}

void ContentsDisplayNoteBodyMountCoordinator::setInlineDocumentSurfaceRequested(const bool requested)
{
    if (m_inlineDocumentSurfaceRequested == requested)
    {
        return;
    }

    m_inlineDocumentSurfaceRequested = requested;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setInlineDocumentSurfaceRequested"),
        QStringLiteral("requested=%1").arg(requested));
    emit inlineDocumentSurfaceRequestedChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::inlineDocumentSurfaceReady() const noexcept
{
    return m_inlineDocumentSurfaceReady;
}

void ContentsDisplayNoteBodyMountCoordinator::setInlineDocumentSurfaceReady(const bool ready)
{
    if (m_inlineDocumentSurfaceReady == ready)
    {
        return;
    }

    m_inlineDocumentSurfaceReady = ready;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setInlineDocumentSurfaceReady"),
        QStringLiteral("ready=%1").arg(ready));
    emit inlineDocumentSurfaceReadyChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::inlineDocumentSurfaceLoading() const noexcept
{
    return m_inlineDocumentSurfaceLoading;
}

void ContentsDisplayNoteBodyMountCoordinator::setInlineDocumentSurfaceLoading(const bool loading)
{
    if (m_inlineDocumentSurfaceLoading == loading)
    {
        return;
    }

    m_inlineDocumentSurfaceLoading = loading;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setInlineDocumentSurfaceLoading"),
        QStringLiteral("loading=%1").arg(loading));
    emit inlineDocumentSurfaceLoadingChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::structuredDocumentSurfaceRequested() const noexcept
{
    return m_structuredDocumentSurfaceRequested;
}

void ContentsDisplayNoteBodyMountCoordinator::setStructuredDocumentSurfaceRequested(const bool requested)
{
    if (m_structuredDocumentSurfaceRequested == requested)
    {
        return;
    }

    m_structuredDocumentSurfaceRequested = requested;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setStructuredDocumentSurfaceRequested"),
        QStringLiteral("requested=%1").arg(requested));
    emit structuredDocumentSurfaceRequestedChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::structuredDocumentSurfaceReady() const noexcept
{
    return m_structuredDocumentSurfaceReady;
}

void ContentsDisplayNoteBodyMountCoordinator::setStructuredDocumentSurfaceReady(const bool ready)
{
    if (m_structuredDocumentSurfaceReady == ready)
    {
        return;
    }

    m_structuredDocumentSurfaceReady = ready;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setStructuredDocumentSurfaceReady"),
        QStringLiteral("ready=%1").arg(ready));
    emit structuredDocumentSurfaceReadyChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::mountPending() const noexcept
{
    if (!m_visible)
    {
        return false;
    }

    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    if (normalizedSelectedNoteId.isEmpty() || !documentSurfaceRequested())
    {
        return false;
    }
    if (m_selectedNoteBodyLoading || documentSurfaceLoading())
    {
        return true;
    }
    if (documentSourceReady())
    {
        return false;
    }
    if (m_pendingMountNoteId == normalizedSelectedNoteId)
    {
        return true;
    }
    return !refreshAttemptedForSelectedNote();
}

bool ContentsDisplayNoteBodyMountCoordinator::noteMounted() const noexcept
{
    if (!m_visible)
    {
        return false;
    }

    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    if (normalizedSelectedNoteId.isEmpty() || !documentSurfaceRequested())
    {
        return false;
    }
    if (mountPending())
    {
        return false;
    }
    return documentSourceReady() && documentSurfaceReady();
}

bool ContentsDisplayNoteBodyMountCoordinator::mountFailed() const noexcept
{
    if (!m_visible)
    {
        return false;
    }

    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    if (normalizedSelectedNoteId.isEmpty() || !documentSurfaceRequested())
    {
        return false;
    }
    if (mountPending())
    {
        return false;
    }
    return !noteMounted();
}

bool ContentsDisplayNoteBodyMountCoordinator::surfaceVisible() const noexcept
{
    return noteMounted() || mountPending();
}

QString ContentsDisplayNoteBodyMountCoordinator::mountFailureReason() const
{
    if (!mountFailed())
    {
        return {};
    }

    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    if (normalizedSelectedNoteId.isEmpty())
    {
        return QStringLiteral("no-selected-note");
    }
    if (!documentSurfaceRequested())
    {
        return QStringLiteral("document-surface-not-requested");
    }
    if (!documentSourceReady())
    {
        return m_selectedNoteBodyNoteId == normalizedSelectedNoteId
            ? QStringLiteral("body-unresolved")
            : QStringLiteral("body-note-mismatch");
    }

    const bool structuredSurfaceUnavailable =
        m_structuredDocumentSurfaceRequested && !m_structuredDocumentSurfaceReady;
    const bool inlineSurfaceUnavailable =
        m_inlineDocumentSurfaceRequested && !m_inlineDocumentSurfaceReady;
    if (structuredSurfaceUnavailable && inlineSurfaceUnavailable)
    {
        return QStringLiteral("document-surface-unavailable");
    }
    if (structuredSurfaceUnavailable)
    {
        return QStringLiteral("structured-document-surface-unavailable");
    }
    if (inlineSurfaceUnavailable)
    {
        return QStringLiteral("inline-document-surface-unavailable");
    }
    if (!documentSurfaceReady())
    {
        return QStringLiteral("document-surface-unavailable");
    }
    return QStringLiteral("document-mount-failed");
}

QString ContentsDisplayNoteBodyMountCoordinator::mountFailureMessage() const
{
    if (!mountFailed())
    {
        return {};
    }
    return mountFailureMessageForReason(mountFailureReason());
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

QVariantMap ContentsDisplayNoteBodyMountCoordinator::currentMountState() const
{
    QVariantMap state;
    state.insert(QStringLiteral("documentSourceReady"), documentSourceReady());
    state.insert(QStringLiteral("documentSurfaceLoading"), documentSurfaceLoading());
    state.insert(QStringLiteral("documentSurfaceReady"), documentSurfaceReady());
    state.insert(QStringLiteral("documentSurfaceRequested"), documentSurfaceRequested());
    state.insert(QStringLiteral("editorBoundNoteId"), m_editorBoundNoteId);
    state.insert(QStringLiteral("editorSessionBoundToSelectedNote"), m_editorSessionBoundToSelectedNote);
    state.insert(QStringLiteral("inlineDocumentSurfaceLoading"), m_inlineDocumentSurfaceLoading);
    state.insert(QStringLiteral("inlineDocumentSurfaceReady"), m_inlineDocumentSurfaceReady);
    state.insert(QStringLiteral("inlineDocumentSurfaceRequested"), m_inlineDocumentSurfaceRequested);
    state.insert(QStringLiteral("mountFailed"), mountFailed());
    state.insert(QStringLiteral("mountFailureReason"), mountFailureReason());
    state.insert(QStringLiteral("mountFailureMessage"), mountFailureMessage());
    state.insert(QStringLiteral("mountPending"), mountPending());
    state.insert(QStringLiteral("noteMounted"), noteMounted());
    state.insert(QStringLiteral("pendingMountNoteId"), m_pendingMountNoteId);
    state.insert(QStringLiteral("selectedNoteBodyLoading"), m_selectedNoteBodyLoading);
    state.insert(QStringLiteral("selectedNoteBodyNoteId"), m_selectedNoteBodyNoteId);
    state.insert(QStringLiteral("selectedNoteBodyResolved"), m_selectedNoteBodyResolved);
    state.insert(QStringLiteral("selectedNoteId"), m_selectedNoteId);
    state.insert(QStringLiteral("selectionSnapshotReady"), selectionSnapshotReady());
    state.insert(QStringLiteral("snapshotRefreshAttemptedNoteId"), m_snapshotRefreshAttemptedNoteId);
    state.insert(QStringLiteral("structuredDocumentSurfaceReady"), m_structuredDocumentSurfaceReady);
    state.insert(QStringLiteral("structuredDocumentSurfaceRequested"), m_structuredDocumentSurfaceRequested);
    state.insert(QStringLiteral("surfaceVisible"), surfaceVisible());
    state.insert(QStringLiteral("visible"), m_visible);
    return state;
}

QString ContentsDisplayNoteBodyMountCoordinator::normalizeNoteId(const QString& noteId)
{
    return normalizedOptionalNoteId(noteId);
}

bool ContentsDisplayNoteBodyMountCoordinator::canFlushMountImmediately() const noexcept
{
    return !m_selectedNoteBodyLoading && !documentSurfaceLoading();
}

bool ContentsDisplayNoteBodyMountCoordinator::documentSurfaceRequested() const noexcept
{
    return m_inlineDocumentSurfaceRequested || m_structuredDocumentSurfaceRequested;
}

bool ContentsDisplayNoteBodyMountCoordinator::documentSurfaceReady() const noexcept
{
    const bool inlineSurfaceReady = m_inlineDocumentSurfaceRequested && m_inlineDocumentSurfaceReady;
    const bool structuredSurfaceReady =
        m_structuredDocumentSurfaceRequested && m_structuredDocumentSurfaceReady;
    return inlineSurfaceReady || structuredSurfaceReady;
}

bool ContentsDisplayNoteBodyMountCoordinator::documentSurfaceLoading() const noexcept
{
    return m_inlineDocumentSurfaceRequested && m_inlineDocumentSurfaceLoading;
}

bool ContentsDisplayNoteBodyMountCoordinator::selectionSnapshotReady() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    if (normalizedSelectedNoteId.isEmpty())
    {
        return false;
    }

    return m_selectedNoteBodyResolved && m_selectedNoteBodyNoteId == normalizedSelectedNoteId;
}

bool ContentsDisplayNoteBodyMountCoordinator::documentSourceReady() const noexcept
{
    if (m_editorSessionBoundToSelectedNote && parserAcceptsSource(m_editorText))
    {
        return true;
    }

    if (selectionSnapshotReady() && parserAcceptsSource(m_selectedNoteBodyText))
    {
        return true;
    }

    return false;
}

bool ContentsDisplayNoteBodyMountCoordinator::refreshAttemptedForSelectedNote() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    return !normalizedSelectedNoteId.isEmpty()
        && m_snapshotRefreshAttemptedNoteId == normalizedSelectedNoteId;
}

bool ContentsDisplayNoteBodyMountCoordinator::parserAcceptsSource(const QString& sourceText) const
{
    if (sourceText.isEmpty())
    {
        return false;
    }

    ContentsWsnBodyBlockParser parser;
    const auto parseResult = parser.parse(sourceText);
    return !parseResult.correctedSourceText.isEmpty()
        || !parseResult.renderedDocumentBlocks.isEmpty()
        || !parseResult.renderedAgendas.isEmpty()
        || !parseResult.renderedCallouts.isEmpty();
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
    if (m_selectedNoteBodyLoading || documentSurfaceLoading())
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

    const bool bodyNoteMatchesSelection = m_selectedNoteBodyNoteId == normalizedSelectedNoteId;
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
                    ? QStringLiteral("refresh-after-body-unresolved")
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
                ? QStringLiteral("body-unresolved")
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
    if (!m_editorSessionBoundToSelectedNote)
    {
        plan.insert(QStringLiteral("attemptEditorSessionMount"), true);
        plan.insert(QStringLiteral("reason"), QStringLiteral("mount-editor-session"));
    }
    else
    {
        plan.insert(QStringLiteral("reason"), QStringLiteral("mounted"));
    }

    if (m_focusEditorOnMountNoteId == normalizedSelectedNoteId && documentSurfaceReady())
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
