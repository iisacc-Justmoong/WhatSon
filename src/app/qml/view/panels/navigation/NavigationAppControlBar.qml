import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: appControlBar

    spacing: 2

    LV.IconButton {
        id: pinWindowButton

        checkable: false
        height: 20
        iconName: "pin"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: alertsButton

        checkable: false
        height: 20
        iconName: "toolwindownotifications"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: timerButton

        checkable: false
        height: 20
        iconName: "startTimer"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
