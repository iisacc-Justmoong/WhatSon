pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: listBarLayout

    property int activeToolbarIndex: 0
    property color hintColor: LV.Theme.descriptionColor
    property bool noteDragActive: false
    readonly property bool noteListMode: activeToolbarIndex === 0 || activeToolbarIndex === 2
    property var noteListModel: null
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ListBarLayout") : null
    property string searchText: ""

    signal viewHookRequested

    function applySearchTextToModel() {
        if (!listBarLayout.noteListMode || !listBarLayout.noteListModel)
            return;

        if (listBarLayout.noteListModel.searchText !== undefined) {
            listBarLayout.noteListModel.searchText = listBarLayout.searchText;
            return;
        }
        if (listBarLayout.noteListModel.setSearchText !== undefined)
            listBarLayout.noteListModel.setSearchText(listBarLayout.searchText);
    }
    function currentIndexFromModel() {
        if (!listBarLayout.noteListModel)
            return -1;
        if (listBarLayout.noteListModel.currentIndex !== undefined)
            return Number(listBarLayout.noteListModel.currentIndex);
        return -1;
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
        if (!listBarLayout.noteListModel)
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
        const nextIndex = listBarLayout.currentIndexFromModel();
        if (noteListView.currentIndex === nextIndex)
            return;
        noteListView.currentIndex = nextIndex;
    }

    color: panelColor

    Component.onCompleted: {
        listBarLayout.syncCurrentIndexFromModel();
    }
    onNoteListModeChanged: applySearchTextToModel()
    onNoteListModelChanged: {
        listBarLayout.noteDragActive = false;
        listBarLayout.applySearchTextToModel();
        listBarLayout.syncCurrentIndexFromModel();
    }
    onSearchTextChanged: applySearchTextToModel()

    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                id: topToolbar

                Layout.fillWidth: true
                Layout.preferredHeight: 24

                ListBarHeader {
                    anchors.fill: parent
                    searchText: listBarLayout.searchText

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
                    interactive: contentHeight > height && !listBarLayout.noteDragActive
                    model: listBarLayout.noteListMode && listBarLayout.noteListModel ? listBarLayout.noteListModel : null
                    spacing: 2
                    visible: listBarLayout.noteListMode

                    delegate: NoteListItem {
                        id: noteItemDelegate

                        required property int index
                        required property var model
                        readonly property var roleModel: typeof noteItemDelegate.model === "object" ? noteItemDelegate.model : null
                        readonly property bool useRuntimeModel: listBarLayout.noteListModel !== null

                        Drag.active: noteDragHandler.active
                        Drag.hotSpot.x: width * 0.5
                        Drag.hotSpot.y: height * 0.5
                        Drag.keys: ["whatson.library.note"]
                        Drag.source: noteItemDelegate
                        Drag.supportedActions: Qt.CopyAction
                        bookmarkColor: useRuntimeModel && roleModel && roleModel.bookmarkColor !== undefined ? String(roleModel.bookmarkColor) : ""
                        bookmarked: useRuntimeModel && roleModel && roleModel.bookmarked !== undefined ? Boolean(roleModel.bookmarked) : false
                        displayDate: useRuntimeModel && roleModel && roleModel.displayDate !== undefined ? String(roleModel.displayDate) : ""
                        folders: useRuntimeModel && roleModel && roleModel.folders !== undefined ? listBarLayout.normalizeEntries(roleModel.folders) : []
                        image: useRuntimeModel && roleModel && roleModel.image !== undefined ? Boolean(roleModel.image) : false
                        imageSource: useRuntimeModel && roleModel && roleModel.imageSource !== undefined ? roleModel.imageSource : ""
                        noteId: useRuntimeModel && roleModel && roleModel.id !== undefined ? String(roleModel.id) : ""
                        opacity: noteDragHandler.active ? 0.72 : 1
                        pressed: ListView.isCurrentItem
                        primaryText: useRuntimeModel && roleModel && roleModel.primaryText !== undefined ? String(roleModel.primaryText) : ""
                        tags: useRuntimeModel && roleModel && roleModel.tags !== undefined ? listBarLayout.normalizeEntries(roleModel.tags) : []
                        width: ListView.view ? ListView.view.width : listBarLayout.width

                        DragHandler {
                            id: noteDragHandler

                            acceptedButtons: Qt.LeftButton
                            dragThreshold: 4
                            grabPermissions: PointerHandler.CanTakeOverFromAnything
                            target: null

                            onActiveChanged: {
                                listBarLayout.noteDragActive = active;
                            }
                        }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            gesturePolicy: TapHandler.DragThreshold

                            onTapped: {
                                if (noteListView.currentIndex !== noteItemDelegate.index)
                                    noteListView.currentIndex = noteItemDelegate.index;
                            }
                        }
                    }

                    onCurrentIndexChanged: {
                        listBarLayout.pushCurrentIndexToModel(noteListView.currentIndex);
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

        target: listBarLayout.noteListModel
    }
}
