import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: exportBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationExportBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: 2

    LV.IconButton {
        id: exportButton

        checkable: false
        iconName: "generalupload"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: printButton

        checkable: false
        iconName: "generalprint"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: mailingButton

        checkable: false
        iconName: "mailer"
        tone: LV.AbstractButton.Borderless
    }
}
