import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.HStack {
    id: applicationPresentationBar

    property bool compactMode: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationPresentationBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: LV.Theme.gap12

    NavigationPreferenceBar {
        Layout.alignment: Qt.AlignVCenter
    }
}
