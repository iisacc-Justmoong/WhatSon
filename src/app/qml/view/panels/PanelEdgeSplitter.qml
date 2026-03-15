import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: splitter

    property var clampSize: null
    property int currentSize: 0
    property int dragDirection: 1
    property int handleThickness: LV.Theme.gap12
    property color splitterColor: LV.Theme.panelBackground10
    property int splitterThickness: LV.Theme.gapNone

    signal sizeDragRequested(int value)

    Layout.fillHeight: true
    Layout.preferredWidth: splitterThickness
    color: splitterColor

    MouseArea {
        id: splitterMouse

        property real dragStartGlobalX: 0
        property int dragStartSize: splitter.currentSize

        acceptedButtons: Qt.LeftButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        cursorShape: Qt.SizeHorCursor
        preventStealing: true
        width: splitter.handleThickness

        onPositionChanged: function (mouse) {
            if (!(pressedButtons & Qt.LeftButton))
                return;
            var movePoint = splitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
            var deltaX = (movePoint.x - dragStartGlobalX) * splitter.dragDirection;
            var nextSize = dragStartSize + deltaX;
            if (splitter.clampSize)
                nextSize = splitter.clampSize(nextSize);
            if (isFinite(nextSize) && nextSize !== splitter.currentSize)
                splitter.sizeDragRequested(Math.floor(nextSize));
        }
        onPressed: function (mouse) {
            var pressPoint = splitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
            dragStartGlobalX = pressPoint.x;
            dragStartSize = splitter.currentSize;
        }
    }
}
