import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: listItemsPlaceholder

    property var noteModel: null

    function normalizeFolders(value) {
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

    clip: true

    ListView {
        id: noteListView

        anchors.fill: parent
        anchors.margins: 2
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        interactive: contentHeight > height
        model: listItemsPlaceholder.noteModel ? listItemsPlaceholder.noteModel : null
        spacing: 2

        delegate: NoteListItem {
            readonly property var roleModel: typeof model === "object" ? model : null
            readonly property bool useRuntimeModel: listItemsPlaceholder.noteModel !== null

            bookmarkColor: useRuntimeModel && roleModel && roleModel.bookmarkColor !== undefined ? String(roleModel.bookmarkColor) : ""
            bookmarked: useRuntimeModel && roleModel && roleModel.bookmarked !== undefined ? Boolean(roleModel.bookmarked) : false
            desc: useRuntimeModel && roleModel && roleModel.desc !== undefined ? String(roleModel.desc) : ""
            folders: useRuntimeModel && roleModel && roleModel.folders !== undefined ? listItemsPlaceholder.normalizeFolders(roleModel.folders) : []
            noteId: useRuntimeModel && roleModel && roleModel.id !== undefined ? String(roleModel.id) : ""
            pressed: ListView.isCurrentItem
            title: useRuntimeModel && roleModel && roleModel.title !== undefined ? String(roleModel.title) : ""
            width: ListView.view ? ListView.view.width : listItemsPlaceholder.width

            TapHandler {
                acceptedButtons: Qt.LeftButton

                onTapped: {
                    if (noteListView.currentIndex !== index)
                        noteListView.currentIndex = index;
                }
            }
        }
    }
}
