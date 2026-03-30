pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: dayCalendarPage

    readonly property var calendarVm: dayCalendarViewModel
    readonly property var dayEntries: calendarVm && calendarVm.dayEntries ? calendarVm.dayEntries : []
    readonly property var timeSlots: calendarVm && calendarVm.timeSlots ? calendarVm.timeSlots : []
    readonly property int hourColumnWidth: LV.Theme.gap24 * 2
    property var dayCalendarViewModel: null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarVm && calendarVm.requestDayView)
            calendarVm.requestDayView(hookReason);
        viewHookRequested(hookReason);
    }
    function jumpToToday() {
        if (dayCalendarPage.calendarVm && dayCalendarPage.calendarVm.setDisplayedDateIso)
            dayCalendarPage.calendarVm.setDisplayedDateIso(Qt.formatDateTime(new Date(), "yyyy-MM-dd"));
        dayCalendarPage.requestViewHook("today");
    }

    color: LV.Theme.panelBackground07
    radius: LV.Theme.radiusMd

    Component.onCompleted: requestViewHook("page-open")

    LV.VStack {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap12
        spacing: LV.Theme.gap12

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: LV.Theme.gap20
            color: LV.Theme.panelBackground10
            radius: LV.Theme.radiusSm

            LV.HStack {
                anchors.fill: parent
                anchors.margins: LV.Theme.gap4
                spacing: LV.Theme.gap4

                LV.LabelButton {
                    text: "Prev"

                    onClicked: {
                        if (dayCalendarPage.calendarVm && dayCalendarPage.calendarVm.shiftDay)
                            dayCalendarPage.calendarVm.shiftDay(-1);
                        dayCalendarPage.requestViewHook("previous-day");
                    }
                }
                LV.LabelButton {
                    text: "Today"

                    onClicked: dayCalendarPage.jumpToToday()
                }
                LV.Label {
                    text: dayCalendarPage.calendarVm ? String(dayCalendarPage.calendarVm.dayLabel) : "Day calendar"
                }
                LV.LabelButton {
                    text: "Next"

                    onClicked: {
                        if (dayCalendarPage.calendarVm && dayCalendarPage.calendarVm.shiftDay)
                            dayCalendarPage.calendarVm.shiftDay(1);
                        dayCalendarPage.requestViewHook("next-day");
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
                LV.Label {
                    color: LV.Theme.descriptionColor
                    text: "Entries " + String(dayCalendarPage.dayEntries.length)
                }
            }
        }
        Rectangle {
            id: dayTimelineViewport

            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LV.Theme.panelBackground09
            radius: LV.Theme.radiusSm

            Flickable {
                anchors.fill: parent
                anchors.margins: LV.Theme.gap3
                clip: true
                contentHeight: timeSlotColumn.implicitHeight
                contentWidth: dayTimelineViewport.width - LV.Theme.gap6

                Column {
                    id: timeSlotColumn

                    spacing: LV.Theme.gap2
                    width: dayTimelineViewport.width - LV.Theme.gap6

                    Repeater {
                        model: dayCalendarPage.timeSlots

                        Rectangle {
                            id: timeSlotRow

                            required property var modelData
                            readonly property var slotModel: timeSlotRow.modelData
                            readonly property var slotEntries: slotModel && slotModel.entries ? slotModel.entries : []

                            color: LV.Theme.panelBackground10
                            height: Math.max(LV.Theme.gap16, slotEntriesColumn.implicitHeight + LV.Theme.gap4)
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
                                        text: timeSlotRow.slotModel && timeSlotRow.slotModel.timeLabel !== undefined
                                              ? String(timeSlotRow.slotModel.timeLabel)
                                              : ""
                                    }
                                }
                                Column {
                                    id: slotEntriesColumn

                                    spacing: LV.Theme.gap2
                                    width: timeSlotRow.width - dayCalendarPage.hourColumnWidth - LV.Theme.gap7

                                    Rectangle {
                                        color: LV.Theme.accentTransparent
                                        height: Math.max(1, LV.Theme.strokeThin)
                                        visible: timeSlotRow.slotEntries.length === 0
                                        width: parent.width
                                    }
                                    Repeater {
                                        model: timeSlotRow.slotEntries

                                        Rectangle {
                                            id: slotEntryCard

                                            required property var modelData
                                            readonly property var entryModel: slotEntryCard.modelData

                                            color: entryModel && String(entryModel.type) === "task"
                                                   ? LV.Theme.panelBackground11
                                                   : LV.Theme.primary
                                            height: LV.Theme.gap12
                                            radius: LV.Theme.radiusSm
                                            width: slotEntriesColumn.width

                                            Row {
                                                anchors.fill: parent
                                                anchors.leftMargin: LV.Theme.gap3
                                                anchors.rightMargin: LV.Theme.gap3
                                                spacing: LV.Theme.gap2

                                                LV.Label {
                                                    color: LV.Theme.panelBackground01
                                                    text: slotEntryCard.entryModel
                                                          && slotEntryCard.entryModel.time !== undefined
                                                          ? String(slotEntryCard.entryModel.time)
                                                          : ""
                                                }
                                                LV.Label {
                                                    color: LV.Theme.panelBackground01
                                                    text: slotEntryCard.entryModel
                                                          && slotEntryCard.entryModel.title !== undefined
                                                          ? String(slotEntryCard.entryModel.title)
                                                          : ""
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
    }
}
