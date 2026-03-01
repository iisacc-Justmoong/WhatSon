import QtQuick
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    property int activeToolbarIndex: 0
    property var bookmarksViewModel: null
    property var eventViewModel: null
    property var libraryViewModel: null
    property color panelColor: LV.Theme.panelBackground04
    property var presetViewModel: null
    property var progressViewModel: null
    property var projectsViewModel: null
    property var resourcesViewModel: null
    readonly property string sidebarSource: {
        switch (hierarchyView.activeToolbarIndex) {
        case 1:
            return "sidebar/HierarchyViewProjects.qml";
        case 2:
            return "sidebar/HierarchyViewBookmarks.qml";
        case 3:
            return "sidebar/HierarchyViewTags.qml";
        case 4:
            return "sidebar/HierarchyViewResources.qml";
        case 5:
            return "sidebar/HierarchyViewProgress.qml";
        case 6:
            return "sidebar/HierarchyViewEvent.qml";
        case 7:
            return "sidebar/HierarchyViewPreset.qml";
        default:
            return "sidebar/HierarchyViewLibrary.qml";
        }
    }
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]

    signal activeToolbarIndexChangeRequested(int index)

    function modelForToolbar(index) {
        switch (index) {
        case 1:
            return hierarchyView.projectsViewModel;
        case 2:
            return hierarchyView.bookmarksViewModel;
        case 3:
            return hierarchyView.tagsViewModel;
        case 4:
            return hierarchyView.resourcesViewModel;
        case 5:
            return hierarchyView.progressViewModel;
        case 6:
            return hierarchyView.eventViewModel;
        case 7:
            return hierarchyView.presetViewModel;
        default:
            return hierarchyView.libraryViewModel;
        }
    }
    function syncSidebarProperties() {
        if (!sidebarLoader.item)
            return;
        sidebarLoader.item.activeToolbarIndex = hierarchyView.activeToolbarIndex;
        sidebarLoader.item.hierarchyViewModel = hierarchyView.modelForToolbar(hierarchyView.activeToolbarIndex);
        sidebarLoader.item.panelColor = hierarchyView.panelColor;
        sidebarLoader.item.toolbarIconNames = hierarchyView.toolbarIconNames;
    }
    function updateActiveToolbarIndex(index) {
        if (index < 0 || hierarchyView.activeToolbarIndex === index)
            return;
        hierarchyView.activeToolbarIndex = index;
        hierarchyView.activeToolbarIndexChangeRequested(index);
    }

    onActiveToolbarIndexChanged: hierarchyView.syncSidebarProperties()
    onBookmarksViewModelChanged: hierarchyView.syncSidebarProperties()
    onEventViewModelChanged: hierarchyView.syncSidebarProperties()
    onLibraryViewModelChanged: hierarchyView.syncSidebarProperties()
    onPanelColorChanged: hierarchyView.syncSidebarProperties()
    onPresetViewModelChanged: hierarchyView.syncSidebarProperties()
    onProgressViewModelChanged: hierarchyView.syncSidebarProperties()
    onProjectsViewModelChanged: hierarchyView.syncSidebarProperties()
    onResourcesViewModelChanged: hierarchyView.syncSidebarProperties()
    onTagsViewModelChanged: hierarchyView.syncSidebarProperties()
    onToolbarIconNamesChanged: hierarchyView.syncSidebarProperties()

    Loader {
        id: sidebarLoader

        anchors.fill: parent
        hierarchyView.sidebarSource

        onLoaded: {
            hierarchyView.syncSidebarProperties();
        }
    }
    Connections {
        function onToolbarIndexChangeRequested(index) {
            hierarchyView.updateActiveToolbarIndex(index);
        }
    }
}
