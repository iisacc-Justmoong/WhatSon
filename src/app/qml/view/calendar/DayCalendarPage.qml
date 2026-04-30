pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: dayCalendarPage

    readonly property var calendarController: dayCalendarController
    readonly property var timeSlots: calendarController && calendarController.timeSlots ? calendarController.timeSlots : []
    readonly property int hourColumnWidth: LV.Theme.gap24 * 2
    property var dayCalendarController: null

    signal noteOpenRequested(string noteId)
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarController && calendarController.requestDayView)
            calendarController.requestDayView(hookReason);
        viewHookRequested(hookReason);
    }
    function jumpToToday() {
        if (dayCalendarPage.calendarController && dayCalendarPage.calendarController.setDisplayedDateIso)
            dayCalendarPage.calendarController.setDisplayedDateIso(Qt.formatDateTime(new Date(), "yyyy-MM-dd"));
        dayCalendarPage.requestViewHook("today");
    }
    function entryCardLabel(entryModel) {
        if (!entryModel)
            return "";
        const titleText = entryModel.title !== undefined ? String(entryModel.title).trim() : "";
        const isAllDay = entryModel.allDay === true;
        const timeText = entryModel.time !== undefined ? String(entryModel.time).trim() : "";
        if (!isAllDay && timeText.length > 0 && titleText.length > 0)
            return timeText + " " + titleText;
        if (titleText.length > 0)
            return titleText;
        return timeText;
    }
    function entryAccent(entryModel) {
        if (!entryModel)
            return LV.Theme.primary;
        const sourceKind = entryModel.sourceKind !== undefined ? String(entryModel.sourceKind) : "";
        if (sourceKind === "note")
            return LV.Theme.accent;
        return LV.Theme.primary;
    }
    function noteIdForEntry(entryModel) {
        if (!entryModel)
            return "";
        const sourceKind = entryModel.sourceKind !== undefined ? String(entryModel.sourceKind).trim() : "";
        if (sourceKind !== "note")
            return "";
        return entryModel.sourceId !== undefined && entryModel.sourceId !== null ? String(entryModel.sourceId).trim() : "";
    }
    function requestOpenNote(entryModel) {
        const noteId = dayCalendarPage.noteIdForEntry(entryModel);
        if (noteId.length === 0)
            return;
        dayCalendarPage.requestViewHook("open-note");
        dayCalendarPage.noteOpenRequested(noteId);
    }

    color: LV.Theme.accentTransparent
    radius: LV.Theme.radiusMd

    Component.onCompleted: requestViewHook("page-open")

    LV.VStack {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap12
        spacing: LV.Theme.gap12

        CalendarTodayControl {
            id: dayCalendarControl

            onPreviousRequested: {
                if (dayCalendarPage.calendarController && dayCalendarPage.calendarController.shiftDay)
                    dayCalendarPage.calendarController.shiftDay(-1);
                dayCalendarPage.requestViewHook("previous-day");
            }
            onTodayRequested: dayCalendarPage.jumpToToday()
            onNextRequested: {
                if (dayCalendarPage.calendarController && dayCalendarPage.calendarController.shiftDay)
                    dayCalendarPage.calendarController.shiftDay(1);
                dayCalendarPage.requestViewHook("next-day");
            }
        }
        Rectangle {
            id: dayTimelineViewport

            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LV.Theme.panelBackground09
            radius: LV.Theme.radiusSm

            Item {
                id: dayTimelineContent

                anchors.fill: parent
                anchors.margins: LV.Theme.gap3
                readonly property int slotCount: Math.max(1, dayCalendarPage.timeSlots.length)
                readonly property real slotSpacing: LV.Theme.gap2
                readonly property real slotHeight: Math.max(1, (height - (slotSpacing * (slotCount - 1))) / slotCount)

                Column {
                    id: timeSlotColumn

                    anchors.fill: parent
                    spacing: dayTimelineContent.slotSpacing

                    Repeater {
                        model: dayCalendarPage.timeSlots

                        Rectangle {
                            id: timeSlotRow

                            required property var modelData
                            readonly property var slotModel: timeSlotRow.modelData
                            readonly property var slotEntries: slotModel && slotModel.entries ? slotModel.entries : []
                            readonly property real slotEntryHeight: Math.max(1, Math.min(LV.Theme.gap12, timeSlotRow.height - LV.Theme.gap4))

                            clip: true
                            color: LV.Theme.panelBackground10
                            height: dayTimelineContent.slotHeight
                            radius: LV.Theme.radiusSm
                            width: timeSlotColumn.width

                            Row {
                                anchors.fill: parent
                                anchors.margins: LV.Theme.gap2
                                spacing: LV.Theme.gap3

                                Rectangle {
                                    color: LV.Theme.panelBackground11
                                    height: parent.height
                                    radius: LV.Theme.radiusSm
                                    width: dayCalendarPage.hourColumnWidth

                                    LV.Label {
                                        anchors.centerIn: parent
                                        text: timeSlotRow.slotModel && timeSlotRow.slotModel.timeLabel !== undefined ? String(timeSlotRow.slotModel.timeLabel) : ""
                                    }
                                }
                                Column {
                                    id: slotEntriesColumn

                                    spacing: LV.Theme.gap2
                                    width: Math.max(0, timeSlotRow.width - dayCalendarPage.hourColumnWidth - LV.Theme.gap7)

                                    Rectangle {
                                        color: LV.Theme.accentTransparent
                                        height: Math.max(1, LV.Theme.strokeThin)
                                        visible: timeSlotRow.slotEntries.length === 0
                                        width: parent.width
                                    }
                                    Repeater {
                                        model: timeSlotRow.slotEntries

                                        CalendarEventCell {
                                            id: slotEntryCard

                                            required property var modelData
                                            readonly property var entryModel: slotEntryCard.modelData

                                            backgroundType: entryModel && String(entryModel.type) === "event" ? slotEntryCard.backgroundColored : slotEntryCard.backgroundDefault
                                            coloredBackgroundColor: dayCalendarPage.entryAccent(slotEntryCard.entryModel)
                                            cornerRadius: LV.Theme.radiusSm
                                            defaultBackgroundColor: LV.Theme.panelBackground11
                                            height: timeSlotRow.slotEntryHeight
                                            horizontalInset: LV.Theme.gap3
                                            interactive: dayCalendarPage.noteIdForEntry(slotEntryCard.entryModel).length > 0
                                            label: dayCalendarPage.entryCardLabel(slotEntryCard.entryModel)
                                            textColor: LV.Theme.panelBackground01
                                            verticalInset: LV.Theme.gap2
                                            width: slotEntriesColumn.width

                                            onActivated: dayCalendarPage.requestOpenNote(slotEntryCard.entryModel)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
