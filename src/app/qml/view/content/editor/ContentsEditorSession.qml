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
    property var selectionBridge: null
    property bool syncingEditorTextFromModel: false

    signal editorTextSynchronized

    function applyDeferredSelectionIfReady() {
        if (!editorSession.deferredSelectionPending || editorSession.pendingBodySave || editorSession.bodySaveInFlight)
            return false;
        const deferredNoteId = editorSession.deferredSelectionNoteId;
        const deferredText = editorSession.deferredSelectionText;
        editorSession.clearDeferredSelectionSync();
        editorSession.syncEditorTextFromSelection(deferredNoteId, deferredText);
        return true;
    }
    function acknowledgeQueuedEditorPersistence(noteId, text) {
        editorSession.bodySaveInFlight = true;
        editorSession.lastQueuedBodySaveNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        editorSession.lastQueuedBodySaveText = text === undefined || text === null ? "" : String(text);
    }
    function acknowledgeSuccessfulEditorPersistence(noteId, text) {
        const completedNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const completedText = text === undefined || text === null ? "" : String(text);
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
        const currentNoteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null ? "" : String(editorSession.editorBoundNoteId);
        const currentText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        if (currentNoteId === completedNoteId && currentText === completedText) {
            editorSession.pendingBodySave = false;
        } else if (trackedCompletion && currentNoteId === completedNoteId) {
            editorSession.pendingBodySave = currentText !== completedText;
        }
        editorSession.applyDeferredSelectionIfReady();
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
            editorSession.scheduleEditorPersistence();
        }
        editorSession.applyDeferredSelectionIfReady();
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
            return false;
        }
        if (!editorSession.selectionBridge
                || editorSession.selectionBridge.flushEditorTextForNote === undefined
                || !editorSession.persistenceAvailable) {
            return false;
        }
        const bodyText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        return !!editorSession.selectionBridge.flushEditorTextForNote(noteId, bodyText);
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
        const noteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null
                ? ""
                : String(editorSession.editorBoundNoteId).trim();
        if (noteId.length === 0) {
            editorSession.pendingBodySave = false;
            return;
        }
        editorSession.pendingBodySave = true;
        if (!editorSession.selectionBridge
                || editorSession.selectionBridge.stageEditorTextForIdleSync === undefined
                || !editorSession.persistenceAvailable) {
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

    Connections {
        function onEditorTextPersistenceQueued(noteId, text) {
            editorSession.acknowledgeQueuedEditorPersistence(noteId, text);
        }

        function onEditorTextPersistenceFinished(noteId, text, success, errorMessage) {
            editorSession.handleEditorPersistenceFinished(noteId, text, success);
        }

        ignoreUnknownSignals: true
        target: editorSession.selectionBridge
    }
}
