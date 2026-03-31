pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: calendarTodayControl

    readonly property string figmaNodeId: "227:8807"
    property string todayText: "Today"

    signal previousRequested
    signal todayRequested
    signal nextRequested
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        viewHookRequested(hookReason);
    }

    spacing: LV.Theme.gap2

    LV.IconButton {
        id: previousButton

        horizontalPadding: LV.Theme.gap2
        iconName: "generalchevronLeft"
        tone: LV.AbstractButton.Default
        verticalPadding: LV.Theme.gap2

        onClicked: {
            calendarTodayControl.previousRequested();
            calendarTodayControl.requestViewHook("previous");
        }
    }
    LV.LabelButton {
        id: todayButton

        font.pixelSize: 12
        horizontalPadding: LV.Theme.gap8
        text: calendarTodayControl.todayText
        tone: LV.AbstractButton.Default
        verticalPadding: LV.Theme.gap4

        onClicked: {
            calendarTodayControl.todayRequested();
            calendarTodayControl.requestViewHook("today");
        }
    }
    LV.IconButton {
        id: nextButton

        horizontalPadding: LV.Theme.gap2
        iconName: "generalchevronLeft"
        rotation: 180
        tone: LV.AbstractButton.Default
        transformOrigin: Item.Center
        verticalPadding: LV.Theme.gap2

        onClicked: {
            calendarTodayControl.nextRequested();
            calendarTodayControl.requestViewHook("next");
        }
    }
}
