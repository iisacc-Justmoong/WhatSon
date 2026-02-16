import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property color panelColor: "#303743"
    property int panelHeight: 36

    Layout.fillWidth: true
    Layout.preferredHeight: root.panelHeight
    color: root.panelColor
}
