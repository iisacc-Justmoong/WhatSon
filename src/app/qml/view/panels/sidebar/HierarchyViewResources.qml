import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewResources

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 4
    frameName: "HierarchyView-Resources"
    frameNodeId: "sidebar.hierarchy.resources"
}
