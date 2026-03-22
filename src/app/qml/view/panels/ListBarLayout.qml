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
    readonly property bool noteDeletionContractAvailable: noteDeletionBridge.deleteContractAvailable && noteDeletionBridge.focusedNoteAvailable
    property var noteDeletionViewModel: null
    property bool noteDragActive: false
    property var noteDragPreviewDelegate: null
    property real noteDragPreviewHotSpotX: 0
    property real noteDragPreviewHotSpotY: 0
    property real noteDragPreviewX: 0
    property real noteDragPreviewY: 0
    property bool notePointerPressed: false
    readonly property bool noteListCurrentIndexContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.noteListModel.currentIndex !== undefined || listBarLayout.noteListModel.setCurrentIndex !== undefined)
    readonly property bool noteListMode: activeToolbarIndex === 0 || activeToolbarIndex === 2
    property var noteListModel: null
    readonly property bool noteListSearchContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.noteListModel.searchText !== undefined || listBarLayout.noteListModel.setSearchText !== undefined)
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ListBarLayout") : null
    property int pendingSelectionIndex: -1
    property int pressedNoteIndex: -1
    readonly property var resolvedNoteListModel: listBarLayout.noteListMode ? listBarLayout.noteListModel : null
    property string searchText: ""
    property int selectionRequestRevision: 0

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
        listBarLayout.pendingSelectionIndex = targetIndex;
        listBarLayout.selectionRequestRevision += 1;
        const requestRevision = listBarLayout.selectionRequestRevision;
        if (noteListView.currentIndex !== targetIndex)
            noteListView.currentIndex = targetIndex;
        listBarLayout.pushCurrentIndexToModel(targetIndex);
        noteListView.forceActiveFocus();
        listBarLayout.noteActivated(targetIndex, normalizedNoteId);
        Qt.callLater(function () {
            if (listBarLayout.selectionRequestRevision !== requestRevision)
                return;
            if (listBarLayout.pendingSelectionIndex !== targetIndex)
                return;
            if (noteListView.currentIndex !== targetIndex)
                noteListView.currentIndex = targetIndex;
            if (listBarLayout.currentIndexFromModel() !== targetIndex)
                listBarLayout.pushCurrentIndexToModel(targetIndex);
            if (normalizedNoteId.length > 0)
                noteDeletionBridge.focusedNoteId = normalizedNoteId;
            listBarLayout.pendingSelectionIndex = -1;
        });
    }
    function applySearchTextToModel() {
        if (!listBarLayout.noteListMode || !listBarLayout.noteListSearchContractAvailable)
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
        listBarLayout.noteDragPreviewDelegate = delegateItem;
        listBarLayout.noteDragPreviewHotSpotX = Math.max(0, Number(hotSpotX) || 0);
        listBarLayout.noteDragPreviewHotSpotY = Math.max(0, Number(hotSpotY) || 0);
        listBarLayout.updateNoteDragPreviewPosition(delegateItem, hotSpotX, hotSpotY);
    }
    function clearNoteDragPreview(delegateItem) {
        if (delegateItem && listBarLayout.noteDragPreviewDelegate !== delegateItem)
            return;
        listBarLayout.noteDragPreviewDelegate = null;
        listBarLayout.noteDragPreviewHotSpotX = 0;
        listBarLayout.noteDragPreviewHotSpotY = 0;
        listBarLayout.noteDragPreviewX = 0;
        listBarLayout.noteDragPreviewY = 0;
    }
    function currentIndexFromModel() {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return -1;
        if (listBarLayout.noteListModel.currentIndex !== undefined)
            return Number(listBarLayout.noteListModel.currentIndex);
        return -1;
    }
    function deleteCurrentNote() {
        if (!listBarLayout.noteDeletionContractAvailable)
            return false;
        const deleted = noteDeletionBridge.deleteFocusedNote();
        if (deleted)
            noteListView.forceActiveFocus();

        return deleted;
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
    function pushCurrentIndexToModel(index) {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return;
        const normalizedIndex = Number(index);
        if (listBarLayout.noteListModel.currentIndex !== undefined) {
            if (Number(listBarLayout.noteListModel.currentIndex) === normalizedIndex)
                return;
            listBarLayout.noteListModel.currentIndex = index;
            return;
        }
        if (listBarLayout.noteListModel.setCurrentIndex !== undefined)
            listBarLayout.noteListModel.setCurrentIndex(normalizedIndex);
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function syncCurrentIndexFromModel() {
        const modelIndex = listBarLayout.currentIndexFromModel();
        if (!isFinite(modelIndex)) {
            if (noteListView.currentIndex !== -1)
                noteListView.currentIndex = -1;
            return;
        }

        const normalizedIndex = Math.max(-1, Math.floor(modelIndex));
        if (noteListView.currentIndex !== normalizedIndex)
            noteListView.currentIndex = normalizedIndex;
    }
    function syncFocusedNoteDeletionState() {
        if (noteListView.currentItem && noteListView.currentItem.noteId !== undefined) {
            noteDeletionBridge.focusedNoteId = String(noteListView.currentItem.noteId).trim();
            return;
        }
        noteDeletionBridge.focusedNoteId = "";
    }
    function updateNoteDragPreviewPosition(delegateItem, localX, localY) {
        if (!delegateItem || listBarLayout.noteDragPreviewDelegate !== delegateItem)
            return;
        const previewParent = noteDragPreview.parent ? noteDragPreview.parent : listBarLayout;
        const mappedPoint = delegateItem.mapToItem(previewParent, Number(localX) || 0, Number(localY) || 0);
        listBarLayout.noteDragPreviewX = Math.round((Number(mappedPoint.x) || 0) - listBarLayout.noteDragPreviewHotSpotX);
        listBarLayout.noteDragPreviewY = Math.round((Number(mappedPoint.y) || 0) - listBarLayout.noteDragPreviewHotSpotY);
    }

    color: panelColor

    Component.onCompleted: {
        listBarLayout.syncCurrentIndexFromModel();
        listBarLayout.syncFocusedNoteDeletionState();
    }
    onNoteListModeChanged: applySearchTextToModel()
    onNoteListModelChanged: {
        listBarLayout.noteDragActive = false;
        listBarLayout.clearNoteDragPreview(null);
        listBarLayout.notePointerPressed = false;
        listBarLayout.pendingSelectionIndex = -1;
        listBarLayout.pressedNoteIndex = -1;
        listBarLayout.selectionRequestRevision += 1;
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
    NoteListItem {
        id: noteDragPreview

        parent: Controls.Overlay.overlay ? Controls.Overlay.overlay : listBarLayout
        visible: listBarLayout.noteDragActive && listBarLayout.noteDragPreviewDelegate !== null
        z: 1000

        active: true
        bookmarkColor: listBarLayout.noteDragPreviewDelegate && listBarLayout.noteDragPreviewDelegate.bookmarkColor !== undefined && listBarLayout.noteDragPreviewDelegate.bookmarkColor !== null ? String(listBarLayout.noteDragPreviewDelegate.bookmarkColor) : ""
        bookmarked: listBarLayout.noteDragPreviewDelegate ? Boolean(listBarLayout.noteDragPreviewDelegate.bookmarked) : false
        displayDate: listBarLayout.noteDragPreviewDelegate && listBarLayout.noteDragPreviewDelegate.displayDate !== undefined && listBarLayout.noteDragPreviewDelegate.displayDate !== null ? String(listBarLayout.noteDragPreviewDelegate.displayDate) : ""
        folders: listBarLayout.noteDragPreviewDelegate ? listBarLayout.normalizeEntries(listBarLayout.noteDragPreviewDelegate.folders) : []
        image: listBarLayout.noteDragPreviewDelegate ? Boolean(listBarLayout.noteDragPreviewDelegate.image) : false
        imageSource: listBarLayout.noteDragPreviewDelegate && listBarLayout.noteDragPreviewDelegate.imageSource !== undefined && listBarLayout.noteDragPreviewDelegate.imageSource !== null ? listBarLayout.noteDragPreviewDelegate.imageSource : ""
        noteId: listBarLayout.noteDragPreviewDelegate && listBarLayout.noteDragPreviewDelegate.noteId !== undefined && listBarLayout.noteDragPreviewDelegate.noteId !== null ? String(listBarLayout.noteDragPreviewDelegate.noteId) : ""
        opacity: 0.96
        primaryText: listBarLayout.noteDragPreviewDelegate && listBarLayout.noteDragPreviewDelegate.primaryText !== undefined && listBarLayout.noteDragPreviewDelegate.primaryText !== null ? String(listBarLayout.noteDragPreviewDelegate.primaryText) : ""
        tags: listBarLayout.noteDragPreviewDelegate ? listBarLayout.normalizeEntries(listBarLayout.noteDragPreviewDelegate.tags) : []
        width: listBarLayout.noteDragPreviewDelegate && listBarLayout.noteDragPreviewDelegate.width !== undefined ? Number(listBarLayout.noteDragPreviewDelegate.width) || implicitWidth : implicitWidth
        x: listBarLayout.noteDragPreviewX
        y: listBarLayout.noteDragPreviewY
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
                    clip: true
                    interactive: contentHeight > height && !listBarLayout.noteDragActive && !listBarLayout.notePointerPressed
                    model: listBarLayout.resolvedNoteListModel
                    spacing: 2
                    visible: listBarLayout.noteListMode

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

                        Drag.active: noteDragHandler.active
                        Drag.dragType: Drag.Internal
                        Drag.hotSpot.x: noteItemDelegate.dragHotSpotX
                        Drag.hotSpot.y: noteItemDelegate.dragHotSpotY
                        Drag.keys: ["whatson.library.note"]
                        Drag.mimeData: ({
                                "application/x-whatson-note-id": noteItemDelegate.noteId,
                                "text/plain": noteItemDelegate.noteId
                            })
                        Drag.source: noteItemDelegate
                        Drag.supportedActions: Qt.CopyAction
                        height: noteCard.implicitHeight
                        width: ListView.view ? ListView.view.width : listBarLayout.width

                        NoteListItem {
                            id: noteCard

                            active: noteListView.currentIndex === noteItemDelegate.index
                            anchors.fill: parent
                            bookmarkColor: noteItemDelegate.bookmarkColor === undefined || noteItemDelegate.bookmarkColor === null ? "" : String(noteItemDelegate.bookmarkColor)
                            bookmarked: noteItemDelegate.bookmarked === undefined ? false : Boolean(noteItemDelegate.bookmarked)
                            displayDate: noteItemDelegate.displayDate === undefined || noteItemDelegate.displayDate === null ? "" : String(noteItemDelegate.displayDate)
                            folders: listBarLayout.normalizeEntries(noteItemDelegate.folders)
                            image: noteItemDelegate.image === undefined ? false : Boolean(noteItemDelegate.image)
                            imageSource: noteItemDelegate.imageSource === undefined || noteItemDelegate.imageSource === null ? "" : noteItemDelegate.imageSource
                            noteId: noteItemDelegate.noteId === undefined || noteItemDelegate.noteId === null ? "" : String(noteItemDelegate.noteId)
                            opacity: noteDragHandler.active ? 0.72 : 1
                            pressed: listBarLayout.pressedNoteIndex === noteItemDelegate.index || noteDragHandler.active
                            primaryText: noteItemDelegate.primaryText === undefined || noteItemDelegate.primaryText === null ? "" : String(noteItemDelegate.primaryText)
                            tags: listBarLayout.normalizeEntries(noteItemDelegate.tags)
                        }
                        DragHandler {
                            id: noteDragHandler

                            acceptedButtons: Qt.LeftButton
                            dragThreshold: 4
                            grabPermissions: PointerHandler.CanTakeOverFromAnything
                            target: null

                            onActiveChanged: {
                                listBarLayout.noteDragActive = active;
                                if (active) {
                                    noteItemDelegate.dragHotSpotX = Number(noteDragHandler.centroid.pressPosition.x) || width * 0.5;
                                    noteItemDelegate.dragHotSpotY = Number(noteDragHandler.centroid.pressPosition.y) || height * 0.5;
                                    listBarLayout.beginNoteDragPreview(
                                                noteItemDelegate,
                                                noteItemDelegate.dragHotSpotX,
                                                noteItemDelegate.dragHotSpotY);
                                    listBarLayout.notePointerPressed = false;
                                } else {
                                    listBarLayout.clearNoteDragPreview(noteItemDelegate);
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
                            }
                            onCanceled: {
                                listBarLayout.noteDragActive = false;
                                listBarLayout.clearNoteDragPreview(noteItemDelegate);
                            }
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            gesturePolicy: TapHandler.DragThreshold
                            grabPermissions: PointerHandler.ApprovesTakeOverByAnything

                            onPressedChanged: {
                                if (!pressed) {
                                    if (listBarLayout.notePointerPressed)
                                        listBarLayout.notePointerPressed = false;
                                    if (listBarLayout.pressedNoteIndex === noteItemDelegate.index && !noteDragHandler.active)
                                        listBarLayout.pressedNoteIndex = -1;
                                    return;
                                }
                                listBarLayout.notePointerPressed = true;
                                listBarLayout.pressedNoteIndex = noteItemDelegate.index;
                                noteDeletionBridge.focusedNoteId = noteCard.noteId;
                            }
                            onTapped: {
                                listBarLayout.notePointerPressed = false;
                                listBarLayout.pressedNoteIndex = -1;
                                listBarLayout.activateNoteIndex(noteItemDelegate.index, noteCard.noteId);
                            }
                        }
                    }

                    onCurrentIndexChanged: {
                        listBarLayout.pressedNoteIndex = -1;
                        listBarLayout.pushCurrentIndexToModel(noteListView.currentIndex);
                        Qt.callLater(function () {
                            listBarLayout.syncFocusedNoteDeletionState();
                        });
                    }
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
        }

        target: listBarLayout.resolvedNoteListModel
    }
}
