pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: weekCalendarPage

    readonly property var calendarVm: weekCalendarViewModel
    readonly property int lazyChunkSize: 4
    readonly property int initialWeekRadius: 6
    readonly property int maxWeekWindowSize: 33
    readonly property int preloadThreshold: 2
    readonly property int hourColumnWidth: LV.Theme.gap24 * 2
    readonly property var hourSlots: weekCalendarPage.buildHourSlots()
    readonly property bool horizontalDayScrollEnabled: LV.Theme.mobileTarget
    readonly property int mobileMinimumDayColumnWidth: LV.Theme.gap24 * 3
    property bool weekWindowInitialized: false
    property var weekCalendarViewModel: null

    signal viewHookRequested(string reason)

    function notifyViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        viewHookRequested(hookReason);
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarVm && calendarVm.requestWeekView)
            calendarVm.requestWeekView(hookReason);
        weekCalendarPage.notifyViewHook(hookReason);
    }
    function buildHourSlots() {
        var values = [];
        for (var hour = 0; hour < 24; ++hour)
            values.push(hour);
        return values;
    }
    function parseIsoDate(dateIso) {
        const normalizedDate = dateIso !== undefined ? String(dateIso).trim() : "";
        const parts = normalizedDate.split("-");
        if (parts.length !== 3)
            return null;
        const year = Number(parts[0]);
        const month = Number(parts[1]);
        const day = Number(parts[2]);
        if (!isFinite(year) || !isFinite(month) || !isFinite(day))
            return null;
        return new Date(year, month - 1, day);
    }
    function toIsoDate(dateObject) {
        if (!dateObject)
            return "";
        const year = String(dateObject.getFullYear());
        const monthNumber = dateObject.getMonth() + 1;
        const dayNumber = dateObject.getDate();
        const month = monthNumber < 10 ? "0" + String(monthNumber) : String(monthNumber);
        const day = dayNumber < 10 ? "0" + String(dayNumber) : String(dayNumber);
        return year + "-" + month + "-" + day;
    }
    function shiftDateIso(dateIso, deltaDays) {
        const baseDate = weekCalendarPage.parseIsoDate(dateIso);
        if (!baseDate)
            return "";
        const shiftedDate = new Date(baseDate.getFullYear(), baseDate.getMonth(), baseDate.getDate() + deltaDays);
        return weekCalendarPage.toIsoDate(shiftedDate);
    }
    function buildWeekLabel(weekStartIso) {
        const weekStartDate = weekCalendarPage.parseIsoDate(weekStartIso);
        if (!weekStartDate)
            return "Week";
        const weekEndDate = new Date(
                    weekStartDate.getFullYear(),
                    weekStartDate.getMonth(),
                    weekStartDate.getDate() + 6);
        return Qt.formatDate(weekStartDate, Qt.DefaultLocaleShortDate)
               + " - "
               + Qt.formatDate(weekEndDate, Qt.DefaultLocaleShortDate);
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
    function slotBackgroundType(entries) {
        if (!entries || entries.length === 0)
            return 0;
        const firstEntry = entries[0];
        const entryType = firstEntry && firstEntry.type !== undefined ? String(firstEntry.type) : "";
        return entryType === "event" ? 1 : 0;
    }
    function buildWeekDayModels(weekStartIso) {
        const weekStartDate = weekCalendarPage.parseIsoDate(weekStartIso);
        if (!weekStartDate)
            return [];

        const todayIso = weekCalendarPage.toIsoDate(new Date());
        var models = [];
        for (var offset = 0; offset < 7; ++offset) {
            const date = new Date(
                        weekStartDate.getFullYear(),
                        weekStartDate.getMonth(),
                        weekStartDate.getDate() + offset);
            const dateIso = weekCalendarPage.toIsoDate(date);
            const entries = weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.entriesForDate
                            ? weekCalendarPage.calendarVm.entriesForDate(dateIso)
                            : [];
            var eventCount = 0;
            var taskCount = 0;
            for (var entryIndex = 0; entryIndex < entries.length; ++entryIndex) {
                const entry = entries[entryIndex];
                const entryType = entry && entry.type !== undefined ? String(entry.type) : "";
                if (entryType === "event")
                    eventCount += 1;
                else if (entryType === "task")
                    taskCount += 1;
            }
            models.push({
                            "dateIso": dateIso,
                            "day": date.getDate(),
                            "dayLabel": Qt.formatDate(date, "ddd d"),
                            "entries": entries,
                            "entryCount": entries.length,
                            "eventCount": eventCount,
                            "taskCount": taskCount,
                            "isToday": dateIso === todayIso
                        });
        }

        return models;
    }
    function buildWeekModel(weekStartIso) {
        const normalizedIso = weekStartIso !== undefined ? String(weekStartIso).trim() : "";
        return {
            "weekStartIso": normalizedIso,
            "weekLabel": weekCalendarPage.buildWeekLabel(normalizedIso),
            "dayModels": weekCalendarPage.buildWeekDayModels(normalizedIso)
        };
    }
    function appendWeeks(count) {
        if (count <= 0)
            return;
        const anchorIso = weekModel.count > 0
                          ? String(weekModel.get(weekModel.count - 1).weekStartIso)
                          : weekCalendarPage.resolveInitialWeekStartIso();
        for (var offset = 1; offset <= count; ++offset) {
            const weekStartIso = weekCalendarPage.shiftDateIso(anchorIso, offset * 7);
            weekModel.append(weekCalendarPage.buildWeekModel(weekStartIso));
        }
    }
    function prependWeeks(count) {
        if (count <= 0 || weekModel.count <= 0)
            return;
        const firstWeekIso = String(weekModel.get(0).weekStartIso);
        var prependedWeeks = [];
        for (var offset = count; offset >= 1; --offset) {
            const weekStartIso = weekCalendarPage.shiftDateIso(firstWeekIso, -offset * 7);
            prependedWeeks.push(weekCalendarPage.buildWeekModel(weekStartIso));
        }
        for (var prependIndex = 0; prependIndex < prependedWeeks.length; ++prependIndex)
            weekModel.insert(prependIndex, prependedWeeks[prependIndex]);
        if (weekCalendarWeeksView.currentIndex >= 0) {
            weekCalendarWeeksView.currentIndex += prependedWeeks.length;
            weekCalendarWeeksView.positionViewAtIndex(weekCalendarWeeksView.currentIndex, ListView.Beginning);
        }
    }
    function trimWeekWindow() {
        while (weekModel.count > weekCalendarPage.maxWeekWindowSize) {
            const trimHead = weekCalendarWeeksView.currentIndex > Math.floor(weekModel.count / 2);
            if (trimHead) {
                weekModel.remove(0, 1);
                if (weekCalendarWeeksView.currentIndex > 0)
                    weekCalendarWeeksView.currentIndex -= 1;
            } else {
                weekModel.remove(weekModel.count - 1, 1);
            }
        }
    }
    function ensureLazyWindow(index) {
        if (index < 0 || weekModel.count <= 0)
            return;
        if (index <= weekCalendarPage.preloadThreshold)
            weekCalendarPage.prependWeeks(weekCalendarPage.lazyChunkSize);
        if (index >= weekModel.count - 1 - weekCalendarPage.preloadThreshold)
            weekCalendarPage.appendWeeks(weekCalendarPage.lazyChunkSize);
        weekCalendarPage.trimWeekWindow();
    }
    function resolveInitialWeekStartIso() {
        const currentVmIso = weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.displayedWeekStartIso !== undefined
                             ? String(weekCalendarPage.calendarVm.displayedWeekStartIso).trim()
                             : "";
        if (currentVmIso.length > 0)
            return currentVmIso;

        const todayIso = weekCalendarPage.toIsoDate(new Date());
        if (weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.setDisplayedWeekStartIso) {
            weekCalendarPage.calendarVm.setDisplayedWeekStartIso(todayIso);
            const normalizedVmIso = weekCalendarPage.calendarVm.displayedWeekStartIso !== undefined
                                    ? String(weekCalendarPage.calendarVm.displayedWeekStartIso).trim()
                                    : "";
            if (normalizedVmIso.length > 0)
                return normalizedVmIso;
        }
        return todayIso;
    }
    function syncDisplayedWeekForCurrentIndex(reason) {
        if (weekCalendarWeeksView.currentIndex < 0 || weekCalendarWeeksView.currentIndex >= weekModel.count)
            return;
        const activeWeekStartIso = String(weekModel.get(weekCalendarWeeksView.currentIndex).weekStartIso);
        const currentVmIso = weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.displayedWeekStartIso !== undefined
                             ? String(weekCalendarPage.calendarVm.displayedWeekStartIso).trim()
                             : "";
        if (weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.setDisplayedWeekStartIso
                && currentVmIso !== activeWeekStartIso)
            weekCalendarPage.calendarVm.setDisplayedWeekStartIso(activeWeekStartIso);
        weekCalendarPage.notifyViewHook(reason);
    }
    function initializeWeekWindow(centerWeekStartIso) {
        const resolvedCenterIso = centerWeekStartIso !== undefined ? String(centerWeekStartIso).trim() : "";
        if (resolvedCenterIso.length === 0)
            return;

        weekCalendarPage.weekWindowInitialized = false;
        weekModel.clear();
        for (var offset = -weekCalendarPage.initialWeekRadius; offset <= weekCalendarPage.initialWeekRadius; ++offset) {
            const weekStartIso = weekCalendarPage.shiftDateIso(resolvedCenterIso, offset * 7);
            weekModel.append(weekCalendarPage.buildWeekModel(weekStartIso));
        }

        weekCalendarWeeksView.currentIndex = weekCalendarPage.initialWeekRadius;
        weekCalendarWeeksView.positionViewAtIndex(weekCalendarWeeksView.currentIndex, ListView.Beginning);
        weekCalendarPage.weekWindowInitialized = true;
        weekCalendarPage.ensureLazyWindow(weekCalendarWeeksView.currentIndex);
        weekCalendarPage.syncDisplayedWeekForCurrentIndex("initialize-week-window");
    }
    function centerOnWeek(weekStartIso) {
        const normalizedIso = weekStartIso !== undefined ? String(weekStartIso).trim() : "";
        if (normalizedIso.length === 0)
            return;

        for (var index = 0; index < weekModel.count; ++index) {
            if (String(weekModel.get(index).weekStartIso) !== normalizedIso)
                continue;
            weekCalendarWeeksView.currentIndex = index;
            weekCalendarWeeksView.positionViewAtIndex(index, ListView.Beginning);
            weekCalendarPage.ensureLazyWindow(index);
            weekCalendarPage.syncDisplayedWeekForCurrentIndex("focus-week");
            return;
        }
        weekCalendarPage.initializeWeekWindow(normalizedIso);
    }
    function refreshLoadedWeeks() {
        for (var index = 0; index < weekModel.count; ++index) {
            const weekStartIso = String(weekModel.get(index).weekStartIso);
            weekModel.setProperty(index, "weekLabel", weekCalendarPage.buildWeekLabel(weekStartIso));
            weekModel.setProperty(index, "dayModels", weekCalendarPage.buildWeekDayModels(weekStartIso));
        }
    }
    function jumpToCurrentWeek() {
        const todayIso = weekCalendarPage.toIsoDate(new Date());
        if (weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.setDisplayedWeekStartIso)
            weekCalendarPage.calendarVm.setDisplayedWeekStartIso(todayIso);
        const centeredWeekIso = weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.displayedWeekStartIso !== undefined
                                ? String(weekCalendarPage.calendarVm.displayedWeekStartIso)
                                : todayIso;
        weekCalendarPage.centerOnWeek(centeredWeekIso);
        weekCalendarPage.requestViewHook("current-week");
    }
    function shiftVisibleWeek(deltaWeeks) {
        if (weekModel.count <= 0 || deltaWeeks === 0)
            return;
        const targetIndex = weekCalendarWeeksView.currentIndex + deltaWeeks;
        if (targetIndex < 0) {
            weekCalendarPage.prependWeeks(Math.max(weekCalendarPage.lazyChunkSize, -targetIndex));
            weekCalendarPage.trimWeekWindow();
        } else if (targetIndex >= weekModel.count) {
            weekCalendarPage.appendWeeks(Math.max(weekCalendarPage.lazyChunkSize, targetIndex - weekModel.count + 1));
            weekCalendarPage.trimWeekWindow();
        }

        const clampedIndex = Math.max(0, Math.min(weekModel.count - 1, weekCalendarWeeksView.currentIndex + deltaWeeks));
        weekCalendarWeeksView.currentIndex = clampedIndex;
        weekCalendarWeeksView.positionViewAtIndex(clampedIndex, ListView.Beginning);
        weekCalendarPage.ensureLazyWindow(clampedIndex);
        weekCalendarPage.syncDisplayedWeekForCurrentIndex(deltaWeeks > 0 ? "next-week" : "previous-week");
    }

    color: "transparent"
    radius: LV.Theme.radiusMd
    Layout.fillHeight: true
    Layout.fillWidth: true

    Component.onCompleted: {
        weekCalendarPage.initializeWeekWindow(weekCalendarPage.resolveInitialWeekStartIso());
        weekCalendarPage.requestViewHook("page-open");
    }

    Connections {
        target: weekCalendarPage.calendarVm
        ignoreUnknownSignals: true

        function onWeekViewChanged() {
            weekCalendarPage.refreshLoadedWeeks();
        }
    }

    ListModel {
        id: weekModel
    }

    LV.VStack {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap12
        spacing: LV.Theme.gap12

        CalendarTodayControl {
            id: weekCalendarControl

            onPreviousRequested: weekCalendarPage.shiftVisibleWeek(-1)
            onTodayRequested: weekCalendarPage.jumpToCurrentWeek()
            onNextRequested: weekCalendarPage.shiftVisibleWeek(1)
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LV.Theme.panelBackground09
            radius: LV.Theme.radiusSm

            ListView {
                id: weekCalendarWeeksView
                anchors.fill: parent
                anchors.margins: LV.Theme.gap3
                boundsBehavior: Flickable.StopAtBounds
                cacheBuffer: Math.max(width, 1) * 2
                clip: true
                interactive: weekModel.count > 1
                model: weekModel
                orientation: ListView.Horizontal
                snapMode: ListView.SnapOneItem

                onCurrentIndexChanged: {
                    if (!weekCalendarPage.weekWindowInitialized || currentIndex < 0)
                        return;
                    weekCalendarPage.ensureLazyWindow(currentIndex);
                    weekCalendarPage.syncDisplayedWeekForCurrentIndex("week-scroll");
                }
                onMovementEnded: {
                    if (!weekCalendarPage.weekWindowInitialized || currentIndex < 0)
                        return;
                    weekCalendarPage.ensureLazyWindow(currentIndex);
                }

                delegate: Item {
                    id: weekPage

                    required property var dayModels
                    required property string weekLabel
                    required property string weekStartIso
                    readonly property int fittedDayColumnWidth: Math.max(
                                                                    1,
                                                                    Math.floor(
                                                                        (weekPage.width - weekCalendarPage.hourColumnWidth - (LV.Theme.gap2 * 6))
                                                                        / 7))
                    readonly property int minimumDayColumnWidth: weekCalendarPage.horizontalDayScrollEnabled
                                                                ? weekCalendarPage.mobileMinimumDayColumnWidth
                                                                : 1
                    readonly property int dayColumnWidth: Math.max(weekPage.minimumDayColumnWidth,
                                                                   weekPage.fittedDayColumnWidth)
                    readonly property int weekTimelineWidth: weekCalendarPage.hourColumnWidth
                                                            + (weekPage.dayColumnWidth * 7)
                                                            + (LV.Theme.gap2 * 6)
                    readonly property bool dayColumnScrollEnabled: weekCalendarPage.horizontalDayScrollEnabled
                                                                   && weekPage.weekTimelineWidth > weekTimelineViewport.width
                    readonly property int hourSlotCount: Math.max(1, weekCalendarPage.hourSlots.length)
                    readonly property real hourRowSpacing: LV.Theme.gap2
                    readonly property real hourRowHeight: Math.max(
                                                              1,
                                                              (weekTimelineViewport.height - (weekPage.hourRowSpacing * (weekPage.hourSlotCount - 1)))
                                                              / weekPage.hourSlotCount)
                    height: weekCalendarWeeksView.height
                    width: weekCalendarWeeksView.width

                    Flickable {
                        id: weekTimelineViewport

                        anchors.fill: parent
                        boundsBehavior: Flickable.StopAtBounds
                        contentHeight: weekTimelineViewport.height
                        contentWidth: weekTimelineColumn.width
                        flickableDirection: Flickable.HorizontalFlick
                        clip: true
                        interactive: weekPage.dayColumnScrollEnabled

                        Column {
                            id: weekTimelineColumn

                            height: weekTimelineViewport.height
                            spacing: weekPage.hourRowSpacing
                            width: weekPage.weekTimelineWidth

                            Repeater {
                                model: weekCalendarPage.hourSlots

                                Rectangle {
                                    id: hourRow

                                    required property var modelData
                                    readonly property int hour: Number(hourRow.modelData)

                                    color: hourRow.hour % 2 === 0 ? LV.Theme.panelBackground10 : LV.Theme.panelBackground11
                                    height: weekPage.hourRowHeight
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
                                            model: weekPage.dayModels

                                            Rectangle {
                                                id: dayHourCell

                                                required property var modelData
                                                readonly property var dayModel: dayHourCell.modelData
                                                readonly property var slotEntries: weekCalendarPage.entriesForHour(
                                                                                     dayHourCell.dayModel,
                                                                                     hourRow.hour)

                                                border.color: LV.Theme.accentTransparent
                                                border.width: Math.max(1, LV.Theme.strokeThin)
                                                color: LV.Theme.accentTransparent
                                                height: hourRow.height
                                                radius: LV.Theme.radiusSm
                                                width: weekPage.dayColumnWidth

                                                CalendarEventCell {
                                                    anchors.fill: parent
                                                    backgroundType: weekCalendarPage.slotBackgroundType(dayHourCell.slotEntries)
                                                    coloredBackgroundColor: LV.Theme.primary
                                                    cornerRadius: LV.Theme.radiusSm
                                                    defaultBackgroundColor: LV.Theme.panelBackground11
                                                    horizontalInset: LV.Theme.gap2
                                                    label: weekCalendarPage.slotSummary(dayHourCell.slotEntries)
                                                    textColor: LV.Theme.titleHeaderColor
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
    }
}
