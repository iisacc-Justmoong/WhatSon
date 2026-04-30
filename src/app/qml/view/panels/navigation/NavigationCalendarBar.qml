import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: calendarBar

    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.NavigationCalendarBar") : null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested(hookReason);
    }

    spacing: LV.Theme.gap2

    LV.IconButton {
        id: agendaButton

        iconName: "toolWindowCheckDetails"

        onClicked: calendarBar.requestViewHook("open-agenda")
    }
    LV.IconButton {
        id: dailyCalButton

        iconName: "newUIlightThemeSelected"

        onClicked: calendarBar.requestViewHook("open-daily-calendar")
    }
    LV.IconButton {
        id: weeklyCalButton

        iconName: "table"

        onClicked: calendarBar.requestViewHook("open-weekly-calendar")
    }
    LV.IconButton {
        id: monthlyCalButton

        iconName: "pnpm"

        onClicked: calendarBar.requestViewHook("open-monthly-calendar")
    }
    LV.IconButton {
        id: yearlyCalButton

        iconName: "runshowCurrentFrame"

        onClicked: calendarBar.requestViewHook("open-yearly-calendar")
    }
}
