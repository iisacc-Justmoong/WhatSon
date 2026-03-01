import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: listItemsPlaceholder

    property var noteModel: null
    property int sampleCount: 8

    clip: true

    ListView {
        anchors.fill: parent
        anchors.margins: 2
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        interactive: contentHeight > height
        model: listItemsPlaceholder.noteModel ? listItemsPlaceholder.noteModel : listItemsPlaceholder.sampleCount
        spacing: 2

        delegate: NoteListItem {
            readonly property var roleModel: typeof model === "object" ? model : null
            readonly property bool useRuntimeModel: listItemsPlaceholder.noteModel !== null

            desc: useRuntimeModel && roleModel && roleModel.summaryText !== undefined ? String(roleModel.summaryText) : "Note Contents Thumbnail... is can has 2 lines..."
            folders: useRuntimeModel && roleModel && roleModel.foldersText !== undefined ? String(roleModel.foldersText).split(",").map(function (entry) {
                return entry.trim();
            }).filter(function (entry) {
                return entry.length > 0;
            }) : ["FolderName1", "FolderName2"]
            title: useRuntimeModel && roleModel && roleModel.titleText !== undefined ? String(roleModel.titleText) : "NoteTitle"
            width: ListView.view ? ListView.view.width : listItemsPlaceholder.width
        }
    }
}
