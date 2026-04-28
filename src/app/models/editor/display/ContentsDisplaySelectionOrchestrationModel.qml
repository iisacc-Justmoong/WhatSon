pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: model

    property var contentsView: null
    property var editorInputPolicyAdapter: null
    property var selectionMountViewModel: null
    property var structuredDocumentFlow: null

    function shouldFlushBlurredEditorState(scheduledNoteId) {
        return model.selectionMountViewModel.shouldFlushBlurredEditorState(scheduledNoteId);
    }

    function nativeEditorCompositionActive() {
        return !!(model.structuredDocumentFlow
                  && model.structuredDocumentFlow.nativeCompositionActive !== undefined
                  && model.structuredDocumentFlow.nativeCompositionActive());
    }

    function nativeTextInputSessionOwnsKeyboard() {
        return !!(model.editorInputPolicyAdapter
                  && model.editorInputPolicyAdapter.nativeTextInputSessionActive);
    }

    function flushEditorStateAfterInputSettles(scheduledNoteId) {
        model.selectionMountViewModel.flushEditorStateAfterInputSettles(scheduledNoteId);
    }

    function focusEditorForSelectedNoteId(noteId) {
        model.selectionMountViewModel.focusEditorForSelectedNoteId(noteId);
    }

    function focusEditorForPendingNote() {
        model.selectionMountViewModel.focusEditorForPendingNote();
    }

    function scheduleEditorEntrySnapshotReconcile() {
        model.selectionMountViewModel.scheduleEditorEntrySnapshotReconcile();
    }

    function pollSelectedNoteSnapshot() {
        model.selectionMountViewModel.pollSelectedNoteSnapshot();
    }

    function reconcileEditorEntrySnapshotOnce() {
        return model.selectionMountViewModel.reconcileEditorEntrySnapshotOnce();
    }

    function scheduleSelectionModelSync(options) {
        model.selectionMountViewModel.scheduleSelectionModelSync(options);
    }

    function executeSelectionDeliveryPlan(plan, fallbackKey) {
        return model.selectionMountViewModel.executeSelectionDeliveryPlan(plan, fallbackKey);
    }

    function resetEditorSelectionCache() {
        model.selectionMountViewModel.resetEditorSelectionCache();
    }

    function scheduleEditorFocusForNote(noteId) {
        model.selectionMountViewModel.scheduleEditorFocusForNote(noteId);
    }
}
