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
                        backgroundColor: listBarLayout.fieldColor
                        backgroundColorDisabled: listBarLayout.fieldColor
                        backgroundColorFocused: listBarLayout.fieldColor
                        backgroundColorHover: listBarLayout.fieldColor
                        backgroundColorPressed: listBarLayout.fieldColor
                        clearButtonVisible: true
                        cornerRadius: 5
                        fieldMinHeight: 18
                        insetHorizontal: LV.Theme.gap7
                        insetVertical: LV.Theme.gap3
                        mode: searchMode
                        selectByMouse: true
                        sideSpacing: LV.Theme.gap5
                        text: listBarLayout.searchText
                        textColor: listBarLayout.titleColor

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
                        checkable: false
                        iconName: "cwmPermissionView"
                        iconSize: 16
                        tone: LV.AbstractButton.Borderless
                    }
                    LV.IconButton {
                        Layout.preferredHeight: 20
                        Layout.preferredWidth: 20
                        checkable: false
                        iconName: "sortByType"
                        iconSize: 16
                        tone: LV.AbstractButton.Borderless
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
                        text: "List content placeholder"
                    }
                }
            }
        }
    }
}
