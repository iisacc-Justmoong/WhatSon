import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: preferenceBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationPreferenceBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: 2

    LV.IconButton {
        id: preferenceButton

        iconName: "actionGroupNew"
    }
    LV.IconButton {
        id: detailPanelControlButton

        iconName: "columnIndex"
    }
}
