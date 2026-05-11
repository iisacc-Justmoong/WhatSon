import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: propertiesBar

    property bool compactMode: false
    property var editorViewModeController: null
    property var navigationModeController: null
    property bool sidebarCollapsed: false
    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.NavigationPropertiesBar") : null

    signal toggleSidebarRequested
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
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

        navigationModeController: propertiesBar.navigationModeController
        visible: !propertiesBar.compactMode
    }
    NavigationEditorViewBar {
        id: editorViewBar

        editorViewModeController: propertiesBar.editorViewModeController
        visible: !propertiesBar.compactMode
    }
}
