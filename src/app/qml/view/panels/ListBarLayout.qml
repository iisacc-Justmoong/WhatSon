pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: listBarLayout

    property int activeToolbarIndex: 0
    readonly property bool hasNoteListModel: listBarLayout.noteListModel !== null && listBarLayout.noteListModel !== undefined
    property color hintColor: LV.Theme.descriptionColor
    readonly property bool noteDeletionContractAvailable: listBarLayout.noteDeletionViewModel !== null && listBarLayout.noteDeletionViewModel !== undefined && listBarLayout.noteDeletionViewModel.deleteNoteById !== undefined
    property var noteDeletionViewModel: null
    property bool noteDragActive: false
    readonly property bool noteListCurrentIndexContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.noteListModel.currentIndex !== undefined || listBarLayout.noteListModel.setCurrentIndex !== undefined)
    readonly property bool noteListMode: activeToolbarIndex === 0 || activeToolbarIndex === 2
    property var noteListModel: null
    readonly property bool noteListSearchContractAvailable: listBarLayout.hasNoteListModel && (listBarLayout.noteListModel.searchText !== undefined || listBarLayout.noteListModel.setSearchText !== undefined)
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ListBarLayout") : null
    readonly property var resolvedNoteListModel: listBarLayout.noteListMode ? listBarLayout.noteListModel : null
    property string searchText: ""

    signal viewHookRequested

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
    function currentIndexFromModel() {
        if (!listBarLayout.noteListCurrentIndexContractAvailable)
            return -1;
        if (listBarLayout.noteListModel.currentIndex !== undefined)
            return Number(listBarLayout.noteListModel.currentIndex);
        return -1;
    }
    function currentNoteIdFromModel() {
        if (!listBarLayout.hasNoteListModel)
            return "";
        if (listBarLayout.noteListModel.currentNoteId !== undefined)
            return String(listBarLayout.noteListModel.currentNoteId).trim();
        if (noteListView.currentItem && noteListView.currentItem.noteId !== undefined)
            return String(noteListView.currentItem.noteId).trim();
        return "";
    }
    function deleteCurrentNote() {
        if (!listBarLayout.noteDeletionContractAvailable)
            return false;
        const noteId = listBarLayout.currentNoteIdFromModel();
        if (!noteId.length)
            return false;
        const deleted = Boolean(listBarLayout.noteDeletionViewModel.deleteNoteById(noteId));
        if (deleted)
            noteListView.forceActiveFocus();

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
                    model: listBarLayout.resolvedNoteListModel
                    spacing: 2
                    visible: listBarLayout.noteListMode

                    delegate: NoteListItem {
                        id: noteItemDelegate

                        required property int index
                        required property var model
                        readonly property var roleModel: noteItemDelegate.model && typeof noteItemDelegate.model === "object" ? noteItemDelegate.model : ({})

                        Drag.active: noteDragHandler.active
                        Drag.hotSpot.x: width * 0.5
                        Drag.hotSpot.y: height * 0.5
                        Drag.keys: ["whatson.library.note"]
                        Drag.source: noteItemDelegate
                        Drag.supportedActions: Qt.CopyAction
                        bookmarkColor: roleModel.bookmarkColor === undefined || roleModel.bookmarkColor === null ? "" : String(roleModel.bookmarkColor)
                        bookmarked: roleModel.bookmarked === undefined ? false : Boolean(roleModel.bookmarked)
                        displayDate: roleModel.displayDate === undefined || roleModel.displayDate === null ? "" : String(roleModel.displayDate)
                        folders: listBarLayout.normalizeEntries(roleModel.folders)
                        image: roleModel.image === undefined ? false : Boolean(roleModel.image)
                        imageSource: roleModel.imageSource === undefined || roleModel.imageSource === null ? "" : roleModel.imageSource
                        noteId: roleModel.id === undefined || roleModel.id === null ? "" : String(roleModel.id)
                        opacity: noteDragHandler.active ? 0.72 : 1
                        pressed: ListView.isCurrentItem
                        primaryText: roleModel.primaryText === undefined || roleModel.primaryText === null ? "" : String(roleModel.primaryText)
                        tags: listBarLayout.normalizeEntries(roleModel.tags)
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
                                noteListView.forceActiveFocus();
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

        target: listBarLayout.resolvedNoteListModel
    }
}
