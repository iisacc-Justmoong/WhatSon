import QtQuick

Item {
    id: editorSession

    property string editorBoundNoteId: ""
    property string editorText: ""
    property bool localEditorAuthority: false
    property double lastLocalEditTimestampMs: 0
    property bool pendingBodySave: false
    property int typingIdleThresholdMs: 1000
    property var selectionBridge: null
    property var agendaBackend: null
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
        const rawBodyText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        const bodyText = editorSession.normalizeModifiedEditorText(rawBodyText);
        if (editorSession.selectionBridge.flushEditorTextForNote !== undefined)
            return !!editorSession.selectionBridge.flushEditorTextForNote(noteId, bodyText);
        if (editorSession.selectionBridge.stageEditorTextForIdleSync !== undefined)
            return !!editorSession.selectionBridge.stageEditorTextForIdleSync(noteId, bodyText);
        return false;
    }
    function currentTimestampMs() {
        return Date.now();
    }
    function normalizeAgendaPlaceholderDates(text) {
        const sourceText = text === undefined || text === null ? "" : String(text);
        if (sourceText.length === 0)
            return sourceText;
        if (!editorSession.agendaBackend
                || editorSession.agendaBackend.normalizeAgendaModifiedDate === undefined) {
            return sourceText;
        }
        return String(editorSession.agendaBackend.normalizeAgendaModifiedDate(sourceText));
    }
    function normalizeModifiedEditorText(text) {
        const normalizedText = editorSession.normalizeAgendaPlaceholderDates(text);
        if (editorSession.editorText !== normalizedText)
            editorSession.editorText = normalizedText;
        return normalizedText;
    }
    function isTypingSessionActive() {
        if (!editorSession.localEditorAuthority)
            return false;
        const thresholdMs = Math.max(0, Number(editorSession.typingIdleThresholdMs) || 0);
        if (thresholdMs <= 0)
            return true;
        const lastEditTimestampMs = Math.max(0, Number(editorSession.lastLocalEditTimestampMs) || 0);
        const elapsedMs = editorSession.currentTimestampMs() - lastEditTimestampMs;
        if (!isFinite(elapsedMs) || elapsedMs < 0)
            return true;
        return elapsedMs < thresholdMs;
    }
    function requestSyncEditorTextFromSelection(noteId, text) {
        const nextNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const nextText = text === undefined || text === null ? "" : String(text);
        const currentNoteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null ? "" : String(editorSession.editorBoundNoteId);
        const currentText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        if (currentNoteId !== nextNoteId && editorSession.pendingBodySave) {
            editorSession.scheduleEditorPersistence();
        }
        if (currentNoteId === nextNoteId && currentText === nextText)
            return false;
        if (currentNoteId === nextNoteId && !editorSession.shouldAcceptModelBodyText(nextNoteId, nextText)) {
            editorSession.scheduleEditorPersistence();
            return false;
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
        editorSession.lastLocalEditTimestampMs = editorSession.currentTimestampMs();
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
        const rawBodyText = editorSession.editorText === undefined || editorSession.editorText === null ? "" : String(editorSession.editorText);
        const bodyText = editorSession.normalizeModifiedEditorText(rawBodyText);
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
        if (editorSession.isTypingSessionActive())
            return false;
        return !editorSession.pendingBodySave;
    }
    function syncEditorTextFromSelection(noteId, text) {
        const nextNoteId = noteId === undefined || noteId === null ? "" : String(noteId);
        const nextText = text === undefined || text === null ? "" : String(text);
        const noteChanged = editorSession.editorBoundNoteId !== nextNoteId;
        const textChanged = editorSession.editorText !== nextText;
        if (!noteChanged && !textChanged)
            return;
        if (noteChanged || textChanged)
            editorSession.pendingBodySave = false;
        editorSession.editorBoundNoteId = nextNoteId;
        if (noteChanged) {
            editorSession.localEditorAuthority = false;
            editorSession.lastLocalEditTimestampMs = 0;
        }
        if (textChanged)
            editorSession.syncingEditorTextFromModel = true;
        if (textChanged)
            editorSession.editorText = nextText;
        if (textChanged)
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
