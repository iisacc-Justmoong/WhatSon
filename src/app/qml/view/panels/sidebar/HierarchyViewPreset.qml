import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewPreset

    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("sidebar.HierarchyViewPreset") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }

    defaultToolbarIndex: 7
}
