import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    readonly property bool createFolderEnabled: sidebarView.createFolderEnabled
    readonly property int currentHierarchy: hierarchyView.sidebarHierarchyController ? hierarchyView.sidebarHierarchyController.resolvedActiveHierarchyIndex : 0
    property bool footerVisible: true
    property int horizontalInset: LV.Theme.gap2
    readonly property var noteDropTargetView: sidebarView
    property color panelColor: "transparent"
    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("HierarchySidebarLayout") : null
    readonly property var resolvedHierarchyController: {
        const sidebarController = hierarchyView.sidebarHierarchyController;
        if (sidebarController && sidebarController.hierarchyControllerForIndex !== undefined) {
            const directHierarchyController = sidebarController.hierarchyControllerForIndex(hierarchyView.currentHierarchy);
            if (directHierarchyController)
                return directHierarchyController;
        }
        const activeHierarchyController = sidebarController ? sidebarController.resolvedHierarchyController : null;
        if (activeHierarchyController)
            return activeHierarchyController;
        return null;
    }
    required property var sidebarHierarchyController
    property bool searchFieldVisible: false
    property int searchHeaderMinHeight: LV.Theme.gap24
    property int searchHeaderTopGap: LV.Theme.gap4
    property int searchListGap: LV.Theme.gapNone
    property int searchHeaderVerticalInset: LV.Theme.gap2
    property string searchText: ""
    property int toolbarFrameWidth: LV.Theme.inputMinWidth + LV.Theme.gap20
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    property int verticalInset: LV.Theme.gap2

    signal activeToolbarIndexChangeRequested(int index)
    signal hierarchyItemActivated(var item, int itemId, int index)
    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal viewHookRequested

    function setActiveHierarchyIndex(index) {
        if (!hierarchyView.sidebarHierarchyController || hierarchyView.sidebarHierarchyController.setActiveHierarchyIndex === undefined)
            return;
        hierarchyView.sidebarHierarchyController.setActiveHierarchyIndex(index);
        hierarchyView.activeToolbarIndexChangeRequested(hierarchyView.sidebarHierarchyController.resolvedActiveHierarchyIndex);
    }
    function requestCreateFolder() {
        sidebarView.requestCreateFolder();
    }

    QtObject {
        id: hierarchyEnum

        readonly property int bookmarks: 2
        readonly property int event: 6
        readonly property int library: 0
        readonly property int preset: 7
        readonly property int progress: 5
        readonly property int projects: 1
        readonly property int resources: 4
        readonly property int tags: 3
    }
    HierarchyDragDropBridge {
        id: hierarchyDragDropBridge

        hierarchyController: hierarchyView.resolvedHierarchyController
    }
    HierarchyInteractionBridge {
        id: hierarchyInteractionBridge

        hierarchyController: hierarchyView.resolvedHierarchyController
    }
    SidebarHierarchyView {
        id: sidebarView

        activeToolbarIndex: hierarchyView.currentHierarchy
        anchors.fill: parent
        defaultToolbarIndex: hierarchyEnum.library
        footerVisible: hierarchyView.footerVisible
        hierarchyDragDropBridge: hierarchyDragDropBridge
        hierarchyInteractionBridge: hierarchyInteractionBridge
        hierarchyEditable: hierarchyDragDropBridge.reorderContractAvailable
        hierarchyController: hierarchyView.resolvedHierarchyController
        horizontalInset: hierarchyView.horizontalInset
        bookmarkPaletteVisualsEnabled: hierarchyView.currentHierarchy === hierarchyEnum.bookmarks
        panelColor: hierarchyView.panelColor
        searchFieldVisible: hierarchyView.searchFieldVisible
        searchHeaderMinHeight: hierarchyView.searchHeaderMinHeight
        searchHeaderTopGap: hierarchyView.searchHeaderTopGap
        searchListGap: hierarchyView.searchListGap
        searchHeaderVerticalInset: hierarchyView.searchHeaderVerticalInset
        searchText: hierarchyView.searchText
        toolbarFrameWidth: hierarchyView.toolbarFrameWidth
        toolbarIconNames: hierarchyView.toolbarIconNames
        verticalInset: hierarchyView.verticalInset

        onSearchSubmitted: function (text) {
            hierarchyView.searchText = text;
            hierarchyView.searchSubmitted(text);
        }
        onHierarchyItemActivated: function (item, itemId, index) {
            hierarchyView.hierarchyItemActivated(item, itemId, index);
        }
        onSearchTextEdited: function (text) {
            hierarchyView.searchText = text;
            hierarchyView.searchTextEdited(text);
        }
        onToolbarIndexChangeRequested: function (index) {
            hierarchyView.setActiveHierarchyIndex(index);
        }
    }
}
