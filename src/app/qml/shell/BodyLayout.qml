import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: root

    property color contentPanelColor: "#39445b"
    property color contentsDisplayColor: "#495473"
    property color drawerColor: "#665d47"
    property int drawerHeight: 255
    property color listViewColor: "#3a5c57"
    property int listViewWidth: 198
    property color rightPanelColor: "#63556a"
    property int rightPanelWidth: 194
    property color sidebarColor: "#3b4b63"
    property int sidebarWidth: 216

    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true

    LV.HStack {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: root.sidebarWidth
            color: root.sidebarColor
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: root.listViewWidth
            color: root.listViewColor
            visible: root.listViewWidth > 0
        }
        ContentViewLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            displayColor: root.contentsDisplayColor
            drawerColor: root.drawerColor
            drawerHeight: root.drawerHeight
            panelColor: root.contentPanelColor
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: root.rightPanelWidth
            color: root.rightPanelColor
            visible: root.rightPanelWidth > 0
        }
    }
}
