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
    readonly property bool listVisible: root.listViewWidth > 0
    property int minContentWidth: 320
    property int minDisplayHeight: 160
    property int minDrawerHeight: 120
    property int minListViewWidth: 132
    property int minRightPanelWidth: 132
    property int minSidebarWidth: 96
    property color rightPanelColor: "#63556a"
    property int rightPanelWidth: 194
    readonly property bool rightVisible: root.rightPanelWidth > 0
    property color sidebarColor: "#3b4b63"
    property int sidebarWidth: 216
    property color splitterColor: "transparent"
    property int splitterHandleThickness: 12
    property int splitterThickness: 0

    signal drawerHeightDragRequested(int value)
    signal listViewWidthDragRequested(int value)
    signal rightPanelWidthDragRequested(int value)
    signal sidebarWidthDragRequested(int value)

    function clampListViewWidth(value) {
        if (!root.listVisible)
            return 0;
        var occupiedWidth = root.sidebarWidth + (root.rightVisible ? root.rightPanelWidth : 0) + root.totalSplitterWidth();
        var maxListWidth = Math.max(root.minListViewWidth, root.width - root.minContentWidth - occupiedWidth);
        return Math.max(root.minListViewWidth, Math.min(maxListWidth, value));
    }
    function clampRightPanelWidth(value) {
        if (!root.rightVisible)
            return 0;
        var occupiedWidth = root.sidebarWidth + (root.listVisible ? root.listViewWidth : 0) + root.totalSplitterWidth();
        var maxRightWidth = Math.max(root.minRightPanelWidth, root.width - root.minContentWidth - occupiedWidth);
        return Math.max(root.minRightPanelWidth, Math.min(maxRightWidth, value));
    }
    function clampSidebarWidth(value) {
        var occupiedWidth = (root.listVisible ? root.listViewWidth : 0) + (root.rightVisible ? root.rightPanelWidth : 0) + root.totalSplitterWidth();
        var maxSidebarWidth = Math.max(root.minSidebarWidth, root.width - root.minContentWidth - occupiedWidth);
        return Math.max(root.minSidebarWidth, Math.min(maxSidebarWidth, value));
    }
    function totalSplitterWidth() {
        var width = root.splitterThickness;
        if (root.listVisible)
            width += root.splitterThickness;
        if (root.rightVisible)
            width += root.splitterThickness;

    }

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
            id: sidebarSplitter

            Layout.fillHeight: true
            Layout.preferredWidth: root.splitterThickness
            color: root.splitterColor

            MouseArea {
                id: sidebarSplitterMouse

                property real dragStartGlobalX: 0
                property int dragStartSidebarWidth: root.sidebarWidth

                acceptedButtons: Qt.LeftButton
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                cursorShape: Qt.SizeHorCursor
                preventStealing: true
                width: root.splitterHandleThickness

                onPositionChanged: function (mouse) {
                    if (!(pressedButtons & Qt.LeftButton))
                        return;
                    var movePoint = sidebarSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    var deltaX = movePoint.x - dragStartGlobalX;
                    var nextSidebarWidth = root.clampSidebarWidth(dragStartSidebarWidth + deltaX);
                    if (isFinite(nextSidebarWidth) && nextSidebarWidth !== root.sidebarWidth)
                        root.sidebarWidthDragRequested(nextSidebarWidth);
                }
                onPressed: function (mouse) {
                    var pressPoint = sidebarSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    dragStartGlobalX = pressPoint.x;
                    dragStartSidebarWidth = root.sidebarWidth;
                }
            }
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: root.listViewWidth
            color: root.listViewColor
            visible: root.listVisible
        }
        Rectangle {
            id: listSplitter

            Layout.fillHeight: true
            Layout.preferredWidth: root.splitterThickness
            color: root.splitterColor
            visible: root.listVisible

            MouseArea {
                id: listSplitterMouse

                property real dragStartGlobalX: 0
                property int dragStartListWidth: root.listViewWidth

                acceptedButtons: Qt.LeftButton
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                cursorShape: Qt.SizeHorCursor
                preventStealing: true
                width: root.splitterHandleThickness

                onPositionChanged: function (mouse) {
                    if (!(pressedButtons & Qt.LeftButton))
                        return;
                    var movePoint = listSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    var deltaX = movePoint.x - dragStartGlobalX;
                    var nextListWidth = root.clampListViewWidth(dragStartListWidth + deltaX);
                    if (isFinite(nextListWidth) && nextListWidth !== root.listViewWidth)
                        root.listViewWidthDragRequested(nextListWidth);
                }
                onPressed: function (mouse) {
                    var pressPoint = listSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    dragStartGlobalX = pressPoint.x;
                    dragStartListWidth = root.listViewWidth;
                }
            }
        }
        ContentViewLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            displayColor: root.contentsDisplayColor
            drawerColor: root.drawerColor
            drawerHeight: root.drawerHeight
            minDisplayHeight: root.minDisplayHeight
            minDrawerHeight: root.minDrawerHeight
            panelColor: root.contentPanelColor
            splitterColor: root.splitterColor
            splitterThickness: root.splitterThickness

            onDrawerHeightDragRequested: function (value) {
                root.drawerHeightDragRequested(value);
            }
        }
        Rectangle {
            id: rightSplitter

            Layout.fillHeight: true
            Layout.preferredWidth: root.splitterThickness
            color: root.splitterColor
            visible: root.rightVisible

            MouseArea {
                id: rightSplitterMouse

                property real dragStartGlobalX: 0
                property int dragStartRightWidth: root.rightPanelWidth

                acceptedButtons: Qt.LeftButton
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                cursorShape: Qt.SizeHorCursor
                preventStealing: true
                width: root.splitterHandleThickness

                onPositionChanged: function (mouse) {
                    if (!(pressedButtons & Qt.LeftButton))
                        return;
                    var movePoint = rightSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    var deltaX = movePoint.x - dragStartGlobalX;
                    var nextRightWidth = root.clampRightPanelWidth(dragStartRightWidth - deltaX);
                    if (isFinite(nextRightWidth) && nextRightWidth !== root.rightPanelWidth)
                        root.rightPanelWidthDragRequested(nextRightWidth);
                }
                onPressed: function (mouse) {
                    var pressPoint = rightSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    dragStartGlobalX = pressPoint.x;
                    dragStartRightWidth = root.rightPanelWidth;
                }
            }
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: root.rightPanelWidth
            color: root.rightPanelColor
            visible: root.rightVisible
        }
    }
}
