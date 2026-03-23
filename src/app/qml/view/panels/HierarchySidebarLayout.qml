import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    readonly property bool createFolderEnabled: sidebarView.createFolderEnabled
    readonly property int currentHierarchy: hierarchyView.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex
    property bool footerVisible: true
    property int horizontalInset: LV.Theme.gap2
    property color panelColor: LV.Theme.panelBackground01
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("HierarchySidebarLayout") : null
    readonly property var resolvedHierarchyViewModel: hierarchyView.sidebarHierarchyViewModel.resolvedHierarchyViewModel
    required property var sidebarHierarchyViewModel
    property bool searchFieldVisible: false
    property int searchHeaderHorizontalInset: LV.Theme.gap2
    property int searchHeaderMinHeight: LV.Theme.gap24
    property int searchHeaderTopGap: LV.Theme.gap4
    property int searchListGap: LV.Theme.gapNone
    property int searchHeaderVerticalInset: LV.Theme.gap2
    property string searchText: ""
    property int toolbarFrameWidth: 200
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    property int verticalInset: LV.Theme.gap2

    signal activeToolbarIndexChangeRequested(int index)
    signal hierarchyItemActivated(var item, int itemId, int index)
    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal viewHookRequested

    function frameNameForHierarchy(index) {
        switch (index) {
        case hierarchyEnum.library:
            return "HierarchyView-Library";
        case hierarchyEnum.projects:
            return "HierarchyView-Projects";
        case hierarchyEnum.bookmarks:
            return "HierarchyView-Bookmarks";
        case hierarchyEnum.tags:
            return "HierarchyView-Tags";
        case hierarchyEnum.resources:
            return "HierarchyView-Resources";
        case hierarchyEnum.progress:
            return "HierarchyView-Progress";
        case hierarchyEnum.event:
            return "HierarchyView-Event";
        case hierarchyEnum.preset:
            return "HierarchyView-Preset";
        default:
            return "HierarchyView-Unknown";
        }
    }
    function frameNodeIdForHierarchy(index) {
        switch (index) {
        case hierarchyEnum.library:
            return "sidebar.hierarchy.library";
        case hierarchyEnum.projects:
            return "sidebar.hierarchy.projects";
        case hierarchyEnum.bookmarks:
            return "sidebar.hierarchy.bookmarks";
        case hierarchyEnum.tags:
            return "sidebar.hierarchy.tags";
        case hierarchyEnum.resources:
            return "sidebar.hierarchy.resources";
        case hierarchyEnum.progress:
            return "sidebar.hierarchy.progress";
        case hierarchyEnum.event:
            return "sidebar.hierarchy.event";
        case hierarchyEnum.preset:
            return "sidebar.hierarchy.preset";
        default:
            return "sidebar.hierarchy.unknown";
        }
    }
    function setActiveHierarchyIndex(index) {
        hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex(index);
        hierarchyView.activeToolbarIndexChangeRequested(hierarchyView.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex);
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

        hierarchyViewModel: hierarchyView.resolvedHierarchyViewModel
    }
    SidebarHierarchyView {
        id: sidebarView

        activeToolbarIndex: hierarchyView.currentHierarchy
        anchors.fill: parent
        defaultToolbarIndex: hierarchyEnum.library
        footerVisible: hierarchyView.footerVisible
        frameName: hierarchyView.frameNameForHierarchy(hierarchyView.currentHierarchy)
        frameNodeId: hierarchyView.frameNodeIdForHierarchy(hierarchyView.currentHierarchy)
        hierarchyDragDropBridge: hierarchyDragDropBridge
        hierarchyEditable: hierarchyDragDropBridge.reorderContractAvailable
        hierarchyViewModel: hierarchyView.resolvedHierarchyViewModel
        horizontalInset: hierarchyView.horizontalInset
        panelColor: hierarchyView.panelColor
        searchFieldVisible: hierarchyView.searchFieldVisible
        searchHeaderHorizontalInset: hierarchyView.searchHeaderHorizontalInset
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
