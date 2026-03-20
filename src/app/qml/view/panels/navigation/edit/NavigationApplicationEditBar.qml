import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import ".." as NavigationShared

LV.HStack {
    id: applicationEditBar

    property bool compactMode: false
    property bool detailPanelCollapsed: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationEditBar") : null

    signal toggleDetailPanelRequested
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: LV.Theme.gap12

    NavigationShared.NavigationPreferenceBar {
        Layout.alignment: Qt.AlignVCenter
        detailPanelCollapsed: applicationEditBar.detailPanelCollapsed

        onToggleDetailPanelRequested: applicationEditBar.toggleDetailPanelRequested()
    }
}
