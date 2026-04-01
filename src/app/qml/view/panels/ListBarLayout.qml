pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Rectangle {
    id: listBarLayout

    property int activeToolbarIndex: 0
    readonly property bool hasNoteListModel: listBarLayout.noteListModel !== null && listBarLayout.noteListModel !== undefined
    property bool headerVisible: true
    property color hintColor: LV.Theme.descriptionColor
    property string contextMenuNoteId: ""
    readonly property bool noteDeletionContractAvailable: noteDeletionBridge.deleteContractAvailable && noteDeletionBridge.focusedNoteAvailable
    property var noteDeletionViewModel: null
    property bool noteDragActive: false
    property bool noteDragCanceled: false
    property var noteDropTarget: null
    readonly property bool noteContextMenuNoteAvailable: listBarLayout.contextMenuNoteId.trim().length > 0
    readonly property bool noteFolderClearContractAvailable: listBarLayout.noteDeletionViewModel !== null
        && listBarLayout.noteDeletionViewModel !== undefined
        && listBarLayout.noteDeletionViewModel.clearNoteFoldersById !== undefined
    readonly property bool noteListCurrentIndexContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.noteListModel.currentIndex !== undefined || listBarLayout.noteListModel.setCurrentIndex !== undefined)
    readonly property bool noteListMode: listBarLayout.hasNoteListModel
    readonly property bool resourceListMode: listBarLayout.noteListMode
        && listBarLayout.resolvedNoteListModel !== null
        && listBarLayout.resolvedNoteListModel !== undefined
        && listBarLayout.resolvedNoteListModel.currentResourceEntry !== undefined
    readonly property var noteContextMenuItems: [
        {
            "label": "Delete note",
            "enabled": listBarLayout.noteDeletionContractAvailable && listBarLayout.noteContextMenuNoteAvailable,
            "eventName": "note.delete"
        },
        {
            "label": "Clear all folders",
            "enabled": listBarLayout.noteFolderClearContractAvailable && listBarLayout.noteContextMenuNoteAvailable,
            "eventName": "note.clearAllFolders"
        }
    ]
    property var noteListModel: null
    readonly property bool noteListSearchContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.noteListModel.searchText !== undefined || listBarLayout.noteListModel.setSearchText !== undefined)
    readonly property int mobileNoteDragHoldInterval: 1000
    property color panelColor: "transparent"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ListBarLayout") : null
    property int pressedNoteIndex: -1
    property real preservedNoteListContentY: 0
    readonly property var resolvedNoteListModel: listBarLayout.noteListMode ? listBarLayout.noteListModel : null
    readonly property int committedNoteIndex: listBarLayout.normalizeCurrentIndex(
                                                  listBarLayout.currentIndexFromModel())
    readonly property real grabbedNoteOpacity: 0.25
    readonly property int noteListFlickDeceleration: 1000000
    property bool noteListViewportRestorePending: false
    readonly property int noteListScrollTick: LV.Theme.gap2
    property string searchText: ""
    property bool syncingNoteListViewport: false
    property bool syncingCurrentIndexFromModel: false
    readonly property bool useInternalNoteDrag: !LV.Theme.mobileTarget

    signal noteActivated(int index, string noteId)
    signal viewHookRequested

    QtObject {
        id: noteSelectionState

        property int pendingIndex: -1
        property int requestRevision: 0
    }
    QtObject {
        id: noteDragPreviewState

        property var delegateItem: null
        property real hotSpotX: 0
        property real hotSpotY: 0
        property real x: 0
        property real y: 0
    }

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
    function applySearchTextToModel() {
        if (!listBarLayout.noteListMode || !listBarLayout.noteListSearchContractAvailable)
            return;
        if (noteListContractBridge.applySearchText(listBarLayout.searchText))
            return;

        if (listBarLayout.noteListModel.searchText !== undefined) {
            listBarLayout.noteListModel.searchText = listBarLayout.searchText;
            return;
        }
        if (listBarLayout.noteListModel.setSearchText !== undefined)
            listBarLayout.noteListModel.setSearchText(listBarLayout.searchText);
    }
    function beginNoteDragPreview(delegateItem, hotSpotX, hotSpotY) {
        if (!delegateItem)
            return;
        noteDragPreviewState.delegateItem = delegateItem;
        noteDragPreviewState.hotSpotX = Math.max(0, Number(hotSpotX) || 0);
        noteDragPreviewState.hotSpotY = Math.max(0, Number(hotSpotY) || 0);
        listBarLayout.updateNoteDragPreviewPosition(delegateItem, hotSpotX, hotSpotY);
    }
    function clearNoteDragPreview(delegateItem) {
        if (delegateItem && noteDragPreviewState.delegateItem !== delegateItem)
            return;
        noteDragPreviewState.delegateItem = null;
        noteDragPreviewState.hotSpotX = 0;
        noteDragPreviewState.hotSpotY = 0;
        noteDragPreviewState.x = 0;
        noteDragPreviewState.y = 0;
    }
    function commitInternalNoteDrop(delegateItem, localX, localY) {
        if (!listBarLayout.useInternalNoteDrag || !delegateItem || !listBarLayout.noteDropTarget)
            return false;
        if (listBarLayout.noteDropTarget.commitNoteDropAtPosition === undefined)
            return false;
        const noteId = delegateItem.noteId !== undefined && delegateItem.noteId !== null ? String(delegateItem.noteId).trim() : "";
        if (noteId.length === 0)
            return false;
        const mappedPoint = delegateItem.mapToItem(listBarLayout.noteDropTarget, Number(localX) || 0, Number(localY) || 0);
        return Boolean(listBarLayout.noteDropTarget.commitNoteDropAtPosition(
                           mappedPoint.x,
                           mappedPoint.y,
                           noteId,
                           listBarLayout.noteDropTarget));
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
    }
    function currentIndexFromModel() {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return -1;
        const bridgeCurrentIndex = Number(noteListContractBridge.readCurrentIndex());
        if (isFinite(bridgeCurrentIndex))
            return bridgeCurrentIndex;
        if (listBarLayout.noteListModel.currentIndex !== undefined)
            return Number(listBarLayout.noteListModel.currentIndex);
        return -1;
    }
    function clearContextMenuNoteFolders() {
        if (!listBarLayout.noteFolderClearContractAvailable || !listBarLayout.noteContextMenuNoteAvailable)
            return false;
        const normalizedNoteId = String(listBarLayout.contextMenuNoteId).trim();
        if (normalizedNoteId.length === 0)
            return false;
        noteDeletionBridge.focusedNoteId = normalizedNoteId;
        const cleared = Boolean(listBarLayout.noteDeletionViewModel.clearNoteFoldersById(normalizedNoteId));
        if (cleared)
            noteListView.forceActiveFocus();
        return cleared;
    }
    function deleteCurrentNote() {
        if (!listBarLayout.noteDeletionContractAvailable)
            return false;
        const deleted = noteDeletionBridge.deleteFocusedNote();
        if (deleted)
            noteListView.forceActiveFocus();

        return deleted;
    }
    function deleteContextMenuNote() {
        if (!listBarLayout.noteContextMenuNoteAvailable)
            return false;
        noteDeletionBridge.focusedNoteId = String(listBarLayout.contextMenuNoteId).trim();
        return listBarLayout.deleteCurrentNote();
    }
    function normalizeCurrentIndex(index) {
        const numericIndex = Number(index);
        if (!isFinite(numericIndex))
            return -1;
        return Math.max(-1, Math.floor(numericIndex));
    }
    function currentIndexChangeIsAuthoritative(index) {
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(index);
        if (listBarLayout.syncingCurrentIndexFromModel)
            return true;
        if (noteSelectionState.pendingIndex === normalizedIndex)
            return true;
        return listBarLayout.currentIndexFromModel() === normalizedIndex;
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
    function noteListMaxContentY() {
        return Math.max(
                    0,
                    (Number(noteListView.contentHeight) || 0)
                    - (Number(noteListView.height) || 0));
    }
    function clampNoteListContentY(value) {
        const numericValue = Number(value);
        if (!isFinite(numericValue))
            return 0;
        return Math.max(0, Math.min(listBarLayout.noteListMaxContentY(), numericValue));
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
    function captureNoteListViewport() {
        if (!listBarLayout.noteListMode)
            return;
        listBarLayout.preservedNoteListContentY = listBarLayout.quantizedNoteListContentY(noteListView.contentY);
        listBarLayout.noteListViewportRestorePending = true;
    }
    function applyNoteListViewportStep(contentY) {
        const targetY = listBarLayout.quantizedNoteListContentY(contentY);
        const previousY = Number(noteListView.contentY) || 0;
        if (Math.abs(targetY - previousY) <= 0.001)
            return;
        listBarLayout.syncingNoteListViewport = true;
        noteListView.contentY = targetY;
        listBarLayout.syncingNoteListViewport = false;
    }
    function restoreNoteListViewport() {
        if (!listBarLayout.noteListViewportRestorePending)
            return;
        listBarLayout.noteListViewportRestorePending = false;
        listBarLayout.applyNoteListViewportStep(listBarLayout.preservedNoteListContentY);
    }
    function settleNoteListViewport() {
        if (listBarLayout.noteListViewportRestorePending) {
            listBarLayout.restoreNoteListViewport();
            return;
        }
        listBarLayout.applyNoteListViewportStep(noteListView.contentY);
    }
    function pushCurrentIndexToModel(index) {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return;
        const normalizedIndex = Number(index);
        if (!isFinite(normalizedIndex))
            return;
        if (noteListContractBridge.pushCurrentIndex(Math.max(-1, Math.floor(normalizedIndex))))
            return;
        if (listBarLayout.noteListModel.currentIndex !== undefined) {
            if (Number(listBarLayout.noteListModel.currentIndex) === normalizedIndex)
                return;
            listBarLayout.noteListModel.currentIndex = index;
            return;
        }
        if (listBarLayout.noteListModel.setCurrentIndex !== undefined)
            listBarLayout.noteListModel.setCurrentIndex(normalizedIndex);
    }
    function openNoteContextMenu(delegateItem, localX, localY) {
        if (!delegateItem)
            return;
        const normalizedNoteId = delegateItem.noteId !== undefined && delegateItem.noteId !== null ? String(delegateItem.noteId).trim() : "";
        if (normalizedNoteId.length === 0)
            return;
        listBarLayout.contextMenuNoteId = normalizedNoteId;
        noteDeletionBridge.focusedNoteId = normalizedNoteId;
        noteContextMenu.openFor(delegateItem, Number(localX) || 0, Number(localY) || 0);
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function syncCurrentIndexFromModel() {
        const normalizedIndex = listBarLayout.normalizeCurrentIndex(
                    listBarLayout.currentIndexFromModel());
        if (noteListView.currentIndex !== normalizedIndex)
            listBarLayout.syncingCurrentIndexFromModel = true;
        if (noteListView.currentIndex !== normalizedIndex)
            noteListView.currentIndex = normalizedIndex;
        listBarLayout.syncingCurrentIndexFromModel = false;
    }
    function syncFocusedNoteDeletionState() {
        const bridgeNoteId = noteListContractBridge.readCurrentNoteId();
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
    function updateNoteDragPreviewPosition(delegateItem, localX, localY) {
        if (!delegateItem || noteDragPreviewState.delegateItem !== delegateItem)
            return;
        const previewParent = noteDragPreview.parent ? noteDragPreview.parent : listBarLayout;
        const mappedPoint = delegateItem.mapToItem(previewParent, Number(localX) || 0, Number(localY) || 0);
        noteDragPreviewState.x = Math.round((Number(mappedPoint.x) || 0) - noteDragPreviewState.hotSpotX);
        noteDragPreviewState.y = Math.round((Number(mappedPoint.y) || 0) - noteDragPreviewState.hotSpotY);
    }
    function updateInternalNoteDropPreview(delegateItem, localX, localY) {
        if (!listBarLayout.useInternalNoteDrag || !delegateItem || !listBarLayout.noteDropTarget)
            return false;
        if (listBarLayout.noteDropTarget.updateNoteDropPreviewAtPosition === undefined)
            return false;
        const noteId = delegateItem.noteId !== undefined && delegateItem.noteId !== null ? String(delegateItem.noteId).trim() : "";
        if (noteId.length === 0)
            return false;
        const mappedPoint = delegateItem.mapToItem(listBarLayout.noteDropTarget, Number(localX) || 0, Number(localY) || 0);
        return Boolean(listBarLayout.noteDropTarget.updateNoteDropPreviewAtPosition(
                           mappedPoint.x,
                           mappedPoint.y,
                           noteId,
                           listBarLayout.noteDropTarget));
    }

    color: panelColor

    Component.onCompleted: {
        listBarLayout.syncCurrentIndexFromModel();
        listBarLayout.syncFocusedNoteDeletionState();
    }
    onNoteListModeChanged: {
        if (noteContextMenu.opened)
            noteContextMenu.close();
        applySearchTextToModel();
    }
    onNoteListModelChanged: {
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
        listBarLayout.pressedNoteIndex = -1;
        noteSelectionState.requestRevision += 1;
        listBarLayout.applySearchTextToModel();
        listBarLayout.syncCurrentIndexFromModel();
        listBarLayout.syncFocusedNoteDeletionState();
    }
    onSearchTextChanged: applySearchTextToModel()

    FocusedNoteDeletionBridge {
        id: noteDeletionBridge

        deletionTarget: listBarLayout.noteDeletionViewModel
        noteListModel: listBarLayout.resolvedNoteListModel
    }
    NoteListModelContractBridge {
        id: noteListContractBridge

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
        onItemEventTriggered: function(eventName, payload, index, item) {
            if (eventName === "note.delete")
                listBarLayout.deleteContextMenuNote();
            else if (eventName === "note.clearAllFolders")
                listBarLayout.clearContextMenuNoteFolders();
        }
    }
    Item {
        id: noteDragPreview

        parent: Controls.Overlay.overlay ? Controls.Overlay.overlay : listBarLayout
        visible: listBarLayout.noteDragActive && noteDragPreviewState.delegateItem !== null
        z: 1000

        height: listBarLayout.resourceListMode ? resourceDragPreviewCard.implicitHeight : noteDragPreviewCard.implicitHeight
        width: noteDragPreviewState.delegateItem && noteDragPreviewState.delegateItem.width !== undefined ? Number(noteDragPreviewState.delegateItem.width) || implicitWidth : implicitWidth
        x: noteDragPreviewState.x
        y: noteDragPreviewState.y

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
    }
    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                id: topToolbar

                Layout.fillWidth: true
                Layout.preferredHeight: listBarLayout.headerVisible ? 24 : 0
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
                    anchors.margins: 2
                    boundsBehavior: Flickable.StopAtBounds
                    boundsMovement: Flickable.StopAtBounds
                    clip: true
                    flickDeceleration: listBarLayout.noteListFlickDeceleration
                    interactive: contentHeight > height && !listBarLayout.noteDragActive
                    maximumFlickVelocity: listBarLayout.noteListScrollTick
                    model: listBarLayout.resolvedNoteListModel
                    pixelAligned: true
                    spacing: 2
                    synchronousDrag: true
                    visible: listBarLayout.noteListMode

                    onContentHeightChanged: listBarLayout.settleNoteListViewport()
                    onContentYChanged: {
                        if (listBarLayout.syncingNoteListViewport || listBarLayout.noteListViewportRestorePending)
                            return;
                        listBarLayout.applyNoteListViewportStep(noteListView.contentY);
                    }
                    onFlickStarted: noteListView.cancelFlick()
                    onHeightChanged: listBarLayout.settleNoteListViewport()
                    onMovementEnded: listBarLayout.settleNoteListViewport()

                    Keys.onPressed: function (event) {
                        if (event.key !== Qt.Key_Backspace && event.key !== Qt.Key_Delete)
                            return;
                        if (!listBarLayout.deleteCurrentNote())
                            return;
                        event.accepted = true;
                    }

                    delegate: Item {
                        id: noteItemDelegate

                        required property string bookmarkColor
                        required property bool bookmarked
                        required property string displayDate
                        required property var folders
                        required property bool image
                        required property var imageSource
                        required property int index
                        required property string noteId
                        required property string primaryText
                        required property var tags
                        property real dragHotSpotX: width * 0.5
                        property real dragHotSpotY: height * 0.5
                        readonly property bool immediatePointerDragEnabled: !noteItemDelegate.pointerDragRequiresLongPress
                        property bool mobileLongPressPendingContextMenu: false
                        property bool mobilePointerDragging: false
                        property bool mobileSuppressNextClick: false
                        readonly property bool pointerDragActive: noteDragHandler.active || noteItemDelegate.mobilePointerDragging
                        readonly property bool pointerDragRequiresLongPress: LV.Theme.mobileTarget

                        Drag.active: noteItemDelegate.pointerDragActive
                        Drag.dragType: listBarLayout.useInternalNoteDrag ? Drag.Internal : Drag.Automatic
                        Drag.hotSpot.x: noteItemDelegate.dragHotSpotX
                        Drag.hotSpot.y: noteItemDelegate.dragHotSpotY
                        Drag.keys: ["whatson.library.note"]
                        Drag.mimeData: ({
                                "application/x-whatson-note-id": noteItemDelegate.noteId,
                                "text/plain": noteItemDelegate.noteId
                            })
                        Drag.source: noteItemDelegate
                        Drag.supportedActions: Qt.CopyAction
                        // Legacy syntax-guard anchor: height: noteCard.implicitHeight
                        height: listBarLayout.resourceListMode ? resourceCard.implicitHeight : noteCard.implicitHeight
                        width: ListView.view ? ListView.view.width : listBarLayout.width

                        NoteListItem {
                            id: noteCard

                            active: listBarLayout.committedNoteIndex === noteItemDelegate.index
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

                            active: listBarLayout.committedNoteIndex === noteItemDelegate.index
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
                                    listBarLayout.beginNoteDragPreview(
                                                noteItemDelegate,
                                                noteItemDelegate.dragHotSpotX,
                                                noteItemDelegate.dragHotSpotY);
                                    listBarLayout.updateInternalNoteDropPreview(
                                                noteItemDelegate,
                                                noteDragHandler.centroid.position.x,
                                                noteDragHandler.centroid.position.y);
                                } else {
                                    if (!listBarLayout.noteDragCanceled)
                                        listBarLayout.commitInternalNoteDrop(
                                                    noteItemDelegate,
                                                    noteDragHandler.centroid.position.x,
                                                    noteDragHandler.centroid.position.y);
                                    listBarLayout.clearInternalNoteDropPreview();
                                    listBarLayout.clearNoteDragPreview(noteItemDelegate);
                                    listBarLayout.noteDragCanceled = false;
                                }
                                if (active && listBarLayout.pressedNoteIndex === noteItemDelegate.index)
                                    listBarLayout.pressedNoteIndex = -1;
                            }
                            onCentroidChanged: {
                                if (!active)
                                    return;
                                listBarLayout.updateNoteDragPreviewPosition(
                                            noteItemDelegate,
                                            noteDragHandler.centroid.position.x,
                                            noteDragHandler.centroid.position.y);
                                listBarLayout.updateInternalNoteDropPreview(
                                            noteItemDelegate,
                                            noteDragHandler.centroid.position.x,
                                            noteDragHandler.centroid.position.y);
                            }
                            onCanceled: {
                                listBarLayout.noteDragCanceled = true;
                                listBarLayout.noteDragActive = false;
                                listBarLayout.clearInternalNoteDropPreview();
                                listBarLayout.clearNoteDragPreview(noteItemDelegate);
                            }
                        }
                        MouseArea {
                            id: mobileLongPressDragArea

                            anchors.fill: parent
                            enabled: noteItemDelegate.pointerDragRequiresLongPress
                            acceptedButtons: Qt.LeftButton
                            hoverEnabled: false
                            preventStealing: noteItemDelegate.mobilePointerDragging
                            pressAndHoldInterval: listBarLayout.mobileNoteDragHoldInterval

                            onPressed: function(mouse) {
                                noteItemDelegate.dragHotSpotX = Number(mouse.x) || width * 0.5;
                                noteItemDelegate.dragHotSpotY = Number(mouse.y) || height * 0.5;
                                noteItemDelegate.mobileLongPressPendingContextMenu = false;
                                noteItemDelegate.mobilePointerDragging = false;
                                noteItemDelegate.mobileSuppressNextClick = false;
                                listBarLayout.pressedNoteIndex = noteItemDelegate.index;
                                noteDeletionBridge.focusedNoteId = noteItemDelegate.noteId;
                            }
                            onClicked: function(mouse) {
                                if (noteItemDelegate.mobileSuppressNextClick) {
                                    noteItemDelegate.mobileSuppressNextClick = false;
                                    return;
                                }
                                if (noteItemDelegate.mobilePointerDragging)
                                    return;
                                listBarLayout.pressedNoteIndex = -1;
                                listBarLayout.activateNoteIndex(noteItemDelegate.index, noteCard.noteId);
                            }
                            onPressAndHold: function(mouse) {
                                noteItemDelegate.mobileLongPressPendingContextMenu = true;
                                noteItemDelegate.mobileSuppressNextClick = true;
                            }
                            onPositionChanged: function(mouse) {
                                if (noteItemDelegate.mobilePointerDragging) {
                                    listBarLayout.updateNoteDragPreviewPosition(
                                                noteItemDelegate,
                                                mouse.x,
                                                mouse.y);
                                    listBarLayout.updateInternalNoteDropPreview(
                                                noteItemDelegate,
                                                mouse.x,
                                                mouse.y);
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
                                listBarLayout.beginNoteDragPreview(
                                            noteItemDelegate,
                                            noteItemDelegate.dragHotSpotX,
                                            noteItemDelegate.dragHotSpotY);
                                listBarLayout.updateNoteDragPreviewPosition(
                                            noteItemDelegate,
                                            mouse.x,
                                            mouse.y);
                                listBarLayout.updateInternalNoteDropPreview(
                                            noteItemDelegate,
                                            mouse.x,
                                            mouse.y);
                                if (listBarLayout.pressedNoteIndex === noteItemDelegate.index)
                                    listBarLayout.pressedNoteIndex = -1;
                            }
                            onReleased: function(mouse) {
                                const dragging = noteItemDelegate.mobilePointerDragging;
                                const openContextMenu = noteItemDelegate.mobileLongPressPendingContextMenu && !dragging;
                                noteItemDelegate.mobileLongPressPendingContextMenu = false;
                                noteItemDelegate.mobilePointerDragging = false;
                                listBarLayout.pressedNoteIndex = -1;
                                if (openContextMenu) {
                                    listBarLayout.openNoteContextMenu(
                                                noteItemDelegate,
                                                mouse.x,
                                                mouse.y);
                                    return;
                                }
                                if (!dragging)
                                    return;
                                listBarLayout.noteDragActive = false;
                                listBarLayout.clearInternalNoteDropPreview();
                                listBarLayout.clearNoteDragPreview(noteItemDelegate);
                                listBarLayout.noteDragCanceled = false;
                            }
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
                        }
                        TapHandler {
                            enabled: noteItemDelegate.immediatePointerDragEnabled
                            acceptedButtons: Qt.LeftButton
                            gesturePolicy: TapHandler.DragThreshold
                            grabPermissions: PointerHandler.ApprovesTakeOverByAnything

                            onPressedChanged: {
                                if (!pressed) {
                                    if (listBarLayout.pressedNoteIndex === noteItemDelegate.index && !noteDragHandler.active)
                                        listBarLayout.pressedNoteIndex = -1;
                                    return;
                                }
                                listBarLayout.pressedNoteIndex = noteItemDelegate.index;
                                noteDeletionBridge.focusedNoteId = noteItemDelegate.noteId;
                            }
                            onTapped: {
                                listBarLayout.pressedNoteIndex = -1;
                                listBarLayout.activateNoteIndex(noteItemDelegate.index, noteCard.noteId);
                            }
                        }
                        TapHandler {
                            enabled: !noteItemDelegate.pointerDragRequiresLongPress
                            acceptedButtons: Qt.RightButton

                            onTapped: function(eventPoint, button) {
                                listBarLayout.pressedNoteIndex = -1;
                                listBarLayout.openNoteContextMenu(
                                            noteItemDelegate,
                                            eventPoint.position.x,
                                            eventPoint.position.y);
                            }
                        }
                    }

                    onCurrentIndexChanged: {
                        listBarLayout.pressedNoteIndex = -1;
                        if (!listBarLayout.currentIndexChangeIsAuthoritative(noteListView.currentIndex)) {
                            Qt.callLater(function () {
                                listBarLayout.syncCurrentIndexFromModel();
                            });
                            return;
                        }
                        listBarLayout.pushCurrentIndexToModel(noteListView.currentIndex);
                        Qt.callLater(function () {
                            listBarLayout.syncFocusedNoteDeletionState();
                        });
                    }
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
        function onModelAboutToBeReset() {
            listBarLayout.captureNoteListViewport();
        }

        function onModelReset() {
            Qt.callLater(function () {
                listBarLayout.restoreNoteListViewport();
            });
        }

        function onCurrentIndexChanged() {
            listBarLayout.syncCurrentIndexFromModel();
        }

        ignoreUnknownSignals: true
        target: listBarLayout.resolvedNoteListModel
    }
}
