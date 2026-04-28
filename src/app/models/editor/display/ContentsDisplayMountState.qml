pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: state

    property var editorSession: null
    property var selectionBridge: null
    property var noteBodyMountCoordinator: null

    readonly property string selectedNoteBodyText: selectionBridge && selectionBridge.selectedNoteBodyText !== undefined && selectionBridge.selectedNoteBodyText !== null ? String(selectionBridge.selectedNoteBodyText) : ""
    readonly property string selectedNoteBodyNoteId: selectionBridge && selectionBridge.selectedNoteBodyNoteId !== undefined && selectionBridge.selectedNoteBodyNoteId !== null ? String(selectionBridge.selectedNoteBodyNoteId) : ""
    readonly property bool selectedNoteBodyResolved: !!(selectionBridge && selectionBridge.selectedNoteBodyResolved)
    readonly property bool selectedNoteBodyLoading: !!(selectionBridge && selectionBridge.selectedNoteBodyLoading)
    readonly property string selectedNoteId: selectionBridge && selectionBridge.selectedNoteId !== undefined && selectionBridge.selectedNoteId !== null ? String(selectionBridge.selectedNoteId) : ""
    readonly property string selectedNoteDirectoryPath: selectionBridge && selectionBridge.selectedNoteDirectoryPath !== undefined && selectionBridge.selectedNoteDirectoryPath !== null ? String(selectionBridge.selectedNoteDirectoryPath) : ""
    readonly property int visibleNoteCount: selectionBridge && selectionBridge.visibleNoteCount !== undefined ? Math.max(0, Number(selectionBridge.visibleNoteCount) || 0) : 0
    readonly property bool noteCountContractAvailable: !!(selectionBridge && selectionBridge.noteCountContractAvailable)
    readonly property bool noteSelectionContractAvailable: !!(selectionBridge && selectionBridge.noteSelectionContractAvailable)
    readonly property bool contentPersistenceContractAvailable: !!(selectionBridge && selectionBridge.contentPersistenceContractAvailable)

    readonly property bool editorSessionBoundToSelectedNote: {
        if (!editorSession)
            return false;
        const boundNoteId = editorSession.editorBoundNoteId === undefined || editorSession.editorBoundNoteId === null
                ? ""
                : String(editorSession.editorBoundNoteId);
        if (boundNoteId !== state.selectedNoteId)
            return false;
        if (state.selectedNoteDirectoryPath === "")
            return true;
        const boundDirectoryPath = editorSession.editorBoundNoteDirectoryPath === undefined || editorSession.editorBoundNoteDirectoryPath === null
                ? ""
                : String(editorSession.editorBoundNoteDirectoryPath);
        return boundDirectoryPath === state.selectedNoteDirectoryPath;
    }

    readonly property bool noteDocumentMountDecisionClean: !!(noteBodyMountCoordinator && noteBodyMountCoordinator.mountDecisionClean)
    readonly property bool noteDocumentMountPending: !!(noteBodyMountCoordinator && noteBodyMountCoordinator.mountPending) && !state.noteDocumentMountDecisionClean
    readonly property bool noteDocumentParseMounted: !!(noteBodyMountCoordinator && noteBodyMountCoordinator.parseMounted)
    readonly property bool noteDocumentSourceMounted: !!(noteBodyMountCoordinator && noteBodyMountCoordinator.sourceMounted)
    readonly property bool noteDocumentMounted: !!(noteBodyMountCoordinator && noteBodyMountCoordinator.noteMounted)
    readonly property bool noteDocumentMountFailureVisible: !!(noteBodyMountCoordinator && noteBodyMountCoordinator.mountFailed)
    readonly property string noteDocumentMountFailureReason: noteBodyMountCoordinator && noteBodyMountCoordinator.mountFailureReason !== undefined && noteBodyMountCoordinator.mountFailureReason !== null ? String(noteBodyMountCoordinator.mountFailureReason) : ""
    readonly property string noteDocumentMountFailureMessage: noteBodyMountCoordinator && noteBodyMountCoordinator.mountFailureMessage !== undefined && noteBodyMountCoordinator.mountFailureMessage !== null ? String(noteBodyMountCoordinator.mountFailureMessage) : ""
    readonly property string noteDocumentExceptionReason: noteBodyMountCoordinator && noteBodyMountCoordinator.exceptionReason !== undefined && noteBodyMountCoordinator.exceptionReason !== null ? String(noteBodyMountCoordinator.exceptionReason) : ""
    readonly property string noteDocumentExceptionTitle: noteBodyMountCoordinator && noteBodyMountCoordinator.exceptionTitle !== undefined && noteBodyMountCoordinator.exceptionTitle !== null ? String(noteBodyMountCoordinator.exceptionTitle) : ""
    readonly property string noteDocumentExceptionMessage: noteBodyMountCoordinator && noteBodyMountCoordinator.exceptionMessage !== undefined && noteBodyMountCoordinator.exceptionMessage !== null ? String(noteBodyMountCoordinator.exceptionMessage) : ""
    readonly property bool noteDocumentExceptionVisible: !!(noteBodyMountCoordinator && noteBodyMountCoordinator.exceptionVisible)
}
