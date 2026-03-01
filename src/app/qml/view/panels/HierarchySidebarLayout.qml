import QtQuick
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    property int activeToolbarIndex: 0
    property var bookmarksViewModel: null
    readonly property int currentHierarchy: normalizeHierarchyIndex(activeToolbarIndex)
    property var eventViewModel: null
    property var libraryViewModel: null
    property color panelColor: LV.Theme.panelBackground04
    property var presetViewModel: null
    property var progressViewModel: null
    property var projectsViewModel: null
    property var resourcesViewModel: null
    property var tagsViewModel: null
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]

    signal activeToolbarIndexChangeRequested(int index)

    function frameNameForHierarchy(index) {
        switch (index) {
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
            return "HierarchyView-Library";
        }
    }
    function frameNodeIdForHierarchy(index) {
        switch (index) {
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
            return "sidebar.hierarchy.library";
        }
    }
    function modelForHierarchy(index) {
        switch (index) {
        case hierarchyEnum.projects:
            return hierarchyView.projectsViewModel;
        case hierarchyEnum.bookmarks:
            return hierarchyView.bookmarksViewModel;
        case hierarchyEnum.tags:
            return hierarchyView.tagsViewModel;
        case hierarchyEnum.resources:
            return hierarchyView.resourcesViewModel;
        case hierarchyEnum.progress:
            return hierarchyView.progressViewModel;
        case hierarchyEnum.event:
            return hierarchyView.eventViewModel;
        case hierarchyEnum.preset:
            return hierarchyView.presetViewModel;
        default:
            return hierarchyView.libraryViewModel;
        }
    }
    function normalizeHierarchyIndex(index) {
        var numericIndex = Number(index);
        if (!isFinite(numericIndex))
            return hierarchyEnum.library;
        var normalizedIndex = Math.floor(numericIndex);
        if (normalizedIndex < hierarchyEnum.library || normalizedIndex > hierarchyEnum.preset)
            return hierarchyEnum.library;
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
        hierarchyViewModel: hierarchyView.modelForHierarchy(hierarchyView.currentHierarchy)
        panelColor: hierarchyView.panelColor
        toolbarIconNames: hierarchyView.toolbarIconNames

        onToolbarIndexChangeRequested: function (index) {
            const nextIndex = hierarchyView.normalizeHierarchyIndex(index);
            if (nextIndex === hierarchyView.currentHierarchy)
                return;
            hierarchyView.activeToolbarIndexChangeRequested(nextIndex);
        }
    }
}
