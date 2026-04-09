import QtQuick

Item {
    id: editorSession

    property string editorBoundNoteId: ""
    property string editorText: ""
    property bool localEditorAuthority: false
    property bool pendingBodySave: false
    property var selectionBridge: null
    property bool syncingEditorTextFromModel: false

    signal editorTextSynchronized

    function handleEditorPersistenceFinished(noteId, text, success) {
        if (!success)
            return;
        const completedNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const completedText = text === undefined || text === null ? "" : String(text);
        if (editorSession.editorBoundNoteId === completedNoteId
                && editorSession.editorText === completedText) {
            editorSession.pendingBodySave = false;
        }
    }
    function flushPendingEditorText() {
        if (!editorSession.pendingBodySave)
            return true;
        const noteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null ? "" : String(editorSession.editorBoundNoteId).trim();
        if (noteId.length === 0) {
            editorSession.pendingBodySave = false;
            return false;
        }
        if (!editorSession.selectionBridge)
            return false;
        const bodyText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        if (editorSession.selectionBridge.flushEditorTextForNote !== undefined)
            return !!editorSession.selectionBridge.flushEditorTextForNote(noteId, bodyText);
        if (editorSession.selectionBridge.stageEditorTextForIdleSync !== undefined)
            return !!editorSession.selectionBridge.stageEditorTextForIdleSync(noteId, bodyText);
        return false;
    }
    function requestSyncEditorTextFromSelection(noteId, text) {
        const nextNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const nextText = text === undefined || text === null ? "" : String(text);
        const currentNoteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null ? "" : String(editorSession.editorBoundNoteId);
        if (currentNoteId !== nextNoteId && editorSession.pendingBodySave) {
            editorSession.scheduleEditorPersistence();
        }
        editorSession.syncEditorTextFromSelection(nextNoteId, nextText);
        return true;
    }
    function releaseSyncGuard() {
        Qt.callLater(function () {
            editorSession.syncingEditorTextFromModel = false;
        });
    }
    function markLocalEditorAuthority() {
        editorSession.localEditorAuthority = true;
    }
    function scheduleEditorPersistence() {
        const noteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null
                ? ""
                : String(editorSession.editorBoundNoteId).trim();
        if (noteId.length === 0) {
            editorSession.pendingBodySave = false;
            return;
        }
        editorSession.pendingBodySave = true;
        if (!editorSession.selectionBridge
                || editorSession.selectionBridge.stageEditorTextForIdleSync === undefined) {
            return;
        }
        const bodyText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        editorSession.selectionBridge.stageEditorTextForIdleSync(noteId, bodyText);
    }
    function shouldAcceptModelBodyText(noteId, text) {
        const nextNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const nextText = text === undefined || text === null ? "" : String(text);
        const currentNoteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null ? "" : String(editorSession.editorBoundNoteId);
        const currentText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        if (nextNoteId !== currentNoteId)
            return true;
        if (currentText === nextText)
            return true;
        return !editorSession.localEditorAuthority;
    }
    function syncEditorTextFromSelection(noteId, text) {
        const nextNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const nextText = text === undefined || text === null ? "" : String(text);
        const noteChanged = editorSession.editorBoundNoteId !== nextNoteId;
        const textChanged = editorSession.editorText !== nextText;
        if (noteChanged || textChanged)
            editorSession.pendingBodySave = false;
        editorSession.editorBoundNoteId = nextNoteId;
        if (noteChanged)
            editorSession.localEditorAuthority = false;
        editorSession.syncingEditorTextFromModel = true;
        if (textChanged)
            editorSession.editorText = nextText;
        editorSession.releaseSyncGuard();
        editorSession.editorTextSynchronized();
    }

    visible: false

    Connections {
        function onEditorTextPersistenceFinished(noteId, text, success, errorMessage) {
            editorSession.handleEditorPersistenceFinished(noteId, text, success);
        }

        ignoreUnknownSignals: true
        target: editorSession.selectionBridge
    }
}
