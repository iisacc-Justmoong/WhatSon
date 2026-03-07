import QtQuick
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    property int activeToolbarIndex: 0
    readonly property int currentHierarchy: normalizeHierarchyIndex(sidebarHierarchyViewModel && sidebarHierarchyViewModel.activeHierarchyIndex !== undefined ? sidebarHierarchyViewModel.activeHierarchyIndex : activeToolbarIndex)
    property int horizontalInset: 2
    property color panelColor: LV.Theme.panelBackground04
    property var sidebarHierarchyViewModel: null
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]

    signal activeToolbarIndexChangeRequested(int index)

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
        if (!hierarchyView.sidebarHierarchyViewModel || hierarchyView.sidebarHierarchyViewModel.hierarchyViewModelForIndex === undefined)
            return null;
        return hierarchyView.sidebarHierarchyViewModel.hierarchyViewModelForIndex(index);
    }
    function normalizeHierarchyIndex(index) {
        var numericIndex = Number(index);
        if (!isFinite(numericIndex))
            return -1;
        var normalizedIndex = Math.floor(numericIndex);
        if (normalizedIndex < hierarchyEnum.library || normalizedIndex > hierarchyEnum.preset)
            return -1;
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

        activeToolbarIndex: hierarchyView.currentHierarchy >= 0 ? hierarchyView.currentHierarchy : hierarchyEnum.library
        anchors.fill: parent
        defaultToolbarIndex: hierarchyEnum.library
        frameName: hierarchyView.frameNameForHierarchy(hierarchyView.currentHierarchy)
        frameNodeId: hierarchyView.frameNodeIdForHierarchy(hierarchyView.currentHierarchy)
        hierarchyViewModel: hierarchyView.sidebarHierarchyViewModel && hierarchyView.sidebarHierarchyViewModel.activeHierarchyViewModel !== undefined ? hierarchyView.sidebarHierarchyViewModel.activeHierarchyViewModel : hierarchyView.modelForHierarchy(hierarchyView.currentHierarchy)
        horizontalInset: hierarchyView.horizontalInset
        panelColor: hierarchyView.panelColor
        toolbarIconNames: hierarchyView.toolbarIconNames

        onToolbarIndexChangeRequested: function (index) {
            const nextIndex = hierarchyView.normalizeHierarchyIndex(index);
            if (nextIndex < 0)
                return;
            if (nextIndex === hierarchyView.currentHierarchy)
                return;
            if (hierarchyView.sidebarHierarchyViewModel && hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex !== undefined)
                hierarchyView.sidebarHierarchyViewModel.setActiveHierarchyIndex(nextIndex);
            hierarchyView.activeToolbarIndexChangeRequested(nextIndex);
        }
    }
}
