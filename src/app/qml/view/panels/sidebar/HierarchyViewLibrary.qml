import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewLibrary

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.HierarchyViewLibrary") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    defaultToolbarIndex: 0
    frameName: "HierarchyView-Library"
    frameNodeId: "sidebar.hierarchy.library"
}
