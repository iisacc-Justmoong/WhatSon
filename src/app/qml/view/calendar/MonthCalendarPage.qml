pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: monthCalendarPage

    readonly property int bodyLabelPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property var calendarVm: monthCalendarViewModel
    readonly property string figmaNodeId: "228:9666"
    readonly property int headerHorizontalPadding: LV.Theme.gap8
    readonly property int maxVisibleEntriesPerCell: 8
    property var monthCalendarViewModel: null
    readonly property int monthHeaderHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(54)))
    property var monthPageModels: []
    readonly property int monthPagerCenterIndex: 1
    property bool monthPagerResetting: false
    readonly property bool monthSwipeEnabled: LV.Theme.mobileTarget
    readonly property string monthTitleText: monthCalendarPage.calendarVm ? String(monthCalendarPage.calendarVm.monthLabel) + ", " + String(monthCalendarPage.calendarVm.displayedYear) : "Month"
    readonly property int weekdayCellHorizontalPadding: LV.Theme.gap12
    readonly property int weekdayHeaderHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(39)))

    signal noteOpenRequested(string noteId)
    signal viewHookRequested(string reason)

    function buildMonthPageModels() {
        if (!monthCalendarPage.calendarVm)
            return [];

        const displayedYear = Number(monthCalendarPage.calendarVm.displayedYear);
        const displayedMonth = Number(monthCalendarPage.calendarVm.displayedMonth);
        return [monthCalendarPage.buildMonthProjection(displayedYear, displayedMonth - 1), monthCalendarPage.buildMonthProjection(displayedYear, displayedMonth), monthCalendarPage.buildMonthProjection(displayedYear, displayedMonth + 1)];
    }
    function buildMonthProjection(year, month) {
        if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.monthProjectionFor !== undefined)
            return monthCalendarPage.calendarVm.monthProjectionFor(year, month);
        return {
            "year": year,
            "month": month,
            "monthLabel": monthCalendarPage.monthTitleText,
            "weekdayLabels": monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.weekdayLabels ? monthCalendarPage.calendarVm.weekdayLabels : [],
            "dayModels": monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.dayModels ? monthCalendarPage.calendarVm.dayModels : []
        };
    }
    function monthProjectionForIndex(index) {
        const normalizedIndex = Math.floor(Number(index));
        if (!isFinite(normalizedIndex) || normalizedIndex < 0 || normalizedIndex >= monthCalendarPage.monthPageModels.length)
            return null;
        return monthCalendarPage.monthPageModels[normalizedIndex];
    }
    function commitMonthSwipeDelta(delta) {
        if (!monthCalendarPage.calendarVm || !monthCalendarPage.calendarVm.shiftMonth || delta === 0) {
            monthCalendarPage.recenterMonthPager();
            return;
        }

        const previousYear = Number(monthCalendarPage.calendarVm.displayedYear);
        const previousMonth = Number(monthCalendarPage.calendarVm.displayedMonth);
        monthCalendarPage.calendarVm.shiftMonth(delta);
        const nextYear = Number(monthCalendarPage.calendarVm.displayedYear);
        const nextMonth = Number(monthCalendarPage.calendarVm.displayedMonth);
        if (previousYear === nextYear && previousMonth === nextMonth) {
            monthCalendarPage.recenterMonthPager();
            return;
        }
        monthCalendarPage.requestViewHook(delta > 0 ? "next-month" : "previous-month");
    }
    function handleMonthPagerMovementEnded() {
        if (!monthCalendarPage.monthSwipeEnabled || monthCalendarPage.monthPagerResetting)
            return;
        const delta = monthCalendarMonthsView.currentIndex - monthCalendarPage.monthPagerCenterIndex;
        if (delta === 0)
            return;
        monthCalendarPage.commitMonthSwipeDelta(delta);
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
    function rebuildMonthPager() {
        monthCalendarPage.monthPageModels = monthCalendarPage.buildMonthPageModels();
        monthCalendarPage.scheduleMonthPagerReset();
    }
    function recenterMonthPager() {
        if (!monthCalendarMonthsView || monthCalendarPage.monthPageModels.length <= monthCalendarPage.monthPagerCenterIndex) {
            monthCalendarPage.monthPagerResetting = false;
            return;
        }
        monthCalendarPage.monthPagerResetting = true;
        monthCalendarMonthsView.currentIndex = monthCalendarPage.monthPagerCenterIndex;
        monthCalendarMonthsView.positionViewAtIndex(monthCalendarPage.monthPagerCenterIndex, ListView.Beginning);
        Qt.callLater(function () {
            monthCalendarPage.monthPagerResetting = false;
        });
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.requestMonthView)
            monthCalendarPage.calendarVm.requestMonthView(hookReason);
        monthCalendarPage.viewHookRequested(hookReason);
    }
    function scheduleMonthPagerReset() {
        Qt.callLater(function () {
            monthCalendarPage.recenterMonthPager();
        });
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    color: "transparent"
    radius: LV.Theme.radiusMd

    Component.onCompleted: {
        monthCalendarPage.rebuildMonthPager();
        monthCalendarPage.requestViewHook("page-open");
    }

    Connections {
        function onMonthViewChanged() {
            monthCalendarPage.rebuildMonthPager();
        }

        ignoreUnknownSignals: true
        target: monthCalendarPage.calendarVm
    }
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

                    onNextRequested: {
                        if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.shiftMonth)
                            monthCalendarPage.calendarVm.shiftMonth(1);
                        monthCalendarPage.requestViewHook("next-month");
                    }
                    onPreviousRequested: {
                        if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.shiftMonth)
                            monthCalendarPage.calendarVm.shiftMonth(-1);
                        monthCalendarPage.requestViewHook("previous-month");
                    }
                    onTodayRequested: monthCalendarPage.jumpToCurrentMonth()
                }
            }
        }
        Item {
            id: monthBody

            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: monthHeaderBand.bottom

            ListView {
                id: monthCalendarMonthsView

                anchors.fill: parent
                boundsBehavior: Flickable.StopAtBounds
                cacheBuffer: Math.max(width, 1) * 2
                clip: true
                interactive: monthCalendarPage.monthSwipeEnabled && monthCalendarPage.monthPageModels.length > 1
                model: monthCalendarPage.monthPageModels.length
                orientation: ListView.Horizontal
                snapMode: ListView.SnapOneItem

                delegate: Item {
                    id: monthPage

                    required property int index
                    readonly property var monthProjection: monthCalendarPage.monthProjectionForIndex(monthPage.index)

                    height: monthCalendarMonthsView.height
                    width: monthCalendarMonthsView.width

                    MonthCalendarGridSurface {
                        anchors.fill: parent
                        bodyLabelPixelSize: monthCalendarPage.bodyLabelPixelSize
                        calendarVm: monthCalendarPage.calendarVm
                        maxVisibleEntries: monthCalendarPage.maxVisibleEntriesPerCell
                        monthProjection: monthPage.monthProjection
                        selectedDateIso: monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.selectedDateIso !== undefined ? String(monthCalendarPage.calendarVm.selectedDateIso) : ""
                        weekdayCellHorizontalPadding: monthCalendarPage.weekdayCellHorizontalPadding
                        weekdayHeaderHeight: monthCalendarPage.weekdayHeaderHeight

                        onDateSelected: function (dateIso) {
                            if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.setSelectedDateIso)
                                monthCalendarPage.calendarVm.setSelectedDateIso(dateIso);
                            monthCalendarPage.requestViewHook("select-date");
                        }
                        onNoteOpenRequested: function (noteId) {
                            monthCalendarPage.noteOpenRequested(noteId);
                        }
                    }
                }

                Component.onCompleted: monthCalendarPage.scheduleMonthPagerReset()
                onMovementEnded: {
                    monthCalendarPage.handleMonthPagerMovementEnded();
                }
            }
        }
    }
}
