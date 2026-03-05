import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: listBarLayout

    property int activeToolbarIndex: 0
    property color fieldColor: LV.Theme.panelBackground10
    property color hintColor: LV.Theme.descriptionColor
    readonly property bool noteListMode: activeToolbarIndex === 0 || activeToolbarIndex === 2
    property var noteListModel: null
    property color panelColor: LV.Theme.panelBackground04
    property string searchText: ""
    property color titleColor: LV.Theme.titleHeaderColor

    color: panelColor

    Item {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                id: topToolbar

                Layout.fillWidth: true
                Layout.preferredHeight: 24

                RowLayout {
                    anchors.bottomMargin: 2
                    anchors.fill: parent
                    anchors.leftMargin: 2
                    anchors.rightMargin: 2
                    anchors.topMargin: 2
                    spacing: 0

                    LV.InputField {
                        id: listSearchField

                        Layout.fillWidth: true
                        Layout.preferredHeight: 18
                        mode: searchMode
                        selectByMouse: true
                        text: listBarLayout.searchText

                        onAccepted: function (text) {
                            listBarLayout.searchText = typeof text === "string" ? text : listSearchField.text;
                        }
                        onTextEdited: function (text) {
                            listBarLayout.searchText = typeof text === "string" ? text : listSearchField.text;
                        }
                    }
                    LV.IconButton {
                        Layout.preferredHeight: 20
                        Layout.preferredWidth: 20
                        iconName: "cwmPermissionView"
                    }
                    LV.IconButton {
                        Layout.preferredHeight: 20
                        Layout.preferredWidth: 20
                        iconName: "sortByType"
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

                    Text {
                        anchors.centerIn: parent
                        color: listBarLayout.hintColor
                        font.family: "Pretendard"
                        font.pixelSize: 11
                        text: "No list data"
                    }
                }
            }
        }
    }
}
