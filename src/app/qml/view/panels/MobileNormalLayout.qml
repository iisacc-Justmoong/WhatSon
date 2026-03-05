import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: mobileNormalLayout

    property color canvasColor: LV.Theme.panelBackground01
    property color controlSurfaceColor: LV.Theme.panelBackground10
    property color hintColor: LV.Theme.descriptionColor
    property string statusPlaceholderText: ""
    property color titleColor: LV.Theme.titleHeaderColor

    Rectangle {
        anchors.fill: parent
        color: mobileNormalLayout.canvasColor
    }
    Item {
        anchors.bottomMargin: 16
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 16

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                id: topNavigationBar

                Layout.fillWidth: true
                Layout.preferredHeight: 24
                color: mobileNormalLayout.controlSurfaceColor
                radius: 32

                Item {
                    anchors.bottomMargin: 2
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    anchors.topMargin: 2

                    LV.IconButton {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        iconName: "loggedInUser"
                    }
                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 12

                        LV.IconButton {
                            iconName: "audioToAudio"
                        }
                        LV.IconMenuButton {
                            iconName: "generalprojectStructure"
                        }
                    }
                }
            }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
            Item {
                id: bottomStatusBar

                Layout.fillWidth: true
                Layout.preferredHeight: 20

                Rectangle {
                    id: statusTextField

                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 1
                    anchors.left: parent.left
                    anchors.right: newFileSlot.left
                    anchors.top: parent.top
                    anchors.topMargin: 1
                    color: mobileNormalLayout.controlSurfaceColor
                    radius: 5

                    Item {
                        anchors.bottomMargin: 3
                        anchors.fill: parent
                        anchors.leftMargin: 7
                        anchors.rightMargin: 7
                        anchors.topMargin: 3

                        Row {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 0

                            Text {
                                color: mobileNormalLayout.titleColor
                                font.family: "Pretendard"
                                font.pixelSize: 12
                                font.weight: 500
                                text: mobileNormalLayout.statusPlaceholderText
                                visible: text.length > 0
                            }
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                color: mobileNormalLayout.hintColor
                                height: 12
                                visible: mobileNormalLayout.statusPlaceholderText.length > 0
                                width: 1
                            }
                        }
                        Text {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            color: mobileNormalLayout.hintColor
                            font.family: "SF Pro"
                            font.pixelSize: 13
                            text: "\u2715"
                        }
                    }
                }
                Item {
                    id: newFileSlot

                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.top: parent.top
                    width: 36

                    LV.IconButton {
                        anchors.centerIn: parent
                        iconName: "addFile"
                    }
                }
            }
        }
    }
}
