import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: propertiesBar

    property bool compactMode: false
    property var editorViewModeViewModel: null
    property var navigationModeViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationPropertiesBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: 12

    NavigationInformationBar {
        id: informationBar

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
