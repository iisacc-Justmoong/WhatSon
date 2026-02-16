import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: root

    property color displayColor: "#495473"
    property color drawerColor: "#665d47"
    property int drawerHeight: 255
    property int minDisplayHeight: 160
    property int minDrawerHeight: 120
    property color panelColor: "#39445b"
    property color splitterColor: "transparent"
    property int splitterHandleThickness: 12
    property int splitterThickness: 0

    signal drawerHeightDragRequested(int value)

    function clampDrawerHeight(value) {
        var maxDrawer = Math.max(root.minDrawerHeight, root.height - root.minDisplayHeight - root.splitterThickness);
        return Math.max(root.minDrawerHeight, Math.min(maxDrawer, value));
    }

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
            id: horizontalSplitter

            Layout.fillWidth: true
            Layout.preferredHeight: root.splitterThickness
            color: root.splitterColor

            MouseArea {
                id: splitterMouse

                property int dragStartDrawerHeight: root.drawerHeight
                property real dragStartGlobalY: 0

                acceptedButtons: Qt.LeftButton
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                cursorShape: Qt.SizeVerCursor
                height: root.splitterHandleThickness
                preventStealing: true

                onPositionChanged: function (mouse) {
                    if (!(pressedButtons & Qt.LeftButton))
                        return;
                    var movePoint = splitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    var deltaY = movePoint.y - dragStartGlobalY;
                    var nextDrawerHeight = root.clampDrawerHeight(dragStartDrawerHeight - deltaY);
                    if (nextDrawerHeight !== root.drawerHeight)
                        root.drawerHeightDragRequested(nextDrawerHeight);
                }
                onPressed: function (mouse) {
                    var pressPoint = splitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    dragStartGlobalY = pressPoint.y;
                    dragStartDrawerHeight = root.drawerHeight;
                }
            }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.clampDrawerHeight(root.drawerHeight)
            color: root.drawerColor
        }
    }
}
