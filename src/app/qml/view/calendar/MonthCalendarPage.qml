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
    readonly property int monthControlBarHeight: LV.Theme.gap20
    readonly property int monthTitleHeight: 44
    readonly property int weekdayHeaderHeight: 40
    readonly property int headerSectionSpacing: LV.Theme.gap2
    readonly property int dayCellVerticalInset: LV.Theme.gap3
    readonly property int dayCellSectionSpacing: LV.Theme.gap2
    readonly property int dayNumberHeight: LV.Theme.gap16
    readonly property int dayNumberPixelSize: 16
    readonly property int entryRowHeight: LV.Theme.gap12
    readonly property int entryRowSpacing: LV.Theme.gap2
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

        Rectangle {
            id: monthControlBand

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: monthCalendarPage.monthControlBarHeight
            color: "transparent"

            CalendarTodayControl {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter

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
        Rectangle {
            id: monthTitleBand

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: monthControlBand.bottom
            anchors.topMargin: monthCalendarPage.headerSectionSpacing
            height: monthCalendarPage.monthTitleHeight
            color: "transparent"

            LV.Label {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: LV.Theme.gap2
                anchors.bottomMargin: LV.Theme.gap2
                color: LV.Theme.titleHeaderColor
                font.pixelSize: 22
                font.weight: Font.Bold
                text: monthCalendarPage.monthTitleText
            }
        }
        GridLayout {
            id: weekdayHeaderGrid

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: monthTitleBand.bottom
            anchors.topMargin: monthCalendarPage.headerSectionSpacing
            height: monthCalendarPage.weekdayHeaderHeight
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
                    color: "transparent"

                    LV.Label {
                        anchors.centerIn: parent
                        color: LV.Theme.titleHeaderColor
                        font.pixelSize: 15
                        font.weight: Font.DemiBold
                        text: monthCalendarPage.weekdayLabelText(weekdayCell.modelData)
                    }
                }
            }
        }
        GridLayout {
            id: monthDayGrid

            anchors.top: weekdayHeaderGrid.bottom
            anchors.topMargin: monthCalendarPage.headerSectionSpacing
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

                Rectangle {
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
                    readonly property int entryCapacity: Math.max(
                                                            0,
                                                            Math.floor(
                                                                (dayCell.height
                                                                 - (monthCalendarPage.dayCellVerticalInset * 2)
                                                                 - monthCalendarPage.dayNumberHeight
                                                                 - monthCalendarPage.dayCellSectionSpacing
                                                                 + monthCalendarPage.entryRowSpacing)
                                                                / (monthCalendarPage.entryRowHeight
                                                                   + monthCalendarPage.entryRowSpacing)))
                    readonly property bool needsOverflowIndicator: dayCell.dayEntries.length > dayCell.entryCapacity
                                                                  && dayCell.entryCapacity > 0
                    readonly property int clippedEntryCount: Math.max(
                                                                0,
                                                                Math.min(
                                                                    dayCell.dayEntries.length,
                                                                    monthCalendarPage.maxVisibleEntriesPerCell,
                                                                    dayCell.needsOverflowIndicator ? dayCell.entryCapacity - 1 : dayCell.entryCapacity))

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: monthDayGrid.dayCellWidth
                    Layout.preferredWidth: monthDayGrid.dayCellWidth
                    Layout.maximumWidth: monthDayGrid.dayCellWidth
                    Layout.minimumHeight: monthDayGrid.dayCellHeight
                    Layout.preferredHeight: monthDayGrid.dayCellHeight
                    Layout.maximumHeight: monthDayGrid.dayCellHeight
                    border.color: dayCell.isToday
                                  ? LV.Theme.primary
                                  : dayCell.isSelectedDate
                                    ? LV.Theme.warning
                                    : LV.Theme.panelBackground06
                    border.width: Math.max(1, LV.Theme.strokeThin)
                    color: "transparent"
                    clip: true

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.setSelectedDateIso
                                    && dayCell.dayModel && dayCell.dayModel.dateIso !== undefined) {
                                monthCalendarPage.calendarVm.setSelectedDateIso(String(dayCell.dayModel.dateIso));
                                monthCalendarPage.requestViewHook("select-date");
                            }
                        }
                    }
                    Column {
                        anchors.fill: parent
                        anchors.margins: monthCalendarPage.dayCellVerticalInset
                        spacing: monthCalendarPage.dayCellSectionSpacing

                        LV.Label {
                            color: LV.Theme.titleHeaderColor
                            font.pixelSize: monthCalendarPage.dayNumberPixelSize
                            font.weight: Font.Medium
                            height: monthCalendarPage.dayNumberHeight
                            horizontalAlignment: Text.AlignRight
                            opacity: dayCell.isCurrentMonth ? 1.0 : 0.4
                            text: dayCell.dayModel && dayCell.dayModel.day !== undefined
                                  ? String(dayCell.dayModel.day)
                                  : ""
                            width: parent.width
                        }
                        Column {
                            spacing: monthCalendarPage.entryRowSpacing
                            width: parent.width

                            Repeater {
                                model: dayCell.clippedEntryCount

                                Row {
                                    id: dayEntryRow

                                    required property int index
                                    readonly property var entryModel: dayCell.dayEntries[dayEntryRow.index]
                                    spacing: LV.Theme.gap2
                                    height: monthCalendarPage.entryRowHeight
                                    width: parent.width

                                    Rectangle {
                                        anchors.verticalCenter: parent.verticalCenter
                                        color: monthCalendarPage.entryAccent(dayEntryRow.entryModel)
                                        height: LV.Theme.gap4
                                        radius: LV.Theme.radiusSm
                                        width: Math.max(1, LV.Theme.strokeThin)
                                    }
                                    LV.Label {
                                        anchors.verticalCenter: parent.verticalCenter
                                        color: LV.Theme.titleHeaderColor
                                        elide: Text.ElideRight
                                        font.pixelSize: 12
                                        opacity: dayCell.isCurrentMonth ? 1.0 : 0.5
                                        text: monthCalendarPage.entryLabel(dayEntryRow.entryModel)
                                        width: dayEntryRow.width
                                               - Math.max(1, LV.Theme.strokeThin)
                                               - LV.Theme.gap2
                                    }
                                }
                            }
                            LV.Label {
                                color: LV.Theme.descriptionColor
                                font.pixelSize: 12
                                opacity: dayCell.isCurrentMonth ? 1.0 : 0.5
                                text: "+" + String(dayCell.dayEntries.length - dayCell.clippedEntryCount)
                                      + " more"
                                visible: dayCell.needsOverflowIndicator
                            }
                        }
                    }
                }
            }
        }
    }
}
