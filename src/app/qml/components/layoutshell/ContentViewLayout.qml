import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: root

    property color displayColor: "#495473"
    property color drawerColor: "#665d47"
    property int drawerHeight: 255
    property color panelColor: "#39445b"

    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true

    Rectangle {
        anchors.fill: parent
        color: root.panelColor
    }
    LV.VStack {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: root.displayColor
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.drawerHeight
            color: root.drawerColor
        }
    }
}
