import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewResources

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.HierarchyViewResources") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    defaultToolbarIndex: 4
    frameName: "HierarchyView-Resources"
    frameNodeId: "sidebar.hierarchy.resources"
}
