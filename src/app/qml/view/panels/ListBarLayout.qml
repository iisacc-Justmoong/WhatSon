import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: listBarLayout

    property int activeToolbarIndex: 0
    property color hintColor: LV.Theme.descriptionColor
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
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    color: panelColor

    onNoteListModeChanged: applySearchTextToModel()
    onNoteListModelChanged: applySearchTextToModel()
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

                ListItemsPlaceholder {
                    anchors.fill: parent
                    noteModel: listBarLayout.noteListModel
                    visible: listBarLayout.noteListMode
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
}
