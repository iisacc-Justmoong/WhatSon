import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewTags

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 3
    frameName: "HierarchyView-Tags"
    frameNodeId: "sidebar.hierarchy.tags"
}
