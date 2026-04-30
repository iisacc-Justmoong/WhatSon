pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: monthCalendarPage

    readonly property int bodyLabelPixelSize: LV.Theme.textBody
    readonly property var calendarController: monthCalendarController
    readonly property string figmaNodeId: "228:9666"
    readonly property int headerHorizontalPadding: LV.Theme.gap8
    readonly property int maxVisibleEntriesPerCell: 8
    property var monthCalendarController: null
    readonly property int monthHeaderHeight: LV.Theme.headerMinHeight - LV.Theme.gap2
    readonly property int monthPagerCenterIndex: 1
    property bool monthPagerResetting: false
    readonly property bool monthSwipeEnabled: LV.Theme.mobileTarget
    readonly property string monthTitleText: monthCalendarPage.calendarController ? String(monthCalendarPage.calendarController.monthLabel) + ", " + String(monthCalendarPage.calendarController.displayedYear) : "Month"
    readonly property var pagerMonthModels: monthCalendarPage.calendarController && monthCalendarPage.calendarController.pagerMonthModels !== undefined ? monthCalendarPage.calendarController.pagerMonthModels : []
    readonly property int weekdayCellHorizontalPadding: LV.Theme.gap12
    readonly property int weekdayHeaderHeight: LV.Theme.controlHeightMd + LV.Theme.gap3

    signal noteOpenRequested(string noteId)
    signal viewHookRequested(string reason)

    function monthProjectionForIndex(index) {
        const normalizedIndex = Math.floor(Number(index));
        if (!isFinite(normalizedIndex) || normalizedIndex < 0 || normalizedIndex >= monthCalendarPage.pagerMonthModels.length)
            return null;
        return monthCalendarPage.pagerMonthModels[normalizedIndex];
    }
    function commitMonthSwipeDelta(delta) {
        if (!monthCalendarPage.calendarController || !monthCalendarPage.calendarController.shiftMonth || delta === 0) {
            monthCalendarPage.recenterMonthPager();
            return;
        }

        const previousYear = Number(monthCalendarPage.calendarController.displayedYear);
        const previousMonth = Number(monthCalendarPage.calendarController.displayedMonth);
        monthCalendarPage.calendarController.shiftMonth(delta);
        const nextYear = Number(monthCalendarPage.calendarController.displayedYear);
        const nextMonth = Number(monthCalendarPage.calendarController.displayedMonth);
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
        if (monthCalendarPage.calendarController && monthCalendarPage.calendarController.focusToday) {
            monthCalendarPage.calendarController.focusToday();
        } else {
            const now = new Date();
            if (monthCalendarPage.calendarController && monthCalendarPage.calendarController.setDisplayedYear)
                monthCalendarPage.calendarController.setDisplayedYear(now.getFullYear());
            if (monthCalendarPage.calendarController && monthCalendarPage.calendarController.setDisplayedMonth)
                monthCalendarPage.calendarController.setDisplayedMonth(now.getMonth() + 1);
            if (monthCalendarPage.calendarController && monthCalendarPage.calendarController.setSelectedDateIso)
                monthCalendarPage.calendarController.setSelectedDateIso(Qt.formatDateTime(now, "yyyy-MM-dd"));
        }
        monthCalendarPage.requestViewHook("current-month");
    }
    function recenterMonthPager() {
        if (!monthCalendarMonthsView || monthCalendarPage.pagerMonthModels.length <= monthCalendarPage.monthPagerCenterIndex) {
            monthCalendarPage.monthPagerResetting = false;
            return;
        }
        if (monthCalendarMonthsView.width <= 0 || monthCalendarMonthsView.height <= 0) {
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
        if (monthCalendarPage.calendarController && monthCalendarPage.calendarController.requestMonthView)
            monthCalendarPage.calendarController.requestMonthView(hookReason);
        monthCalendarPage.viewHookRequested(hookReason);
    }
    function scheduleMonthPagerReset() {
        Qt.callLater(function () {
            monthCalendarPage.recenterMonthPager();
        });
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    color: LV.Theme.accentTransparent
    radius: LV.Theme.radiusMd

    Component.onCompleted: {
        monthCalendarPage.scheduleMonthPagerReset();
        monthCalendarPage.requestViewHook("page-open");
    }
    onVisibleChanged: {
        if (visible)
            monthCalendarPage.scheduleMonthPagerReset();
    }

    Connections {
        function onMonthViewChanged() {
            monthCalendarPage.scheduleMonthPagerReset();
        }

        ignoreUnknownSignals: true
        target: monthCalendarPage.calendarController
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
                    font.pixelSize: LV.Theme.textTitle2
                    font.weight: Font.Bold
                    text: monthCalendarPage.monthTitleText
                }
                Item {
                    Layout.fillWidth: true
                }
                CalendarTodayControl {
                    Layout.alignment: Qt.AlignVCenter

                    onNextRequested: {
                        if (monthCalendarPage.calendarController && monthCalendarPage.calendarController.shiftMonth)
                            monthCalendarPage.calendarController.shiftMonth(1);
                        monthCalendarPage.requestViewHook("next-month");
                    }
                    onPreviousRequested: {
                        if (monthCalendarPage.calendarController && monthCalendarPage.calendarController.shiftMonth)
                            monthCalendarPage.calendarController.shiftMonth(-1);
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
                interactive: monthCalendarPage.monthSwipeEnabled && monthCalendarPage.pagerMonthModels.length > 1
                model: monthCalendarPage.pagerMonthModels.length
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
                        calendarController: monthCalendarPage.calendarController
                        maxVisibleEntries: monthCalendarPage.maxVisibleEntriesPerCell
                        monthProjection: monthPage.monthProjection
                        selectedDateIso: monthCalendarPage.calendarController && monthCalendarPage.calendarController.selectedDateIso !== undefined ? String(monthCalendarPage.calendarController.selectedDateIso) : ""
                        weekdayCellHorizontalPadding: monthCalendarPage.weekdayCellHorizontalPadding
                        weekdayHeaderHeight: monthCalendarPage.weekdayHeaderHeight

                        onDateSelected: function (dateIso) {
                            if (monthCalendarPage.calendarController && monthCalendarPage.calendarController.setSelectedDateIso)
                                monthCalendarPage.calendarController.setSelectedDateIso(dateIso);
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
