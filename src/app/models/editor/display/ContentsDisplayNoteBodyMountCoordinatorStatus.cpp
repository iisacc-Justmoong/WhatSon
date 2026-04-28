#include "app/models/editor/display/ContentsDisplayNoteBodyMountCoordinator.hpp"

namespace
{
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
        if (reason == QStringLiteral("body-source-unavailable"))
        {
            return QStringLiteral("No usable note body source was available for the current selection.");
        }
        if (reason == QStringLiteral("document-surface-not-requested"))
        {
            return QStringLiteral("The current selection did not request a note document surface.");
        }
        return QStringLiteral("The selected note document could not be mounted.");
    }
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
    if (documentSourceReady())
    {
        return false;
    }
    if (m_selectedNoteBodyLoading)
    {
        return true;
    }
    if (m_pendingMountNoteId == normalizedSelectedNoteId)
    {
        return true;
    }
    return !refreshAttemptedForSelectedNote();
}

bool ContentsDisplayNoteBodyMountCoordinator::mountDecisionClean() const noexcept
{
    return m_mountDecisionClean;
}

bool ContentsDisplayNoteBodyMountCoordinator::parseMounted() const noexcept
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

    return documentSourceReady();
}

bool ContentsDisplayNoteBodyMountCoordinator::sourceMounted() const noexcept
{
    if (mountPending())
    {
        return false;
    }
    return parseMounted();
}

bool ContentsDisplayNoteBodyMountCoordinator::noteMounted() const noexcept
{
    return sourceMounted()
        && editorSessionSynchronizedToSelectedSource();
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
    return !parseMounted();
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
    if (!parseMounted())
    {
        if (!m_selectedNoteBodyNoteId.isEmpty() && m_selectedNoteBodyNoteId != normalizedSelectedNoteId)
        {
            return QStringLiteral("body-note-mismatch");
        }
        return QStringLiteral("body-source-unavailable");
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

QString ContentsDisplayNoteBodyMountCoordinator::exceptionReason() const
{
    if (m_selectedNoteId.isEmpty())
    {
        return QStringLiteral("no-selected-note");
    }

    return mountFailureReason().trimmed();
}

QString ContentsDisplayNoteBodyMountCoordinator::exceptionTitle() const
{
    const QString reason = exceptionReason();
    if (reason.isEmpty())
    {
        return {};
    }
    if (reason == QStringLiteral("no-selected-note"))
    {
        return QStringLiteral("No note selected");
    }
    if (reason == QStringLiteral("body-note-mismatch"))
    {
        return QStringLiteral("Selected note changed");
    }
    if (reason == QStringLiteral("body-unresolved")
        || reason == QStringLiteral("body-source-unavailable"))
    {
        return QStringLiteral("Note body unavailable");
    }
    return QStringLiteral("Note document unavailable");
}

QString ContentsDisplayNoteBodyMountCoordinator::exceptionMessage() const
{
    const QString reason = exceptionReason();
    if (reason.isEmpty())
    {
        return {};
    }
    if (reason == QStringLiteral("no-selected-note"))
    {
        return QStringLiteral("Select a note to open its document.");
    }
    return mountFailureMessage();
}

bool ContentsDisplayNoteBodyMountCoordinator::exceptionVisible() const noexcept
{
    return m_visible && mountFailed() && !parseMounted();
}

QVariantMap ContentsDisplayNoteBodyMountCoordinator::currentMountState() const
{
    QVariantMap state;
    state.insert(QStringLiteral("documentSourceReady"), documentSourceReady());
    state.insert(QStringLiteral("editorBoundNoteId"), m_editorBoundNoteId);
    state.insert(
        QStringLiteral("editorSessionBoundToSelectedNoteId"),
        editorSessionBoundToSelectedNoteId());
    state.insert(
        QStringLiteral("editorSessionReadyForPresentation"),
        editorSessionReadyForPresentation());
    state.insert(
        QStringLiteral("editorSessionRequiresSelectionMount"),
        editorSessionRequiresSelectionMount());
    state.insert(
        QStringLiteral("editorSessionSynchronizedToSelectedSource"),
        editorSessionSynchronizedToSelectedSource());
    state.insert(QStringLiteral("editorSessionBoundToSelectedNote"), m_editorSessionBoundToSelectedNote);
    state.insert(QStringLiteral("mountFailed"), mountFailed());
    state.insert(QStringLiteral("mountFailureReason"), mountFailureReason());
    state.insert(QStringLiteral("mountFailureMessage"), mountFailureMessage());
    state.insert(QStringLiteral("mountDecisionClean"), mountDecisionClean());
    state.insert(QStringLiteral("mountPending"), mountPending());
    state.insert(QStringLiteral("parseMounted"), parseMounted());
    state.insert(QStringLiteral("sourceMounted"), sourceMounted());
    state.insert(QStringLiteral("noteMounted"), noteMounted());
    state.insert(QStringLiteral("pendingMountNoteId"), m_pendingMountNoteId);
    state.insert(QStringLiteral("selectedNoteBodyLoading"), m_selectedNoteBodyLoading);
    state.insert(QStringLiteral("selectedNoteBodyNoteId"), m_selectedNoteBodyNoteId);
    state.insert(QStringLiteral("selectedNoteBodyResolved"), m_selectedNoteBodyResolved);
    state.insert(QStringLiteral("selectedNoteId"), m_selectedNoteId);
    state.insert(QStringLiteral("selectedBodyReadyForPresentation"), selectedBodyReadyForPresentation());
    state.insert(QStringLiteral("selectionSnapshotReady"), selectionSnapshotReady());
    state.insert(QStringLiteral("snapshotRefreshAttemptedNoteId"), m_snapshotRefreshAttemptedNoteId);
    state.insert(QStringLiteral("structuredDocumentSurfaceRequested"), m_structuredDocumentSurfaceRequested);
    state.insert(QStringLiteral("visible"), m_visible);
    return state;
}

bool ContentsDisplayNoteBodyMountCoordinator::canFlushMountImmediately() const noexcept
{
    return documentSourceReady() || !m_selectedNoteBodyLoading;
}

bool ContentsDisplayNoteBodyMountCoordinator::documentSurfaceRequested() const noexcept
{
    return m_structuredDocumentSurfaceRequested;
}

bool ContentsDisplayNoteBodyMountCoordinator::selectionSnapshotReady() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    if (normalizedSelectedNoteId.isEmpty())
    {
        return false;
    }

    if (!m_selectedNoteBodyText.isEmpty())
    {
        if (m_selectedNoteBodyResolved)
        {
            return true;
        }
        if (m_selectedNoteBodyNoteId.isEmpty() || m_selectedNoteBodyNoteId == normalizedSelectedNoteId)
        {
            return true;
        }
    }

    return m_selectedNoteBodyResolved && m_selectedNoteBodyNoteId == normalizedSelectedNoteId;
}

bool ContentsDisplayNoteBodyMountCoordinator::selectionSnapshotRepresentsExplicitEmptyBody() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    return !normalizedSelectedNoteId.isEmpty()
        && m_selectedNoteBodyResolved
        && m_selectedNoteBodyNoteId == normalizedSelectedNoteId
        && m_selectedNoteBodyText.isEmpty();
}

bool ContentsDisplayNoteBodyMountCoordinator::selectedBodyReadyForPresentation() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    return !normalizedSelectedNoteId.isEmpty()
        && m_selectedNoteBodyResolved
        && m_selectedNoteBodyNoteId == normalizedSelectedNoteId;
}

bool ContentsDisplayNoteBodyMountCoordinator::editorSessionRepresentsExplicitEmptyBody() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    const QString normalizedEditorNoteId = normalizeNoteId(m_editorBoundNoteId);
    return !normalizedSelectedNoteId.isEmpty()
        && m_editorSessionBoundToSelectedNote
        && normalizedEditorNoteId == normalizedSelectedNoteId
        && m_pendingBodySave
        && m_editorText.isEmpty();
}

bool ContentsDisplayNoteBodyMountCoordinator::editorSessionBoundToSelectedNoteId() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    const QString normalizedEditorNoteId = normalizeNoteId(m_editorBoundNoteId);
    return !normalizedSelectedNoteId.isEmpty()
        && m_editorSessionBoundToSelectedNote
        && normalizedEditorNoteId == normalizedSelectedNoteId;
}

bool ContentsDisplayNoteBodyMountCoordinator::editorSessionReadyForPresentation() const noexcept
{
    return editorSessionBoundToSelectedNoteId()
        && (!m_editorText.isEmpty() || editorSessionRepresentsExplicitEmptyBody());
}

bool ContentsDisplayNoteBodyMountCoordinator::editorSessionSynchronizedToSelectedSource() const noexcept
{
    if (!editorSessionBoundToSelectedNoteId())
    {
        return false;
    }

    if (m_pendingBodySave)
    {
        return !m_editorText.isEmpty() || editorSessionRepresentsExplicitEmptyBody();
    }

    if (!selectedBodyReadyForPresentation())
    {
        return false;
    }

    return m_editorText == m_selectedNoteBodyText;
}

bool ContentsDisplayNoteBodyMountCoordinator::editorSessionRequiresSelectionMount() const noexcept
{
    if (m_pendingBodySave)
    {
        return !editorSessionSynchronizedToSelectedSource();
    }

    if (!selectedBodyReadyForPresentation())
    {
        return false;
    }

    return !editorSessionSynchronizedToSelectedSource();
}

bool ContentsDisplayNoteBodyMountCoordinator::documentSourceReady() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    if (normalizedSelectedNoteId.isEmpty())
    {
        return false;
    }

    const bool bodyMatchesSelection = m_selectedNoteBodyNoteId == normalizedSelectedNoteId;
    const bool bodyOwnedBySelection =
        bodyMatchesSelection || m_selectedNoteBodyNoteId.isEmpty();
    const bool bodyHasText = !m_selectedNoteBodyText.isEmpty();
    const bool bodyAvailable = bodyOwnedBySelection
        && (bodyHasText || m_selectedNoteBodyResolved);
    const bool bodyReadyForPresentation = selectedBodyReadyForPresentation();
    const bool editorReadyForPresentation = editorSessionReadyForPresentation();
    const bool preferEditorSource =
        editorReadyForPresentation
        && (m_pendingBodySave || !bodyAvailable);

    return preferEditorSource || bodyReadyForPresentation;
}

bool ContentsDisplayNoteBodyMountCoordinator::refreshAttemptedForSelectedNote() const noexcept
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    return !normalizedSelectedNoteId.isEmpty()
        && m_snapshotRefreshAttemptedNoteId == normalizedSelectedNoteId;
}
