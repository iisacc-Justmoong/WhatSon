pragma ComponentBehavior: Bound

import QtQuick
import "../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace

QtObject {
    id: controller

    property var contentEditor: null
    property var contentsView: null
    property var editorSelectionController: null
    property var editorSession: null
    property var editorTypingController: null
    property var noteBodyMountCoordinator: null
    property var selectionBridge: null
    property var selectionSyncCoordinator: null
    property var traceFormatter: null

    function shouldFlushBlurredEditorState(scheduledNoteId) {
        const normalizedScheduledNoteId = scheduledNoteId === undefined || scheduledNoteId === null
                ? ""
                : String(scheduledNoteId).trim();
        if (normalizedScheduledNoteId.length === 0 || !controller.editorSession)
            return false;
        const currentBoundNoteId = controller.editorSession.editorBoundNoteId !== undefined
                && controller.editorSession.editorBoundNoteId !== null
                ? String(controller.editorSession.editorBoundNoteId).trim()
                : "";
        const currentSelectedNoteId = controller.contentsView.selectedNoteId === undefined
                || controller.contentsView.selectedNoteId === null
                ? ""
                : String(controller.contentsView.selectedNoteId).trim();
        if (currentBoundNoteId !== normalizedScheduledNoteId
                || currentSelectedNoteId !== normalizedScheduledNoteId) {
            return false;
        }
        const hasLocalEditorAuthority = controller.editorSession.localEditorAuthority !== undefined
                && !!controller.editorSession.localEditorAuthority;
        const hasPendingBodySave = controller.editorSession.pendingBodySave !== undefined
                && !!controller.editorSession.pendingBodySave;
        return hasLocalEditorAuthority || hasPendingBodySave;
    }

    function flushEditorStateAfterInputSettles(scheduledNoteId) {
        const normalizedScheduledNoteId = scheduledNoteId === undefined || scheduledNoteId === null
                ? ""
                : String(scheduledNoteId).trim();
        if (!controller.shouldFlushBlurredEditorState(normalizedScheduledNoteId))
            return;
        if (controller.contentsView.nativeEditorCompositionActive())
            return;
        const currentBoundNoteId = controller.editorSession
                && controller.editorSession.editorBoundNoteId !== undefined
                && controller.editorSession.editorBoundNoteId !== null
                ? String(controller.editorSession.editorBoundNoteId).trim()
                : "";
        const currentSelectedNoteId = controller.contentsView.selectedNoteId === undefined
                || controller.contentsView.selectedNoteId === null
                ? ""
                : String(controller.contentsView.selectedNoteId).trim();
        if (normalizedScheduledNoteId.length > 0
                && (currentBoundNoteId !== normalizedScheduledNoteId
                    || currentSelectedNoteId !== normalizedScheduledNoteId)) {
            return;
        }
        controller.editorTypingController.handleEditorTextEdited();
        controller.editorSession.flushPendingEditorText();
    }

    function focusEditorForSelectedNoteId(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length === 0 || !controller.contentsView.hasSelectedNote)
            return;

        Qt.callLater(function () {
            const activeNoteId = controller.contentsView.selectedNoteId === undefined
                    || controller.contentsView.selectedNoteId === null
                    ? ""
                    : String(controller.contentsView.selectedNoteId).trim();
            if (activeNoteId !== normalizedNoteId)
                return;

            const cursorPosition = Math.max(0, controller.contentsView.resolvedLogicalTextLength);
            controller.contentEditor.forceActiveFocus();
            if (controller.contentEditor.editorItem
                    && controller.contentEditor.editorItem.forceActiveFocus !== undefined)
                controller.contentEditor.editorItem.forceActiveFocus();
            if (controller.contentEditor.setCursorPositionPreservingNativeInput !== undefined)
                controller.contentEditor.setCursorPositionPreservingNativeInput(cursorPosition);
            else if (controller.contentEditor.cursorPosition !== undefined)
                controller.contentEditor.cursorPosition = cursorPosition;
        });
    }

    function focusEditorForPendingNote() {
        const selectedNoteId = controller.contentsView.selectedNoteId === undefined
                || controller.contentsView.selectedNoteId === null
                ? ""
                : String(controller.contentsView.selectedNoteId).trim();
        const pendingNoteId = controller.selectionSyncCoordinator.takePendingEditorFocusNoteId(selectedNoteId);
        if (pendingNoteId.length === 0)
            return;

        controller.focusEditorForSelectedNoteId(pendingNoteId);
    }

    function scheduleEditorEntrySnapshotReconcile() {
        controller.selectionSyncCoordinator.scheduleSnapshotReconcile();
    }

    function pollSelectedNoteSnapshot() {
        const pollPlan = controller.selectionSyncCoordinator.snapshotPollPlan();
        const normalizedNoteId = pollPlan.noteId === undefined || pollPlan.noteId === null
                ? ""
                : String(pollPlan.noteId).trim();
        EditorTrace.trace(
                    "displayView",
                    "selectionFlow.pollPlan",
                    "plan={" + controller.traceFormatter.describeSelectionPlan(pollPlan) + "}"
                    + " comparedSnapshotNoteId=" + controller.contentsView.editorEntrySnapshotComparedNoteId
                    + " pendingSnapshotNoteId=" + controller.contentsView.editorEntrySnapshotPendingNoteId,
                    controller.contentsView)
        if (pollPlan.attemptReconcile
                && controller.selectionBridge
                && controller.selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote !== undefined
                && normalizedNoteId.length > 0) {
            const sessionText = controller.editorSession.editorText === undefined || controller.editorSession.editorText === null
                    ? ""
                    : String(controller.editorSession.editorText);
            const preferViewSessionOnMismatch = controller.editorSession
                    && controller.editorSession.localEditorAuthority !== undefined
                    && !!controller.editorSession.localEditorAuthority;
            const reconcileAccepted = controller.selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote(
                        normalizedNoteId,
                        sessionText,
                        preferViewSessionOnMismatch);
            EditorTrace.trace(
                        "displayView",
                        "selectionFlow.pollReconcileRequested",
                        "accepted=" + reconcileAccepted
                        + " noteId=" + normalizedNoteId
                        + " preferViewSessionOnMismatch=" + preferViewSessionOnMismatch
                        + " sessionText={" + EditorTrace.describeText(sessionText, 48) + "}",
                        controller.contentsView)
            if (reconcileAccepted) {
                controller.selectionSyncCoordinator.markSnapshotReconcileStarted(normalizedNoteId);
                return;
            }
        }
        if (!pollPlan.allowSnapshotRefresh
                || !controller.selectionBridge
                || controller.selectionBridge.refreshSelectedNoteSnapshot === undefined) {
            EditorTrace.trace(
                        "displayView",
                        "selectionFlow.pollSkipped",
                        "plan={" + controller.traceFormatter.describeSelectionPlan(pollPlan) + "}",
                        controller.contentsView)
            return;
        }
        const snapshotRefreshAccepted = !!controller.selectionBridge.refreshSelectedNoteSnapshot();
        EditorTrace.trace(
                    "displayView",
                    "selectionFlow.pollSnapshotRefresh",
                    "accepted=" + snapshotRefreshAccepted
                    + " noteId=" + normalizedNoteId,
                    controller.contentsView)
        controller.contentsView.scheduleGutterRefresh(2);
    }

    function reconcileEditorEntrySnapshotOnce() {
        const reconcilePlan = controller.selectionSyncCoordinator.snapshotReconcilePlan();
        const normalizedNoteId = reconcilePlan.noteId === undefined || reconcilePlan.noteId === null
                ? ""
                : String(reconcilePlan.noteId).trim();
        EditorTrace.trace(
                    "displayView",
                    "selectionFlow.reconcilePlan",
                    "plan={" + controller.traceFormatter.describeSelectionPlan(reconcilePlan) + "}",
                    controller.contentsView)
        if (!reconcilePlan.attemptReconcile)
            return false;
        if (!controller.selectionBridge
                || controller.selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote === undefined) {
            EditorTrace.trace(
                        "displayView",
                        "selectionFlow.reconcileSkipped",
                        "reason=bridge-unavailable noteId=" + normalizedNoteId,
                        controller.contentsView)
            return false;
        }
        const sessionText = controller.editorSession.editorText === undefined || controller.editorSession.editorText === null
                ? ""
                : String(controller.editorSession.editorText);
        const preferViewSessionOnMismatch = controller.editorSession
                && controller.editorSession.localEditorAuthority !== undefined
                && !!controller.editorSession.localEditorAuthority;
        const reconcileAccepted = controller.selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote(
                    normalizedNoteId,
                    sessionText,
                    preferViewSessionOnMismatch);
        EditorTrace.trace(
                    "displayView",
                    "selectionFlow.reconcileRequested",
                    "accepted=" + reconcileAccepted
                    + " noteId=" + normalizedNoteId
                    + " preferViewSessionOnMismatch=" + preferViewSessionOnMismatch
                    + " sessionText={" + EditorTrace.describeText(sessionText, 48) + "}",
                    controller.contentsView)
        if (!reconcileAccepted)
            return false;
        controller.selectionSyncCoordinator.markSnapshotReconcileStarted(normalizedNoteId);
        return true;
    }

    function scheduleSelectionModelSync(options) {
        const normalizedOptions = options && typeof options === "object" ? options : ({});
        EditorTrace.trace(
                    "displayView",
                    "selectionFlow.scheduleSelectionModelSync",
                    "options={" + controller.traceFormatter.describeSelectionSyncOptions(normalizedOptions) + "}"
                    + " selectedNoteId=" + controller.contentsView.selectedNoteId
                    + " bodyNoteId=" + controller.contentsView.selectedNoteBodyNoteId
                    + " bodyResolved=" + controller.contentsView.selectedNoteBodyResolved
                    + " bodyLoading=" + controller.contentsView.selectedNoteBodyLoading
                    + " editorBoundNoteId=" + controller.contentsView.editorBoundNoteId,
                    controller.contentsView)
        controller.selectionSyncCoordinator.scheduleSelectionSync(normalizedOptions);
        controller.noteBodyMountCoordinator.scheduleMount(normalizedOptions);
    }

    function executeSelectionDeliveryPlan(plan, fallbackKey) {
        const normalizedPlan = plan && typeof plan === "object" ? plan : ({});
        if (normalizedPlan.resetSelectionCache)
            controller.contentsView.resetEditorSelectionCache();
        if (normalizedPlan.flushPendingEditorText
                && controller.editorSession
                && controller.editorSession.flushPendingEditorText !== undefined) {
            controller.editorSession.flushPendingEditorText();
        }
        let selectionSynced = false;
        if ((normalizedPlan.attemptSelectionSync || normalizedPlan.attemptEditorSessionMount)
                && controller.editorSession
                && controller.editorSession.requestSyncEditorTextFromSelection !== undefined) {
            selectionSynced = controller.editorSession.requestSyncEditorTextFromSelection(
                        String(normalizedPlan.selectedNoteId || ""),
                        String(normalizedPlan.selectedNoteBodyText || ""),
                        String(normalizedPlan.selectedNoteBodyNoteId || ""),
                        String(controller.contentsView.selectedNoteDirectoryPath || ""));
        }
        if (normalizedPlan.scheduleSnapshotReconcile)
            controller.contentsView.scheduleEditorEntrySnapshotReconcile();
        if (normalizedPlan.forceVisualRefresh
                || (!selectionSynced && normalizedPlan[fallbackKey])) {
            controller.contentsView.scheduleMinimapSnapshotRefresh(true);
            controller.contentsView.scheduleDocumentPresentationRefresh(true);
            controller.contentsView.scheduleGutterRefresh(4);
        }
        if (normalizedPlan.focusEditorForSelectedNote)
            controller.selectionSyncCoordinator.scheduleEditorFocusForNote(
                        String(normalizedPlan.selectedNoteId || ""));
        return selectionSynced;
    }

    function scheduleEditorFocusForNote(noteId) {
        controller.selectionSyncCoordinator.scheduleEditorFocusForNote(noteId);
    }

    function resetEditorSelectionCache() {
        controller.editorSelectionController.resetEditorSelectionCache();
    }
}
