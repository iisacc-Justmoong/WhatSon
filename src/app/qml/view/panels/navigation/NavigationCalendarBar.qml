import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: calendarBar

    spacing: 2

    LV.IconButton {
        id: todoListButton

        height: 20
        iconName: "toolWindowCheckDetails"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: dailyCalButton

        height: 20
        iconName: "newUIlightThemeSelected"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: weeklyCalButton

        height: 20
        iconName: "table"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: monthlyCalButton

        height: 20
        iconName: "pnpm"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: yearlyCalButton

        height: 20
        iconName: "runshowCurrentFrame"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
