pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: listItemsPlaceholder

    property var noteModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ListItemsPlaceholder") : null

    function currentIndexFromModel() {
        if (!listItemsPlaceholder.noteModel)
            return -1;
        if (listItemsPlaceholder.noteModel.currentIndex !== undefined)
            return Number(listItemsPlaceholder.noteModel.currentIndex);
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
        if (!listItemsPlaceholder.noteModel)
            return;
        const normalizedIndex = Number(index);
        if (listItemsPlaceholder.noteModel.currentIndex !== undefined) {
            if (Number(listItemsPlaceholder.noteModel.currentIndex) === normalizedIndex)
                return;
            listItemsPlaceholder.noteModel.currentIndex = index;
            return;
        }
        if (listItemsPlaceholder.noteModel.setCurrentIndex !== undefined)
            listItemsPlaceholder.noteModel.setCurrentIndex(normalizedIndex);
    }
    function syncCurrentIndexFromModel() {
        const nextIndex = listItemsPlaceholder.currentIndexFromModel();
        if (noteListView.currentIndex === nextIndex)
            return;
        noteListView.currentIndex = nextIndex;
    }

    clip: true

    Component.onCompleted: {
        listItemsPlaceholder.syncCurrentIndexFromModel();
    }
    onNoteModelChanged: {
        listItemsPlaceholder.syncCurrentIndexFromModel();
    }

    ListView {
        id: noteListView

        anchors.fill: parent
        anchors.margins: 2
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        interactive: contentHeight > height
        model: listItemsPlaceholder.noteModel ? listItemsPlaceholder.noteModel : null
        spacing: 2

        onCurrentIndexChanged: {
            listItemsPlaceholder.pushCurrentIndexToModel(noteListView.currentIndex);
        }

        delegate: NoteListItem {
            id: noteItemDelegate

            required property int index
            required property var model
            readonly property var roleModel: typeof noteItemDelegate.model === "object" ? noteItemDelegate.model : null
            readonly property bool useRuntimeModel: listItemsPlaceholder.noteModel !== null

            Drag.active: noteDragHandler.active
            Drag.keys: ["whatson.library.note"]
            Drag.source: noteItemDelegate
            bookmarkColor: useRuntimeModel && roleModel && roleModel.bookmarkColor !== undefined ? String(roleModel.bookmarkColor) : ""
            bookmarked: useRuntimeModel && roleModel && roleModel.bookmarked !== undefined ? Boolean(roleModel.bookmarked) : false
            displayDate: useRuntimeModel && roleModel && roleModel.displayDate !== undefined ? String(roleModel.displayDate) : ""
            folders: useRuntimeModel && roleModel && roleModel.folders !== undefined ? listItemsPlaceholder.normalizeEntries(roleModel.folders) : []
            noteId: useRuntimeModel && roleModel && roleModel.id !== undefined ? String(roleModel.id) : ""
            opacity: noteDragHandler.active ? 0.72 : 1
            pressed: ListView.isCurrentItem
            primaryText: useRuntimeModel && roleModel && roleModel.primaryText !== undefined ? String(roleModel.primaryText) : ""
            tags: useRuntimeModel && roleModel && roleModel.tags !== undefined ? listItemsPlaceholder.normalizeEntries(roleModel.tags) : []
            width: ListView.view ? ListView.view.width : listItemsPlaceholder.width

            DragHandler {
                id: noteDragHandler

                target: null
            }
            TapHandler {
                acceptedButtons: Qt.LeftButton

                onTapped: {
                    if (noteListView.currentIndex !== noteItemDelegate.index)
                        noteListView.currentIndex = noteItemDelegate.index;
                }
            }
        }
    }
    Connections {
        function onCurrentIndexChanged() {
            listItemsPlaceholder.syncCurrentIndexFromModel();
        }

        target: listItemsPlaceholder.noteModel
    }
}
