import QtQuick
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    property int activeToolbarIndex: 0
    readonly property int currentHierarchy: normalizeHierarchyIndex(hierarchyView.resolvedActiveHierarchyIndex)
    property int horizontalInset: 2
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("HierarchySidebarLayout") : null
    readonly property int resolvedActiveHierarchyIndex: resolveActiveHierarchyIndex()
    readonly property var resolvedHierarchyViewModel: resolveHierarchyViewModel(hierarchyView.currentHierarchy)
    property var sidebarHierarchyViewModel: null
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]

    signal activeToolbarIndexChangeRequested(int index)
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
    function modelForHierarchy(index) {
        return hierarchyView.resolveHierarchyViewModel(index);
    }
    function normalizeHierarchyIndex(index) {
        var numericIndex = Number(index);
        if (!isFinite(numericIndex))
            return hierarchyEnum.library;
        var normalizedIndex = Math.floor(numericIndex);
        if (normalizedIndex < hierarchyEnum.library || normalizedIndex > hierarchyEnum.preset)


    }
    function resolveActiveHierarchyIndex() {
        if (!hierarchyView.sidebarHierarchyViewModel || hierarchyView.sidebarHierarchyViewModel.activeHierarchyIndex === undefined)
            return hierarchyView.activeToolbarIndex;
        var numericIndex = Number(hierarchyView.sidebarHierarchyViewModel.activeHierarchyIndex);
        if (!isFinite(numericIndex))
            return hierarchyView.activeToolbarIndex;
        return Math.floor(numericIndex);
    }
    function resolveHierarchyViewModel(index) {
        var normalizedIndex = hierarchyView.normalizeHierarchyIndex(index);
        if (!hierarchyView.sidebarHierarchyViewModel)
            return null;
        if (hierarchyView.sidebarHierarchyViewModel.hierarchyViewModelForIndex !== undefined)
            return hierarchyView.sidebarHierarchyViewModel.hierarchyViewModelForIndex(normalizedIndex);
        if (normalizedIndex === hierarchyView.normalizeHierarchyIndex(hierarchyView.resolveActiveHierarchyIndex()) && hierarchyView.sidebarHierarchyViewModel.activeHierarchyViewModel !== undefined) {
            return hierarchyView.sidebarHierarchyViewModel.activeHierarchyViewModel;
        }
        return null;
    }
    function setActiveHierarchyIndex(index) {
        var normalizedIndex = hierarchyView.normalizeHierarchyIndex(index);
        if (hierarchyView.sidebarHierarchyViewModel && hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex !== undefined) {
            hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex(normalizedIndex);
        } else if (hierarchyView.activeToolbarIndex !== normalizedIndex) {
            hierarchyView.activeToolbarIndex = normalizedIndex;
        }
        hierarchyView.activeToolbarIndexChangeRequested(normalizedIndex);
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
    SidebarHierarchyView {
        id: sidebarView

        activeToolbarIndex: hierarchyView.currentHierarchy
        anchors.fill: parent
        defaultToolbarIndex: hierarchyEnum.library
        frameName: hierarchyView.frameNameForHierarchy(hierarchyView.currentHierarchy)
        frameNodeId: hierarchyView.frameNodeIdForHierarchy(hierarchyView.currentHierarchy)
        hierarchyViewModel: hierarchyView.resolvedHierarchyViewModel
        horizontalInset: hierarchyView.horizontalInset
        panelColor: hierarchyView.panelColor
        toolbarIconNames: hierarchyView.toolbarIconNames

        onToolbarIndexChangeRequested: function (index) {
            hierarchyView.setActiveHierarchyIndex(index);
        }
    }
}
