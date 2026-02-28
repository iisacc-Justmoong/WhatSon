import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 2

    LV.IconButton {
        height: 20
        iconName: "toolWindowCheckDetails"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "newUIlightThemeSelected"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "table"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "pnpm"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "runshowCurrentFrame"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
