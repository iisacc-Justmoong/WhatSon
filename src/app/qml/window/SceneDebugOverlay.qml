import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: root

    property var hostWindow: null
    property bool enabled: true
    property real pointerX: width * 0.5
    property real pointerY: height * 0.5

    parent: hostWindow ? hostWindow.contentItem : null
    anchors.fill: parent
    visible: enabled && parent !== null
    z: 999999

    HoverHandler {
        id: hoverHandler
        target: null
        enabled: root.visible

        onPointChanged: function(point) {
            root.pointerX = point.position.x
            root.pointerY = point.position.y
        }
    }

    Rectangle {
        x: Math.max(0, root.pointerX - 1)
        y: 0
        width: 2
        height: root.height
        color: "#66ff3355"
    }

    Rectangle {
        x: 0
        y: Math.max(0, root.pointerY - 1)
        width: root.width
        height: 2
        color: "#6633d1ff"
    }

    Rectangle {
        x: Math.max(0, root.pointerX - 10)
        y: Math.max(0, root.pointerY - 10)
        width: 20
        height: 20
        color: "transparent"
        border.width: 2
        border.color: "#ffffcc33"
        radius: 2
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 2
        border.color: "#88ff00ff"
    }

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: LV.Theme.gap6
        width: Math.max(LV.Theme.gap20 * 14, debugColumn.implicitWidth + LV.Theme.gap8 * 2)
        height: debugColumn.implicitHeight + LV.Theme.gap8 * 2
        color: "#cc111111"
        border.width: 1
        border.color: "#88ffffff"
        radius: LV.Theme.gap2

        ColumnLayout {
            id: debugColumn
            anchors.fill: parent
            anchors.margins: LV.Theme.gap4
            spacing: LV.Theme.gap2

            LV.Label {
                Layout.fillWidth: true
                text: "Scene Debug Overlay"
                style: header3
            }
            LV.Label {
                Layout.fillWidth: true
                wrapMode: Text.NoWrap
                text: "pointer: (" + Math.round(root.pointerX) + ", " + Math.round(root.pointerY) + ")"
            }
            LV.Label {
                Layout.fillWidth: true
                wrapMode: Text.NoWrap
                text: "window: " + Math.round(root.width) + " x " + Math.round(root.height)
            }
            LV.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "현재는 1차 오버레이입니다. 다음 단계에서 실제 QQuickItem bounds/type/objectName 하이라이트를 붙입니다."
            }
        }
    }
}
