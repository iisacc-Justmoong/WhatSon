import QtQuick
import WhatSon.App.Internal 1.0

Item {
    id: editorSession
    objectName: "contentsEditorSession"
    visible: false

    property alias agendaBackend: sessionController.agendaBackend
    property alias editorBoundNoteId: sessionController.editorBoundNoteId
    property alias editorBoundNoteDirectoryPath: sessionController.editorBoundNoteDirectoryPath
    property alias editorText: sessionController.editorText
    property alias lastLocalEditTimestampMs: sessionController.lastLocalEditTimestampMs
    property alias localEditorAuthority: sessionController.localEditorAuthority
    property alias pendingBodySave: sessionController.pendingBodySave
    property alias selectionBridge: sessionController.selectionBridge
    property alias syncingEditorTextFromModel: sessionController.syncingEditorTextFromModel
    property alias typingIdleThresholdMs: sessionController.typingIdleThresholdMs

    signal editorTextSynchronized

    function flushPendingEditorText() {
        return sessionController.flushPendingEditorText();
    }

    function isTypingSessionActive() {
        return sessionController.isTypingSessionActive();
    }

    function requestSyncEditorTextFromSelection(noteId, text, bodyNoteId, noteDirectoryPath) {
        return sessionController.requestSyncEditorTextFromSelection(
                    noteId === undefined || noteId === null ? "" : String(noteId),
                    text === undefined || text === null ? "" : String(text),
                    bodyNoteId === undefined || bodyNoteId === null ? "" : String(bodyNoteId),
                    noteDirectoryPath === undefined || noteDirectoryPath === null ? "" : String(noteDirectoryPath));
    }

    function markLocalEditorAuthority() {
        sessionController.markLocalEditorAuthority();
    }

    function scheduleEditorPersistence() {
        return sessionController.scheduleEditorPersistence();
    }

    function persistEditorTextImmediately(text) {
        if (text === undefined || text === null)
            return sessionController.persistEditorTextImmediately();
        return sessionController.persistEditorTextImmediatelyWithText(String(text));
    }

    ContentsEditorSessionController {
        id: sessionController
    }

    Connections {
        function onEditorTextSynchronized() {
            editorSession.editorTextSynchronized();
        }

        target: sessionController
    }
}
