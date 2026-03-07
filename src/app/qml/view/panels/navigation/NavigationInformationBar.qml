import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: informationBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationInformationBar") : null

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
    }
    LV.IconButton {
        id: profileButton

        iconName: "loggedInUser"
    }
}
