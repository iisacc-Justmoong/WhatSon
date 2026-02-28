import QtQuick
import LVRS 1.0 as LV
import "../sidebar" as SidebarView

Item {
    id: hierarchyView

    readonly property int activeToolbarIndex: hierarchyView.hierarchyStore ? hierarchyView.hierarchyStore.activeIndex : 0
    readonly property var hierarchyStore: (typeof sidebarHierarchyStore !== "undefined" && sidebarHierarchyStore) ? sidebarHierarchyStore : null
    property color panelColor: LV.Theme.panelBackground04
    readonly property var toolbarIconNames: hierarchyView.hierarchyStore ? hierarchyView.hierarchyStore.toolbarIconNames : []

    SidebarView.LibraryView {
        activeToolbarIndex: hierarchyView.activeToolbarIndex
        anchors.fill: parent
        depthItems: []
        hierarchyViewModel: (typeof libraryHierarchyViewModel !== "undefined" && libraryHierarchyViewModel) ? libraryHierarchyViewModel : null
        panelColor: hierarchyView.panelColor
        toolbarIconNames: hierarchyView.toolbarIconNames

        onToolbarIndexChangeRequested: function (index) {
            if (index < 0 || !hierarchyView.hierarchyStore || hierarchyView.hierarchyStore.activeIndex === index)
                return;
            hierarchyView.hierarchyStore.activeIndex = index;
        }
    }
}
