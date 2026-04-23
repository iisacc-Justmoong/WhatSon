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
    readonly property string committedNoteId: listBarLayout.currentNoteIdFromModel()
    readonly property int committedNoteIndex: listBarLayout.normalizeCurrentIndex(listBarLayout.currentIndexFromModel())
    property string contextMenuNoteId: ""
    property var contextMenuNoteIds: []
    property int contextMenuNoteIndex: -1
    readonly property int dragCountBadgeHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(20)))
    readonly property int dragCountBadgeInset: LV.Theme.gap8
    readonly property int dragCountBadgeMinWidth: dragCountBadgeHeight
    readonly property int dragCountBadgeWidthPadding: Math.max(0, Math.round(LV.Theme.scaleMetric(10)))
    readonly property real grabbedNoteOpacity: 0.25
    readonly property bool hasNoteListModel: noteListContractBridge.hasNoteListModel
    property bool headerVisible: true
    property var hierarchyViewModel: null
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
    readonly property bool noteDeletionBatchContractAvailable: listBarLayout.noteDeletionViewModel !== null && listBarLayout.noteDeletionViewModel !== undefined && listBarLayout.noteDeletionViewModel.deleteNotesByIds !== undefined
    readonly property bool noteDeletionContractAvailable: listBarLayout.noteDeletionHandlerAvailable && (listBarLayout.resolvedSelectedNoteIds.length > 0 || noteDeletionBridge.focusedNoteAvailable)
    readonly property bool noteDeletionHandlerAvailable: listBarLayout.noteDeletionBatchContractAvailable || listBarLayout.noteDeletionSingleContractAvailable
    readonly property bool noteDeletionSingleContractAvailable: noteDeletionBridge.deleteContractAvailable || (listBarLayout.noteDeletionViewModel !== null && listBarLayout.noteDeletionViewModel !== undefined && listBarLayout.noteDeletionViewModel.deleteNoteById !== undefined)
    property var noteDeletionViewModel: null
    property bool noteDragActive: false
    property bool noteDragCanceled: false
    property var noteDropTarget: null
    readonly property bool noteFolderClearContractAvailable: listBarLayout.noteDeletionViewModel !== null && listBarLayout.noteDeletionViewModel !== undefined && (listBarLayout.noteDeletionViewModel.clearNoteFoldersByIds !== undefined || listBarLayout.noteDeletionViewModel.clearNoteFoldersById !== undefined)
    readonly property bool noteListCurrentIndexContractAvailable: listBarLayout.hasNoteListModel
                                                           && noteListContractBridge.currentIndexContractAvailable
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
    property var displayedNoteListEntries: []
    property string displayedNoteListEntriesSignature: "[]"
    property bool displayedNoteListEntriesSyncDeferred: false
    property bool displayedNoteListEntriesForceRefreshDeferred: false
    property bool displayedNoteListEntriesSyncQueued: false
    property bool displayedNoteListEntriesForceRefreshQueued: false
    property int noteListModelTransitionRevision: 0
    readonly property int noteListScrollTick: LV.Theme.gap2
    readonly property bool noteListSearchContractAvailable: listBarLayout.hasNoteListModel
                                                      && noteListContractBridge.searchContractAvailable
    property bool noteListViewportRestorePending: false
    property alias noteSelectionAnchorIndex: noteSelectionController.selectionAnchorIndex
    property color panelColor: "transparent"
    property var panelViewModelRegistry: null
    readonly property var panelViewModel: listBarLayout.panelViewModelRegistry ? listBarLayout.panelViewModelRegistry.panelViewModel("ListBarLayout") : null
    property real preservedNoteListContentY: 0
    property int pressedNoteIndex: -1
    readonly property var resolvedNoteListModel: noteListContractBridge.noteListModel
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
        console.log("[whatson:qml][ListBarLayout][activateNoteIndex] targetIndex=" + targetIndex
                    + " noteId=" + normalizedNoteId
                    + " currentIndex(before)=" + noteListView.currentIndex
                    + " modelCurrentIndex(before)=" + listBarLayout.currentIndexFromModel())
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
        if (noteListContractBridge.applySearchText(listBarLayout.searchText))
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
        if (normalizedNoteIds.length === 0 || !listBarLayout.noteDeletionViewModel)
            return false;

        if (listBarLayout.noteDeletionViewModel.clearNoteFoldersByIds !== undefined)
            return Boolean(listBarLayout.noteDeletionViewModel.clearNoteFoldersByIds(normalizedNoteIds));

        if (listBarLayout.noteDeletionViewModel.clearNoteFoldersById === undefined)
            return false;

        let clearedAny = false;
        for (let index = 0; index < normalizedNoteIds.length; ++index) {
            if (listBarLayout.noteDeletionViewModel.clearNoteFoldersById(normalizedNoteIds[index]))
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
        const bridgeCurrentIndex = Number(noteListContractBridge.currentIndex);
        if (isFinite(bridgeCurrentIndex))
            return bridgeCurrentIndex;
        if (listBarLayout.resolvedNoteListModel.currentIndex !== undefined)
            return Number(listBarLayout.resolvedNoteListModel.currentIndex);
        return -1;
    }
    function currentNoteIdFromModel() {
        const bridgeNoteId = noteListContractBridge.currentNoteId;
        const normalizedBridgeNoteId = bridgeNoteId === undefined || bridgeNoteId === null ? "" : String(bridgeNoteId).trim();
        if (normalizedBridgeNoteId.length > 0)
            return normalizedBridgeNoteId;
        if (listBarLayout.resolvedNoteListModel
                && listBarLayout.resolvedNoteListModel.currentNoteId !== undefined
                && listBarLayout.resolvedNoteListModel.currentNoteId !== null) {
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

        if (listBarLayout.noteDeletionViewModel && listBarLayout.noteDeletionViewModel.deleteNotesByIds !== undefined)
            return Boolean(listBarLayout.noteDeletionViewModel.deleteNotesByIds(normalizedNoteIds));

        let deletedAny = false;
        if (listBarLayout.noteDeletionViewModel && listBarLayout.noteDeletionViewModel.deleteNoteById !== undefined) {
            for (let index = 0; index < normalizedNoteIds.length; ++index) {
                if (listBarLayout.noteDeletionViewModel.deleteNoteById(normalizedNoteIds[index]))
                    deletedAny = true;
            }
        }

        if (!deletedAny && normalizedNoteIds.length === 1 && noteDeletionBridge.deleteContractAvailable) {
            noteDeletionBridge.focusedNoteId = normalizedNoteIds[0];
            deletedAny = noteDeletionBridge.deleteFocusedNote();
        }
        return deletedAny;
    }
    function flushDeferredDisplayedNoteListEntriesSync() {
        if (!listBarLayout.displayedNoteListEntriesSyncDeferred)
            return false;
        const forceRefresh = listBarLayout.displayedNoteListEntriesForceRefreshDeferred;
        listBarLayout.displayedNoteListEntriesSyncDeferred = false;
        listBarLayout.displayedNoteListEntriesForceRefreshDeferred = false;
        return listBarLayout.syncDisplayedNoteListEntries(forceRefresh);
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
        if (normalizedIndex < 0 || !noteListContractBridge || noteListContractBridge.readNoteIdAt === undefined)
            return "";
        return String(noteListContractBridge.readNoteIdAt(normalizedIndex) || "").trim();
    }
    function noteListEntriesSignature(entries) {
        const normalizedEntries = entries === undefined || entries === null ? [] : entries;
        try {
            return JSON.stringify(normalizedEntries);
        } catch (error) {
            return "";
        }
    }
    function noteListMaxContentY() {
        return Math.max(0, (Number(noteListView.contentHeight) || 0) - (Number(noteListView.height) || 0));
    }
    function noteListViewportTargetY(value) {
        if (listBarLayout.noteListKineticViewportEnabled)
            return listBarLayout.clampNoteListContentY(value);
        return listBarLayout.quantizedNoteListContentY(value);
    }
    function readDisplayedNoteListEntriesFromModel(noteListModelOverride) {
        const targetModel = noteListModelOverride !== undefined
                ? noteListModelOverride
                : listBarLayout.resolvedNoteListModel;
        if (!targetModel || !noteListContractBridge)
            return [];
        let rows = [];
        if (noteListContractBridge.readAllRowsForModel !== undefined)
            rows = noteListContractBridge.readAllRowsForModel(targetModel);
        else if (targetModel === listBarLayout.resolvedNoteListModel && noteListContractBridge.readAllRows !== undefined)
            rows = noteListContractBridge.readAllRows();
        if (Array.isArray(rows))
            return rows;
        if (rows && rows.length !== undefined)
            return Array.prototype.slice.call(rows);
        return [];
    }
    function noteSelectionContainsIndex(index) {
        return noteSelectionController.noteSelectionContainsIndex(index);
    }
    function openNoteContextMenu(delegateItem, localX, localY) {
        if (!delegateItem)
            return;
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(delegateItem.index);
        const normalizedNoteId = delegateItem.noteId !== undefined && delegateItem.noteId !== null ? String(delegateItem.noteId).trim() : "";
        if (normalizedNoteId.length === 0)
            return;
        const actionNoteIds = listBarLayout.selectedNoteIdsForDelegateAction(normalizedIndex, normalizedNoteId);
        if (actionNoteIds.length === 0)
            return;
        listBarLayout.contextMenuNoteIndex = normalizedIndex;
        listBarLayout.contextMenuNoteId = normalizedNoteId;
        listBarLayout.contextMenuNoteIds = actionNoteIds;
        noteDeletionBridge.focusedNoteId = normalizedNoteId;
        noteContextMenu.openFor(delegateItem, Number(localX) || 0, Number(localY) || 0);
    }
    function pushCurrentIndexToModel(index) {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return;
        const normalizedIndex = Number(index);
        if (!isFinite(normalizedIndex))
            return;
        if (noteListContractBridge.pushCurrentIndex(Math.max(-1, Math.floor(normalizedIndex))))
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
    function resetDisplayedNoteListEntries() {
        listBarLayout.displayedNoteListEntries = [];
        listBarLayout.displayedNoteListEntriesSignature = "[]";
        listBarLayout.displayedNoteListEntriesSyncDeferred = false;
        listBarLayout.displayedNoteListEntriesForceRefreshDeferred = false;
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
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
    function syncDisplayedNoteListEntriesForModel(noteListModel, forceRefresh) {
        const nextEntries = listBarLayout.readDisplayedNoteListEntriesFromModel(noteListModel);
        const nextSignature = listBarLayout.noteListEntriesSignature(nextEntries);
        if (!Boolean(forceRefresh) && nextSignature === listBarLayout.displayedNoteListEntriesSignature)
            return false;
        listBarLayout.displayedNoteListEntries = nextEntries;
        listBarLayout.displayedNoteListEntriesSignature = nextSignature;
        return true;
    }
    function syncDisplayedNoteListEntries(forceRefresh) {
        return listBarLayout.syncDisplayedNoteListEntriesForModel(
                    listBarLayout.resolvedNoteListModel,
                    forceRefresh);
    }
    function requestDisplayedNoteListEntriesSync(forceRefresh) {
        const requestedForceRefresh = Boolean(forceRefresh);
        if (listBarLayout.noteDragActive) {
            listBarLayout.displayedNoteListEntriesSyncDeferred = true;
            listBarLayout.displayedNoteListEntriesForceRefreshDeferred = listBarLayout.displayedNoteListEntriesForceRefreshDeferred || requestedForceRefresh;
            return false;
        }
        return listBarLayout.syncDisplayedNoteListEntries(requestedForceRefresh);
    }
    function scheduleDisplayedNoteListEntriesSync(forceRefresh) {
        const requestedForceRefresh = Boolean(forceRefresh);
        listBarLayout.displayedNoteListEntriesForceRefreshQueued = listBarLayout.displayedNoteListEntriesForceRefreshQueued || requestedForceRefresh;
        if (listBarLayout.displayedNoteListEntriesSyncQueued)
            return false;
        listBarLayout.displayedNoteListEntriesSyncQueued = true;
        Qt.callLater(function () {
            const scheduledForceRefresh = listBarLayout.displayedNoteListEntriesForceRefreshQueued;
            listBarLayout.displayedNoteListEntriesSyncQueued = false;
            listBarLayout.displayedNoteListEntriesForceRefreshQueued = false;
            listBarLayout.requestDisplayedNoteListEntriesSync(scheduledForceRefresh);
        });
        return true;
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
            listBarLayout.syncDisplayedNoteListEntriesForModel(expectedModel, true);
            listBarLayout.syncCurrentIndexFromModel();
            listBarLayout.syncSelectionFromCommittedState();
            listBarLayout.syncFocusedNoteDeletionState();
        });
    }
    function syncFocusedNoteDeletionState() {
        const bridgeNoteId = noteListContractBridge.currentNoteId;
        if (bridgeNoteId.length > 0) {
            noteDeletionBridge.focusedNoteId = bridgeNoteId;
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
        listBarLayout.syncDisplayedNoteListEntries(true);
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
        listBarLayout.displayedNoteListEntriesSyncQueued = false;
        listBarLayout.displayedNoteListEntriesForceRefreshQueued = false;
        listBarLayout.resetDisplayedNoteListEntries();
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
        listBarLayout.syncDisplayedNoteListEntriesForModel(listBarLayout.resolvedNoteListModel, true);
        listBarLayout.scheduleNoteListModelTransitionSync();
    }
    onSearchTextChanged: applySearchTextToModel()
    onNoteDragActiveChanged: {
        if (listBarLayout.noteDragActive)
            return;
        listBarLayout.flushDeferredDisplayedNoteListEntriesSync();
    }

    QtObject {
        id: noteSelectionState

        property int pendingIndex: -1
        property int requestRevision: 0
    }
    ListBarSelectionController {
        id: noteSelectionController

        view: listBarLayout
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

        deletionTarget: listBarLayout.noteDeletionViewModel
        noteListModel: listBarLayout.resolvedNoteListModel
    }
    NoteListModelContractBridge {
        id: noteListContractBridge

        hierarchyViewModel: listBarLayout.hierarchyViewModel
        noteListModel: listBarLayout.noteListModel
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
            width: Math.max(listBarLayout.dragCountBadgeMinWidth,
                            dragCountLabel.implicitWidth + listBarLayout.dragCountBadgeWidthPadding)

            LV.Label {
                id: dragCountLabel

                anchors.centerIn: parent
                color: "white"
                style: caption
                text: String(noteDragPreviewState.noteIds.length)
            }
        }
    }
    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

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
                    model: listBarLayout.displayedNoteListEntries
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
                                    listBarLayout.openNoteContextMenu(noteItemDelegate, mouse.x, mouse.y);
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
                                listBarLayout.openNoteContextMenu(noteItemDelegate, eventPoint.position.x, eventPoint.position.y);
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
                        console.log("[whatson:qml][ListBarLayout][noteListView.currentIndexChanged] currentIndex="
                                    + noteListView.currentIndex
                                    + " authoritative=" + listBarLayout.currentIndexChangeIsAuthoritative(noteListView.currentIndex)
                                    + " modelCurrentIndex=" + listBarLayout.currentIndexFromModel())
                        listBarLayout.pressedNoteIndex = -1;
                        if (!listBarLayout.currentIndexChangeIsAuthoritative(noteListView.currentIndex)) {
                            console.log("[whatson:qml][ListBarLayout][noteListView.currentIndexChanged.nonAuthoritative] currentIndex="
                                        + noteListView.currentIndex)
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
        function onItemsChanged() {
            listBarLayout.scheduleDisplayedNoteListEntriesSync(false);
        }
        function onModelAboutToBeReset() {
            listBarLayout.captureNoteListViewport();
        }
        function onModelReset() {
            Qt.callLater(function () {
                listBarLayout.restoreNoteListViewport();
            });
        }
        function onRowsInserted() {
            listBarLayout.scheduleDisplayedNoteListEntriesSync(false);
        }
        function onRowsMoved() {
            listBarLayout.scheduleDisplayedNoteListEntriesSync(false);
        }
        function onRowsRemoved() {
            listBarLayout.scheduleDisplayedNoteListEntriesSync(false);
        }
        function onDataChanged() {
            listBarLayout.scheduleDisplayedNoteListEntriesSync(false);
        }
        function onLayoutChanged() {
            listBarLayout.scheduleDisplayedNoteListEntriesSync(false);
        }
        function onItemCountChanged() {
            listBarLayout.scheduleDisplayedNoteListEntriesSync(false);
        }

        ignoreUnknownSignals: true
        target: listBarLayout.resolvedNoteListModel
    }
}
