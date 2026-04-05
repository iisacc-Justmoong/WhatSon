pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: calendarTodayControl

    readonly property string figmaNodeId: "238:7843"
    readonly property int controlButtonExtent: 20
    readonly property int controlIconExtent: 16

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

        backgroundColor: LV.Theme.panelBackground12
        backgroundColorDisabled: LV.Theme.panelBackground12
        backgroundColorHover: LV.Theme.panelBackground12
        backgroundColorPressed: LV.Theme.panelBackground12
        cornerRadius: LV.Theme.radiusSm
        height: calendarTodayControl.controlButtonExtent
        horizontalPadding: LV.Theme.gap2
        iconName: "generalchevronUpLarge"
        iconSize: calendarTodayControl.controlIconExtent
        rotation: -90
        tone: LV.AbstractButton.Borderless
        transformOrigin: Item.Center
        width: calendarTodayControl.controlButtonExtent
        verticalPadding: LV.Theme.gap2

        onClicked: {
            calendarTodayControl.previousRequested();
            calendarTodayControl.requestViewHook("previous");
        }
    }
    LV.IconButton {
        id: todayButton

        backgroundColor: LV.Theme.panelBackground12
        backgroundColorDisabled: LV.Theme.panelBackground12
        backgroundColorHover: LV.Theme.panelBackground12
        backgroundColorPressed: LV.Theme.panelBackground12
        cornerRadius: LV.Theme.radiusSm
        height: calendarTodayControl.controlButtonExtent
        horizontalPadding: LV.Theme.gap2
        iconName: "threadAtBreakpoint"
        iconSize: calendarTodayControl.controlIconExtent
        tone: LV.AbstractButton.Borderless
        width: calendarTodayControl.controlButtonExtent
        verticalPadding: LV.Theme.gap2

        onClicked: {
            calendarTodayControl.todayRequested();
            calendarTodayControl.requestViewHook("today");
        }
    }
    LV.IconButton {
        id: nextButton

        backgroundColor: LV.Theme.panelBackground12
        backgroundColorDisabled: LV.Theme.panelBackground12
        backgroundColorHover: LV.Theme.panelBackground12
        backgroundColorPressed: LV.Theme.panelBackground12
        cornerRadius: LV.Theme.radiusSm
        height: calendarTodayControl.controlButtonExtent
        horizontalPadding: LV.Theme.gap2
        iconName: "generalchevronUpLarge"
        iconSize: calendarTodayControl.controlIconExtent
        rotation: 90
        tone: LV.AbstractButton.Borderless
        transformOrigin: Item.Center
        width: calendarTodayControl.controlButtonExtent
        verticalPadding: LV.Theme.gap2

        onClicked: {
            calendarTodayControl.nextRequested();
            calendarTodayControl.requestViewHook("next");
        }
    }
}
