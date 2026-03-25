pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

QtObject {
    id: noteCreationCoordinator

    property var activeContentViewModel: null
    property var activeNoteListModel: null
    property var activePageRouter: null
    readonly property bool lvrsViewModelsAvailable: LV.ViewModels !== undefined
    property var noteCreationViewModel: null
    property string pendingCreatedNoteId: ""
    property var windowInteractions: null

    signal openEditorRequested(string noteId, int index)

    function syncNoteCreationViewModel() {
        if (!noteCreationCoordinator.lvrsViewModelsAvailable
                || !noteCreationCoordinator.windowInteractions
                || noteCreationCoordinator.windowInteractions.resolveLibraryNoteCreationViewModel === undefined) {
            noteCreationCoordinator.noteCreationViewModel = null;
            return;
        }

        const resolvedViewModel = noteCreationCoordinator.windowInteractions.resolveLibraryNoteCreationViewModel();
        noteCreationCoordinator.noteCreationViewModel = resolvedViewModel !== undefined ? resolvedViewModel : null;
    }
    function requestCreateNote() {
        if (noteCreationCoordinator.windowInteractions
                && noteCreationCoordinator.windowInteractions.createNoteFromShortcut !== undefined)
            noteCreationCoordinator.windowInteractions.createNoteFromShortcut();
    }
    function routePendingCreatedNoteToEditor() {
        const pendingNoteId = noteCreationCoordinator.pendingCreatedNoteId === undefined
                || noteCreationCoordinator.pendingCreatedNoteId === null
                ? ""
                : String(noteCreationCoordinator.pendingCreatedNoteId).trim();
        if (pendingNoteId.length === 0
                || !noteCreationCoordinator.activeContentViewModel
                || !noteCreationCoordinator.activeNoteListModel
                || !noteCreationCoordinator.activePageRouter)
            return false;
        noteCreationCoordinator.pendingCreatedNoteId = "";
        noteCreationCoordinator.openEditorRequested(pendingNoteId, -1);
        return true;
    }
    function scheduleCreatedNoteEditorRoute(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length === 0)
            return false;
        noteCreationCoordinator.pendingCreatedNoteId = normalizedNoteId;
        Qt.callLater(function () {
            noteCreationCoordinator.routePendingCreatedNoteToEditor();
        });
        return true;
    }

    Component.onCompleted: noteCreationCoordinator.syncNoteCreationViewModel()
    onWindowInteractionsChanged: noteCreationCoordinator.syncNoteCreationViewModel()

    property Connections noteCreationViewModelConnections: Connections {
        target: noteCreationCoordinator.noteCreationViewModel
        ignoreUnknownSignals: true

        function onEmptyNoteCreated(noteId) {
            noteCreationCoordinator.scheduleCreatedNoteEditorRoute(noteId);
        }
    }
}
