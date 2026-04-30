import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: exportBar

    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.NavigationExportBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }

    spacing: LV.Theme.gap2

    LV.IconButton {
        id: exportButton

        iconName: "generalupload"
    }
    LV.IconButton {
        id: printButton

        iconName: "generalprint"
    }
    LV.IconButton {
        id: mailingButton

        iconName: "mailer"
    }
}
