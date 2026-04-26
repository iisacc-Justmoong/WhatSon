#include "app/models/editor/display/ContentsDisplayRefreshCoordinator.hpp"

#include <QtGlobal>

ContentsDisplayRefreshCoordinator::ContentsDisplayRefreshCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayRefreshCoordinator::~ContentsDisplayRefreshCoordinator() = default;

bool ContentsDisplayRefreshCoordinator::lineGeometryRefreshEnabled() const noexcept
{
    return m_lineGeometryRefreshEnabled;
}

void ContentsDisplayRefreshCoordinator::setLineGeometryRefreshEnabled(const bool enabled)
{
    if (m_lineGeometryRefreshEnabled == enabled)
    {
        return;
    }

    m_lineGeometryRefreshEnabled = enabled;
    emit lineGeometryRefreshEnabledChanged();
}

bool ContentsDisplayRefreshCoordinator::minimapRefreshEnabled() const noexcept
{
    return m_minimapRefreshEnabled;
}

void ContentsDisplayRefreshCoordinator::setMinimapRefreshEnabled(const bool enabled)
{
    if (m_minimapRefreshEnabled == enabled)
    {
        return;
    }

    m_minimapRefreshEnabled = enabled;
    emit minimapRefreshEnabledChanged();
}

bool ContentsDisplayRefreshCoordinator::preferNativeInputHandling() const noexcept
{
    return m_preferNativeInputHandling;
}

void ContentsDisplayRefreshCoordinator::setPreferNativeInputHandling(const bool enabled)
{
    if (m_preferNativeInputHandling == enabled)
    {
        return;
    }

    m_preferNativeInputHandling = enabled;
    emit preferNativeInputHandlingChanged();
}

bool ContentsDisplayRefreshCoordinator::showEditorGutter() const noexcept
{
    return m_showEditorGutter;
}

void ContentsDisplayRefreshCoordinator::setShowEditorGutter(const bool enabled)
{
    if (m_showEditorGutter == enabled)
    {
        return;
    }

    m_showEditorGutter = enabled;
    emit showEditorGutterChanged();
}

bool ContentsDisplayRefreshCoordinator::showPrintEditorLayout() const noexcept
{
    return m_showPrintEditorLayout;
}

void ContentsDisplayRefreshCoordinator::setShowPrintEditorLayout(const bool enabled)
{
    if (m_showPrintEditorLayout == enabled)
    {
        return;
    }

    m_showPrintEditorLayout = enabled;
    emit showPrintEditorLayoutChanged();
}

bool ContentsDisplayRefreshCoordinator::editorInputFocused() const noexcept
{
    return m_editorInputFocused;
}

void ContentsDisplayRefreshCoordinator::setEditorInputFocused(const bool focused)
{
    if (m_editorInputFocused == focused)
    {
        return;
    }

    m_editorInputFocused = focused;
    emit editorInputFocusedChanged();
}

bool ContentsDisplayRefreshCoordinator::structuredHostGeometryActive() const noexcept
{
    return m_structuredHostGeometryActive;
}

void ContentsDisplayRefreshCoordinator::setStructuredHostGeometryActive(const bool active)
{
    if (m_structuredHostGeometryActive == active)
    {
        return;
    }

    m_structuredHostGeometryActive = active;
    emit structuredHostGeometryActiveChanged();
}

int ContentsDisplayRefreshCoordinator::liveLogicalLineCount() const noexcept
{
    return m_liveLogicalLineCount;
}

void ContentsDisplayRefreshCoordinator::setLiveLogicalLineCount(const int lineCount)
{
    const int normalizedLineCount = qMax(1, lineCount);
    if (m_liveLogicalLineCount == normalizedLineCount)
    {
        return;
    }

    m_liveLogicalLineCount = normalizedLineCount;
    emit liveLogicalLineCountChanged();
}

bool ContentsDisplayRefreshCoordinator::minimapSnapshotForceFullRefresh() const noexcept
{
    return m_minimapSnapshotForceFullRefresh;
}

void ContentsDisplayRefreshCoordinator::setMinimapSnapshotForceFullRefresh(const bool forceFull)
{
    if (m_minimapSnapshotForceFullRefresh == forceFull)
    {
        return;
    }

    m_minimapSnapshotForceFullRefresh = forceFull;
    emit minimapSnapshotForceFullRefreshChanged();
}

bool ContentsDisplayRefreshCoordinator::minimapSnapshotRefreshQueued() const noexcept
{
    return m_minimapSnapshotRefreshQueued;
}

bool ContentsDisplayRefreshCoordinator::cursorDrivenUiRefreshQueued() const noexcept
{
    return m_cursorDrivenUiRefreshQueued;
}

bool ContentsDisplayRefreshCoordinator::viewportGutterRefreshQueued() const noexcept
{
    return m_viewportGutterRefreshQueued;
}

bool ContentsDisplayRefreshCoordinator::typingViewportCorrectionQueued() const noexcept
{
    return m_typingViewportCorrectionQueued;
}

bool ContentsDisplayRefreshCoordinator::typingViewportForceCorrectionRequested() const noexcept
{
    return m_typingViewportForceCorrectionRequested;
}

int ContentsDisplayRefreshCoordinator::gutterRefreshPassesRemaining() const noexcept
{
    return m_gutterRefreshPassesRemaining;
}

QString ContentsDisplayRefreshCoordinator::pendingNoteEntryGutterRefreshNoteId() const
{
    return m_pendingNoteEntryGutterRefreshNoteId;
}

void ContentsDisplayRefreshCoordinator::setPendingNoteEntryGutterRefreshNoteId(const QString& noteId)
{
    const QString normalizedNoteId = normalizeNoteId(noteId);
    if (m_pendingNoteEntryGutterRefreshNoteId == normalizedNoteId)
    {
        return;
    }

    m_pendingNoteEntryGutterRefreshNoteId = normalizedNoteId;
    emit pendingNoteEntryGutterRefreshNoteIdChanged();
}

bool ContentsDisplayRefreshCoordinator::shouldScheduleGutterRefreshForReason(
    const QString& reason,
    const int activeLogicalLineCount) const
{
    const QString normalizedReason = reason.trimmed().toLower();
    if (normalizedReason != QStringLiteral("line-structure"))
    {
        return true;
    }

    if (!m_editorInputFocused)
    {
        return true;
    }

    return qMax(1, activeLogicalLineCount) != qMax(1, m_liveLogicalLineCount);
}

QVariantMap ContentsDisplayRefreshCoordinator::scheduleGutterRefresh(
    const int passCount,
    const QString& reason,
    const int activeLogicalLineCount)
{
    QVariantMap plan;
    if (!shouldScheduleGutterRefreshForReason(reason, activeLogicalLineCount))
    {
        return plan;
    }

    const int requestedPassCount = qMax(1, passCount);
    if (requestedPassCount > m_gutterRefreshPassesRemaining)
    {
        setGutterRefreshPassesRemaining(requestedPassCount);
    }

    plan.insert(QStringLiteral("startTimer"), true);
    return plan;
}

QVariantMap ContentsDisplayRefreshCoordinator::scheduleNoteEntryGutterRefresh(const QString& noteId)
{
    QVariantMap plan;
    const QString normalizedNoteId = normalizeNoteId(noteId);
    setPendingNoteEntryGutterRefreshNoteId(normalizedNoteId);
    if (normalizedNoteId.isEmpty())
    {
        return plan;
    }

    plan.insert(QStringLiteral("resetNoteEntryLineGeometry"), true);
    plan.insert(QStringLiteral("requestStructuredLayoutRefresh"), true);
    plan.insert(QStringLiteral("scheduleViewportGutterRefresh"), true);
    plan.insert(QStringLiteral("gutterPassCount"), 6);
    plan.insert(QStringLiteral("gutterReason"), QStringLiteral("note-entry"));
    return plan;
}

QVariantMap ContentsDisplayRefreshCoordinator::scheduleCursorDrivenUiRefresh()
{
    QVariantMap plan;
    if (!m_minimapRefreshEnabled && m_preferNativeInputHandling)
    {
        return plan;
    }

    if (m_cursorDrivenUiRefreshQueued)
    {
        return plan;
    }

    setCursorDrivenUiRefreshQueued(true);
    plan.insert(QStringLiteral("queueCallLater"), true);
    return plan;
}

QVariantMap ContentsDisplayRefreshCoordinator::scheduleViewportGutterRefresh()
{
    QVariantMap plan;
    if (!m_showEditorGutter || m_viewportGutterRefreshQueued)
    {
        return plan;
    }

    setViewportGutterRefreshQueued(true);
    plan.insert(QStringLiteral("queueCallLater"), true);
    return plan;
}

QVariantMap ContentsDisplayRefreshCoordinator::scheduleMinimapSnapshotRefresh(const bool forceFull)
{
    QVariantMap plan;
    if (!m_lineGeometryRefreshEnabled)
    {
        return plan;
    }

    if (forceFull)
    {
        setMinimapSnapshotForceFullRefresh(true);
    }

    if (m_minimapSnapshotRefreshQueued)
    {
        return plan;
    }

    setMinimapSnapshotRefreshQueued(true);
    plan.insert(QStringLiteral("queueCallLater"), true);
    return plan;
}

QVariantMap ContentsDisplayRefreshCoordinator::scheduleTypingViewportCorrection(const bool forceAnchor)
{
    QVariantMap plan;
    if (m_showPrintEditorLayout)
    {
        return plan;
    }

    if (forceAnchor)
    {
        setTypingViewportForceCorrectionRequested(true);
    }

    if (m_typingViewportCorrectionQueued)
    {
        return plan;
    }

    setTypingViewportCorrectionQueued(true);
    plan.insert(QStringLiteral("queueCallLater"), true);
    return plan;
}

void ContentsDisplayRefreshCoordinator::clearCursorDrivenUiRefreshQueued()
{
    setCursorDrivenUiRefreshQueued(false);
}

void ContentsDisplayRefreshCoordinator::clearViewportGutterRefreshQueued()
{
    setViewportGutterRefreshQueued(false);
}

void ContentsDisplayRefreshCoordinator::clearMinimapSnapshotRefreshQueued()
{
    setMinimapSnapshotRefreshQueued(false);
}

int ContentsDisplayRefreshCoordinator::takeGutterRefreshPassesRemaining()
{
    const int passes = m_gutterRefreshPassesRemaining;
    setGutterRefreshPassesRemaining(0);
    return passes;
}

bool ContentsDisplayRefreshCoordinator::takeTypingViewportForceCorrectionRequested()
{
    const bool requested = m_typingViewportForceCorrectionRequested;
    setTypingViewportForceCorrectionRequested(false);
    return requested;
}

void ContentsDisplayRefreshCoordinator::clearTypingViewportCorrectionQueued()
{
    setTypingViewportCorrectionQueued(false);
}

void ContentsDisplayRefreshCoordinator::clearTypingViewportCorrectionState()
{
    setTypingViewportForceCorrectionRequested(false);
    setTypingViewportCorrectionQueued(false);
}

void ContentsDisplayRefreshCoordinator::setMinimapSnapshotRefreshQueued(const bool queued)
{
    if (m_minimapSnapshotRefreshQueued == queued)
    {
        return;
    }

    m_minimapSnapshotRefreshQueued = queued;
    emit minimapSnapshotRefreshQueuedChanged();
}

void ContentsDisplayRefreshCoordinator::setCursorDrivenUiRefreshQueued(const bool queued)
{
    if (m_cursorDrivenUiRefreshQueued == queued)
    {
        return;
    }

    m_cursorDrivenUiRefreshQueued = queued;
    emit cursorDrivenUiRefreshQueuedChanged();
}

void ContentsDisplayRefreshCoordinator::setViewportGutterRefreshQueued(const bool queued)
{
    if (m_viewportGutterRefreshQueued == queued)
    {
        return;
    }

    m_viewportGutterRefreshQueued = queued;
    emit viewportGutterRefreshQueuedChanged();
}

void ContentsDisplayRefreshCoordinator::setTypingViewportCorrectionQueued(const bool queued)
{
    if (m_typingViewportCorrectionQueued == queued)
    {
        return;
    }

    m_typingViewportCorrectionQueued = queued;
    emit typingViewportCorrectionQueuedChanged();
}

void ContentsDisplayRefreshCoordinator::setTypingViewportForceCorrectionRequested(const bool requested)
{
    if (m_typingViewportForceCorrectionRequested == requested)
    {
        return;
    }

    m_typingViewportForceCorrectionRequested = requested;
    emit typingViewportForceCorrectionRequestedChanged();
}

void ContentsDisplayRefreshCoordinator::setGutterRefreshPassesRemaining(const int passes)
{
    const int normalizedPasses = qMax(0, passes);
    if (m_gutterRefreshPassesRemaining == normalizedPasses)
    {
        return;
    }

    m_gutterRefreshPassesRemaining = normalizedPasses;
    emit gutterRefreshPassesRemainingChanged();
}

QString ContentsDisplayRefreshCoordinator::normalizeNoteId(const QString& noteId)
{
    return noteId.trimmed();
}
