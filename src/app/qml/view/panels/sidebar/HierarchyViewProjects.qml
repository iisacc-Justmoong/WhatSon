import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewProjects

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 1
    frameName: "HierarchyView-Projects"
    frameNodeId: "sidebar.hierarchy.projects"
}
