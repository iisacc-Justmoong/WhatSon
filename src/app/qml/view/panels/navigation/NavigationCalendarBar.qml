import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: calendarBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationCalendarBar") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    spacing: 2

    LV.IconButton {
        id: todoListButton

        checkable: false
        height: 20
        iconName: "toolWindowCheckDetails"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: dailyCalButton

        checkable: false
        height: 20
        iconName: "newUIlightThemeSelected"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: weeklyCalButton

        checkable: false
        height: 20
        iconName: "table"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: monthlyCalButton

        checkable: false
        height: 20
        iconName: "pnpm"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: yearlyCalButton

        checkable: false
        height: 20
        iconName: "runshowCurrentFrame"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
