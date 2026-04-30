import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: appControlBar

    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.NavigationAppControlBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }

    spacing: LV.Theme.gap2

    LV.IconButton {
        id: makeStickyNoteButton

        iconName: "generalpaste"
    }
    LV.IconButton {
        id: pinWindowButton

        iconName: "pin"
    }
    LV.IconButton {
        id: alertsButton

        iconName: "toolwindownotifications"
    }
    LV.IconButton {
        id: timerButton

        iconName: "startTimer"
    }
}
