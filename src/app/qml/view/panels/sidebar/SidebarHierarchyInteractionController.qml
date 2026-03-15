import QtQuick

QtObject {
    id: interactionController

    property int activationRequestRevision: 0
    property int editingIndex: -1
    property string editingText: ""
    property bool folderDropAsChild: true
    property bool folderDropBefore: false
    property int folderDropTargetIndex: -1
    property var folderRepeater: null
    property var hierarchyList: null
    property var hierarchyViewModel: null
    property var hierarchyViewport: null
    property int noteDropTargetIndex: -1
    property int pendingActivationIndex: -1
    property var requestViewHook: null
    property bool rootDropHighlighted: false
    property int selectedFolderIndex: -1
    property var viewRoot: null

    function activateHierarchyDelegate(delegate, index) {
        if (!delegate || index < 0)
            return;
        if (interactionController.editingIndex >= 0 && interactionController.editingIndex !== index)
            interactionController.commitRename();
        interactionController.pendingActivationIndex = index;
        interactionController.activationRequestRevision += 1;
        const requestRevision = interactionController.activationRequestRevision;
        if (interactionController.hierarchyList && interactionController.hierarchyList.requestActivate !== undefined)
            interactionController.hierarchyList.requestActivate(delegate, true);
        if (interactionController.viewRoot && interactionController.viewRoot.forceActiveFocus !== undefined)
            interactionController.viewRoot.forceActiveFocus();
        Qt.callLater(function () {
            if (interactionController.activationRequestRevision !== requestRevision)
                return;
            if (interactionController.pendingActivationIndex !== index)
                return;
            if (interactionController.hierarchyViewModel && interactionController.hierarchyViewModel.setSelectedIndex !== undefined && interactionController.selectedFolderIndex !== index)
                interactionController.hierarchyViewModel.setSelectedIndex(index);
            var replayDelegate = delegate;
            if (interactionController.folderRepeater && interactionController.folderRepeater.itemAt !== undefined) {
                const candidate = interactionController.folderRepeater.itemAt(index);
                if (candidate)
                    replayDelegate = candidate;
            }
            if (replayDelegate && replayDelegate.visible !== false && replayDelegate.height > 0 && interactionController.hierarchyList && interactionController.hierarchyList.requestActivate !== undefined)
                interactionController.hierarchyList.requestActivate(replayDelegate, true);
            if (interactionController.viewRoot && interactionController.viewRoot.forceActiveFocus !== undefined)
                interactionController.viewRoot.forceActiveFocus();
            interactionController.pendingActivationIndex = -1;
        });
    }
    function activateSelectedHierarchyItem(focusView) {
        if (interactionController.selectedFolderIndex < 0 || !interactionController.folderRepeater)
            return;
        var delegate = interactionController.folderRepeater.itemAt(interactionController.selectedFolderIndex);
        if (!delegate || delegate.visible === false || delegate.height <= 0)
            return;
        if (interactionController.hierarchyList && interactionController.hierarchyList.requestActivate !== undefined)
            interactionController.hierarchyList.requestActivate(delegate, true);
        if (focusView && interactionController.viewRoot && interactionController.viewRoot.forceActiveFocus !== undefined)
            interactionController.viewRoot.forceActiveFocus();
        if (!interactionController.hierarchyViewport)
            return;
        var itemTop = delegate.y;
        var itemBottom = itemTop + delegate.height;
        if (itemTop < interactionController.hierarchyViewport.contentY)
            interactionController.hierarchyViewport.contentY = itemTop;
        else if (itemBottom > interactionController.hierarchyViewport.contentY + interactionController.hierarchyViewport.height)
            interactionController.hierarchyViewport.contentY = itemBottom - interactionController.hierarchyViewport.height;
    }
    function assignNoteToFolder(index, noteId) {
        if (index < 0 || !interactionController.hierarchyViewModel)
            return false;
        if (noteId === undefined || noteId === null || String(noteId).trim().length === 0)
            return false;
        if (interactionController.hierarchyViewModel.assignNoteToFolder === undefined)
            return false;
        return interactionController.hierarchyViewModel.assignNoteToFolder(index, noteId);
    }
    function beginRename(index, currentLabel) {
        if (index < 0)
            return;
        if (!interactionController.canRenameAtIndex(index))
            return;
        if (interactionController.editingIndex >= 0 && interactionController.editingIndex !== index)
            interactionController.commitRename();
        if (interactionController.hierarchyViewModel)
            interactionController.hierarchyViewModel.setSelectedIndex(index);
        interactionController.editingIndex = index;
        interactionController.editingText = currentLabel;
    }
    function canAcceptFolderDrop(sourceIndex, targetIndex, asChild) {
        if (sourceIndex < 0 || targetIndex < 0 || !interactionController.hierarchyViewModel)
            return false;
        if (interactionController.hierarchyViewModel.canAcceptFolderDrop === undefined)
            return false;
        return interactionController.hierarchyViewModel.canAcceptFolderDrop(sourceIndex, targetIndex, asChild);
    }
    function canAcceptFolderDropBefore(sourceIndex, targetIndex) {
        if (sourceIndex < 0 || targetIndex < 0 || !interactionController.hierarchyViewModel)
            return false;
        if (interactionController.hierarchyViewModel.canAcceptFolderDropBefore === undefined)
            return false;
        return interactionController.hierarchyViewModel.canAcceptFolderDropBefore(sourceIndex, targetIndex);
    }
    function canAcceptNoteDrop(index, noteId) {
        if (index < 0 || !interactionController.hierarchyViewModel)
            return false;
        if (noteId === undefined || noteId === null || String(noteId).trim().length === 0)
            return false;
        if (interactionController.hierarchyViewModel.canAcceptNoteDrop === undefined)
            return false;
        return interactionController.hierarchyViewModel.canAcceptNoteDrop(index, noteId);
    }
    function canMoveFolder(index) {
        if (index < 0 || !interactionController.hierarchyViewModel)
            return false;
        if (interactionController.hierarchyViewModel.canMoveFolder === undefined)
            return false;
        return interactionController.hierarchyViewModel.canMoveFolder(index);
    }
    function canMoveFolderToRoot(sourceIndex) {
        if (sourceIndex < 0 || !interactionController.hierarchyViewModel)
            return false;
        if (interactionController.hierarchyViewModel.canMoveFolderToRoot === undefined)
            return false;
        return interactionController.hierarchyViewModel.canMoveFolderToRoot(sourceIndex);
    }
    function canRenameAtIndex(index) {
        if (index < 0 || !interactionController.hierarchyViewModel)
            return false;
        if (interactionController.hierarchyViewModel.canRenameItem !== undefined)
            return interactionController.hierarchyViewModel.canRenameItem(index);
        return false;
    }
    function cancelRename() {
        interactionController.editingIndex = -1;
        interactionController.editingText = "";
    }
    function clearHierarchySelection() {
        if (interactionController.editingIndex >= 0)
            interactionController.commitRename();
        interactionController.pendingActivationIndex = -1;
        interactionController.activationRequestRevision += 1;
        interactionController.resetDropTargets();
        if (interactionController.hierarchyList && interactionController.hierarchyList.clearActiveItem !== undefined)
            interactionController.hierarchyList.clearActiveItem();
        if (interactionController.hierarchyViewModel && interactionController.hierarchyViewModel.setSelectedIndex !== undefined)
            interactionController.hierarchyViewModel.setSelectedIndex(-1);
    }
    function commitRename() {
        if (interactionController.editingIndex < 0)
            return;
        if (interactionController.hierarchyViewModel && interactionController.canRenameAtIndex(interactionController.editingIndex))
            interactionController.hierarchyViewModel.renameItem(interactionController.editingIndex, interactionController.editingText);
        interactionController.editingIndex = -1;
        interactionController.editingText = "";
    }
    function draggedFolderIndex(event) {
        if (!event || !event.source || event.source.sourceIndex === undefined || event.source.sourceIndex === null)
            return -1;
        var parsed = Number(event.source.sourceIndex);
        if (!isFinite(parsed))
            return -1;
        return Math.floor(parsed);
    }
    function draggedNoteId(event) {
        if (!event || !event.source || event.source.noteId === undefined || event.source.noteId === null)
            return "";
        return String(event.source.noteId).trim();
    }
    function moveFolder(sourceIndex, targetIndex, asChild) {
        if (!interactionController.hierarchyViewModel || interactionController.hierarchyViewModel.moveFolder === undefined)
            return false;
        return interactionController.hierarchyViewModel.moveFolder(sourceIndex, targetIndex, asChild);
    }
    function moveFolderBefore(sourceIndex, targetIndex) {
        if (!interactionController.hierarchyViewModel || interactionController.hierarchyViewModel.moveFolderBefore === undefined)
            return false;
        return interactionController.hierarchyViewModel.moveFolderBefore(sourceIndex, targetIndex);
    }
    function moveFolderToRoot(sourceIndex) {
        if (!interactionController.hierarchyViewModel || interactionController.hierarchyViewModel.moveFolderToRoot === undefined)
            return false;
        return interactionController.hierarchyViewModel.moveFolderToRoot(sourceIndex);
    }
    function notifyAcceptedInteraction(reason) {
        if (interactionController.requestViewHook)
            interactionController.requestViewHook(reason);
    }
    function resetDropTargets() {
        interactionController.noteDropTargetIndex = -1;
        interactionController.folderDropTargetIndex = -1;
        interactionController.folderDropAsChild = true;
        interactionController.folderDropBefore = false;
        interactionController.rootDropHighlighted = false;
    }
}
