import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: informationBar

    property bool sidebarCollapsed: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationInformationBar") : null

    signal toggleSidebarRequested
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: 4

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
