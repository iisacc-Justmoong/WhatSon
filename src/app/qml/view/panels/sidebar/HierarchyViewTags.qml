import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewTags

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.HierarchyViewTags") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 3
    frameName: "HierarchyView-Tags"
    frameNodeId: "sidebar.hierarchy.tags"
}
