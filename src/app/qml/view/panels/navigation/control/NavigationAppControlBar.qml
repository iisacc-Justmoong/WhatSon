import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: appControlBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationAppControlBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: 2

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
