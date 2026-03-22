import QtQuick

Item {
    id: editorSession

    property string editorBoundNoteId: ""
    property string editorText: ""
    property bool localEditorAuthority: false
    property bool pendingBodySave: false
    readonly property bool persistenceAvailable: selectionBridge && selectionBridge.contentPersistenceContractAvailable !== undefined ? !!selectionBridge.contentPersistenceContractAvailable : false
    property int saveDebounceMs: 120
    property var selectionBridge: null
    property bool syncingEditorTextFromModel: false

    signal editorTextSynchronized

    function flushPendingEditorText() {
        if (!editorSession.pendingBodySave)
            return true;
        const noteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null ? "" : String(editorSession.editorBoundNoteId).trim();
        if (noteId.length === 0) {
            editorSession.pendingBodySave = false;
            bodySaveTimer.stop();
            return false;
        }
        if (!editorSession.selectionBridge || editorSession.selectionBridge.persistEditorTextForNote === undefined || !editorSession.persistenceAvailable) {
            return false;
        }
        const saved = editorSession.selectionBridge.persistEditorTextForNote(noteId, editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText));
        if (saved) {
            editorSession.pendingBodySave = false;
            bodySaveTimer.stop();
            return true;
        }
        return false;
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
        editorSession.pendingBodySave = true;
        if (!bodySaveTimer.running)
            bodySaveTimer.start();
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
        bodySaveTimer.stop();
        editorSession.pendingBodySave = false;
        editorSession.editorBoundNoteId = nextNoteId;
        if (noteChanged || textChanged)
            editorSession.localEditorAuthority = false;
        editorSession.syncingEditorTextFromModel = true;
        if (textChanged)
            editorSession.editorText = nextText;
        editorSession.releaseSyncGuard();
        editorSession.editorTextSynchronized();
    }

    visible: false

    Timer {
        id: bodySaveTimer

        interval: editorSession.saveDebounceMs
        repeat: true

        onTriggered: {
            if (!editorSession.pendingBodySave) {
                stop();
                return;
            }
            if (editorSession.flushPendingEditorText() || !editorSession.pendingBodySave)
                stop();
        }
    }
}
