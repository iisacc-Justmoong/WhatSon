import QtQuick
import LVRS 1.0 as LV
import "sidebar" as SidebarView

Item {
    id: hierarchyView

    readonly property int activeToolbarIndex: hierarchyView.selectionStore ? hierarchyView.selectionStore.activeIndex : 0
    readonly property var hierarchyStore: (typeof sidebarHierarchyStore !== "undefined" && sidebarHierarchyStore) ? sidebarHierarchyStore : null
    property color panelColor: LV.Theme.panelBackground04
    readonly property var selectionStore: (typeof sidebarSelectionStore !== "undefined" && sidebarSelectionStore) ? sidebarSelectionStore : null
    readonly property var toolbarIconNames: hierarchyView.selectionStore ? hierarchyView.selectionStore.toolbarIconNames : []

    function updateActiveToolbarIndex(index) {
        if (index < 0 || !hierarchyView.selectionStore || hierarchyView.selectionStore.activeIndex === index)
            return;
        hierarchyView.selectionStore.activeIndex = index;
    }

    SidebarView.LibraryView {
        id: sidebarView

        activeToolbarIndex: hierarchyView.activeToolbarIndex
        anchors.fill: parent
        hierarchyViewModel: hierarchyView.selectionStore
        panelColor: hierarchyView.panelColor
        toolbarIconNames: hierarchyView.toolbarIconNames

        onToolbarIndexChangeRequested: function (index) {
            hierarchyView.updateActiveToolbarIndex(index);
        }
    }
}
