import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: informationBar

    property bool sidebarCollapsed: false
    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.NavigationInformationBar") : null

    signal toggleSidebarRequested
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }

    spacing: LV.Theme.gap4

    LV.IconButton {
        id: sidebarControlButton

        iconName: "columnIndex"

        onClicked: {
            informationBar.requestViewHook(informationBar.sidebarCollapsed ? "expand-sidebar" : "collapse-sidebar");
            informationBar.toggleSidebarRequested();
        }
    }
    LV.IconButton {
        id: profileButton

        iconName: "loggedInUser"
    }
}
