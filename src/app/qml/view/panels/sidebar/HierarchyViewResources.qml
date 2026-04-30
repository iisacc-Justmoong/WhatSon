import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewResources

    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("sidebar.HierarchyViewResources") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }

    defaultToolbarIndex: 4
}
