import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: calendarBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationCalendarBar") : null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested(hookReason);
    }

    spacing: 2

    LV.IconButton {
        id: todoListButton

        iconName: "toolWindowCheckDetails"
    }
    LV.IconButton {
        id: dailyCalButton

        iconName: "newUIlightThemeSelected"
    }
    LV.IconButton {
        id: weeklyCalButton

        iconName: "table"
    }
    LV.IconButton {
        id: monthlyCalButton

        iconName: "pnpm"
    }
    LV.IconButton {
        id: yearlyCalButton

        iconName: "runshowCurrentFrame"

        onClicked: calendarBar.requestViewHook("open-yearly-calendar")
    }
}
