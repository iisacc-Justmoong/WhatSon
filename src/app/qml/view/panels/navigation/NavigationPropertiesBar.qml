import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: propertiesBar

    property bool compactMode: false
    property var editorViewModeViewModel: null
    property var navigationModeViewModel: null
    property bool sidebarCollapsed: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationPropertiesBar") : null

    signal toggleSidebarRequested
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: LV.Theme.gap12

    NavigationInformationBar {
        id: informationBar

        sidebarCollapsed: propertiesBar.sidebarCollapsed

        onToggleSidebarRequested: propertiesBar.toggleSidebarRequested()
    }
    NavigationModeBar {
        id: modeBar

        navigationModeViewModel: propertiesBar.navigationModeViewModel
        visible: !propertiesBar.compactMode
    }
    NavigationEditorViewBar {
        id: editorViewBar

        editorViewModeViewModel: propertiesBar.editorViewModeViewModel
        visible: !propertiesBar.compactMode
    }
}
