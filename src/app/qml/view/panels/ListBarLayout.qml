pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtQuick.Window
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Rectangle {
    id: listBarLayout

    property int activeToolbarIndex: 0
    readonly property var committedNoteEntry: listBarLayout.currentNoteEntryFromModel()
    readonly property string committedNoteId: listBarLayout.currentNoteIdFromModel()
    readonly property int committedNoteIndex: listBarLayout.normalizeCurrentIndex(listBarLayout.currentIndexFromModel())
    property string contextMenuNoteId: ""
    property var contextMenuNoteIds: []
    property int contextMenuNoteIndex: -1
    readonly property int dragCountBadgeHeight: LV.Theme.gap20
    readonly property int dragCountBadgeInset: LV.Theme.gap8
    readonly property int dragCountBadgeMinWidth: dragCountBadgeHeight
    readonly property int dragCountBadgeWidthPadding: LV.Theme.gap10
    readonly property real grabbedNoteOpacity: 0.25
    readonly property bool hasNoteListModel: listBarLayout.noteListModel !== null && listBarLayout.noteListModel !== undefined
    property bool headerVisible: true
    property var hierarchyController: null
    property color hintColor: LV.Theme.descriptionColor
    readonly property int mobileNoteDragHoldInterval: 1000
    readonly property var noteContextMenuItems: [
        {
            "label": listBarLayout.noteContextMenuSelectionCount > 1 ? "Delete notes" : "Delete note",
            "enabled": listBarLayout.noteDeletionContractAvailable && listBarLayout.noteContextMenuNoteAvailable,
            "eventName": "note.delete"
        },
        {
            "label": listBarLayout.noteContextMenuSelectionCount > 1 ? "Clear all folders for notes" : "Clear all folders",
            "enabled": listBarLayout.noteFolderClearContractAvailable && listBarLayout.noteContextMenuNoteAvailable,
            "eventName": "note.clearAllFolders"
        }
    ]
    readonly property bool noteContextMenuNoteAvailable: listBarLayout.contextMenuNoteIds.length > 0
    readonly property int noteContextMenuSelectionCount: listBarLayout.contextMenuNoteIds.length
    readonly property bool noteDeletionBatchContractAvailable: listBarLayout.noteDeletionController !== null && listBarLayout.noteDeletionController !== undefined && listBarLayout.noteDeletionController.deleteNotesByIds !== undefined
    readonly property bool noteDeletionContractAvailable: listBarLayout.noteDeletionHandlerAvailable && (listBarLayout.resolvedSelectedNoteIds.length > 0 || noteDeletionBridge.focusedNoteAvailable)
    readonly property bool noteDeletionHandlerAvailable: listBarLayout.noteDeletionBatchContractAvailable || listBarLayout.noteDeletionSingleContractAvailable
    readonly property bool noteDeletionSingleContractAvailable: noteDeletionBridge.deleteContractAvailable || (listBarLayout.noteDeletionController !== null && listBarLayout.noteDeletionController !== undefined && listBarLayout.noteDeletionController.deleteNoteById !== undefined)
    property var noteDeletionController: null
    property bool noteDragActive: false
    property bool noteDragCanceled: false
    property var noteDropTarget: null
    readonly property bool noteFolderClearContractAvailable: listBarLayout.noteDeletionController !== null && listBarLayout.noteDeletionController !== undefined && (listBarLayout.noteDeletionController.clearNoteFoldersByIds !== undefined || listBarLayout.noteDeletionController.clearNoteFoldersById !== undefined)
    readonly property bool noteListCurrentIndexContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.resolvedNoteListModel.currentIndex !== undefined || listBarLayout.resolvedNoteListModel.setCurrentIndex !== undefined)
    property bool isMobilePlatform: false
    readonly property bool noteListKineticViewportEnabled: LV.Theme.mobileTarget || listBarLayout.isMobilePlatform
    readonly property int noteListBoundsBehavior: listBarLayout.noteListKineticViewportEnabled ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
    readonly property int noteListBoundsMovement: listBarLayout.noteListKineticViewportEnabled ? Flickable.FollowBoundsBehavior : Flickable.StopAtBounds
    readonly property int noteListDesktopFlickDeceleration: 1000000
    readonly property int noteListFlickDeceleration: listBarLayout.noteListKineticViewportEnabled ? Math.max(0, Math.round(LV.Theme.scaleMetric(2800))) : listBarLayout.noteListDesktopFlickDeceleration
    readonly property bool noteListMode: listBarLayout.hasNoteListModel
    readonly property int noteListMaximumFlickVelocity: listBarLayout.noteListKineticViewportEnabled ? Math.max(0, Math.round(LV.Theme.scaleMetric(12000))) : listBarLayout.noteListScrollTick
    property var noteListModel: null
    readonly property int noteListViewportInset: LV.Theme.gap2
    property int noteListModelTransitionRevision: 0
    readonly property int noteListScrollTick: LV.Theme.gap2
    readonly property bool noteListSearchContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.resolvedNoteListModel.searchText !== undefined || listBarLayout.resolvedNoteListModel.setSearchText !== undefined)
    property bool noteListViewportRestorePending: false
    property alias noteSelectionAnchorIndex: noteSelectionController.selectionAnchorIndex
    property color panelColor: "transparent"
    property var panelControllerRegistry: null
    readonly property var panelController: null
    property real preservedNoteListContentY: 0
    property int pressedNoteIndex: -1
    readonly property var resolvedNoteListModel: listBarLayout.noteListModel
    readonly property var resolvedSelectedNoteIds: listBarLayout.selectedNoteIdsFromIndices(listBarLayout.selectedNoteIndices)
    readonly property bool resourceListMode: listBarLayout.noteListMode && listBarLayout.resolvedNoteListModel !== null && listBarLayout.resolvedNoteListModel !== undefined && listBarLayout.resolvedNoteListModel.currentResourceEntry !== undefined
    property string searchText: ""
    property alias selectedNoteIndices: noteSelectionController.selectedIndices
    property bool syncingCurrentIndexFromModel: false
    property bool syncingNoteListViewport: false
    readonly property int topToolbarHeight: LV.Theme.gap24
    readonly property bool useInternalNoteDrag: !LV.Theme.mobileTarget

    signal noteActivated(int index, string noteId)
    signal viewHookRequested

    function activateNoteIndex(index, noteId) {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return;
        const normalizedIndex = Number(index);
        if (!isFinite(normalizedIndex))
            return;
        const targetIndex = Math.max(-1, Math.floor(normalizedIndex));
        const normalizedNoteId = noteId !== undefined && noteId !== null ? String(noteId).trim() : "";
        noteDeletionBridge.focusedNoteId = normalizedNoteId;
        noteSelectionState.pendingIndex = targetIndex;
        noteSelectionState.requestRevision += 1;
        const requestRevision = noteSelectionState.requestRevision;
        console.log("[whatson:qml][ListBarLayout][activateNoteIndex] targetIndex=" + targetIndex + " noteId=" + normalizedNoteId + " currentIndex(before)=" + noteListView.currentIndex + " modelCurrentIndex(before)=" + listBarLayout.currentIndexFromModel());
        if (noteListView.currentIndex !== targetIndex)
            noteListView.currentIndex = targetIndex;
        listBarLayout.pushCurrentIndexToModel(targetIndex);
        noteListView.forceActiveFocus();
        listBarLayout.noteActivated(targetIndex, normalizedNoteId);
        Qt.callLater(function () {
            if (noteSelectionState.requestRevision !== requestRevision)
                return;
            if (noteSelectionState.pendingIndex !== targetIndex)
                return;
            if (noteListView.currentIndex !== targetIndex)
                noteListView.currentIndex = targetIndex;
            if (listBarLayout.currentIndexFromModel() !== targetIndex)
                listBarLayout.pushCurrentIndexToModel(targetIndex);
            if (normalizedNoteId.length > 0)
                noteDeletionBridge.focusedNoteId = normalizedNoteId;
            noteSelectionState.pendingIndex = -1;
        });
    }
    function applyNoteListViewportStep(contentY) {
        const targetY = listBarLayout.noteListViewportTargetY(contentY);
        const previousY = Number(noteListView.contentY) || 0;
        if (Math.abs(targetY - previousY) <= 0.001)
            return;
        listBarLayout.syncingNoteListViewport = true;
        noteListView.contentY = targetY;
        listBarLayout.syncingNoteListViewport = false;
    }
    function applySearchTextToModel() {
        if (!listBarLayout.noteListMode || !listBarLayout.noteListSearchContractAvailable)
            return;

        if (listBarLayout.resolvedNoteListModel.searchText !== undefined) {
            listBarLayout.resolvedNoteListModel.searchText = listBarLayout.searchText;
            return;
        }
        if (listBarLayout.resolvedNoteListModel.setSearchText !== undefined)
            listBarLayout.resolvedNoteListModel.setSearchText(listBarLayout.searchText);
    }
    function beginNoteDragPreview(delegateItem, hotSpotX, hotSpotY) {
        if (!delegateItem)
            return;
        noteDragPreviewState.delegateItem = delegateItem;
        noteDragPreviewState.hotSpotX = Math.max(0, Number(hotSpotX) || 0);
        noteDragPreviewState.hotSpotY = Math.max(0, Number(hotSpotY) || 0);
        noteDragPreviewState.noteIds = listBarLayout.selectedNoteIdsForDelegateAction(delegateItem.index, delegateItem.noteId);
        listBarLayout.updateNoteDragPreviewPosition(delegateItem, hotSpotX, hotSpotY);
    }
    function captureNoteListViewport() {
        if (!listBarLayout.noteListMode)
            return;
        listBarLayout.preservedNoteListContentY = listBarLayout.noteListViewportTargetY(noteListView.contentY);
        listBarLayout.noteListViewportRestorePending = true;
    }
    function clampNoteListContentY(value) {
        const numericValue = Number(value);
        if (!isFinite(numericValue))
            return 0;
        return Math.max(0, Math.min(listBarLayout.noteListMaxContentY(), numericValue));
    }
    function clearContextMenuNoteFolders() {
        if (!listBarLayout.noteFolderClearContractAvailable || !listBarLayout.noteContextMenuNoteAvailable)
            return false;
        const targetNoteIds = listBarLayout.uniqueTrimmedStringList(listBarLayout.contextMenuNoteIds);
        if (targetNoteIds.length === 0)
            return false;
        noteDeletionBridge.focusedNoteId = targetNoteIds[0];
        const cleared = listBarLayout.clearFoldersForNoteIds(targetNoteIds);
        if (cleared)
            noteListView.forceActiveFocus();
        return cleared;
    }
    function clearFoldersForNoteIds(noteIds) {
        const normalizedNoteIds = listBarLayout.uniqueTrimmedStringList(noteIds);
        if (normalizedNoteIds.length === 0 || !listBarLayout.noteDeletionController)
            return false;

        if (listBarLayout.noteDeletionController.clearNoteFoldersByIds !== undefined)
            return Boolean(listBarLayout.noteDeletionController.clearNoteFoldersByIds(normalizedNoteIds));

        if (listBarLayout.noteDeletionController.clearNoteFoldersById === undefined)
            return false;

        let clearedAny = false;
        for (let index = 0; index < normalizedNoteIds.length; ++index) {
            if (listBarLayout.noteDeletionController.clearNoteFoldersById(normalizedNoteIds[index]))
                clearedAny = true;
        }
        return clearedAny;
    }
    function clearInternalNoteDropPreview() {
        if (!listBarLayout.noteDropTarget || listBarLayout.noteDropTarget.clearNoteDropPreview === undefined)
            return;
        listBarLayout.noteDropTarget.clearNoteDropPreview();
    }
    function clearNoteContextMenuState() {
        const normalizedNoteId = String(listBarLayout.contextMenuNoteId).trim();
        if (normalizedNoteId.length > 0 && noteDeletionBridge.focusedNoteId === normalizedNoteId)
            noteDeletionBridge.focusedNoteId = "";
        listBarLayout.contextMenuNoteId = "";
        listBarLayout.contextMenuNoteIndex = -1;
        listBarLayout.contextMenuNoteIds = [];
    }
    function clearNoteDragPreview(delegateItem) {
        if (delegateItem && noteDragPreviewState.delegateItem !== delegateItem)
            return;
        noteDragPreviewState.delegateItem = null;
        noteDragPreviewState.hotSpotX = 0;
        noteDragPreviewState.hotSpotY = 0;
        noteDragPreviewState.noteIds = [];
        noteDragPreviewState.x = 0;
        noteDragPreviewState.y = 0;
    }
    function commitInternalNoteDrop(delegateItem, localX, localY) {
        if (!listBarLayout.useInternalNoteDrag || !delegateItem || !listBarLayout.noteDropTarget)
            return false;
        if (listBarLayout.noteDropTarget.commitNoteDropAtPosition === undefined)
            return false;
        const noteIds = listBarLayout.selectedNoteIdsForDelegateAction(delegateItem.index, delegateItem.noteId);
        if (noteIds.length === 0)
            return false;
        const mappedPoint = delegateItem.mapToItem(listBarLayout.noteDropTarget, Number(localX) || 0, Number(localY) || 0);
        return Boolean(listBarLayout.noteDropTarget.commitNoteDropAtPosition(mappedPoint.x, mappedPoint.y, noteIds, listBarLayout.noteDropTarget));
    }
    function currentIndexChangeIsAuthoritative(index) {
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(index);
        if (listBarLayout.syncingCurrentIndexFromModel)
            return true;
        if (noteSelectionState.pendingIndex === normalizedIndex)
            return true;
        return listBarLayout.currentIndexFromModel() === normalizedIndex;
    }
    function currentIndexFromModel() {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return -1;
        if (listBarLayout.resolvedNoteListModel.currentIndex !== undefined)
            return Number(listBarLayout.resolvedNoteListModel.currentIndex);
        return -1;
    }
    function currentNoteEntryFromModel() {
        if (listBarLayout.resolvedNoteListModel && listBarLayout.resolvedNoteListModel.currentNoteEntry !== undefined && listBarLayout.resolvedNoteListModel.currentNoteEntry !== null && typeof listBarLayout.resolvedNoteListModel.currentNoteEntry === "object") {
            return listBarLayout.resolvedNoteListModel.currentNoteEntry;
        }
        return ({});
    }
    function currentNoteIdFromModel() {
        const currentNoteEntry = listBarLayout.committedNoteEntry;
        const entryNoteId = listBarLayout.noteIdFromEntry(currentNoteEntry);
        if (entryNoteId.length > 0)
            return entryNoteId;
        if (listBarLayout.resolvedNoteListModel && listBarLayout.resolvedNoteListModel.currentNoteId !== undefined && listBarLayout.resolvedNoteListModel.currentNoteId !== null) {
            return String(listBarLayout.resolvedNoteListModel.currentNoteId).trim();
        }
        return "";
    }
    function deleteContextMenuNote() {
        if (!listBarLayout.noteContextMenuNoteAvailable)
            return false;
        const targetNoteIds = listBarLayout.uniqueTrimmedStringList(listBarLayout.contextMenuNoteIds);
        if (targetNoteIds.length === 0)
            return false;
        noteDeletionBridge.focusedNoteId = targetNoteIds[0];
        const deleted = listBarLayout.deleteNoteIds(targetNoteIds);
        if (deleted)
            noteListView.forceActiveFocus();
        return deleted;
    }
    function deleteCurrentNote() {
        if (!listBarLayout.noteDeletionContractAvailable)
            return false;
        const targetNoteIds = listBarLayout.selectedNoteIdsForCurrentAction();
        const deleted = listBarLayout.deleteNoteIds(targetNoteIds);
        if (deleted)
            noteListView.forceActiveFocus();
        return deleted;
    }
    function deleteNoteIds(noteIds) {
        const normalizedNoteIds = listBarLayout.uniqueTrimmedStringList(noteIds);
        if (normalizedNoteIds.length === 0)
            return false;

        if (listBarLayout.noteDeletionController && listBarLayout.noteDeletionController.deleteNotesByIds !== undefined)
            return Boolean(listBarLayout.noteDeletionController.deleteNotesByIds(normalizedNoteIds));

        let deletedAny = false;
        if (listBarLayout.noteDeletionController && listBarLayout.noteDeletionController.deleteNoteById !== undefined) {
            for (let index = 0; index < normalizedNoteIds.length; ++index) {
                if (listBarLayout.noteDeletionController.deleteNoteById(normalizedNoteIds[index]))
                    deletedAny = true;
            }
        }

        if (!deletedAny && normalizedNoteIds.length === 1 && noteDeletionBridge.deleteContractAvailable) {
            noteDeletionBridge.focusedNoteId = normalizedNoteIds[0];
            deletedAny = noteDeletionBridge.deleteFocusedNote();
        }
        return deletedAny;
    }
    function isDelegateActive(index, noteId) {
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(index);
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedIndex >= 0 && listBarLayout.noteSelectionContainsIndex(normalizedIndex))
            return true;
        if (normalizedIndex >= 0 && listBarLayout.committedNoteIndex === normalizedIndex)
            return true;
        if (normalizedNoteId.length === 0 || listBarLayout.committedNoteId.length === 0)
            return false;
        return normalizedNoteId === listBarLayout.committedNoteId;
    }
    function normalizeCurrentIndex(index) {
        const numericIndex = Number(index);
        if (!isFinite(numericIndex))
            return -1;
        return Math.max(-1, Math.floor(numericIndex));
    }
    function normalizeEntries(value) {
        if (value === undefined || value === null)
            return [];
        if (typeof value === "string")
            return value.split(",").map(function (entry) {
                return entry.trim();
            }).filter(function (entry) {
                return entry.length > 0;
            });
        if (Array.isArray(value))
            return value.map(function (entry) {
                return String(entry).trim();
            }).filter(function (entry) {
                return entry.length > 0;
            });
        if (value.length !== undefined)
            return Array.prototype.slice.call(value).map(function (entry) {
                return String(entry).trim();
            }).filter(function (entry) {
                return entry.length > 0;
            });
        return [];
    }
    function normalizeSelectedNoteIndices(indices) {
        return noteSelectionController.normalizeSelectedNoteIndices(indices);
    }
    function normalizedKeyboardModifiers(modifiers) {
        return noteSelectionController.normalizedKeyboardModifiers(modifiers);
    }
    function noteIdAtIndex(index) {
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(index);
        const model = listBarLayout.resolvedNoteListModel;
        if (normalizedIndex < 0 || !model || model.index === undefined || model.data === undefined)
            return "";
        const modelIndex = model.index(normalizedIndex, 0);
        if (!modelIndex)
            return "";
        const noteId = model.data(modelIndex, model.NoteIdRole !== undefined ? model.NoteIdRole : 258);
        return String(noteId || "").trim();
    }
    function noteIdFromEntry(noteEntry) {
        if (noteEntry === undefined || noteEntry === null || typeof noteEntry !== "object")
            return "";
        if (noteEntry.noteId !== undefined && noteEntry.noteId !== null) {
            const normalizedNoteId = String(noteEntry.noteId).trim();
            if (normalizedNoteId.length > 0)
                return normalizedNoteId;
        }
        if (noteEntry.id !== undefined && noteEntry.id !== null)
            return String(noteEntry.id).trim();
        return "";
    }
    function noteListMaxContentY() {
        return Math.max(0, (Number(noteListView.contentHeight) || 0) - (Number(noteListView.height) || 0));
    }
    function noteListViewportTargetY(value) {
        if (listBarLayout.noteListKineticViewportEnabled)
            return listBarLayout.clampNoteListContentY(value);
        return listBarLayout.quantizedNoteListContentY(value);
    }
    function noteSelectionContainsIndex(index) {
        return noteSelectionController.noteSelectionContainsIndex(index);
    }
    function openNoteContextMenu(delegateItem, localX, localY) {
        if (!delegateItem)
            return false;
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(delegateItem.index);
        const normalizedNoteId = delegateItem.noteId !== undefined && delegateItem.noteId !== null ? String(delegateItem.noteId).trim() : "";
        if (normalizedNoteId.length === 0)
            return false;
        const actionNoteIds = listBarLayout.selectedNoteIdsForDelegateAction(normalizedIndex, normalizedNoteId);
        if (actionNoteIds.length === 0)
            return false;
        listBarLayout.contextMenuNoteIndex = normalizedIndex;
        listBarLayout.contextMenuNoteId = normalizedNoteId;
        listBarLayout.contextMenuNoteIds = actionNoteIds;
        noteDeletionBridge.focusedNoteId = normalizedNoteId;
        noteContextMenu.openFor(delegateItem, Number(localX) || 0, Number(localY) || 0);
        return true;
    }
    function noteContextMenuPointerTriggerAccepted(triggerKind) {
        const normalizedTrigger = triggerKind === undefined || triggerKind === null ? "" : String(triggerKind).trim().toLowerCase();
        if (normalizedTrigger === "rightclick" || normalizedTrigger === "right-click" || normalizedTrigger === "contextmenu" || normalizedTrigger === "context-menu") {
            return true;
        }
        if (normalizedTrigger === "longpress" || normalizedTrigger === "long-press" || normalizedTrigger === "pressandhold" || normalizedTrigger === "press-and-hold") {
            return listBarLayout.noteListKineticViewportEnabled;
        }
        return false;
    }
    function openNoteContextMenuFromPointer(delegateItem, localX, localY, triggerKind) {
        if (!listBarLayout.noteContextMenuPointerTriggerAccepted(triggerKind))
            return false;
        return listBarLayout.openNoteContextMenu(delegateItem, localX, localY);
    }
    function pushCurrentIndexToModel(index) {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return;
        const normalizedIndex = Number(index);
        if (!isFinite(normalizedIndex))
            return;
        if (listBarLayout.resolvedNoteListModel.currentIndex !== undefined) {
            if (Number(listBarLayout.resolvedNoteListModel.currentIndex) === normalizedIndex)
                return;
            listBarLayout.resolvedNoteListModel.currentIndex = normalizedIndex;
            return;
        }
        if (listBarLayout.resolvedNoteListModel.setCurrentIndex !== undefined)
            listBarLayout.resolvedNoteListModel.setCurrentIndex(normalizedIndex);
    }
    function quantizedNoteListContentY(value) {
        const clampedValue = listBarLayout.clampNoteListContentY(value);
        const tick = Math.max(1, Number(listBarLayout.noteListScrollTick) || 1);
        const maxContentY = listBarLayout.noteListMaxContentY();
        if (clampedValue <= 0)
            return 0;
        if (clampedValue >= maxContentY - tick)
            return maxContentY;
        return Math.max(0, Math.min(maxContentY, Math.round(clampedValue / tick) * tick));
    }
    function requestNoteSelection(index, noteId, modifiers) {
        noteSelectionController.requestNoteSelection(index, noteId, modifiers);
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }
    function resolveSelectionModifiers(modifiers, cachedModifiers, cachedCapturedAtMs) {
        return noteSelectionController.resolveSelectionModifiers(modifiers, cachedModifiers, cachedCapturedAtMs);
    }
    function restoreNoteListViewport() {
        if (!listBarLayout.noteListViewportRestorePending)
            return;
        listBarLayout.noteListViewportRestorePending = false;
        listBarLayout.applyNoteListViewportStep(listBarLayout.preservedNoteListContentY);
    }
    function selectedNoteIdsForCurrentAction() {
        const selectedIds = listBarLayout.selectedNoteIdsFromIndices(listBarLayout.selectedNoteIndices);
        if (selectedIds.length > 0)
            return selectedIds;

        const focusedNoteId = noteDeletionBridge && noteDeletionBridge.focusedNoteId !== undefined ? String(noteDeletionBridge.focusedNoteId || "").trim() : "";
        if (focusedNoteId.length > 0)
            return [focusedNoteId];

        const currentNoteId = listBarLayout.currentNoteIdFromModel();
        return currentNoteId.length > 0 ? [currentNoteId] : [];
    }
    function selectedNoteIdsForDelegateAction(index, noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(index);
        const normalizedSelection = listBarLayout.normalizeSelectedNoteIndices(listBarLayout.selectedNoteIndices);
        if (normalizedIndex >= 0 && normalizedSelection.length > 1 && normalizedSelection.indexOf(normalizedIndex) >= 0) {
            const selectedIds = listBarLayout.selectedNoteIdsFromIndices(normalizedSelection);
            if (selectedIds.length > 0)
                return selectedIds;
        }
        return normalizedNoteId.length > 0 ? [normalizedNoteId] : [];
    }
    function selectedNoteIdsFromIndices(indices) {
        const normalizedIndices = listBarLayout.normalizeSelectedNoteIndices(indices);
        const noteIds = [];
        for (let selectionIndex = 0; selectionIndex < normalizedIndices.length; ++selectionIndex) {
            const noteId = listBarLayout.noteIdAtIndex(normalizedIndices[selectionIndex]);
            if (!noteId.length || noteIds.indexOf(noteId) >= 0)
                continue;
            noteIds.push(noteId);
        }
        return noteIds;
    }
    function selectionModifierPressed(modifiers) {
        return noteSelectionController.selectionModifierPressed(modifiers);
    }
    function selectionRangeIndices(anchorIndex, targetIndex) {
        return noteSelectionController.selectionRangeIndices(anchorIndex, targetIndex);
    }
    function selectionRangeModifierPressed(modifiers) {
        return noteSelectionController.selectionRangeModifierPressed(modifiers);
    }
    function selectionToggleModifierPressed(modifiers) {
        return noteSelectionController.selectionToggleModifierPressed(modifiers);
    }
    function setSelectedNoteIndices(indices) {
        noteSelectionController.setSelectedNoteIndices(indices);
    }
    function settleNoteListViewport() {
        if (listBarLayout.noteListViewportRestorePending) {
            listBarLayout.restoreNoteListViewport();
            return;
        }
        if (listBarLayout.noteListKineticViewportEnabled) {
            const clampedContentY = listBarLayout.clampNoteListContentY(noteListView.contentY);
            if (Math.abs(clampedContentY - (Number(noteListView.contentY) || 0)) > 0.001)
                listBarLayout.applyNoteListViewportStep(clampedContentY);
            return;
        }
        listBarLayout.applyNoteListViewportStep(noteListView.contentY);
    }
    function syncCurrentIndexFromModel() {
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(listBarLayout.currentIndexFromModel());
        if (noteListView.currentIndex !== normalizedIndex)
            listBarLayout.syncingCurrentIndexFromModel = true;
        if (noteListView.currentIndex !== normalizedIndex)
            noteListView.currentIndex = normalizedIndex;
        listBarLayout.syncingCurrentIndexFromModel = false;
    }
    function scheduleNoteListModelTransitionSync() {
        const transitionRevision = listBarLayout.noteListModelTransitionRevision + 1;
        const expectedModel = listBarLayout.resolvedNoteListModel;
        listBarLayout.noteListModelTransitionRevision = transitionRevision;
        Qt.callLater(function () {
            if (listBarLayout.noteListModelTransitionRevision !== transitionRevision)
                return;
            if (listBarLayout.resolvedNoteListModel !== expectedModel)
                return;
            listBarLayout.applySearchTextToModel();
            listBarLayout.syncCurrentIndexFromModel();
            listBarLayout.syncSelectionFromCommittedState();
            listBarLayout.syncFocusedNoteDeletionState();
        });
    }
    function syncFocusedNoteDeletionState() {
        const currentNoteEntry = listBarLayout.committedNoteEntry;
        const currentNoteId = listBarLayout.noteIdFromEntry(currentNoteEntry);
        if (currentNoteId.length > 0) {
            noteDeletionBridge.focusedNoteId = currentNoteId;
            return;
        }
        const noteModel = listBarLayout.resolvedNoteListModel;
        if (noteModel && noteModel.currentNoteId !== undefined && noteModel.currentNoteId !== null) {
            noteDeletionBridge.focusedNoteId = String(noteModel.currentNoteId).trim();
            return;
        }
        noteDeletionBridge.focusedNoteId = "";
    }
    function syncSelectionFromCommittedState() {
        noteSelectionController.syncSelectionFromCommittedState();
    }
    function uniqueTrimmedStringList(values) {
        if (values === undefined || values === null)
            return [];

        var sourceValues = values;
        if (typeof sourceValues === "string")
            sourceValues = sourceValues.split(/\r?\n/);
        else if (!Array.isArray(sourceValues) && sourceValues.length !== undefined)
            sourceValues = Array.prototype.slice.call(sourceValues);
        else if (!Array.isArray(sourceValues))
            sourceValues = [sourceValues];

        const normalized = [];
        for (let index = 0; index < sourceValues.length; ++index) {
            const normalizedValue = String(sourceValues[index] === undefined || sourceValues[index] === null ? "" : sourceValues[index]).trim();
            if (!normalizedValue.length || normalized.indexOf(normalizedValue) >= 0)
                continue;
            normalized.push(normalizedValue);
        }
        return normalized;
    }
    function updateInternalNoteDropPreview(delegateItem, localX, localY) {
        if (!listBarLayout.useInternalNoteDrag || !delegateItem || !listBarLayout.noteDropTarget)
            return false;
        if (listBarLayout.noteDropTarget.updateNoteDropPreviewAtPosition === undefined)
            return false;
        const noteIds = listBarLayout.selectedNoteIdsForDelegateAction(delegateItem.index, delegateItem.noteId);
        if (noteIds.length === 0)
            return false;
        const mappedPoint = delegateItem.mapToItem(listBarLayout.noteDropTarget, Number(localX) || 0, Number(localY) || 0);
        return Boolean(listBarLayout.noteDropTarget.updateNoteDropPreviewAtPosition(mappedPoint.x, mappedPoint.y, noteIds, listBarLayout.noteDropTarget));
    }
    function updateNoteDragPreviewPosition(delegateItem, localX, localY) {
        if (!delegateItem || noteDragPreviewState.delegateItem !== delegateItem)
            return;
        const previewParent = noteDragPreview.parent ? noteDragPreview.parent : listBarLayout;
        const mappedPoint = delegateItem.mapToItem(previewParent, Number(localX) || 0, Number(localY) || 0);
        noteDragPreviewState.x = Math.round((Number(mappedPoint.x) || 0) - noteDragPreviewState.hotSpotX);
        noteDragPreviewState.y = Math.round((Number(mappedPoint.y) || 0) - noteDragPreviewState.hotSpotY);
    }

    color: panelColor

    Component.onCompleted: {
        listBarLayout.syncCurrentIndexFromModel();
        listBarLayout.syncSelectionFromCommittedState();
        listBarLayout.syncFocusedNoteDeletionState();
    }
    onNoteListModeChanged: {
        if (noteContextMenu.opened)
            noteContextMenu.close();
        applySearchTextToModel();
    }
    onResolvedNoteListModelChanged: {
        listBarLayout.noteDragCanceled = true;
        listBarLayout.noteDragActive = false;
        listBarLayout.noteListViewportRestorePending = false;
        listBarLayout.preservedNoteListContentY = 0;
        listBarLayout.clearInternalNoteDropPreview();
        listBarLayout.clearNoteDragPreview(null);
        if (noteContextMenu.opened)
            noteContextMenu.close();
        listBarLayout.clearNoteContextMenuState();
        noteSelectionState.pendingIndex = -1;
        listBarLayout.noteSelectionAnchorIndex = -1;
        listBarLayout.setSelectedNoteIndices([]);
        listBarLayout.pressedNoteIndex = -1;
        noteSelectionState.requestRevision += 1;
        listBarLayout.scheduleNoteListModelTransitionSync();
    }
    onSearchTextChanged: applySearchTextToModel()

    QtObject {
        id: noteSelectionState

        property int pendingIndex: -1
        property int requestRevision: 0
    }
    QtObject {
        id: noteSelectionController

        property var view: listBarLayout
        property int selectionAnchorIndex: -1
        property var selectedIndices: []

        function normalizedKeyboardModifiers(modifiers) {
            return modifiers === undefined || modifiers === null ? Qt.NoModifier : modifiers;
        }

        function selectionToggleModifierPressed(modifiers) {
            const normalizedModifiers = noteSelectionController.normalizedKeyboardModifiers(modifiers);
            const toggleMask = Qt.ControlModifier | Qt.MetaModifier;
            return Boolean(normalizedModifiers & toggleMask);
        }

        function selectionRangeModifierPressed(modifiers) {
            const normalizedModifiers = noteSelectionController.normalizedKeyboardModifiers(modifiers);
            return Boolean(normalizedModifiers & Qt.ShiftModifier);
        }

        function selectionModifierPressed(modifiers) {
            return noteSelectionController.selectionRangeModifierPressed(modifiers) || noteSelectionController.selectionToggleModifierPressed(modifiers);
        }

        function resolveSelectionModifiers(modifiers, cachedModifiers, cachedCapturedAtMs) {
            const normalizedModifiers = noteSelectionController.normalizedKeyboardModifiers(modifiers);
            if (noteSelectionController.selectionModifierPressed(normalizedModifiers))
                return normalizedModifiers;
            const capturedAtMs = Number(cachedCapturedAtMs);
            const cacheAgeMs = Date.now() - capturedAtMs;
            const cacheFresh = capturedAtMs > 0 && isFinite(cacheAgeMs) && cacheAgeMs >= 0 && cacheAgeMs <= 800;
            const normalizedCachedModifiers = noteSelectionController.normalizedKeyboardModifiers(cachedModifiers);
            if (cacheFresh && noteSelectionController.selectionModifierPressed(normalizedCachedModifiers))
                return normalizedCachedModifiers;
            return normalizedModifiers;
        }

        function normalizeSelectedNoteIndices(indices) {
            if (!indices || indices.length === undefined || !noteSelectionController.view)
                return [];
            const normalized = [];
            for (let row = 0; row < indices.length; ++row) {
                const normalizedIndex = noteSelectionController.view.normalizeCurrentIndex(indices[row]);
                if (normalizedIndex < 0)
                    continue;
                if (normalized.indexOf(normalizedIndex) >= 0)
                    continue;
                normalized.push(normalizedIndex);
            }
            normalized.sort(function (left, right) {
                return left - right;
            });
            return normalized;
        }

        function noteSelectionContainsIndex(index) {
            if (!noteSelectionController.view)
                return false;
            const normalizedIndex = noteSelectionController.view.normalizeCurrentIndex(index);
            if (normalizedIndex < 0)
                return false;
            const normalizedSelection = noteSelectionController.normalizeSelectedNoteIndices(noteSelectionController.selectedIndices);
            return normalizedSelection.indexOf(normalizedIndex) >= 0;
        }

        function setSelectedNoteIndices(indices) {
            noteSelectionController.selectedIndices = noteSelectionController.normalizeSelectedNoteIndices(indices);
        }

        function selectionRangeIndices(anchorIndex, targetIndex) {
            if (!noteSelectionController.view)
                return [];
            const normalizedAnchor = noteSelectionController.view.normalizeCurrentIndex(anchorIndex);
            const normalizedTarget = noteSelectionController.view.normalizeCurrentIndex(targetIndex);
            if (normalizedTarget < 0)
                return [];
            if (normalizedAnchor < 0)
                return [normalizedTarget];
            const begin = Math.min(normalizedAnchor, normalizedTarget);
            const end = Math.max(normalizedAnchor, normalizedTarget);
            const range = [];
            for (let candidate = begin; candidate <= end; ++candidate)
                range.push(candidate);
            return range;
        }

        function syncSelectionFromCommittedState() {
            if (!noteSelectionController.view)
                return;
            const normalizedIndex = noteSelectionController.view.normalizeCurrentIndex(noteSelectionController.view.currentIndexFromModel());
            if (normalizedIndex < 0) {
                noteSelectionController.setSelectedNoteIndices([]);
                noteSelectionController.selectionAnchorIndex = -1;
                return;
            }
            noteSelectionController.setSelectedNoteIndices([normalizedIndex]);
            noteSelectionController.selectionAnchorIndex = normalizedIndex;
        }

        function requestNoteSelection(index, noteId, modifiers) {
            if (!noteSelectionController.view)
                return;
            const normalizedIndex = noteSelectionController.view.normalizeCurrentIndex(index);
            if (normalizedIndex < 0)
                return;
            const normalizedModifiers = noteSelectionController.normalizedKeyboardModifiers(modifiers);
            if (noteSelectionController.selectionRangeModifierPressed(normalizedModifiers)) {
                let anchorIndex = noteSelectionController.view.normalizeCurrentIndex(noteSelectionController.selectionAnchorIndex);
                if (anchorIndex < 0)
                    anchorIndex = noteSelectionController.view.normalizeCurrentIndex(noteSelectionController.view.currentIndexFromModel());
                if (anchorIndex < 0)
                    anchorIndex = normalizedIndex;
                const rangeSelection = noteSelectionController.selectionRangeIndices(anchorIndex, normalizedIndex);
                if (noteSelectionController.selectionToggleModifierPressed(normalizedModifiers)) {
                    const selectedIndices = noteSelectionController.normalizeSelectedNoteIndices(noteSelectionController.selectedIndices);
                    for (let selectionIndex = 0; selectionIndex < rangeSelection.length; ++selectionIndex)
                        selectedIndices.push(rangeSelection[selectionIndex]);
                    noteSelectionController.setSelectedNoteIndices(selectedIndices);
                } else {
                    noteSelectionController.setSelectedNoteIndices(rangeSelection);
                }
                noteSelectionController.selectionAnchorIndex = anchorIndex;
                noteSelectionController.view.activateNoteIndex(normalizedIndex, noteId);
                return;
            }
            if (noteSelectionController.selectionToggleModifierPressed(normalizedModifiers)) {
                const selectedIndices = noteSelectionController.normalizeSelectedNoteIndices(noteSelectionController.selectedIndices);
                const existingSelectionIndex = selectedIndices.indexOf(normalizedIndex);
                if (existingSelectionIndex < 0) {
                    selectedIndices.push(normalizedIndex);
                    noteSelectionController.setSelectedNoteIndices(selectedIndices);
                    noteSelectionController.selectionAnchorIndex = normalizedIndex;
                    noteSelectionController.view.activateNoteIndex(normalizedIndex, noteId);
                    return;
                }
                if (selectedIndices.length <= 1) {
                    noteSelectionController.setSelectedNoteIndices([normalizedIndex]);
                    noteSelectionController.selectionAnchorIndex = normalizedIndex;
                    noteSelectionController.view.activateNoteIndex(normalizedIndex, noteId);
                    return;
                }
                selectedIndices.splice(existingSelectionIndex, 1);
                noteSelectionController.setSelectedNoteIndices(selectedIndices);
                const committedIndex = noteSelectionController.view.normalizeCurrentIndex(noteSelectionController.view.currentIndexFromModel());
                const committedSelectionRetained = noteSelectionController.noteSelectionContainsIndex(committedIndex);
                if (committedSelectionRetained)
                    return;
                const fallbackIndex = selectedIndices.length > 0 ? selectedIndices[selectedIndices.length - 1] : -1;
                if (fallbackIndex >= 0)
                    noteSelectionController.view.activateNoteIndex(fallbackIndex, "");
                return;
            }
            noteSelectionController.setSelectedNoteIndices([normalizedIndex]);
            noteSelectionController.selectionAnchorIndex = normalizedIndex;
            noteSelectionController.view.activateNoteIndex(normalizedIndex, noteId);
        }
    }
    QtObject {
        id: noteDragPreviewState

        property var delegateItem: null
        property real hotSpotX: 0
        property real hotSpotY: 0
        property var noteIds: []
        property real x: 0
        property real y: 0
    }
    FocusedNoteDeletionBridge {
        id: noteDeletionBridge

        deletionTarget: listBarLayout.noteDeletionController
        noteListModel: listBarLayout.resolvedNoteListModel
    }
    LV.ContextMenu {
        id: noteContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        items: listBarLayout.noteContextMenuItems
        modal: false
        parent: Controls.Overlay.overlay

        onClosed: listBarLayout.clearNoteContextMenuState()
        onItemEventTriggered: function (eventName, payload, index, item) {
            if (eventName === "note.delete")
                listBarLayout.deleteContextMenuNote();
            else if (eventName === "note.clearAllFolders")
                listBarLayout.clearContextMenuNoteFolders();
        }
    }
    Item {
        id: noteDragPreview

        height: listBarLayout.resourceListMode ? resourceDragPreviewCard.implicitHeight : noteDragPreviewCard.implicitHeight
        parent: Controls.Overlay.overlay ? Controls.Overlay.overlay : listBarLayout
        visible: listBarLayout.noteDragActive && noteDragPreviewState.delegateItem !== null
        width: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.width !== undefined ? Number(noteDragPreviewState.delegateItem.width) || implicitWidth : implicitWidth
        x: noteDragPreviewState.x
        y: noteDragPreviewState.y
        z: 1000

        NoteListItem {
            id: noteDragPreviewCard

            active: true
            anchors.fill: parent
            bookmarkColor: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.bookmarkColor !== undefined && noteDragPreviewState.delegateItem.bookmarkColor !== null ? String(noteDragPreviewState.delegateItem.bookmarkColor) : ""
            bookmarked: noteDragPreviewState.delegateItem ? Boolean(noteDragPreviewState.delegateItem.bookmarked) : false
            displayDate: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.displayDate !== undefined && noteDragPreviewState.delegateItem.displayDate !== null ? String(noteDragPreviewState.delegateItem.displayDate) : ""
            folders: noteDragPreviewState.delegateItem ? listBarLayout.normalizeEntries(noteDragPreviewState.delegateItem.folders) : []
            image: noteDragPreviewState.delegateItem ? Boolean(noteDragPreviewState.delegateItem.image) : false
            imageSource: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.imageSource !== undefined && noteDragPreviewState.delegateItem.imageSource !== null ? noteDragPreviewState.delegateItem.imageSource : ""
            noteId: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.noteId !== undefined && noteDragPreviewState.delegateItem.noteId !== null ? String(noteDragPreviewState.delegateItem.noteId) : ""
            opacity: listBarLayout.grabbedNoteOpacity
            primaryText: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.primaryText !== undefined && noteDragPreviewState.delegateItem.primaryText !== null ? String(noteDragPreviewState.delegateItem.primaryText) : ""
            tags: noteDragPreviewState.delegateItem ? listBarLayout.normalizeEntries(noteDragPreviewState.delegateItem.tags) : []
            visible: !listBarLayout.resourceListMode
        }
        ResourceListItem {
            id: resourceDragPreviewCard

            active: true
            anchors.fill: parent
            opacity: listBarLayout.grabbedNoteOpacity
            previewSource: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.imageSource !== undefined && noteDragPreviewState.delegateItem.imageSource !== null ? noteDragPreviewState.delegateItem.imageSource : ""
            titleText: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.primaryText !== undefined && noteDragPreviewState.delegateItem.primaryText !== null ? String(noteDragPreviewState.delegateItem.primaryText) : ""
            visible: listBarLayout.resourceListMode
        }
        Rectangle {
            anchors.right: parent.right
            anchors.rightMargin: listBarLayout.dragCountBadgeInset
            anchors.top: parent.top
            anchors.topMargin: listBarLayout.dragCountBadgeInset
            color: LV.Theme.accentBlue
            height: listBarLayout.dragCountBadgeHeight
            radius: height * 0.5
            visible: noteDragPreviewState.noteIds.length > 1
            width: Math.max(listBarLayout.dragCountBadgeMinWidth, dragCountLabel.implicitWidth + listBarLayout.dragCountBadgeWidthPadding)

            LV.Label {
                id: dragCountLabel

                anchors.centerIn: parent
                color: LV.Theme.accentWhite
                style: caption
                text: String(noteDragPreviewState.noteIds.length)
            }
        }
    }
    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: LV.Theme.gapNone

            Item {
                id: topToolbar

                Layout.fillWidth: true
                Layout.preferredHeight: listBarLayout.headerVisible ? listBarLayout.topToolbarHeight : 0
                visible: listBarLayout.headerVisible

                ListBarHeader {
                    anchors.fill: parent
                    searchText: listBarLayout.searchText
                    visible: listBarLayout.headerVisible

                    onSearchSubmitted: function (text) {
                        listBarLayout.searchText = text;
                        listBarLayout.applySearchTextToModel();
                    }
                    onSearchTextEdited: function (text) {
                        listBarLayout.searchText = text;
                        listBarLayout.applySearchTextToModel();
                    }
                }
            }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                ListView {
                    id: noteListView

                    anchors.fill: parent
                    anchors.margins: listBarLayout.noteListViewportInset
                    boundsBehavior: listBarLayout.noteListBoundsBehavior
                    boundsMovement: listBarLayout.noteListBoundsMovement
                    cacheBuffer: Math.max(0, height * 2)
                    clip: true
                    flickDeceleration: listBarLayout.noteListFlickDeceleration
                    interactive: contentHeight > height && !listBarLayout.noteDragActive
                    maximumFlickVelocity: listBarLayout.noteListMaximumFlickVelocity
                    model: listBarLayout.resolvedNoteListModel
                    pixelAligned: true
                    reuseItems: !listBarLayout.noteDragActive
                    spacing: listBarLayout.noteListViewportInset
                    synchronousDrag: !listBarLayout.noteListKineticViewportEnabled
                    visible: listBarLayout.noteListMode

                    delegate: Item {
                        id: noteItemDelegate

                        required property string bookmarkColor
                        required property bool bookmarked
                        required property string displayDate
                        property real dragHotSpotX: width * 0.5
                        property real dragHotSpotY: height * 0.5
                        readonly property var draggedNoteIds: listBarLayout.selectedNoteIdsForDelegateAction(noteItemDelegate.index, noteItemDelegate.noteId)
                        required property var folders
                        required property bool image
                        required property var imageSource
                        readonly property bool immediatePointerDragEnabled: !noteItemDelegate.pointerDragRequiresLongPress
                        required property int index
                        property bool mobileLongPressPendingContextMenu: false
                        property bool mobilePointerDragging: false
                        property bool mobileSuppressNextClick: false
                        required property string noteId
                        readonly property bool pointerDragActive: noteDragHandler.active || noteItemDelegate.mobilePointerDragging
                        readonly property bool pointerDragRequiresLongPress: LV.Theme.mobileTarget
                        property int pointerSelectionModifiers: Qt.NoModifier
                        property double pointerSelectionModifiersCapturedAtMs: 0
                        required property string primaryText
                        required property var tags

                        Drag.active: noteItemDelegate.pointerDragActive
                        Drag.dragType: listBarLayout.useInternalNoteDrag ? Drag.Internal : Drag.Automatic
                        Drag.hotSpot.x: noteItemDelegate.dragHotSpotX
                        Drag.hotSpot.y: noteItemDelegate.dragHotSpotY
                        Drag.keys: ["whatson.library.note"]
                        Drag.mimeData: ({
                                "application/x-whatson-note-id": noteItemDelegate.draggedNoteIds.length > 0 ? noteItemDelegate.draggedNoteIds[0] : noteItemDelegate.noteId,
                                "application/x-whatson-note-ids": JSON.stringify(noteItemDelegate.draggedNoteIds),
                                "text/plain": noteItemDelegate.draggedNoteIds.join("\n")
                            })
                        Drag.source: noteItemDelegate
                        Drag.supportedActions: Qt.CopyAction
                        // Legacy syntax-guard anchor: height: noteCard.implicitHeight
                        height: listBarLayout.resourceListMode ? resourceCard.implicitHeight : noteCard.implicitHeight
                        width: ListView.view ? ListView.view.width : listBarLayout.width

                        NoteListItem {
                            id: noteCard

                            active: listBarLayout.isDelegateActive(noteItemDelegate.index, noteItemDelegate.noteId)
                            anchors.fill: parent
                            bookmarkColor: noteItemDelegate.bookmarkColor === undefined || noteItemDelegate.bookmarkColor === null ? "" : String(noteItemDelegate.bookmarkColor)
                            bookmarked: noteItemDelegate.bookmarked === undefined ? false : Boolean(noteItemDelegate.bookmarked)
                            displayDate: noteItemDelegate.displayDate === undefined || noteItemDelegate.displayDate === null ? "" : String(noteItemDelegate.displayDate)
                            folders: listBarLayout.normalizeEntries(noteItemDelegate.folders)
                            image: noteItemDelegate.image === undefined ? false : Boolean(noteItemDelegate.image)
                            imageSource: noteItemDelegate.imageSource === undefined || noteItemDelegate.imageSource === null ? "" : noteItemDelegate.imageSource
                            noteId: noteItemDelegate.noteId === undefined || noteItemDelegate.noteId === null ? "" : String(noteItemDelegate.noteId)
                            opacity: noteItemDelegate.pointerDragActive ? listBarLayout.grabbedNoteOpacity : 1
                            pressed: listBarLayout.pressedNoteIndex === noteItemDelegate.index || noteItemDelegate.pointerDragActive
                            primaryText: noteItemDelegate.primaryText === undefined || noteItemDelegate.primaryText === null ? "" : String(noteItemDelegate.primaryText)
                            tags: listBarLayout.normalizeEntries(noteItemDelegate.tags)
                            visible: !listBarLayout.resourceListMode
                        }
                        ResourceListItem {
                            id: resourceCard

                            active: listBarLayout.isDelegateActive(noteItemDelegate.index, noteItemDelegate.noteId)
                            anchors.fill: parent
                            opacity: noteItemDelegate.pointerDragActive ? listBarLayout.grabbedNoteOpacity : 1
                            pressed: listBarLayout.pressedNoteIndex === noteItemDelegate.index || noteItemDelegate.pointerDragActive
                            previewSource: noteItemDelegate.imageSource === undefined || noteItemDelegate.imageSource === null ? "" : noteItemDelegate.imageSource
                            titleText: noteItemDelegate.primaryText === undefined || noteItemDelegate.primaryText === null ? "" : String(noteItemDelegate.primaryText)
                            visible: listBarLayout.resourceListMode
                        }
                        DragHandler {
                            id: noteDragHandler

                            acceptedButtons: Qt.LeftButton
                            dragThreshold: 4
                            enabled: noteItemDelegate.immediatePointerDragEnabled
                            grabPermissions: PointerHandler.CanTakeOverFromAnything
                            target: null

                            onActiveChanged: {
                                listBarLayout.noteDragActive = active;
                                if (active) {
                                    listBarLayout.noteDragCanceled = false;
                                    noteItemDelegate.dragHotSpotX = Number(noteDragHandler.centroid.pressPosition.x) || noteItemDelegate.width * 0.5;
                                    noteItemDelegate.dragHotSpotY = Number(noteDragHandler.centroid.pressPosition.y) || noteItemDelegate.height * 0.5;
                                    listBarLayout.beginNoteDragPreview(noteItemDelegate, noteItemDelegate.dragHotSpotX, noteItemDelegate.dragHotSpotY);
                                    listBarLayout.updateInternalNoteDropPreview(noteItemDelegate, noteDragHandler.centroid.position.x, noteDragHandler.centroid.position.y);
                                } else {
                                    if (!listBarLayout.noteDragCanceled)
                                        listBarLayout.commitInternalNoteDrop(noteItemDelegate, noteDragHandler.centroid.position.x, noteDragHandler.centroid.position.y);
                                    listBarLayout.clearInternalNoteDropPreview();
                                    listBarLayout.clearNoteDragPreview(noteItemDelegate);
                                    listBarLayout.noteDragCanceled = false;
                                }
                                if (active && listBarLayout.pressedNoteIndex === noteItemDelegate.index)
                                    listBarLayout.pressedNoteIndex = -1;
                            }
                            onCanceled: {
                                listBarLayout.noteDragCanceled = true;
                                listBarLayout.noteDragActive = false;
                                listBarLayout.clearInternalNoteDropPreview();
                                listBarLayout.clearNoteDragPreview(noteItemDelegate);
                            }
                            onCentroidChanged: {
                                if (!active)
                                    return;
                                listBarLayout.updateNoteDragPreviewPosition(noteItemDelegate, noteDragHandler.centroid.position.x, noteDragHandler.centroid.position.y);
                                listBarLayout.updateInternalNoteDropPreview(noteItemDelegate, noteDragHandler.centroid.position.x, noteDragHandler.centroid.position.y);
                            }
                        }
                        MouseArea {
                            id: mobileLongPressDragArea

                            acceptedButtons: Qt.LeftButton
                            anchors.fill: parent
                            enabled: noteItemDelegate.pointerDragRequiresLongPress
                            hoverEnabled: false
                            pressAndHoldInterval: listBarLayout.mobileNoteDragHoldInterval
                            preventStealing: noteItemDelegate.mobilePointerDragging

                            onCanceled: {
                                const dragging = noteItemDelegate.mobilePointerDragging;
                                noteItemDelegate.mobileLongPressPendingContextMenu = false;
                                noteItemDelegate.mobilePointerDragging = false;
                                noteItemDelegate.mobileSuppressNextClick = false;
                                listBarLayout.pressedNoteIndex = -1;
                                if (!dragging)
                                    return;
                                listBarLayout.noteDragCanceled = true;
                                listBarLayout.noteDragActive = false;
                                listBarLayout.clearInternalNoteDropPreview();
                                listBarLayout.clearNoteDragPreview(noteItemDelegate);
                                listBarLayout.noteDragCanceled = false;
                            }
                            onClicked: function (mouse) {
                                if (noteItemDelegate.mobileSuppressNextClick) {
                                    noteItemDelegate.mobileSuppressNextClick = false;
                                    return;
                                }
                                if (noteItemDelegate.mobilePointerDragging)
                                    return;
                                listBarLayout.pressedNoteIndex = -1;
                                listBarLayout.requestNoteSelection(noteItemDelegate.index, noteItemDelegate.noteId, mouse.modifiers);
                            }
                            onPositionChanged: function (mouse) {
                                if (noteItemDelegate.mobilePointerDragging) {
                                    listBarLayout.updateNoteDragPreviewPosition(noteItemDelegate, mouse.x, mouse.y);
                                    listBarLayout.updateInternalNoteDropPreview(noteItemDelegate, mouse.x, mouse.y);
                                    return;
                                }
                                if (!noteItemDelegate.mobileLongPressPendingContextMenu)
                                    return;
                                const deltaX = Math.abs((Number(mouse.x) || 0) - noteItemDelegate.dragHotSpotX);
                                const deltaY = Math.abs((Number(mouse.y) || 0) - noteItemDelegate.dragHotSpotY);
                                if (Math.max(deltaX, deltaY) < 4)
                                    return;
                                noteItemDelegate.mobileLongPressPendingContextMenu = false;
                                listBarLayout.noteDragCanceled = false;
                                listBarLayout.noteDragActive = true;
                                noteItemDelegate.mobilePointerDragging = true;
                                listBarLayout.beginNoteDragPreview(noteItemDelegate, noteItemDelegate.dragHotSpotX, noteItemDelegate.dragHotSpotY);
                                listBarLayout.updateNoteDragPreviewPosition(noteItemDelegate, mouse.x, mouse.y);
                                listBarLayout.updateInternalNoteDropPreview(noteItemDelegate, mouse.x, mouse.y);
                                if (listBarLayout.pressedNoteIndex === noteItemDelegate.index)
                                    listBarLayout.pressedNoteIndex = -1;
                            }
                            onPressAndHold: function (mouse) {
                                noteItemDelegate.mobileLongPressPendingContextMenu = true;
                                noteItemDelegate.mobileSuppressNextClick = true;
                            }
                            onPressed: function (mouse) {
                                noteItemDelegate.dragHotSpotX = Number(mouse.x) || width * 0.5;
                                noteItemDelegate.dragHotSpotY = Number(mouse.y) || height * 0.5;
                                noteItemDelegate.mobileLongPressPendingContextMenu = false;
                                noteItemDelegate.mobilePointerDragging = false;
                                noteItemDelegate.mobileSuppressNextClick = false;
                                listBarLayout.pressedNoteIndex = noteItemDelegate.index;
                                noteDeletionBridge.focusedNoteId = noteItemDelegate.noteId;
                            }
                            onReleased: function (mouse) {
                                const dragging = noteItemDelegate.mobilePointerDragging;
                                const openContextMenu = noteItemDelegate.mobileLongPressPendingContextMenu && !dragging;
                                noteItemDelegate.mobileLongPressPendingContextMenu = false;
                                noteItemDelegate.mobilePointerDragging = false;
                                listBarLayout.pressedNoteIndex = -1;
                                if (openContextMenu) {
                                    listBarLayout.openNoteContextMenuFromPointer(noteItemDelegate, mouse.x, mouse.y, "longPress");
                                    return;
                                }
                                if (!dragging)
                                    return;
                                listBarLayout.noteDragActive = false;
                                listBarLayout.clearInternalNoteDropPreview();
                                listBarLayout.clearNoteDragPreview(noteItemDelegate);
                                listBarLayout.noteDragCanceled = false;
                            }
                        }
                        TapHandler {
                            id: noteTapHandler

                            acceptedButtons: Qt.LeftButton
                            acceptedModifiers: Qt.KeyboardModifierMask
                            enabled: noteItemDelegate.immediatePointerDragEnabled
                            gesturePolicy: TapHandler.DragThreshold
                            grabPermissions: PointerHandler.ApprovesTakeOverByAnything

                            onCanceled: {
                                noteItemDelegate.pointerSelectionModifiers = Qt.NoModifier;
                                noteItemDelegate.pointerSelectionModifiersCapturedAtMs = 0;
                            }
                            onPressedChanged: {
                                if (!pressed) {
                                    if (listBarLayout.pressedNoteIndex === noteItemDelegate.index && !noteDragHandler.active)
                                        listBarLayout.pressedNoteIndex = -1;
                                    return;
                                }
                                const pressModifiers = noteTapHandler.point && noteTapHandler.point.modifiers !== undefined ? noteTapHandler.point.modifiers : Qt.NoModifier;
                                noteItemDelegate.pointerSelectionModifiers = listBarLayout.normalizedKeyboardModifiers(pressModifiers);
                                noteItemDelegate.pointerSelectionModifiersCapturedAtMs = Date.now();
                                listBarLayout.pressedNoteIndex = noteItemDelegate.index;
                                noteDeletionBridge.focusedNoteId = noteItemDelegate.noteId;
                            }
                            onTapped: function (eventPoint, button) {
                                listBarLayout.pressedNoteIndex = -1;
                                const eventModifiers = eventPoint && eventPoint.modifiers !== undefined ? eventPoint.modifiers : noteItemDelegate.pointerSelectionModifiers;
                                const selectionModifiers = listBarLayout.resolveSelectionModifiers(eventModifiers, noteItemDelegate.pointerSelectionModifiers, noteItemDelegate.pointerSelectionModifiersCapturedAtMs);
                                noteItemDelegate.pointerSelectionModifiers = Qt.NoModifier;
                                noteItemDelegate.pointerSelectionModifiersCapturedAtMs = 0;
                                listBarLayout.requestNoteSelection(noteItemDelegate.index, noteItemDelegate.noteId, selectionModifiers);
                            }
                        }
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            enabled: !noteItemDelegate.pointerDragRequiresLongPress

                            onTapped: function (eventPoint, button) {
                                listBarLayout.pressedNoteIndex = -1;
                                listBarLayout.openNoteContextMenuFromPointer(noteItemDelegate, eventPoint.position.x, eventPoint.position.y, "rightClick");
                            }
                        }
                    }

                    Keys.onPressed: function (event) {
                        if (event.key !== Qt.Key_Backspace && event.key !== Qt.Key_Delete)
                            return;
                        if (!listBarLayout.deleteCurrentNote())
                            return;
                        event.accepted = true;
                    }
                    onContentHeightChanged: listBarLayout.settleNoteListViewport()
                    onContentYChanged: {
                        if (listBarLayout.syncingNoteListViewport || listBarLayout.noteListViewportRestorePending)
                            return;
                        if (listBarLayout.noteListKineticViewportEnabled)
                            return;
                        listBarLayout.applyNoteListViewportStep(noteListView.contentY);
                    }
                    onCurrentIndexChanged: {
                        console.log("[whatson:qml][ListBarLayout][noteListView.currentIndexChanged] currentIndex=" + noteListView.currentIndex + " authoritative=" + listBarLayout.currentIndexChangeIsAuthoritative(noteListView.currentIndex) + " modelCurrentIndex=" + listBarLayout.currentIndexFromModel());
                        listBarLayout.pressedNoteIndex = -1;
                        if (!listBarLayout.currentIndexChangeIsAuthoritative(noteListView.currentIndex)) {
                            console.log("[whatson:qml][ListBarLayout][noteListView.currentIndexChanged.nonAuthoritative] currentIndex=" + noteListView.currentIndex);
                            Qt.callLater(function () {
                                listBarLayout.syncCurrentIndexFromModel();
                            });
                            return;
                        }
                        listBarLayout.pushCurrentIndexToModel(noteListView.currentIndex);
                        if (listBarLayout.selectedNoteIndices.length === 0 || !listBarLayout.noteSelectionContainsIndex(noteListView.currentIndex))
                            listBarLayout.syncSelectionFromCommittedState();
                        Qt.callLater(function () {
                            listBarLayout.syncFocusedNoteDeletionState();
                        });
                    }
                    onFlickStarted: {
                        if (!LV.Theme.mobileTarget)
                            noteListView.cancelFlick();
                    }
                    onHeightChanged: listBarLayout.settleNoteListViewport()
                    onMovementEnded: listBarLayout.settleNoteListViewport()
                }
                LV.WheelScrollGuard {
                    anchors.fill: parent
                    consumeInside: true
                    fallbackStep: listBarLayout.noteListScrollTick
                    targetFlickable: noteListView
                    visible: noteListView.visible
                }
                Item {
                    anchors.fill: parent
                    visible: !listBarLayout.noteListMode

                    LV.Label {
                        anchors.centerIn: parent
                        color: listBarLayout.hintColor
                        style: caption
                        text: "No list data"
                    }
                }
            }
        }
    }
    Connections {
        function onCurrentIndexChanged() {
            listBarLayout.syncCurrentIndexFromModel();
            if (listBarLayout.selectedNoteIndices.length === 0 || !listBarLayout.noteSelectionContainsIndex(listBarLayout.committedNoteIndex))
                listBarLayout.syncSelectionFromCommittedState();
        }
        function onModelAboutToBeReset() {
            listBarLayout.captureNoteListViewport();
        }
        function onModelReset() {
            Qt.callLater(function () {
                listBarLayout.restoreNoteListViewport();
            });
        }

        ignoreUnknownSignals: true
        target: listBarLayout.resolvedNoteListModel
    }
}
