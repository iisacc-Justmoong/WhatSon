pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: weekCalendarPage

    readonly property var calendarVm: weekCalendarViewModel
    readonly property int dayColumnSpacing: LV.Theme.gap2
    readonly property int hourColumnWidth: LV.Theme.gap24 * 2
    readonly property var hourSlots: weekCalendarPage.buildHourSlots()
    readonly property int initialDateRadius: 21
    readonly property int lazyChunkSize: 14
    readonly property int maxDateWindowSize: 120
    readonly property int preloadThreshold: 12
    readonly property int visibleDayColumnCount: 3
    readonly property int centeredDayColumnOffset: Math.floor(weekCalendarPage.visibleDayColumnCount / 2)
    property bool dateWindowInitialized: false
    property bool suppressViewportSync: false
    property var dateEntriesCache: ({})
    property var weekCalendarViewModel: null

    signal viewHookRequested(string reason)

    function appendDates(count) {
        if (count <= 0)
            return;
        const anchorIso = dateModel.count > 0
                        ? String(dateModel.get(dateModel.count - 1).dateIso)
                        : weekCalendarPage.resolveInitialDateIso();
        for (var offset = 1; offset <= count; ++offset)
            dateModel.append(weekCalendarPage.buildDateModel(weekCalendarPage.shiftDateIso(anchorIso, offset)));
    }
    function buildDateModel(dateIso) {
        const date = weekCalendarPage.parseIsoDate(dateIso);
        if (!date)
            return ({});

        const normalizedDateIso = weekCalendarPage.toIsoDate(date);
        const entries = weekCalendarPage.entriesForDate(normalizedDateIso);
        const todayIso = weekCalendarPage.toIsoDate(new Date());
        const currentWeekStartIso = weekCalendarPage.normalizedWeekStartIso(todayIso);
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

        return {
            "dateIso": normalizedDateIso,
            "entryCount": entries.length,
            "eventCount": eventCount,
            "taskCount": taskCount,
            "weekdayLabel": Qt.formatDate(date, "ddd"),
            "dateLabel": Qt.formatDate(date, "M/d"),
            "isInCurrentWeek": weekCalendarPage.normalizedWeekStartIso(normalizedDateIso) === currentWeekStartIso,
            "isToday": normalizedDateIso === todayIso
        };
    }
    function buildHourSlots() {
        var values = [];
        for (var hour = 0; hour < 24; ++hour)
            values.push(hour);
        return values;
    }
    function centerOnDate(dateIso) {
        const normalizedIso = dateIso !== undefined ? String(dateIso).trim() : "";
        if (normalizedIso.length === 0)
            return;

        for (var index = 0; index < dateModel.count; ++index) {
            if (String(dateModel.get(index).dateIso) !== normalizedIso)
                continue;
            weekCalendarPage.setViewportContentX(
                        Math.max(0, (index - weekCalendarPage.centeredDayColumnOffset) * timelineScaffold.dayColumnSpan));
            weekCalendarPage.ensureLazyDates();
            weekCalendarPage.syncDisplayedWeekForDate(normalizedIso, "focus-date");
            return;
        }

        weekCalendarPage.initializeDateWindow(normalizedIso, true, "focus-date");
    }
    function currentFocusedDateIndex() {
        if (dateModel.count <= 0)
            return -1;
        if (timelineScaffold.dayColumnSpan <= 0)
            return 0;
        const viewportCenterX = dateColumnsFlickable.contentX + (timelineScaffold.dateViewportWidth / 2);
        const centeredIndex = Math.round(
                    (viewportCenterX - (timelineScaffold.dayColumnWidth / 2))
                    / timelineScaffold.dayColumnSpan);
        return Math.max(0, Math.min(dateModel.count - 1, centeredIndex));
    }
    function currentLeadingDateIndex() {
        if (dateModel.count <= 0)
            return -1;
        if (timelineScaffold.dayColumnSpan <= 0)
            return 0;
        return Math.max(
                    0,
                    Math.min(
                        dateModel.count - 1,
                        Math.floor(dateColumnsFlickable.contentX / timelineScaffold.dayColumnSpan)));
    }
    function ensureLazyDates() {
        if (!weekCalendarPage.dateWindowInitialized || dateModel.count <= 0)
            return;
        const leadingIndex = weekCalendarPage.currentLeadingDateIndex();
        if (leadingIndex <= weekCalendarPage.preloadThreshold)
            weekCalendarPage.prependDates(weekCalendarPage.lazyChunkSize);
        if (leadingIndex >= dateModel.count - weekCalendarPage.visibleDayColumnCount - weekCalendarPage.preloadThreshold)
            weekCalendarPage.appendDates(weekCalendarPage.lazyChunkSize);
        weekCalendarPage.trimDateWindow();
    }
    function entriesForDate(dateIso) {
        const normalizedDateIso = dateIso !== undefined ? String(dateIso).trim() : "";
        if (normalizedDateIso.length === 0)
            return [];
        if (weekCalendarPage.dateEntriesCache[normalizedDateIso] !== undefined)
            return weekCalendarPage.dateEntriesCache[normalizedDateIso];
        const resolvedEntries = weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.entriesForDate
                ? weekCalendarPage.calendarVm.entriesForDate(normalizedDateIso)
                : [];
        weekCalendarPage.dateEntriesCache[normalizedDateIso] = resolvedEntries;
        return resolvedEntries;
    }
    function entriesForHour(dayModel, hour) {
        if (!dayModel || dayModel.dateIso === undefined)
            return [];
        const dayEntries = weekCalendarPage.entriesForDate(String(dayModel.dateIso));
        var matches = [];
        for (var index = 0; index < dayEntries.length; ++index) {
            const entry = dayEntries[index];
            if (!entry || entry.time === undefined)
                continue;
            const timeParts = String(entry.time).split(":");
            if (timeParts.length === 0)
                continue;
            const entryHour = Number(timeParts[0]);
            if (!isFinite(entryHour) || Math.floor(entryHour) !== hour)
                continue;
            matches.push(entry);
        }
        return matches;
    }
    function initializeDateWindow(anchorDateIso, centerAnchor, syncReason) {
        const resolvedAnchorIso = anchorDateIso !== undefined ? String(anchorDateIso).trim() : "";
        if (resolvedAnchorIso.length === 0)
            return;

        weekCalendarPage.dateWindowInitialized = false;
        weekCalendarPage.dateEntriesCache = ({});
        dateModel.clear();
        for (var offset = -weekCalendarPage.initialDateRadius; offset <= weekCalendarPage.initialDateRadius; ++offset)
            dateModel.append(weekCalendarPage.buildDateModel(weekCalendarPage.shiftDateIso(resolvedAnchorIso, offset)));

        Qt.callLater(function() {
            const anchorIndex = weekCalendarPage.initialDateRadius;
            const initialContentX = centerAnchor === true
                    ? Math.max(0, (anchorIndex - weekCalendarPage.centeredDayColumnOffset) * timelineScaffold.dayColumnSpan)
                    : anchorIndex * timelineScaffold.dayColumnSpan;
            weekCalendarPage.setViewportContentX(initialContentX);
            weekCalendarPage.dateWindowInitialized = true;
            weekCalendarPage.ensureLazyDates();
            if (centerAnchor === true)
                weekCalendarPage.syncDisplayedWeekForDate(resolvedAnchorIso, syncReason !== undefined ? String(syncReason) : "initialize-date-window");
            else
                weekCalendarPage.syncDisplayedWeekFromViewport(syncReason !== undefined ? String(syncReason) : "initialize-date-window");
        });
    }
    function jumpToCurrentWeek() {
        const todayIso = weekCalendarPage.toIsoDate(new Date());
        weekCalendarPage.centerOnDate(todayIso);
        weekCalendarPage.requestViewHook("current-week");
    }
    function focusedDateIso() {
        const focusedIndex = weekCalendarPage.currentFocusedDateIndex();
        if (focusedIndex >= 0 && focusedIndex < dateModel.count)
            return String(dateModel.get(focusedIndex).dateIso);
        return weekCalendarPage.resolveInitialDateIso();
    }
    function normalizedWeekStartIso(dateIso) {
        const baseDate = weekCalendarPage.parseIsoDate(dateIso);
        if (!baseDate)
            return "";
        const dayOfWeek = baseDate.getDay();
        const distanceFromMonday = dayOfWeek === 0 ? 6 : dayOfWeek - 1;
        const weekStartDate = new Date(
                    baseDate.getFullYear(),
                    baseDate.getMonth(),
                    baseDate.getDate() - distanceFromMonday);
        return weekCalendarPage.toIsoDate(weekStartDate);
    }
    function notifyViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        viewHookRequested(hookReason);
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
    function prependDates(count) {
        if (count <= 0 || dateModel.count <= 0)
            return;
        const firstDateIso = String(dateModel.get(0).dateIso);
        var prependModels = [];
        for (var offset = count; offset >= 1; --offset)
            prependModels.push(weekCalendarPage.buildDateModel(weekCalendarPage.shiftDateIso(firstDateIso, -offset)));
        for (var index = 0; index < prependModels.length; ++index)
            dateModel.insert(index, prependModels[index]);
        weekCalendarPage.setViewportContentX(dateColumnsFlickable.contentX + (prependModels.length * timelineScaffold.dayColumnSpan));
    }
    function refreshLoadedDates() {
        weekCalendarPage.dateEntriesCache = ({});
        for (var index = 0; index < dateModel.count; ++index) {
            const dateIso = String(dateModel.get(index).dateIso);
            dateModel.set(index, weekCalendarPage.buildDateModel(dateIso));
        }
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarVm && calendarVm.requestWeekView)
            calendarVm.requestWeekView(hookReason);
        weekCalendarPage.notifyViewHook(hookReason);
    }
    function resolveInitialDateIso() {
        const todayIso = weekCalendarPage.toIsoDate(new Date());
        const currentWeekStartIso = weekCalendarPage.normalizedWeekStartIso(todayIso);
        const currentVmIso = weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.displayedWeekStartIso !== undefined
                ? String(weekCalendarPage.calendarVm.displayedWeekStartIso).trim()
                : "";
        if (currentVmIso.length > 0) {
            if (currentVmIso === currentWeekStartIso)
                return todayIso;
            return currentVmIso;
        }

        if (weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.setDisplayedWeekStartIso)
            weekCalendarPage.calendarVm.setDisplayedWeekStartIso(currentWeekStartIso);
        return todayIso;
    }
    function setViewportContentX(nextContentX) {
        weekCalendarPage.suppressViewportSync = true;
        dateColumnsFlickable.contentX = Math.max(0, nextContentX);
        Qt.callLater(function() {
            weekCalendarPage.suppressViewportSync = false;
        });
    }
    function shiftDateIso(dateIso, deltaDays) {
        const baseDate = weekCalendarPage.parseIsoDate(dateIso);
        if (!baseDate)
            return "";
        const shiftedDate = new Date(
                    baseDate.getFullYear(),
                    baseDate.getMonth(),
                    baseDate.getDate() + deltaDays);
        return weekCalendarPage.toIsoDate(shiftedDate);
    }
    function slotBackgroundType(entries) {
        if (!entries || entries.length === 0)
            return 0;
        const firstEntry = entries[0];
        const entryType = firstEntry && firstEntry.type !== undefined ? String(firstEntry.type) : "";
        return entryType === "event" ? 1 : 0;
    }
    function slotSummary(entries) {
        if (!entries || entries.length === 0)
            return "";
        const firstEntry = entries[0];
        const title = firstEntry && firstEntry.title !== undefined ? String(firstEntry.title) : "Item";
        if (entries.length === 1)
            return title;
        return title + " +" + String(entries.length - 1);
    }
    function syncDisplayedWeekForDate(dateIso, reason) {
        const activeWeekStartIso = weekCalendarPage.normalizedWeekStartIso(dateIso);
        const currentVmIso = weekCalendarPage.calendarVm && weekCalendarPage.calendarVm.displayedWeekStartIso !== undefined
                ? String(weekCalendarPage.calendarVm.displayedWeekStartIso).trim()
                : "";
        if (weekCalendarPage.calendarVm
                && weekCalendarPage.calendarVm.setDisplayedWeekStartIso
                && activeWeekStartIso.length > 0
                && currentVmIso !== activeWeekStartIso) {
            weekCalendarPage.calendarVm.setDisplayedWeekStartIso(activeWeekStartIso);
        }
        weekCalendarPage.notifyViewHook(reason);
    }
    function syncDisplayedWeekFromViewport(reason) {
        const focusedIndex = weekCalendarPage.currentFocusedDateIndex();
        if (focusedIndex < 0 || focusedIndex >= dateModel.count)
            return;
        weekCalendarPage.syncDisplayedWeekForDate(String(dateModel.get(focusedIndex).dateIso), reason);
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
    function trimDateWindow() {
        while (dateModel.count > weekCalendarPage.maxDateWindowSize) {
            const leadingIndex = weekCalendarPage.currentLeadingDateIndex();
            const removeHead = leadingIndex > Math.floor(dateModel.count / 2);
            const removeCount = Math.min(
                        weekCalendarPage.lazyChunkSize,
                        dateModel.count - weekCalendarPage.maxDateWindowSize);
            if (removeCount <= 0)
                return;
            if (removeHead) {
                dateModel.remove(0, removeCount);
                weekCalendarPage.setViewportContentX(
                            Math.max(0, dateColumnsFlickable.contentX - (removeCount * timelineScaffold.dayColumnSpan)));
            } else {
                dateModel.remove(dateModel.count - removeCount, removeCount);
            }
        }
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    color: "transparent"
    radius: LV.Theme.radiusMd

    Component.onCompleted: {
        Qt.callLater(function() {
            weekCalendarPage.initializeDateWindow(weekCalendarPage.resolveInitialDateIso(), true, "initialize-date-window");
            weekCalendarPage.requestViewHook("page-open");
        });
    }

    Connections {
        function onWeekViewChanged() {
            weekCalendarPage.refreshLoadedDates();
        }

        ignoreUnknownSignals: true
        target: weekCalendarPage.calendarVm
    }
    ListModel {
        id: dateModel
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap12
        spacing: LV.Theme.gap12

        CalendarTodayControl {
            id: weekCalendarControl

            onNextRequested: weekCalendarPage.centerOnDate(weekCalendarPage.shiftDateIso(weekCalendarPage.focusedDateIso(), 7))
            onPreviousRequested: weekCalendarPage.centerOnDate(weekCalendarPage.shiftDateIso(weekCalendarPage.focusedDateIso(), -7))
            onTodayRequested: weekCalendarPage.jumpToCurrentWeek()
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LV.Theme.panelBackground09
            radius: LV.Theme.radiusSm

            Item {
                id: timelineScaffold

                anchors.fill: parent
                anchors.margins: LV.Theme.gap3
                readonly property real dateViewportWidth: Math.max(1, width - weekCalendarPage.hourColumnWidth - weekCalendarPage.dayColumnSpacing)
                readonly property real dayColumnWidth: Math.max(
                                                           1,
                                                           (dateViewportWidth
                                                            - (weekCalendarPage.dayColumnSpacing
                                                               * (weekCalendarPage.visibleDayColumnCount - 1)))
                                                           / weekCalendarPage.visibleDayColumnCount)
                readonly property real dayColumnSpan: dayColumnWidth + weekCalendarPage.dayColumnSpacing
                readonly property real headerHeight: Math.max(44, LV.Theme.gap24 * 2)
                readonly property int hourSlotCount: Math.max(1, weekCalendarPage.hourSlots.length)
                readonly property real bodyHeight: Math.max(1, height - headerHeight - weekCalendarPage.dayColumnSpacing)
                readonly property real hourRowHeight: Math.max(
                                                          1,
                                                          (bodyHeight
                                                           - (weekCalendarPage.dayColumnSpacing * (hourSlotCount - 1)))
                                                          / hourSlotCount)

                Column {
                    id: timeColumn

                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.top: parent.top
                    spacing: weekCalendarPage.dayColumnSpacing
                    width: weekCalendarPage.hourColumnWidth

                    Rectangle {
                        color: LV.Theme.panelBackground11
                        height: timelineScaffold.headerHeight
                        radius: LV.Theme.radiusSm
                        width: parent.width

                        LV.Label {
                            anchors.centerIn: parent
                            color: LV.Theme.descriptionColor
                            text: "Time"
                        }
                    }
                    Repeater {
                        model: weekCalendarPage.hourSlots

                        Rectangle {
                            id: hourLabelCell

                            readonly property int hour: Number(hourLabelCell.modelData)
                            required property var modelData

                            color: LV.Theme.panelBackground12
                            height: timelineScaffold.hourRowHeight
                            radius: LV.Theme.radiusSm
                            width: timeColumn.width

                            LV.Label {
                                anchors.centerIn: parent
                                text: Qt.formatTime(new Date(2000, 0, 1, hourLabelCell.hour, 0, 0), "HH:mm")
                            }
                        }
                    }
                }
                Flickable {
                    id: dateColumnsFlickable

                    anchors.bottom: parent.bottom
                    anchors.left: timeColumn.right
                    anchors.leftMargin: weekCalendarPage.dayColumnSpacing
                    anchors.right: parent.right
                    anchors.top: parent.top
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    contentHeight: height
                    contentWidth: dateColumnsContent.width
                    flickableDirection: Flickable.HorizontalFlick
                    interactive: dateModel.count > weekCalendarPage.visibleDayColumnCount

                    onContentXChanged: {
                        if (!weekCalendarPage.dateWindowInitialized || weekCalendarPage.suppressViewportSync)
                            return;
                        weekCalendarPage.ensureLazyDates();
                    }
                    onMovementEnded: {
                        if (!weekCalendarPage.dateWindowInitialized || weekCalendarPage.suppressViewportSync)
                            return;
                        weekCalendarPage.ensureLazyDates();
                        weekCalendarPage.syncDisplayedWeekFromViewport("date-scroll");
                    }

                    Item {
                        id: dateColumnsContent

                        height: dateColumnsFlickable.height
                        width: dateModel.count > 0
                               ? (dateModel.count * timelineScaffold.dayColumnWidth)
                                 + ((dateModel.count - 1) * weekCalendarPage.dayColumnSpacing)
                               : 0

                        Repeater {
                            model: dateModel

                            Rectangle {
                                id: dayHeaderCell

                                required property int index
                                readonly property var dayModel: dayHeaderCell.modelData
                                required property var modelData

                                border.color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isToday
                                              ? LV.Theme.accent
                                              : LV.Theme.accentTransparent
                                border.width: Math.max(1, LV.Theme.strokeThin)
                                color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isToday
                                       ? LV.Theme.accentTransparent
                                       : "transparent"
                                height: timelineScaffold.headerHeight
                                radius: LV.Theme.radiusSm
                                width: timelineScaffold.dayColumnWidth
                                x: dayHeaderCell.index * timelineScaffold.dayColumnSpan
                                y: 0

                                Column {
                                    anchors.centerIn: parent
                                    spacing: LV.Theme.gapNone

                                    LV.Label {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isToday
                                               ? LV.Theme.accent
                                               : (dayHeaderCell.dayModel
                                                  && dayHeaderCell.dayModel.isInCurrentWeek
                                                  ? LV.Theme.titleHeaderColor
                                                  : LV.Theme.descriptionColor)
                                        font.weight: Font.Medium
                                        text: dayHeaderCell.dayModel
                                              && dayHeaderCell.dayModel.weekdayLabel !== undefined
                                              ? String(dayHeaderCell.dayModel.weekdayLabel)
                                              : ""
                                    }
                                    LV.Label {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isInCurrentWeek
                                               ? LV.Theme.descriptionColor
                                               : Qt.darker(LV.Theme.descriptionColor, 1.2)
                                        text: dayHeaderCell.dayModel
                                              && dayHeaderCell.dayModel.dateLabel !== undefined
                                              ? String(dayHeaderCell.dayModel.dateLabel)
                                              : ""
                                    }
                                }
                            }
                        }
                        Column {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.topMargin: timelineScaffold.headerHeight + weekCalendarPage.dayColumnSpacing
                            spacing: weekCalendarPage.dayColumnSpacing
                            width: parent.width

                            Repeater {
                                model: weekCalendarPage.hourSlots

                                Rectangle {
                                    id: hourRow

                                    readonly property int hour: Number(hourRow.modelData)
                                    required property var modelData

                                    color: "transparent"
                                    height: timelineScaffold.hourRowHeight
                                    radius: LV.Theme.radiusSm
                                    width: parent.width

                                    Repeater {
                                        model: dateModel

                                        Rectangle {
                                            id: dayHourCell

                                            required property int index
                                            readonly property var dayModel: dayHourCell.modelData
                                            required property var modelData
                                            readonly property var slotEntries: weekCalendarPage.entriesForHour(
                                                                                  dayHourCell.dayModel,
                                                                                  hourRow.hour)

                                            border.color: dayHourCell.dayModel && dayHourCell.dayModel.isToday
                                                          ? LV.Theme.accentTransparent
                                                          : LV.Theme.accentTransparent
                                            border.width: Math.max(1, LV.Theme.strokeThin)
                                            color: "transparent"
                                            height: timelineScaffold.hourRowHeight
                                            radius: LV.Theme.radiusSm
                                            width: timelineScaffold.dayColumnWidth
                                            x: dayHourCell.index * timelineScaffold.dayColumnSpan

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
