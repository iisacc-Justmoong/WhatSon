import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewEvent

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 6
    frameName: "HierarchyView-Event"
    frameNodeId: "sidebar.hierarchy.event"
}
