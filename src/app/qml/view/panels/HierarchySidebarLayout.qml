import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    readonly property bool createFolderEnabled: sidebarView.createFolderEnabled
    readonly property int currentHierarchy: hierarchyView.sidebarHierarchyViewModel ? hierarchyView.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex : 0
    readonly property var hierarchyViewBindings: LV.ViewModels.bindings
    readonly property string hierarchyViewId: "HierarchySidebarLayout.activeHierarchy"
    readonly property string resolvedHierarchyViewModelKey: hierarchyView.hierarchyViewModelKeyForIndex(hierarchyView.currentHierarchy)
    property bool footerVisible: true
    property int horizontalInset: LV.Theme.gap2
    readonly property var noteDropTargetView: sidebarView
    property color panelColor: "transparent"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("HierarchySidebarLayout") : null
    readonly property var resolvedHierarchyViewModel: {
        const _ = hierarchyView.hierarchyViewBindings;
        const registeredViewModel = LV.ViewModels.getForView(hierarchyView.hierarchyViewId);
        if (registeredViewModel)
            return registeredViewModel;
        return hierarchyView.sidebarHierarchyViewModel ? hierarchyView.sidebarHierarchyViewModel.resolvedHierarchyViewModel : null;
    }
    required property var sidebarHierarchyViewModel
    property bool searchFieldVisible: false
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

    function setActiveHierarchyIndex(index) {
        if (!hierarchyView.sidebarHierarchyViewModel || hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex === undefined)
            return;
        hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex(index);
        hierarchyView.activeToolbarIndexChangeRequested(hierarchyView.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex);
    }
    function syncHierarchyViewBinding() {
        if (!hierarchyView.resolvedHierarchyViewModelKey.length) {
            LV.ViewModels.unbindView(hierarchyView.hierarchyViewId);
            return;
        }
        if (!LV.ViewModels.bindView(hierarchyView.hierarchyViewId, hierarchyView.resolvedHierarchyViewModelKey, true))
            console.warn("[whatson:mvvm][sidebar] viewId=" + hierarchyView.hierarchyViewId + " key=" + hierarchyView.resolvedHierarchyViewModelKey + " error=" + LV.ViewModels.lastError);
    }
    function requestCreateFolder() {
        sidebarView.requestCreateFolder();
    }
    function hierarchyViewModelKeyForIndex(index) {
        switch (Math.max(0, Math.floor(Number(index) || 0))) {
        case hierarchyEnum.library:
            return "libraryHierarchyViewModel";
        case hierarchyEnum.projects:
            return "projectsHierarchyViewModel";
        case hierarchyEnum.bookmarks:
            return "bookmarksHierarchyViewModel";
        case hierarchyEnum.tags:
            return "tagsHierarchyViewModel";
        case hierarchyEnum.resources:
            return "resourcesHierarchyViewModel";
        case hierarchyEnum.progress:
            return "progressHierarchyViewModel";
        case hierarchyEnum.event:
            return "eventHierarchyViewModel";
        case hierarchyEnum.preset:
            return "presetHierarchyViewModel";
        default:
            return "";
        }
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
    HierarchyInteractionBridge {
        id: hierarchyInteractionBridge

        hierarchyViewModel: hierarchyView.resolvedHierarchyViewModel
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
        hierarchyViewModel: hierarchyView.resolvedHierarchyViewModel
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
    Component.onCompleted: {
        hierarchyView.syncHierarchyViewBinding();
    }
    Component.onDestruction: {
        LV.ViewModels.unbindView(hierarchyView.hierarchyViewId);
    }
    onResolvedHierarchyViewModelKeyChanged: {
        hierarchyView.syncHierarchyViewBinding();
    }
}
