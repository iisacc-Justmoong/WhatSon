import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: hStack

    readonly property int activeHierarchyIndex: hStack.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex
    readonly property var activeHierarchyViewModel: hStack.sidebarHierarchyViewModel.resolvedHierarchyViewModel
    readonly property var activeNoteListModel: hStack.sidebarHierarchyViewModel.resolvedNoteListModel
    property color compactCanvasColor: LV.Theme.panelBackground01
    property bool compactMode: false
    property color contentsDisplayColor: LV.Theme.surfaceAlt
    property color drawerColor: LV.Theme.panelBackground08
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property int effectiveMinSidebarWidth: Math.max(minSidebarWidth, LV.Theme.gap20 * 7 + LV.Theme.gap12)
    property color listViewColor: LV.Theme.panelBackground08
    property int listViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property bool listVisible: hStack.listViewWidth > 0
    property int minContentWidth: LV.Theme.dialogMaxWidth - LV.Theme.gap20 * 2
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    property int minListViewWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    property int minRightPanelWidth: 145
    property int minSidebarWidth: LV.Theme.gap20 * 7 + LV.Theme.gap12
    property var noteDeletionViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("BodyLayout") : null
    property color rightPanelColor: LV.Theme.panelBackground06
    property int rightPanelWidth: 194
    readonly property bool rightVisible: hStack.rightPanelWidth > 0
    property color sidebarColor: LV.Theme.panelBackground04
    required property var sidebarHierarchyViewModel
    readonly property bool sidebarVisible: hStack.sidebarWidth > 0
    property int sidebarHorizontalInset: 2
    property int sidebarWidth: LV.Theme.gap24 * 9
    property color splitterColor: LV.Theme.panelBackground10
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]

    signal drawerHeightDragRequested(int value)
    signal listViewWidthDragRequested(int value)
    signal rightPanelWidthDragRequested(int value)
    signal sidebarWidthDragRequested(int value)
    signal viewHookRequested

    function clampListViewWidth(value) {
        const numericValue = Number(value);
        const resolvedValue = isFinite(numericValue) ? numericValue : hStack.listViewWidth;
        const occupiedWidth = hStack.totalSplitterWidth() + (hStack.sidebarVisible ? hStack.sidebarWidth : 0) + hStack.minContentWidth + (hStack.rightVisible ? hStack.minRightPanelWidth : 0);
        const maxWidth = Math.max(hStack.minListViewWidth, hStack.width - occupiedWidth);
        return Math.max(hStack.minListViewWidth, Math.min(maxWidth, Math.floor(resolvedValue)));
    }
    function clampRightPanelWidth(value) {
        const numericValue = Number(value);
        const resolvedValue = isFinite(numericValue) ? numericValue : hStack.rightPanelWidth;
        const occupiedWidth = hStack.totalSplitterWidth() + (hStack.sidebarVisible ? hStack.sidebarWidth : 0) + hStack.minContentWidth + (hStack.listVisible ? hStack.minListViewWidth : 0);
        const maxWidth = Math.max(hStack.minRightPanelWidth, hStack.width - occupiedWidth);
        return Math.max(hStack.minRightPanelWidth, Math.min(maxWidth, Math.floor(resolvedValue)));
    }
    function clampSidebarWidth(value) {
        const numericValue = Number(value);
        const resolvedValue = isFinite(numericValue) ? numericValue : hStack.sidebarWidth;
        const occupiedWidth = hStack.totalSplitterWidth() + hStack.minContentWidth + (hStack.listVisible ? hStack.minListViewWidth : 0) + (hStack.rightVisible ? hStack.minRightPanelWidth : 0);
        const maxWidth = Math.max(hStack.effectiveMinSidebarWidth, hStack.width - occupiedWidth);
        return Math.max(hStack.effectiveMinSidebarWidth, Math.min(maxWidth, Math.floor(resolvedValue)));
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function totalSplitterWidth() {
        // CRITICAL REGRESSION GUARD:
        // This function MUST return a finite number. If return is removed or undefined/NaN leaks,
        // clamp math fails and panel edge drag-resizing becomes blocked (historically repeated issue).
        var width = hStack.sidebarVisible ? hStack.splitterThickness : 0;
        if (hStack.listVisible)
            width += hStack.splitterThickness;
        if (hStack.rightVisible)
            width += hStack.splitterThickness;
        return isFinite(width) ? width : 0;
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
                Layout.minimumWidth: hStack.sidebarVisible ? hStack.effectiveMinSidebarWidth : 0
                Layout.preferredWidth: hStack.sidebarVisible ? hStack.sidebarWidth : 0
                horizontalInset: hStack.sidebarHorizontalInset
                panelColor: hStack.sidebarColor
                sidebarHierarchyViewModel: hStack.sidebarHierarchyViewModel
                toolbarIconNames: hStack.toolbarIconNames
                visible: hStack.sidebarVisible
            }
            PanelEdgeSplitter {
                id: sideBarSplitter

                clampSize: function (value) {
                    return hStack.clampSidebarWidth(value);
                }
                currentSize: hStack.sidebarWidth
                dragDirection: 1
                handleThickness: hStack.splitterHandleThickness
                splitterColor: hStack.splitterColor
                splitterThickness: hStack.splitterThickness
                visible: hStack.sidebarVisible

                onSizeDragRequested: function (value) {
                    hStack.sidebarWidthDragRequested(value);
                }
            }
            Rectangle {
                id: listBar

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.minListViewWidth
                Layout.preferredWidth: hStack.listVisible ? hStack.listViewWidth : 0
                color: LV.Theme.accentTransparent
                visible: hStack.listVisible

                ListBarLayout {
                    activeToolbarIndex: hStack.activeHierarchyIndex
                    anchors.fill: parent
                    noteDeletionViewModel: hStack.noteDeletionViewModel
                    noteListModel: hStack.activeNoteListModel
                    panelColor: hStack.sidebarColor
                }
            }
            PanelEdgeSplitter {
                id: listSplitter

                clampSize: function (value) {
                    return hStack.clampListViewWidth(value);
                }
                currentSize: hStack.listViewWidth
                dragDirection: 1
                handleThickness: hStack.splitterHandleThickness
                splitterColor: hStack.splitterColor
                splitterThickness: hStack.splitterThickness
                visible: hStack.listVisible

                onSizeDragRequested: function (value) {
                    hStack.listViewWidthDragRequested(value);
                }
            }
            ContentViewLayout {
                id: contentsView

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumWidth: hStack.minContentWidth
                contentViewModel: hStack.activeHierarchyViewModel
                displayColor: hStack.contentsDisplayColor
                drawerColor: hStack.drawerColor
                drawerHeight: hStack.drawerHeight
                libraryHierarchyViewModel: libraryHierarchyViewModel
                minDisplayHeight: hStack.minDisplayHeight
                minDrawerHeight: hStack.minDrawerHeight
                noteListModel: hStack.activeNoteListModel
                splitterColor: hStack.splitterColor
                splitterThickness: hStack.splitterThickness

                onDrawerHeightDragRequested: function (value) {
                    hStack.drawerHeightDragRequested(value);
                }
            }
            PanelEdgeSplitter {
                id: rightSplitter

                clampSize: function (value) {
                    return hStack.clampRightPanelWidth(value);
                }
                currentSize: hStack.rightPanelWidth
                dragDirection: -1
                handleThickness: hStack.splitterHandleThickness
                splitterColor: hStack.splitterColor
                splitterThickness: hStack.splitterThickness
                visible: hStack.rightVisible

                onSizeDragRequested: function (value) {
                    hStack.rightPanelWidthDragRequested(value);
                }
            }
            DetailPanelLayout {
                id: rightPanel

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.rightVisible ? hStack.minRightPanelWidth : 0
                Layout.preferredWidth: hStack.rightVisible ? hStack.rightPanelWidth : 0
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
