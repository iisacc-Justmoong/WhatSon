import QtQuick

Item {
    id: editorSession

    property string editorBoundNoteId: ""
    property string editorText: ""
    property bool pendingBodySave: false
    readonly property bool persistenceAvailable: selectionBridge && selectionBridge.contentPersistenceContractAvailable !== undefined ? !!selectionBridge.contentPersistenceContractAvailable : false
    property int saveDebounceMs: 300
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
            bodySaveTimer.restart();
            return false;
        }
        const saved = editorSession.selectionBridge.persistEditorTextForNote(noteId, editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText));
        if (saved) {
            editorSession.pendingBodySave = false;
            bodySaveTimer.stop();
            return true;
        }
        bodySaveTimer.restart();
        return false;
    }
    function releaseSyncGuard() {
        Qt.callLater(function () {
            editorSession.syncingEditorTextFromModel = false;
        });
    }
    function scheduleEditorPersistence() {
        editorSession.pendingBodySave = true;
        bodySaveTimer.restart();
    }
    function syncEditorTextFromSelection(noteId, text) {
        const nextNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const nextText = text === undefined || text === null ? "" : String(text);
        bodySaveTimer.stop();
        editorSession.pendingBodySave = false;
        editorSession.editorBoundNoteId = nextNoteId;
        editorSession.syncingEditorTextFromModel = true;
        if (editorSession.editorText !== nextText)
            editorSession.editorText = nextText;
        editorSession.releaseSyncGuard();
        editorSession.editorTextSynchronized();
    }

    visible: false

    Timer {
        id: bodySaveTimer

        interval: editorSession.saveDebounceMs
        repeat: false

        onTriggered: editorSession.flushPendingEditorText()
    }
}
