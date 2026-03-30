pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: weekCalendarPage

    readonly property var calendarVm: weekCalendarViewModel
    readonly property var dayModels: calendarVm && calendarVm.dayModels ? calendarVm.dayModels : []
    readonly property var weekdayLabels: calendarVm && calendarVm.weekdayLabels ? calendarVm.weekdayLabels : []
    readonly property int dayColumnWidth: LV.Theme.gap24 * 5
    readonly property int hourColumnWidth: LV.Theme.gap24 * 2
    readonly property int hourRowHeight: LV.Theme.gap16
    readonly property var hourSlots: weekCalendarPage.buildHourSlots()
    property var weekCalendarViewModel: null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarVm && calendarVm.requestWeekView)
            calendarVm.requestWeekView(hookReason);
        viewHookRequested(hookReason);
    }
    function buildHourSlots() {
        var values = [];
        for (var hour = 0; hour < 24; ++hour)
            values.push(hour);
        return values;
    }
    function entriesForHour(dayModel, hour) {
        if (!dayModel || !dayModel.entries)
            return [];
        var matches = [];
        for (var i = 0; i < dayModel.entries.length; ++i) {
            var entry = dayModel.entries[i];
            if (!entry || entry.time === undefined)
                continue;
            var timeParts = String(entry.time).split(":");
            if (timeParts.length === 0)
                continue;
            var entryHour = Number(timeParts[0]);
            if (!isFinite(entryHour) || Math.floor(entryHour) !== hour)
                continue;
            matches.push(entry);
        }
        return matches;
    }
    function slotSummary(entries) {
        if (!entries || entries.length === 0)
            return "";
        var firstEntry = entries[0];
        var title = firstEntry && firstEntry.title !== undefined ? String(firstEntry.title) : "Item";
        if (entries.length === 1)
            return title;
        return title + " +" + String(entries.length - 1);
    }
    function jumpToCurrentWeek() {
        if (weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.setDisplayedWeekStartIso)
            weekCalendarPage.calendarVm.setDisplayedWeekStartIso(Qt.formatDateTime(new Date(), "yyyy-MM-dd"));
        weekCalendarPage.requestViewHook("current-week");
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
                        if (weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.shiftWeek)
                            weekCalendarPage.calendarVm.shiftWeek(-1);
                        weekCalendarPage.requestViewHook("previous-week");
                    }
                }
                LV.LabelButton {
                    text: "Today"

                    onClicked: weekCalendarPage.jumpToCurrentWeek()
                }
                LV.Label {
                    text: weekCalendarPage.calendarVm ? String(weekCalendarPage.calendarVm.weekLabel) : "Week calendar"
                }
                LV.LabelButton {
                    text: "Next"

                    onClicked: {
                        if (weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.shiftWeek)
                            weekCalendarPage.calendarVm.shiftWeek(1);
                        weekCalendarPage.requestViewHook("next-week");
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
                LV.Label {
                    color: LV.Theme.descriptionColor
                    text: "Week view"
                }
            }
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LV.Theme.panelBackground09
            radius: LV.Theme.radiusSm

            Flickable {
                id: weekTimelineViewport

                anchors.fill: parent
                anchors.margins: LV.Theme.gap3
                clip: true
                contentHeight: weekTimelineColumn.implicitHeight
                contentWidth: weekTimelineColumn.width

                Column {
                    id: weekTimelineColumn

                    spacing: LV.Theme.gap2
                    width: weekCalendarPage.hourColumnWidth + (weekCalendarPage.dayColumnWidth * 7) + (LV.Theme.gap2 * 6)

                    Row {
                        spacing: LV.Theme.gap2

                        Rectangle {
                            color: LV.Theme.panelBackground11
                            height: LV.Theme.gap20
                            radius: LV.Theme.radiusSm
                            width: weekCalendarPage.hourColumnWidth

                            LV.Label {
                                anchors.centerIn: parent
                                text: "Time"
                            }
                        }
                        Repeater {
                            model: weekCalendarPage.dayModels

                            Rectangle {
                                id: dayHeaderCell

                                required property var modelData
                                readonly property var dayModel: dayHeaderCell.modelData

                                color: dayModel && dayModel.isToday === true
                                       ? LV.Theme.primary
                                       : LV.Theme.panelBackground10
                                height: LV.Theme.gap20
                                radius: LV.Theme.radiusSm
                                width: weekCalendarPage.dayColumnWidth

                                Column {
                                    anchors.centerIn: parent
                                    spacing: LV.Theme.gapNone

                                    LV.Label {
                                        color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isToday === true
                                               ? LV.Theme.panelBackground01
                                               : LV.Theme.titleHeaderColor
                                        text: dayHeaderCell.dayModel && dayHeaderCell.dayModel.dayLabel !== undefined
                                              ? String(dayHeaderCell.dayModel.dayLabel)
                                              : ""
                                    }
                                    LV.Label {
                                        color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isToday === true
                                               ? LV.Theme.panelBackground01
                                               : LV.Theme.descriptionColor
                                        text: dayHeaderCell.dayModel && dayHeaderCell.dayModel.entryCount !== undefined
                                              ? String(dayHeaderCell.dayModel.entryCount) + " items"
                                              : "0 items"
                                    }
                                }
                            }
                        }
                    }
                    Repeater {
                        model: weekCalendarPage.hourSlots

                        Rectangle {
                            id: hourRow

                            required property var modelData
                            readonly property int hour: Number(hourRow.modelData)

                            color: hourRow.hour % 2 === 0 ? LV.Theme.panelBackground10 : LV.Theme.panelBackground11
                            height: weekCalendarPage.hourRowHeight
                            radius: LV.Theme.radiusSm
                            width: weekTimelineColumn.width

                            Row {
                                anchors.fill: parent
                                spacing: LV.Theme.gap2

                                Rectangle {
                                    color: LV.Theme.panelBackground12
                                    height: parent.height
                                    radius: LV.Theme.radiusSm
                                    width: weekCalendarPage.hourColumnWidth

                                    LV.Label {
                                        anchors.centerIn: parent
                                        text: Qt.formatTime(new Date(2000, 0, 1, hourRow.hour, 0, 0), "HH:mm")
                                    }
                                }
                                Repeater {
                                    model: weekCalendarPage.dayModels

                                    Rectangle {
                                        id: dayHourCell

                                        required property var modelData
                                        readonly property var dayModel: dayHourCell.modelData
                                        readonly property var slotEntries: weekCalendarPage.entriesForHour(
                                                                             dayHourCell.dayModel,
                                                                             hourRow.hour)

                                        border.color: LV.Theme.accentTransparent
                                        border.width: Math.max(1, LV.Theme.strokeThin)
                                        color: dayHourCell.slotEntries.length > 0
                                               ? LV.Theme.panelBackground07
                                               : LV.Theme.accentTransparent
                                        height: hourRow.height
                                        radius: LV.Theme.radiusSm
                                        width: weekCalendarPage.dayColumnWidth

                                        LV.Label {
                                            anchors.left: parent.left
                                            anchors.leftMargin: LV.Theme.gap2
                                            anchors.right: parent.right
                                            anchors.rightMargin: LV.Theme.gap2
                                            anchors.verticalCenter: parent.verticalCenter
                                            color: LV.Theme.titleHeaderColor
                                            elide: Text.ElideRight
                                            text: weekCalendarPage.slotSummary(dayHourCell.slotEntries)
                                            visible: dayHourCell.slotEntries.length > 0
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
