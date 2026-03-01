import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: hStack

    property int activeToolbarIndex: 0
    property var bookmarksViewModel: null
    property color compactCanvasColor: LV.Theme.panelBackground01
    property bool compactMode: false
    property color contentPanelColor: LV.Theme.panelBackground07
    property color contentsDisplayColor: LV.Theme.panelBackground09
    property color drawerColor: LV.Theme.panelBackground11
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property int effectiveMinSidebarWidth: Math.max(minSidebarWidth, LV.Theme.gap20 * 7 + LV.Theme.gap12)
    property var libraryViewModel: null
    property color listViewColor: LV.Theme.panelBackground08
    property int listViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property bool listVisible: hStack.listViewWidth > 0
    property int minContentWidth: LV.Theme.dialogMaxWidth - LV.Theme.gap20 * 2
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    property int minListViewWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    property int minRightPanelWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    property int minSidebarWidth: LV.Theme.gap20 * 7 + LV.Theme.gap12
    property var presetViewModel: null
    property var progressViewModel: null
    property var projectsViewModel: null
    property var resourcesViewModel: null
    property color rightPanelColor: LV.Theme.panelBackground08
    property int rightPanelWidth: LV.Theme.inputWidthMd - LV.Theme.gap12
    readonly property bool rightVisible: hStack.rightPanelWidth > 0
    property color sidebarColor: LV.Theme.panelBackground04
    property int sidebarWidth: LV.Theme.gap24 * 9
    property color splitterColor: LV.Theme.panelBackground10
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property var tagsViewModel: null
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]

    signal activeToolbarIndexChangeRequested(int index)
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
            spacing: LV.Theme.gapNone

            HierarchySidebarLayout {
                id: sideBar

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.effectiveMinSidebarWidth
                Layout.preferredWidth: hStack.sidebarWidth
                activeToolbarIndex: hStack.activeToolbarIndex
                bookmarksViewModel: hStack.bookmarksViewModel
                eventViewModel: hStack.eventViewModel
                libraryViewModel: hStack.libraryViewModel
                panelColor: hStack.sidebarColor
                presetViewModel: hStack.presetViewModel
                progressViewModel: hStack.progressViewModel
                projectsViewModel: hStack.projectsViewModel
                resourcesViewModel: hStack.resourcesViewModel
                tagsViewModel: hStack.tagsViewModel
                toolbarIconNames: hStack.toolbarIconNames

                onActiveToolbarIndexChangeRequested: function (index) {
                    hStack.activeToolbarIndexChangeRequested(index);
                }
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
                color: LV.Theme.accentTransparent
                visible: hStack.listVisible

                ListBarLayout {
                    activeToolbarIndex: hStack.activeToolbarIndex
                    anchors.fill: parent
                    noteListModel: (hStack.activeToolbarIndex === 0 && hStack.libraryViewModel) ? hStack.libraryViewModel.noteListModel : null
                    panelColor: hStack.sidebarColor
                }
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
