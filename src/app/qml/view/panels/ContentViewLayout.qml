import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: contentsView

    property color displayColor: LV.Theme.panelBackground09
    property color drawerColor: LV.Theme.panelBackground11
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    property color panelColor: LV.Theme.panelBackground07
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ContentViewLayout") : null
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone

    signal drawerHeightDragRequested(int value)
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    function clampDrawerHeight(value) {
        var maxDrawer = Math.max(contentsView.minDrawerHeight, contentsView.height - contentsView.minDisplayHeight - contentsView.splitterThickness);
        return Math.max(contentsView.minDrawerHeight, Math.min(maxDrawer, value));
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true

    Rectangle {
        anchors.fill: parent
        color: contentsView.panelColor
    }
    LV.VStack {
        id: drawerView

        anchors.fill: parent
        spacing: LV.Theme.gapNone

        Rectangle {
            id: contentsDisplayView

            Layout.fillHeight: true
            Layout.fillWidth: true
            color: contentsView.displayColor
        }
        Rectangle {
            id: drawerSplitter

            Layout.fillWidth: true
            Layout.preferredHeight: contentsView.splitterThickness
            color: contentsView.splitterColor

            MouseArea {
                id: drawerSplitterMouse

                property int dragStartDrawerHeight: contentsView.drawerHeight
                property real dragStartGlobalY: 0

                acceptedButtons: Qt.LeftButton
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                cursorShape: Qt.SizeVerCursor
                height: contentsView.splitterHandleThickness
                preventStealing: true

                onPositionChanged: function (mouse) {
                    if (!(pressedButtons & Qt.LeftButton))
                        return;
                    var movePoint = drawerSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    var deltaY = movePoint.y - dragStartGlobalY;
                    var nextDrawerHeight = contentsView.clampDrawerHeight(dragStartDrawerHeight - deltaY);
                    if (nextDrawerHeight !== contentsView.drawerHeight)
                        contentsView.drawerHeightDragRequested(nextDrawerHeight);
                }
                onPressed: function (mouse) {
                    var pressPoint = drawerSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    dragStartGlobalY = pressPoint.y;
                    dragStartDrawerHeight = contentsView.drawerHeight;
                }
            }
        }
        Rectangle {
            id: drawer

            Layout.fillWidth: true
            Layout.preferredHeight: contentsView.clampDrawerHeight(contentsView.drawerHeight)
            color: contentsView.drawerColor
        }
    }
}
