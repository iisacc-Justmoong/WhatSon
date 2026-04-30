pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: weekCalendarPage

    readonly property var calendarController: weekCalendarController
    readonly property int dayColumnSpacing: LV.Theme.gap2
    readonly property int hourColumnWidth: LV.Theme.gap24 * 2
    readonly property var hourSlots: weekCalendarPage.buildHourSlots()
    readonly property int initialDateRadius: LV.Theme.gap20 + Math.round(LV.Theme.strokeThin)
    readonly property int lazyChunkSize: 14
    readonly property int maxDateWindowSize: 120
    readonly property int preloadThreshold: 12
    readonly property int timelineDayCount: weekCalendarPage.timelineDayModels.length
    readonly property var timelineDayModels: weekCalendarPage.calendarController && weekCalendarPage.calendarController.timelineDayModels ? weekCalendarPage.calendarController.timelineDayModels : []
    readonly property int visibleDayColumnCount: 3
    readonly property int centeredDayColumnOffset: Math.floor(weekCalendarPage.visibleDayColumnCount / 2)
    property bool dateWindowInitialized: false
    property bool suppressViewportSync: false
    property var weekCalendarController: null

    signal noteOpenRequested(string noteId)
    signal viewHookRequested(string reason)

    function appendDates(count) {
        if (count <= 0 || !weekCalendarPage.calendarController || !weekCalendarPage.calendarController.appendTimelineDates)
            return;
        weekCalendarPage.calendarController.appendTimelineDates(count);
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

        for (var index = 0; index < weekCalendarPage.timelineDayCount; ++index) {
            const dayModel = weekCalendarPage.timelineDayModels[index];
            if (!dayModel || String(dayModel.dateIso) !== normalizedIso)
                continue;
            weekCalendarPage.setViewportContentX(Math.max(0, (index - weekCalendarPage.centeredDayColumnOffset) * timelineScaffold.dayColumnSpan));
            weekCalendarPage.ensureLazyDates();
            weekCalendarPage.syncDisplayedWeekForDate(normalizedIso, "focus-date");
            return;
        }

        weekCalendarPage.initializeDateWindow(normalizedIso, true, "focus-date");
    }
    function currentFocusedDateIndex() {
        if (weekCalendarPage.timelineDayCount <= 0)
            return -1;
        if (timelineScaffold.dayColumnSpan <= 0)
            return 0;
        const viewportCenterX = dateColumnsFlickable.contentX + (timelineScaffold.dateViewportWidth / 2);
        const centeredIndex = Math.round((viewportCenterX - (timelineScaffold.dayColumnWidth / 2)) / timelineScaffold.dayColumnSpan);
        return Math.max(0, Math.min(weekCalendarPage.timelineDayCount - 1, centeredIndex));
    }
    function currentLeadingDateIndex() {
        if (weekCalendarPage.timelineDayCount <= 0)
            return -1;
        if (timelineScaffold.dayColumnSpan <= 0)
            return 0;
        return Math.max(0, Math.min(weekCalendarPage.timelineDayCount - 1, Math.floor(dateColumnsFlickable.contentX / timelineScaffold.dayColumnSpan)));
    }
    function ensureLazyDates() {
        if (!weekCalendarPage.dateWindowInitialized || weekCalendarPage.timelineDayCount <= 0)
            return;
        const leadingIndex = weekCalendarPage.currentLeadingDateIndex();
        if (leadingIndex <= weekCalendarPage.preloadThreshold)
            weekCalendarPage.prependDates(weekCalendarPage.lazyChunkSize);
        if (leadingIndex >= weekCalendarPage.timelineDayCount - weekCalendarPage.visibleDayColumnCount - weekCalendarPage.preloadThreshold)
            weekCalendarPage.appendDates(weekCalendarPage.lazyChunkSize);
        weekCalendarPage.trimDateWindow();
    }
    function entriesForHour(dayModel, hour) {
        if (!dayModel || dayModel.dateIso === undefined)
            return [];
        const dayEntries = dayModel.entries !== undefined && dayModel.entries !== null ? dayModel.entries : [];
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
        if (weekCalendarPage.calendarController && weekCalendarPage.calendarController.initializeTimelineWindow)
            weekCalendarPage.calendarController.initializeTimelineWindow(resolvedAnchorIso, weekCalendarPage.initialDateRadius);

        Qt.callLater(function () {
            const anchorIndex = weekCalendarPage.initialDateRadius;
            const initialContentX = centerAnchor === true ? Math.max(0, (anchorIndex - weekCalendarPage.centeredDayColumnOffset) * timelineScaffold.dayColumnSpan) : anchorIndex * timelineScaffold.dayColumnSpan;
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
        if (focusedIndex >= 0 && focusedIndex < weekCalendarPage.timelineDayCount)
            return String(weekCalendarPage.timelineDayModels[focusedIndex].dateIso);
        return weekCalendarPage.resolveInitialDateIso();
    }
    function normalizedWeekStartIso(dateIso) {
        const baseDate = weekCalendarPage.parseIsoDate(dateIso);
        if (!baseDate)
            return "";
        const dayOfWeek = baseDate.getDay();
        const distanceFromMonday = dayOfWeek === 0 ? 6 : dayOfWeek - 1;
        const weekStartDate = new Date(baseDate.getFullYear(), baseDate.getMonth(), baseDate.getDate() - distanceFromMonday);
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
        if (count <= 0 || weekCalendarPage.timelineDayCount <= 0 || !weekCalendarPage.calendarController || !weekCalendarPage.calendarController.prependTimelineDates)
            return;
        const prependedCount = Number(weekCalendarPage.calendarController.prependTimelineDates(count));
        if (isFinite(prependedCount) && prependedCount > 0)
            weekCalendarPage.setViewportContentX(dateColumnsFlickable.contentX + (prependedCount * timelineScaffold.dayColumnSpan));
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarController && calendarController.requestWeekView)
            calendarController.requestWeekView(hookReason);
        weekCalendarPage.notifyViewHook(hookReason);
    }
    function resolveInitialDateIso() {
        const todayIso = weekCalendarPage.toIsoDate(new Date());
        const currentWeekStartIso = weekCalendarPage.normalizedWeekStartIso(todayIso);
        const currentVmIso = weekCalendarPage.calendarController && weekCalendarPage.calendarController.displayedWeekStartIso !== undefined ? String(weekCalendarPage.calendarController.displayedWeekStartIso).trim() : "";
        if (currentVmIso.length > 0) {
            if (currentVmIso === currentWeekStartIso)
                return todayIso;
            return currentVmIso;
        }

        if (weekCalendarPage.calendarController && weekCalendarPage.calendarController.setDisplayedWeekStartIso)
            weekCalendarPage.calendarController.setDisplayedWeekStartIso(currentWeekStartIso);
        return todayIso;
    }
    function setViewportContentX(nextContentX) {
        weekCalendarPage.suppressViewportSync = true;
        dateColumnsFlickable.contentX = Math.max(0, nextContentX);
        Qt.callLater(function () {
            weekCalendarPage.suppressViewportSync = false;
        });
    }
    function shiftDateIso(dateIso, deltaDays) {
        const baseDate = weekCalendarPage.parseIsoDate(dateIso);
        if (!baseDate)
            return "";
        const shiftedDate = new Date(baseDate.getFullYear(), baseDate.getMonth(), baseDate.getDate() + deltaDays);
        return weekCalendarPage.toIsoDate(shiftedDate);
    }
    function slotBackgroundType(entries) {
        if (!entries || entries.length === 0)
            return 0;
        const firstEntry = entries[0];
        const entryType = firstEntry && firstEntry.type !== undefined ? String(firstEntry.type) : "";
        return entryType === "event" ? 1 : 0;
    }
    function slotAccent(entries) {
        if (!entries || entries.length === 0)
            return LV.Theme.primary;
        const firstEntry = entries[0];
        const sourceKind = firstEntry && firstEntry.sourceKind !== undefined ? String(firstEntry.sourceKind) : "";
        if (sourceKind === "note")
            return LV.Theme.accent;
        return LV.Theme.primary;
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
    function noteIdForEntries(entries) {
        if (!entries || entries.length !== 1)
            return "";
        const firstEntry = entries[0];
        const sourceKind = firstEntry && firstEntry.sourceKind !== undefined ? String(firstEntry.sourceKind).trim() : "";
        if (sourceKind !== "note")
            return "";
        return firstEntry && firstEntry.sourceId !== undefined && firstEntry.sourceId !== null ? String(firstEntry.sourceId).trim() : "";
    }
    function requestOpenNoteForEntries(entries) {
        const noteId = weekCalendarPage.noteIdForEntries(entries);
        if (noteId.length === 0)
            return;
        weekCalendarPage.requestViewHook("open-note");
        weekCalendarPage.noteOpenRequested(noteId);
    }
    function syncDisplayedWeekForDate(dateIso, reason) {
        const activeWeekStartIso = weekCalendarPage.normalizedWeekStartIso(dateIso);
        const currentVmIso = weekCalendarPage.calendarController && weekCalendarPage.calendarController.displayedWeekStartIso !== undefined ? String(weekCalendarPage.calendarController.displayedWeekStartIso).trim() : "";
        if (weekCalendarPage.calendarController && weekCalendarPage.calendarController.setDisplayedWeekStartIso && activeWeekStartIso.length > 0 && currentVmIso !== activeWeekStartIso) {
            weekCalendarPage.calendarController.setDisplayedWeekStartIso(activeWeekStartIso);
        }
        weekCalendarPage.notifyViewHook(reason);
    }
    function syncDisplayedWeekFromViewport(reason) {
        const focusedIndex = weekCalendarPage.currentFocusedDateIndex();
        if (focusedIndex < 0 || focusedIndex >= weekCalendarPage.timelineDayCount)
            return;
        weekCalendarPage.syncDisplayedWeekForDate(String(weekCalendarPage.timelineDayModels[focusedIndex].dateIso), reason);
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
        if (!weekCalendarPage.calendarController || !weekCalendarPage.calendarController.trimTimelineWindow)
            return;
        const trimResult = weekCalendarPage.calendarController.trimTimelineWindow(weekCalendarPage.currentLeadingDateIndex(), weekCalendarPage.maxDateWindowSize, weekCalendarPage.lazyChunkSize);
        const removedHead = trimResult && trimResult.removedHead !== undefined ? Number(trimResult.removedHead) : 0;
        if (isFinite(removedHead) && removedHead > 0) {
            weekCalendarPage.setViewportContentX(Math.max(0, dateColumnsFlickable.contentX - (removedHead * timelineScaffold.dayColumnSpan)));
        }
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    color: LV.Theme.accentTransparent
    radius: LV.Theme.radiusMd

    Component.onCompleted: {
        Qt.callLater(function () {
            weekCalendarPage.initializeDateWindow(weekCalendarPage.resolveInitialDateIso(), true, "initialize-date-window");
            weekCalendarPage.requestViewHook("page-open");
        });
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
                readonly property real dayColumnWidth: Math.max(1, (dateViewportWidth - (weekCalendarPage.dayColumnSpacing * (weekCalendarPage.visibleDayColumnCount - 1))) / weekCalendarPage.visibleDayColumnCount)
                readonly property real dayColumnSpan: dayColumnWidth + weekCalendarPage.dayColumnSpacing
                readonly property real headerHeight: Math.max(44, LV.Theme.gap24 * 2)
                readonly property int hourSlotCount: Math.max(1, weekCalendarPage.hourSlots.length)
                readonly property real bodyHeight: Math.max(1, height - headerHeight - weekCalendarPage.dayColumnSpacing)
                readonly property real hourRowHeight: Math.max(1, (bodyHeight - (weekCalendarPage.dayColumnSpacing * (hourSlotCount - 1))) / hourSlotCount)

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
                    interactive: weekCalendarPage.timelineDayCount > weekCalendarPage.visibleDayColumnCount

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
                        width: weekCalendarPage.timelineDayCount > 0 ? (weekCalendarPage.timelineDayCount * timelineScaffold.dayColumnWidth) + ((weekCalendarPage.timelineDayCount - 1) * weekCalendarPage.dayColumnSpacing) : 0

                        Repeater {
                            model: weekCalendarPage.timelineDayModels

                            Rectangle {
                                id: dayHeaderCell

                                required property int index
                                readonly property var dayModel: dayHeaderCell.modelData
                                required property var modelData

                                border.color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isToday ? LV.Theme.accent : LV.Theme.accentTransparent
                                border.width: Math.max(1, LV.Theme.strokeThin)
                                color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isToday ? LV.Theme.accentTransparent : "transparent"
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
                                        color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isToday ? LV.Theme.accent : (dayHeaderCell.dayModel && dayHeaderCell.dayModel.isInCurrentWeek ? LV.Theme.titleHeaderColor : LV.Theme.descriptionColor)
                                        font.weight: Font.Medium
                                        text: dayHeaderCell.dayModel && dayHeaderCell.dayModel.weekdayLabel !== undefined ? String(dayHeaderCell.dayModel.weekdayLabel) : ""
                                    }
                                    LV.Label {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        color: dayHeaderCell.dayModel && dayHeaderCell.dayModel.isInCurrentWeek ? LV.Theme.descriptionColor : Qt.darker(LV.Theme.descriptionColor, 1.2)
                                        text: dayHeaderCell.dayModel && dayHeaderCell.dayModel.dateLabel !== undefined ? String(dayHeaderCell.dayModel.dateLabel) : ""
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

                                    color: LV.Theme.accentTransparent
                                    height: timelineScaffold.hourRowHeight
                                    radius: LV.Theme.radiusSm
                                    width: parent.width

                                    Repeater {
                                        model: weekCalendarPage.timelineDayModels

                                        Rectangle {
                                            id: dayHourCell

                                            required property int index
                                            readonly property var dayModel: dayHourCell.modelData
                                            required property var modelData
                                            readonly property var slotEntries: weekCalendarPage.entriesForHour(dayHourCell.dayModel, hourRow.hour)

                                            border.color: dayHourCell.dayModel && dayHourCell.dayModel.isToday ? LV.Theme.accentTransparent : LV.Theme.accentTransparent
                                            border.width: Math.max(1, LV.Theme.strokeThin)
                                            color: LV.Theme.accentTransparent
                                            height: timelineScaffold.hourRowHeight
                                            radius: LV.Theme.radiusSm
                                            width: timelineScaffold.dayColumnWidth
                                            x: dayHourCell.index * timelineScaffold.dayColumnSpan

                                            CalendarEventCell {
                                                anchors.fill: parent
                                                backgroundType: weekCalendarPage.slotBackgroundType(dayHourCell.slotEntries)
                                                coloredBackgroundColor: weekCalendarPage.slotAccent(dayHourCell.slotEntries)
                                                cornerRadius: LV.Theme.radiusSm
                                                defaultBackgroundColor: LV.Theme.panelBackground11
                                                horizontalInset: LV.Theme.gap2
                                                interactive: weekCalendarPage.noteIdForEntries(dayHourCell.slotEntries).length > 0
                                                label: weekCalendarPage.slotSummary(dayHourCell.slotEntries)
                                                textColor: LV.Theme.titleHeaderColor
                                                visible: dayHourCell.slotEntries.length > 0

                                                onActivated: weekCalendarPage.requestOpenNoteForEntries(dayHourCell.slotEntries)
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
