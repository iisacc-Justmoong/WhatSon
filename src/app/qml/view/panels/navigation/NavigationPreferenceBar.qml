import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: preferenceBar

    property bool detailPanelCollapsed: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationPreferenceBar") : null

    signal toggleDetailPanelRequested
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

        iconName: "audioToAudio"
    }
    LV.IconButton {
        id: detailPanelControlButton

        iconName: "columnIndex"
        rotation: 180
        transformOrigin: Item.Center

        onClicked: {
            preferenceBar.requestViewHook(preferenceBar.detailPanelCollapsed ? "expand-detail-panel" : "collapse-detail-panel");
            preferenceBar.toggleDetailPanelRequested();
        }
    }
}
