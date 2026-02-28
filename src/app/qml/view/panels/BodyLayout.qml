import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: hStack

    property color compactCanvasColor: LV.Theme.panelBackground01
    property bool compactMode: false
    property color contentPanelColor: "#39445b"
    property color contentsDisplayColor: "#495473"
    property color drawerColor: "#665d47"
    property int drawerHeight: 255
    property color listViewColor: "#3a5c57"
    property int listViewWidth: 198
    readonly property bool listVisible: hStack.listViewWidth > 0
    property int minContentWidth: 320
    property int minDisplayHeight: 160
    property int minDrawerHeight: 120
    property int minListViewWidth: 132
    property int minRightPanelWidth: 132
    property int minSidebarWidth: 152
    property color rightPanelColor: "#63556a"
    property int rightPanelWidth: 194
    readonly property bool rightVisible: hStack.rightPanelWidth > 0
    property color sidebarColor: "#3b4b63"
    property int sidebarWidth: 216
    property color splitterColor: "#445066"
    property int splitterHandleThickness: 12
    property int splitterThickness: 0

    signal drawerHeightDragRequested(int value)
    signal listViewWidthDragRequested(int value)
    signal rightPanelWidthDragRequested(int value)
    signal sidebarWidthDragRequested(int value)

    function clampListViewWidth(value) {
        if (!hStack.listVisible)
            return 0;
        var occupiedWidth = hStack.sidebarWidth + (hStack.rightVisible ? hStack.rightPanelWidth : 0) + hStack.totalSplitterWidth();
        var maxListWidth = Math.max(hStack.minListViewWidth, hStack.width - hStack.minContentWidth - occupiedWidth);
        return Math.max(hStack.minListViewWidth, Math.min(maxListWidth, value));
    }
    function clampRightPanelWidth(value) {
        if (!hStack.rightVisible)
            return 0;
        var occupiedWidth = hStack.sidebarWidth + (hStack.listVisible ? hStack.listViewWidth : 0) + hStack.totalSplitterWidth();
        var maxRightWidth = Math.max(hStack.minRightPanelWidth, hStack.width - hStack.minContentWidth - occupiedWidth);
        return Math.max(hStack.minRightPanelWidth, Math.min(maxRightWidth, value));
    }
    function clampSidebarWidth(value) {
        var occupiedWidth = (hStack.listVisible ? hStack.listViewWidth : 0) + (hStack.rightVisible ? hStack.rightPanelWidth : 0) + hStack.totalSplitterWidth();
        var maxSidebarWidth = Math.max(hStack.effectiveMinSidebarWidth, hStack.width - hStack.minContentWidth - occupiedWidth);
        return Math.max(hStack.effectiveMinSidebarWidth, Math.min(maxSidebarWidth, value));
    }
    function totalSplitterWidth() {
        var width = hStack.splitterThickness;
        if (hStack.listVisible)
            width += hStack.splitterThickness;
        if (hStack.rightVisible)
            width += hStack.splitterThickness;
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true

    Item {
        anchors.fill: parent
        visible: !hStack.compactMode

        LV.HStack {
            anchors.fill: parent
            spacing: 0

            HierarchySidebarLayout {
                id: sideBar

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.effectiveMinSidebarWidth
                Layout.preferredWidth: hStack.sidebarWidth
            }
            Rectangle {
                id: sideBarSplitter

                Layout.fillHeight: true
                Layout.preferredWidth: hStack.splitterThickness
                color: hStack.splitterColor

                MouseArea {
                    id: sideBarSplitterMouse

                    property real dragStartGlobalX: 0
                    property int dragStartSidebarWidth: hStack.sidebarWidth

                    acceptedButtons: Qt.LeftButton
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    cursorShape: Qt.SizeHorCursor
                    preventStealing: true
                    width: hStack.splitterHandleThickness

                    onPositionChanged: function (mouse) {
                        if (!(pressedButtons & Qt.LeftButton))
                            return;
                        var movePoint = sideBarSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                        var deltaX = movePoint.x - dragStartGlobalX;
                        var nextSidebarWidth = hStack.clampSidebarWidth(dragStartSidebarWidth + deltaX);
                        if (isFinite(nextSidebarWidth) && nextSidebarWidth !== hStack.sidebarWidth)
                            hStack.sidebarWidthDragRequested(nextSidebarWidth);
                    }
                    onPressed: function (mouse) {
                        var pressPoint = sideBarSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                        dragStartGlobalX = pressPoint.x;
                        dragStartSidebarWidth = hStack.sidebarWidth;
                    }
                }
            }
            Rectangle {
                id: listBar

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.minListViewWidth
                Layout.preferredWidth: hStack.listViewWidth
                color: hStack.listViewColor
                visible: hStack.listVisible
            }
            Rectangle {
                id: listSplitter

                Layout.fillHeight: true
                Layout.preferredWidth: hStack.splitterThickness
                color: hStack.splitterColor
                visible: hStack.listVisible

                MouseArea {
                    id: listSplitterMouse

                    property real dragStartGlobalX: 0
                    property int dragStartListWidth: hStack.listViewWidth

                    acceptedButtons: Qt.LeftButton
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    cursorShape: Qt.SizeHorCursor
                    preventStealing: true
                    width: hStack.splitterHandleThickness

                    onPositionChanged: function (mouse) {
                        if (!(pressedButtons & Qt.LeftButton))
                            return;
                        var movePoint = listSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                        var deltaX = movePoint.x - dragStartGlobalX;
                        var nextListWidth = hStack.clampListViewWidth(dragStartListWidth + deltaX);
                        if (isFinite(nextListWidth) && nextListWidth !== hStack.listViewWidth)
                            hStack.listViewWidthDragRequested(nextListWidth);
                    }
                    onPressed: function (mouse) {
                        var pressPoint = listSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                        dragStartGlobalX = pressPoint.x;
                        dragStartListWidth = hStack.listViewWidth;
                    }
                }
            }
            ContentViewLayout {
                id: contentsView

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumWidth: hStack.minContentWidth
                displayColor: hStack.contentsDisplayColor
                drawerColor: hStack.drawerColor
                drawerHeight: hStack.drawerHeight
                minDisplayHeight: hStack.minDisplayHeight
                minDrawerHeight: hStack.minDrawerHeight
                panelColor: hStack.contentPanelColor
                splitterColor: hStack.splitterColor
                splitterThickness: hStack.splitterThickness

                onDrawerHeightDragRequested: function (value) {
                    hStack.drawerHeightDragRequested(value);
                }
            }
            Rectangle {
                id: rightSplitter

                Layout.fillHeight: true
                Layout.preferredWidth: hStack.splitterThickness
                color: hStack.splitterColor
                visible: hStack.rightVisible

                MouseArea {
                    id: rightSplitterMouse

                    property real dragStartGlobalX: 0
                    property int dragStartRightWidth: hStack.rightPanelWidth

                    acceptedButtons: Qt.LeftButton
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    cursorShape: Qt.SizeHorCursor
                    preventStealing: true
                    width: hStack.splitterHandleThickness

                    onPositionChanged: function (mouse) {
                        if (!(pressedButtons & Qt.LeftButton))
                            return;
                        var movePoint = rightSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                        var deltaX = movePoint.x - dragStartGlobalX;
                        var nextRightWidth = hStack.clampRightPanelWidth(dragStartRightWidth - deltaX);
                        if (isFinite(nextRightWidth) && nextRightWidth !== hStack.rightPanelWidth)
                            hStack.rightPanelWidthDragRequested(nextRightWidth);
                    }
                    onPressed: function (mouse) {
                        var pressPoint = rightSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                        dragStartGlobalX = pressPoint.x;
                        dragStartRightWidth = hStack.rightPanelWidth;
                    }
                }
            }
            DetailPanelLayout {
                id: rightPanel

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.minRightPanelWidth
                Layout.preferredWidth: hStack.rightPanelWidth
                panelColor: hStack.rightPanelColor
                visible: hStack.rightVisible
            }
        }
    }
    Item {
        anchors.fill: parent
        visible: hStack.compactMode

        Rectangle {
            anchors.fill: parent
            color: hStack.compactCanvasColor
        }
    }
}
