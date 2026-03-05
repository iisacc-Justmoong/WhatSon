import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: calendarBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationCalendarBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: 2

    LV.IconButton {
        id: todoListButton

        checkable: false
        iconName: "toolWindowCheckDetails"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: dailyCalButton

        checkable: false
        iconName: "newUIlightThemeSelected"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: weeklyCalButton

        checkable: false
        iconName: "table"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: monthlyCalButton

        checkable: false
        iconName: "pnpm"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: yearlyCalButton

        checkable: false
        iconName: "runshowCurrentFrame"
        tone: LV.AbstractButton.Borderless
    }
}
