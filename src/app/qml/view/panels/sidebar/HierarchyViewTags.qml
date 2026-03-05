import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewTags

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.HierarchyViewTags") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    defaultToolbarIndex: 3
    frameName: "HierarchyView-Tags"
    frameNodeId: "sidebar.hierarchy.tags"
}
