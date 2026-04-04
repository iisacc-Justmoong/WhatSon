pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: monthCalendarGridSurface

    property int bodyLabelPixelSize: 12
    property var calendarVm: null
    readonly property var dayModels: monthCalendarGridSurface.monthProjection && monthCalendarGridSurface.monthProjection.dayModels ? monthCalendarGridSurface.monthProjection.dayModels : []
    readonly property int eventBackgroundColored: 1
    readonly property int eventBackgroundDefault: 0
    property int maxVisibleEntries: 8
    property var monthProjection: null
    property string selectedDateIso: ""
    readonly property var visibleDayModels: monthCalendarGridSurface.buildVisibleDayModels()
    readonly property int visibleWeekRowCount: Math.max(1, Math.ceil(monthCalendarGridSurface.visibleDayModels.length / 7))
    property int weekdayCellHorizontalPadding: 12
    property int weekdayHeaderHeight: 39
    readonly property var weekdayLabels: monthCalendarGridSurface.monthProjection && monthCalendarGridSurface.monthProjection.weekdayLabels ? monthCalendarGridSurface.monthProjection.weekdayLabels : []

    signal dateSelected(string dateIso)
    signal viewHookRequested(string reason)

    function buildEntryCellModels(dayEntries) {
        var cellModels = [];
        if (!dayEntries)
            return cellModels;
        for (var index = 0; index < dayEntries.length; ++index) {
            const entryModel = dayEntries[index];
            const entryType = entryModel && entryModel.type !== undefined ? String(entryModel.type) : "";
            cellModels.push({
                "label": monthCalendarGridSurface.entryLabel(entryModel),
                "backgroundType": entryType === "event" ? monthCalendarGridSurface.eventBackgroundColored : monthCalendarGridSurface.eventBackgroundDefault,
                "backgroundColor": monthCalendarGridSurface.entryAccent(entryModel)
            });
        }

    }
    function buildVisibleDayModels() {
        if (!monthCalendarGridSurface.dayModels || monthCalendarGridSurface.dayModels.length === 0)
            return [];
        var visibleCount = monthCalendarGridSurface.dayModels.length;
        while (visibleCount > 35) {
            const rowStart = visibleCount - 7;
            var hasCurrentMonthDate = false;
            for (var index = rowStart; index < visibleCount; ++index) {
                const model = monthCalendarGridSurface.dayModels[index];
                if (model && model.inCurrentMonth === true) {
                    hasCurrentMonthDate = true;
                    break;
                }
            }
            if (hasCurrentMonthDate)
                break;
            visibleCount -= 7;
        }
        return monthCalendarGridSurface.dayModels.slice(0, visibleCount);
    }
    function entriesForDate(dayModel) {
        if (!dayModel || dayModel.dateIso === undefined || !monthCalendarGridSurface.calendarVm || !monthCalendarGridSurface.calendarVm.entriesForDate)
            return [];
        const dateIso = String(dayModel.dateIso).trim();
        if (dateIso.length === 0)
            return [];
        return monthCalendarGridSurface.calendarVm.entriesForDate(dateIso);
    }
    function entryAccent(entryModel) {
        const entryType = entryModel && entryModel.type !== undefined ? String(entryModel.type) : "";
        if (entryType === "task") {
            const completed = entryModel && entryModel.completed === true;
            return completed ? LV.Theme.descriptionColor : LV.Theme.warning;
        }

    }
    function entryLabel(entryModel) {
        const title = entryModel && entryModel.title !== undefined && String(entryModel.title).trim().length > 0 ? String(entryModel.title).trim() : "Untitled";
        const timeText = entryModel && entryModel.time !== undefined ? String(entryModel.time).trim() : "";
        if (timeText.length === 0)
            return title;
        return title + " " + timeText;
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        monthCalendarGridSurface.viewHookRequested(hookReason);
    }
    function weekdayLabelText(label) {
        if (label === undefined)
            return "";
        const normalizedText = String(label).trim();
        if (normalizedText.length === 0)
            return "";
        return normalizedText.length > 3 ? normalizedText.slice(0, 3).toUpperCase() : normalizedText.toUpperCase();
    }

    Rectangle {
        id: weekdayHeaderBand

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        color: LV.Theme.panelBackground06
        height: monthCalendarGridSurface.weekdayHeaderHeight

        GridLayout {
            anchors.fill: parent
            columnSpacing: 0
            columns: 7
            rowSpacing: 0

            Repeater {
                model: monthCalendarGridSurface.weekdayLabels

                Rectangle {
                    id: weekdayCell

                    required property var modelData

                    Layout.fillWidth: true
                    Layout.maximumHeight: monthCalendarGridSurface.weekdayHeaderHeight
                    Layout.minimumHeight: monthCalendarGridSurface.weekdayHeaderHeight
                    Layout.preferredHeight: monthCalendarGridSurface.weekdayHeaderHeight
                    border.color: LV.Theme.panelBackground06
                    border.width: Math.max(1, LV.Theme.strokeThin)
                    color: LV.Theme.panelBackground06

                    LV.Label {
                        anchors.left: parent.left
                        anchors.leftMargin: monthCalendarGridSurface.weekdayCellHorizontalPadding
                        anchors.verticalCenter: parent.verticalCenter
                        color: LV.Theme.titleHeaderColor
                        font.pixelSize: monthCalendarGridSurface.bodyLabelPixelSize
                        font.weight: Font.Medium
                        text: monthCalendarGridSurface.weekdayLabelText(weekdayCell.modelData)
                    }
                }
            }
        }
    }
    GridLayout {
        id: monthDayGrid

        readonly property int dayCellHeight: Math.max(1, Math.floor(monthDayGrid.height / monthCalendarGridSurface.visibleWeekRowCount))
        readonly property int dayCellWidth: Math.max(1, Math.floor(monthDayGrid.width / 7))

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: weekdayHeaderBand.bottom
        columnSpacing: 0
        columns: 7
        rowSpacing: 0

        Repeater {
            model: monthCalendarGridSurface.visibleDayModels

            MonthCalendarDayCell {
                id: dayCell

                readonly property var dayEntries: monthCalendarGridSurface.entriesForDate(dayCell.dayModel)
                readonly property var dayEntryCells: monthCalendarGridSurface.buildEntryCellModels(dayCell.dayEntries)
                readonly property var dayModel: dayCell.modelData
                readonly property bool isCurrentMonth: dayModel && dayModel.inCurrentMonth === true
                readonly property bool isSelectedDate: dayModel && dayModel.dateIso !== undefined && String(dayModel.dateIso) === monthCalendarGridSurface.selectedDateIso
                readonly property bool isToday: dayModel && dayModel.isToday === true
                required property var modelData

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.maximumHeight: monthDayGrid.dayCellHeight
                Layout.maximumWidth: monthDayGrid.dayCellWidth
                Layout.minimumHeight: monthDayGrid.dayCellHeight
                Layout.minimumWidth: monthDayGrid.dayCellWidth
                Layout.preferredHeight: monthDayGrid.dayCellHeight
                Layout.preferredWidth: monthDayGrid.dayCellWidth
                dayNumber: dayCell.dayModel && dayCell.dayModel.day !== undefined ? Number(dayCell.dayModel.day) : 0
                disable: !dayCell.isCurrentMonth
                entryCells: dayCell.dayEntryCells
                maxVisibleEntries: monthCalendarGridSurface.maxVisibleEntries
                selected: dayCell.isSelectedDate
                today: dayCell.isToday

                onClicked: {
                    if (!dayCell.dayModel || dayCell.dayModel.dateIso === undefined)
                        return;
                    const dateIso = String(dayCell.dayModel.dateIso);
                    if (dateIso.length === 0)
                        return;
                    monthCalendarGridSurface.requestViewHook("select-date");
                    monthCalendarGridSurface.dateSelected(dateIso);
                }
            }
        }
    }
}
