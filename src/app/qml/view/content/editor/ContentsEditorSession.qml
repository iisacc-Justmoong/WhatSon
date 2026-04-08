import QtQuick

Item {
    id: editorSession

    property bool bodySaveInFlight: false
    property string editorBoundNoteId: ""
    property string editorText: ""
    property string deferredSelectionNoteId: ""
    property string deferredSelectionText: ""
    property bool deferredSelectionPending: false
    property string lastQueuedBodySaveNoteId: ""
    property string lastQueuedBodySaveText: ""
    property bool localEditorAuthority: false
    property bool pendingBodySave: false
    readonly property bool persistenceAvailable: selectionBridge && selectionBridge.contentPersistenceContractAvailable !== undefined ? !!selectionBridge.contentPersistenceContractAvailable : false
    property int saveDebounceMs: 120
    property var selectionBridge: null
    property bool syncingEditorTextFromModel: false

    signal editorTextSynchronized

    function acknowledgeQueuedEditorPersistence(noteId, text) {
        editorSession.bodySaveInFlight = true;
        editorSession.lastQueuedBodySaveNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        editorSession.lastQueuedBodySaveText = text === undefined || text === null ? "" : String(text);
        editorSession.pendingBodySave = false;
        bodySaveTimer.stop();
    }
    function acknowledgeSuccessfulEditorPersistence(noteId, text) {
        const completedNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const completedText = text === undefined || text === null ? "" : String(text);
        const synchronousCompletion = !editorSession.bodySaveInFlight
                && editorSession.lastQueuedBodySaveNoteId.length === 0
                && editorSession.lastQueuedBodySaveText.length === 0;
        const trackedCompletion = editorSession.lastQueuedBodySaveNoteId === completedNoteId
                && editorSession.lastQueuedBodySaveText === completedText;
        if (completedNoteId.length === 0
                || trackedCompletion) {
            editorSession.bodySaveInFlight = false;
        }
        if (trackedCompletion) {
            editorSession.lastQueuedBodySaveNoteId = "";
            editorSession.lastQueuedBodySaveText = "";
        }
        if (synchronousCompletion) {
            editorSession.pendingBodySave = false;
            bodySaveTimer.stop();
            return;
        }
        if (!editorSession.pendingBodySave)
            bodySaveTimer.stop();
    }
    function handleEditorPersistenceFinished(noteId, text, success) {
        const completedNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const completedText = text === undefined || text === null ? "" : String(text);
        if (success) {
            editorSession.acknowledgeSuccessfulEditorPersistence(completedNoteId, completedText);
            return;
        }

        if (editorSession.lastQueuedBodySaveNoteId === completedNoteId
                && editorSession.lastQueuedBodySaveText === completedText) {
            editorSession.bodySaveInFlight = false;
            editorSession.lastQueuedBodySaveNoteId = "";
            editorSession.lastQueuedBodySaveText = "";
        }
        if (editorSession.editorBoundNoteId === completedNoteId
                && editorSession.editorText === completedText) {
            editorSession.pendingBodySave = true;
            if (!bodySaveTimer.running)
                bodySaveTimer.start();
        }
    }
    function clearDeferredSelectionSync() {
        editorSession.deferredSelectionPending = false;
        editorSession.deferredSelectionNoteId = "";
        editorSession.deferredSelectionText = "";
    }
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
        const bodyText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        const saved = editorSession.selectionBridge.persistEditorTextForNote(noteId, bodyText);
        if (saved) {
            const directPersistenceContractAvailable = editorSession.selectionBridge.directPersistenceContractAvailable !== undefined
                    ? !!editorSession.selectionBridge.directPersistenceContractAvailable
                    : false;
            if (directPersistenceContractAvailable)
                editorSession.acknowledgeQueuedEditorPersistence(noteId, bodyText);
            else
                editorSession.acknowledgeSuccessfulEditorPersistence(noteId, bodyText);
            return true;
        }
        return false;
    }
    function requestSyncEditorTextFromSelection(noteId, text) {
        const nextNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const nextText = text === undefined || text === null ? "" : String(text);
        const currentNoteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null ? "" : String(editorSession.editorBoundNoteId);
        if (currentNoteId !== nextNoteId && editorSession.pendingBodySave) {
            if (!editorSession.flushPendingEditorText()) {
                editorSession.deferredSelectionPending = true;
                editorSession.deferredSelectionNoteId = nextNoteId;
                editorSession.deferredSelectionText = nextText;
                if (!bodySaveTimer.running)
                    bodySaveTimer.start();
                return false;
            }
            if (editorSession.editorBoundNoteId === nextNoteId && editorSession.editorText === nextText)
                return true;
        }
        editorSession.clearDeferredSelectionSync();
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
        if (noteChanged || !textChanged)
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
    Connections {
        function onEditorTextPersistenceFinished(noteId, text, success, errorMessage) {
            editorSession.handleEditorPersistenceFinished(noteId, text, success);
        }

        ignoreUnknownSignals: true
        target: editorSession.selectionBridge
    }
}
