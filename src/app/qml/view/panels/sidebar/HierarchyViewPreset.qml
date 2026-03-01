import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewPreset

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 7
    frameName: "HierarchyView-Preset"
    frameNodeId: "sidebar.hierarchy.preset"
}
