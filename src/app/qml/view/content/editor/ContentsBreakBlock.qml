pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: breakBlock

    required property var blockData

    signal documentEndEditRequested()

    implicitHeight: Math.max(Math.round(LV.Theme.scaleMetric(18)), divider.height + Math.round(LV.Theme.scaleMetric(10)) * 2)
    width: parent ? parent.width : implicitWidth

    Rectangle {
        id: divider

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        color: "#43464B"
        height: 1
        radius: 1
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: breakBlock.documentEndEditRequested()
    }
}
