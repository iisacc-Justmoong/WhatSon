import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: calendarBar

    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.view.NavigationApplicationViewCalendarBar") : null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested(hookReason);
    }

    spacing: LV.Theme.gap2

    LV.IconButton {
        id: todoListButton

        horizontalPadding: LV.Theme.gap2
        iconName: "validator"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: calendarBar.requestViewHook("view-open-agenda")
    }
    LV.IconButton {
        id: dailyCalButton

        horizontalPadding: LV.Theme.gap2
        iconName: "newUIlightThemeSelected"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: calendarBar.requestViewHook("view-open-daily-calendar")
    }
    LV.IconButton {
        id: weeklyCalButton

        horizontalPadding: LV.Theme.gap2
        iconName: "table"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: calendarBar.requestViewHook("view-open-weekly-calendar")
    }
    LV.IconButton {
        id: monthlyCalButton

        horizontalPadding: LV.Theme.gap2
        iconName: "pnpm"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: calendarBar.requestViewHook("view-open-monthly-calendar")
    }
    LV.IconButton {
        id: yearlyCalButton

        horizontalPadding: LV.Theme.gap2
        iconName: "runshowCurrentFrame"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: calendarBar.requestViewHook("view-open-yearly-calendar")
    }
}
