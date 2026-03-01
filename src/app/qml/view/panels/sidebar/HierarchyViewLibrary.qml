import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewLibrary

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 0
    frameName: "HierarchyView-Library"
    frameNodeId: "sidebar.hierarchy.library"
}
