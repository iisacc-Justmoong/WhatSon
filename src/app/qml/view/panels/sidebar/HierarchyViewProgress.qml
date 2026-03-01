import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewProgress

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 5
    frameName: "HierarchyView-Progress"
    frameNodeId: "sidebar.hierarchy.progress"
}
