import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import ".." as NavigationShared

LV.HStack {
    id: applicationViewBar

    property bool compactMode: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationViewBar") : null

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
    }
}
