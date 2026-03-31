pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: monthCalendarPage

    readonly property var calendarVm: monthCalendarViewModel
    readonly property var dayModels: calendarVm && calendarVm.dayModels ? calendarVm.dayModels : []
    readonly property var weekdayLabels: calendarVm && calendarVm.weekdayLabels ? calendarVm.weekdayLabels : []
    readonly property string monthTitleText: monthCalendarPage.calendarVm
                                             ? String(monthCalendarPage.calendarVm.monthLabel)
                                               + ", " + String(monthCalendarPage.calendarVm.displayedYear)
                                             : "Month"
    readonly property var visibleDayModels: monthCalendarPage.buildVisibleDayModels()
    readonly property int visibleWeekRowCount: Math.max(
                                                   1,
                                                   Math.ceil(monthCalendarPage.visibleDayModels.length / 7))
    readonly property int maxVisibleEntriesPerCell: 8
    readonly property int eventBackgroundDefault: 0
    readonly property int eventBackgroundColored: 1
    readonly property int monthHeaderHeight: 54
    readonly property int weekdayHeaderHeight: 39
    readonly property int headerHorizontalPadding: 8
    readonly property int weekdayCellHorizontalPadding: 12
    readonly property int bodyLabelPixelSize: 12
    readonly property string figmaNodeId: "228:9666"
    property var monthCalendarViewModel: null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.requestMonthView)
            monthCalendarPage.calendarVm.requestMonthView(hookReason);
        monthCalendarPage.viewHookRequested(hookReason);
    }
    function buildVisibleDayModels() {
        if (!monthCalendarPage.dayModels || monthCalendarPage.dayModels.length === 0)
            return [];
        var visibleCount = monthCalendarPage.dayModels.length;
        while (visibleCount > 35) {
            const rowStart = visibleCount - 7;
            var hasCurrentMonthDate = false;
            for (var index = rowStart; index < visibleCount; ++index) {
                const model = monthCalendarPage.dayModels[index];
                if (model && model.inCurrentMonth === true) {
                    hasCurrentMonthDate = true;
                    break;
                }
            }
            if (hasCurrentMonthDate)
                break;
            visibleCount -= 7;
        }
        return monthCalendarPage.dayModels.slice(0, visibleCount);
    }
    function weekdayLabelText(label) {
        if (label === undefined)
            return "";
        const normalizedText = String(label).trim();
        if (normalizedText.length === 0)
            return "";
        return normalizedText.length > 3
               ? normalizedText.slice(0, 3).toUpperCase()
               : normalizedText.toUpperCase();
    }
    function entriesForDate(dayModel) {
        if (!dayModel || dayModel.dateIso === undefined || !monthCalendarPage.calendarVm
                || !monthCalendarPage.calendarVm.entriesForDate)
            return [];
        const dateIso = String(dayModel.dateIso).trim();
        if (dateIso.length === 0)
            return [];
        return monthCalendarPage.calendarVm.entriesForDate(dateIso);
    }
    function entryAccent(entryModel) {
        const entryType = entryModel && entryModel.type !== undefined ? String(entryModel.type) : "";
        if (entryType === "task") {
            const completed = entryModel && entryModel.completed === true;
            return completed ? LV.Theme.descriptionColor : LV.Theme.warning;
        }
        return LV.Theme.primary;
    }
    function entryLabel(entryModel) {
        const title = entryModel && entryModel.title !== undefined && String(entryModel.title).trim().length > 0
                      ? String(entryModel.title).trim()
                      : "Untitled";
        const timeText = entryModel && entryModel.time !== undefined
                         ? String(entryModel.time).trim()
                         : "";
        if (timeText.length === 0)
            return title;
        return title + " " + timeText;
    }
    function buildEntryCellModels(dayEntries) {
        var cellModels = [];
        if (!dayEntries)
            return cellModels;
        for (var index = 0; index < dayEntries.length; ++index) {
            const entryModel = dayEntries[index];
            const entryType = entryModel && entryModel.type !== undefined
                              ? String(entryModel.type)
                              : "";
            cellModels.push({
                                "label": monthCalendarPage.entryLabel(entryModel),
                                "backgroundType": entryType === "event"
                                                  ? monthCalendarPage.eventBackgroundColored
                                                  : monthCalendarPage.eventBackgroundDefault,
                                "backgroundColor": monthCalendarPage.entryAccent(entryModel)
                            });
        }
        return cellModels;
    }
    function jumpToCurrentMonth() {
        if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.focusToday) {
            monthCalendarPage.calendarVm.focusToday();
        } else {
            const now = new Date();
            if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.setDisplayedYear)
                monthCalendarPage.calendarVm.setDisplayedYear(now.getFullYear());
            if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.setDisplayedMonth)
                monthCalendarPage.calendarVm.setDisplayedMonth(now.getMonth() + 1);
            if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.setSelectedDateIso)
                monthCalendarPage.calendarVm.setSelectedDateIso(Qt.formatDateTime(now, "yyyy-MM-dd"));
        }
        monthCalendarPage.requestViewHook("current-month");
    }

    color: "transparent"
    radius: LV.Theme.radiusMd
    Layout.fillHeight: true
    Layout.fillWidth: true

    Component.onCompleted: monthCalendarPage.requestViewHook("page-open")

    Item {
        id: monthCalendarSurface

        anchors.fill: parent

        Item {
            id: monthHeaderBand

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: monthCalendarPage.monthHeaderHeight

            LV.HStack {
                anchors.fill: parent
                anchors.leftMargin: monthCalendarPage.headerHorizontalPadding
                anchors.rightMargin: monthCalendarPage.headerHorizontalPadding
                spacing: LV.Theme.gap4

                LV.Label {
                    Layout.alignment: Qt.AlignVCenter
                    color: LV.Theme.titleHeaderColor
                    font.pixelSize: 22
                    font.weight: Font.Bold
                    text: monthCalendarPage.monthTitleText
                }
                Item {
                    Layout.fillWidth: true
                }
                CalendarTodayControl {
                    Layout.alignment: Qt.AlignVCenter

                    onPreviousRequested: {
                        if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.shiftMonth)
                            monthCalendarPage.calendarVm.shiftMonth(-1);
                        monthCalendarPage.requestViewHook("previous-month");
                    }
                    onTodayRequested: monthCalendarPage.jumpToCurrentMonth()
                    onNextRequested: {
                        if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.shiftMonth)
                            monthCalendarPage.calendarVm.shiftMonth(1);
                        monthCalendarPage.requestViewHook("next-month");
                    }
                }
            }
        }
        Item {
            id: monthBody

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: monthHeaderBand.bottom
            anchors.bottom: parent.bottom

            Rectangle {
                id: weekdayHeaderBand

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: monthCalendarPage.weekdayHeaderHeight
                color: LV.Theme.panelBackground06

                GridLayout {
                    id: weekdayHeaderGrid

                    anchors.fill: parent
                    columns: 7
                    rowSpacing: 0
                    columnSpacing: 0

                    Repeater {
                        model: monthCalendarPage.weekdayLabels

                        Rectangle {
                            id: weekdayCell

                            required property var modelData

                            Layout.fillWidth: true
                            Layout.minimumHeight: monthCalendarPage.weekdayHeaderHeight
                            Layout.preferredHeight: monthCalendarPage.weekdayHeaderHeight
                            Layout.maximumHeight: monthCalendarPage.weekdayHeaderHeight
                            border.color: LV.Theme.panelBackground06
                            border.width: Math.max(1, LV.Theme.strokeThin)
                            color: LV.Theme.panelBackground06

                            LV.Label {
                                anchors.left: parent.left
                                anchors.leftMargin: monthCalendarPage.weekdayCellHorizontalPadding
                                anchors.verticalCenter: parent.verticalCenter
                                color: LV.Theme.titleHeaderColor
                                font.pixelSize: monthCalendarPage.bodyLabelPixelSize
                                font.weight: Font.Medium
                                text: monthCalendarPage.weekdayLabelText(weekdayCell.modelData)
                            }
                        }
                    }
                }
            }
            GridLayout {
                id: monthDayGrid

                anchors.top: weekdayHeaderBand.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                columns: 7
                rowSpacing: 0
                columnSpacing: 0
                readonly property int dayCellWidth: Math.max(1, Math.floor(monthDayGrid.width / 7))
                readonly property int dayCellHeight: Math.max(1, Math.floor(monthDayGrid.height / monthCalendarPage.visibleWeekRowCount))

                Repeater {
                    model: monthCalendarPage.visibleDayModels

                    MonthCalendarDayCell {
                        id: dayCell

                        required property var modelData
                        readonly property var dayModel: dayCell.modelData
                        readonly property bool isCurrentMonth: dayModel && dayModel.inCurrentMonth === true
                        readonly property bool isSelectedDate: monthCalendarPage.calendarVm
                                                                && dayModel
                                                                && dayModel.dateIso !== undefined
                                                                && String(dayModel.dateIso) === String(monthCalendarPage.calendarVm.selectedDateIso)
                        readonly property bool isToday: dayModel && dayModel.isToday === true
                        readonly property var dayEntries: monthCalendarPage.entriesForDate(dayCell.dayModel)
                        readonly property var dayEntryCells: monthCalendarPage.buildEntryCellModels(dayCell.dayEntries)

                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: monthDayGrid.dayCellWidth
                        Layout.preferredWidth: monthDayGrid.dayCellWidth
                        Layout.maximumWidth: monthDayGrid.dayCellWidth
                        Layout.minimumHeight: monthDayGrid.dayCellHeight
                        Layout.preferredHeight: monthDayGrid.dayCellHeight
                        Layout.maximumHeight: monthDayGrid.dayCellHeight

                        dayNumber: dayCell.dayModel && dayCell.dayModel.day !== undefined
                                   ? Number(dayCell.dayModel.day)
                                   : 0
                        disable: !dayCell.isCurrentMonth
                        entryCells: dayCell.dayEntryCells
                        maxVisibleEntries: monthCalendarPage.maxVisibleEntriesPerCell
                        selected: dayCell.isSelectedDate
                        today: dayCell.isToday

                        onClicked: {
                            if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.setSelectedDateIso
                                    && dayCell.dayModel && dayCell.dayModel.dateIso !== undefined) {
                                monthCalendarPage.calendarVm.setSelectedDateIso(String(dayCell.dayModel.dateIso));
                                monthCalendarPage.requestViewHook("select-date");
                            }
                        }
                    }
                }
            }
        }
    }
}
