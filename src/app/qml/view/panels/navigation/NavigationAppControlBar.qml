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

        checkable: false
        iconName: "generalprojectStructure"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: pinWindowButton

        checkable: false
        iconName: "pin"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: alertsButton

        checkable: false
        iconName: "toolwindownotifications"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: timerButton

        checkable: false
        iconName: "startTimer"
        tone: LV.AbstractButton.Borderless
    }
}
