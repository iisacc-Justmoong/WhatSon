import QtQuick
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    readonly property int activeToolbarIndex: hierarchyView.selectionStore ? hierarchyView.selectionStore.activeIndex : 0
    property color panelColor: LV.Theme.panelBackground04
    readonly property var selectionStore: (typeof sidebarSelectionStore !== "undefined" && sidebarSelectionStore) ? sidebarSelectionStore : null
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
    readonly property var toolbarIconNames: hierarchyView.selectionStore ? hierarchyView.selectionStore.toolbarIconNames : []

    function syncSidebarProperties() {
        if (!sidebarLoader.item)
            return;
        sidebarLoader.item.activeToolbarIndex = hierarchyView.activeToolbarIndex;
        sidebarLoader.item.hierarchyViewModel = hierarchyView.selectionStore;
        sidebarLoader.item.panelColor = hierarchyView.panelColor;
        sidebarLoader.item.toolbarIconNames = hierarchyView.toolbarIconNames;
    }
    function updateActiveToolbarIndex(index) {
        if (index < 0 || !hierarchyView.selectionStore || hierarchyView.selectionStore.activeIndex === index)
            return;
        hierarchyView.selectionStore.activeIndex = index;
    }

    onActiveToolbarIndexChanged: hierarchyView.syncSidebarProperties()
    onPanelColorChanged: hierarchyView.syncSidebarProperties()
    onSelectionStoreChanged: hierarchyView.syncSidebarProperties()
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
