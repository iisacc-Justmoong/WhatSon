import QtQuick

Rectangle {
    id: drawerSplitter

    property var clampDrawerHeightResolver: null
    property int drawerHeight: 0
    property color splitterColor: "transparent"
    property int splitterHandleThickness: 0

    signal drawerHeightDragRequested(int value)

    color: splitterColor

    MouseArea {
        id: drawerSplitterMouse

        property int dragStartDrawerHeight: drawerSplitter.drawerHeight
        property real dragStartGlobalY: 0

        acceptedButtons: Qt.LeftButton
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        cursorShape: Qt.SizeVerCursor
        height: drawerSplitter.splitterHandleThickness
        preventStealing: true

        onPositionChanged: function (mouse) {
            if (!(pressedButtons & Qt.LeftButton) || !drawerSplitter.clampDrawerHeightResolver)
                return;
            const movePoint = drawerSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
            const deltaY = movePoint.y - dragStartGlobalY;
            const nextDrawerHeight = Number(drawerSplitter.clampDrawerHeightResolver(dragStartDrawerHeight - deltaY)) || 0;
            if (nextDrawerHeight !== drawerSplitter.drawerHeight)
                drawerSplitter.drawerHeightDragRequested(nextDrawerHeight);
        }
        onPressed: function (mouse) {
            const pressPoint = drawerSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
            dragStartGlobalY = pressPoint.y;
            dragStartDrawerHeight = drawerSplitter.drawerHeight;
        }
    }
}
