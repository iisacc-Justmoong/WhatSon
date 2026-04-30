pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: yearCalendarPage

    readonly property var calendarVm: yearCalendarViewModel
    readonly property var monthModels: calendarVm && calendarVm.monthModels ? calendarVm.monthModels : []
    readonly property bool mobileYearListMode: LV.Theme.mobileTarget
    readonly property int desktopYearGridColumnCount: 4
    readonly property int desktopYearGridRowCount: 3
    readonly property int mobileYearGridColumnCount: 1
    readonly property int yearGridColumnCount: yearCalendarPage.mobileYearListMode ? yearCalendarPage.mobileYearGridColumnCount : yearCalendarPage.desktopYearGridColumnCount
    readonly property real desktopResponsiveScale: yearCalendarPage.mobileYearListMode ? 1.0 : Math.min(1.2, Math.max(0.72, Math.min(yearCalendarPage.width / 1600, yearCalendarPage.height / 1020)))
    readonly property int yearGridSpacing: Math.max(LV.Theme.gap4, Math.round(LV.Theme.gap24 * yearCalendarPage.desktopResponsiveScale))
    readonly property int monthCardPadding: Math.max(LV.Theme.gap4, Math.round(LV.Theme.gap14 * yearCalendarPage.desktopResponsiveScale))
    readonly property int monthSectionSpacing: Math.max(LV.Theme.gap2, Math.round(LV.Theme.gap8 * yearCalendarPage.desktopResponsiveScale))
    readonly property int monthTitlePixelSize: Math.max(LV.Theme.gap18, Math.round((LV.Theme.controlHeightMd + LV.Theme.gap8) * yearCalendarPage.desktopResponsiveScale / 2))
    readonly property int monthWeekdayPixelSize: LV.Theme.textBody
    readonly property int monthDayPixelSize: LV.Theme.textBody
    readonly property int monthCellWidthFloor: Math.max(LV.Theme.gap10, Math.round(LV.Theme.gap14 * yearCalendarPage.desktopResponsiveScale))
    readonly property color monthTitleColor: LV.Theme.accent
    readonly property color weekdayTextColor: LV.Theme.descriptionColor
    readonly property color activeDayColor: LV.Theme.titleHeaderColor
    readonly property color adjacentDayColor: Qt.darker(yearCalendarPage.activeDayColor, 1.2)
    readonly property color todayBadgeColor: LV.Theme.disabledColor
    readonly property int mobileMonthCardMinHeight: LV.Theme.scaffoldBlobPrimaryRadius
    readonly property var weekdayLabels: calendarVm && calendarVm.weekdayLabels ? calendarVm.weekdayLabels : []
    property var yearCalendarViewModel: null

    signal viewHookRequested(string reason)
    signal monthCalendarOpenRequested(int year, int month, string selectedDateIso)

    function firstCurrentMonthDayModel(monthModel) {
        const dayModels = monthModel && monthModel.days ? monthModel.days : [];
        for (let index = 0; index < dayModels.length; ++index) {
            const dayModel = dayModels[index];
            if (dayModel && dayModel.inCurrentMonth === true && dayModel.dateIso !== undefined)
                return dayModel;
        }
        return null;
    }
    function requestOpenMonthFromDayModel(dayModel, reason) {
        if (!dayModel)
            return;
        const targetYear = Number(dayModel.year);
        const targetMonth = Number(dayModel.month);
        const selectedDateIso = dayModel.dateIso === undefined || dayModel.dateIso === null ? "" : String(dayModel.dateIso).trim();
        if (!isFinite(targetYear) || !isFinite(targetMonth) || selectedDateIso.length === 0)
            return;
        yearCalendarPage.requestViewHook(reason !== undefined ? reason : "select-month-date");
        yearCalendarPage.monthCalendarOpenRequested(Math.floor(targetYear), Math.floor(targetMonth), selectedDateIso);
    }
    function requestOpenMonthFromMonthModel(monthModel, reason) {
        const firstCurrentMonthDay = yearCalendarPage.firstCurrentMonthDayModel(monthModel);
        if (!firstCurrentMonthDay)
            return;
        yearCalendarPage.requestOpenMonthFromDayModel(firstCurrentMonthDay, reason !== undefined ? reason : "select-month");
    }
    function requestViewHook(reason) {
        var hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarVm && calendarVm.requestYearView)
            calendarVm.requestYearView(hookReason);
        viewHookRequested(hookReason);
    }
    function weekdaySymbol(label) {
        if (label === undefined)
            return "";
        var normalizedLabel = String(label).trim();
        if (normalizedLabel.length === 0)
            return "";
        return normalizedLabel.charAt(0).toUpperCase();
    }
    function dayTextColor(dayModel, isToday) {
        if (isToday)
            return LV.Theme.titleHeaderColor;
        if (dayModel && dayModel.inCurrentMonth === true)
            return yearCalendarPage.activeDayColor;
        return yearCalendarPage.adjacentDayColor;
    }
    function jumpToCurrentYear() {
        if (yearCalendarPage.calendarVm && yearCalendarPage.calendarVm.focusToday) {
            yearCalendarPage.calendarVm.focusToday();
        } else if (yearCalendarPage.calendarVm && yearCalendarPage.calendarVm.setDisplayedYear) {
            yearCalendarPage.calendarVm.setDisplayedYear((new Date()).getFullYear());
        }
        yearCalendarPage.requestViewHook("current-year");
    }

    color: LV.Theme.accentTransparent
    radius: LV.Theme.radiusMd

    Component.onCompleted: requestViewHook("page-open")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: yearCalendarPage.mobileYearListMode ? LV.Theme.gap12 : Math.max(LV.Theme.gap10, Math.round(LV.Theme.gap20 * yearCalendarPage.desktopResponsiveScale))
        spacing: yearCalendarPage.mobileYearListMode ? LV.Theme.gap8 : yearCalendarPage.yearGridSpacing

        Rectangle {
            id: calendarToolbar

            Layout.fillWidth: true
            Layout.preferredHeight: headerColumn.implicitHeight + LV.Theme.gap8
            color: LV.Theme.panelBackground10
            radius: LV.Theme.radiusSm
            visible: true

            Column {
                id: headerColumn

                anchors.fill: parent
                anchors.margins: LV.Theme.gap4
                spacing: LV.Theme.gap4

                LV.HStack {
                    spacing: LV.Theme.gap4

                    CalendarTodayControl {
                        id: yearCalendarControl

                        Layout.alignment: Qt.AlignVCenter

                        onPreviousRequested: {
                            if (yearCalendarPage.calendarVm && yearCalendarPage.calendarVm.shiftYear)
                                yearCalendarPage.calendarVm.shiftYear(-1);
                            yearCalendarPage.requestViewHook("previous-year");
                        }
                        onTodayRequested: yearCalendarPage.jumpToCurrentYear()
                        onNextRequested: {
                            if (yearCalendarPage.calendarVm && yearCalendarPage.calendarVm.shiftYear)
                                yearCalendarPage.calendarVm.shiftYear(1);
                            yearCalendarPage.requestViewHook("next-year");
                        }
                    }
                    LV.Label {
                        text: yearCalendarPage.calendarVm ? String(yearCalendarPage.calendarVm.displayedYear) + " · " + String(yearCalendarPage.calendarVm.calendarSystemName) : "Year calendar"
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }
        Flickable {
            id: monthsFlickable

            Layout.fillHeight: true
            Layout.fillWidth: true
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            contentHeight: yearCalendarPage.mobileYearListMode ? monthGrid.implicitHeight : monthsFlickable.height
            contentWidth: monthsFlickable.width
            flickableDirection: Flickable.VerticalFlick
            interactive: yearCalendarPage.mobileYearListMode

            Grid {
                id: monthGrid

                property int desktopCardWidth: Math.max(1, Math.floor((width - (yearCalendarPage.yearGridSpacing * (yearCalendarPage.desktopYearGridColumnCount - 1))) / yearCalendarPage.desktopYearGridColumnCount))
                property int desktopCardHeight: Math.max(1, Math.floor((monthsFlickable.height - (yearCalendarPage.yearGridSpacing * (yearCalendarPage.desktopYearGridRowCount - 1))) / yearCalendarPage.desktopYearGridRowCount))

                columns: yearCalendarPage.yearGridColumnCount
                spacing: yearCalendarPage.yearGridSpacing
                width: monthsFlickable.width

                Repeater {
                    model: yearCalendarPage.monthModels

                    Rectangle {
                        id: monthCard

                        required property var modelData
                        readonly property var monthModel: monthCard.modelData
                        readonly property int dayCellWidth: Math.max(yearCalendarPage.monthCellWidthFloor, Math.floor((monthCard.width - (yearCalendarPage.monthCardPadding * 2) - (yearCalendarPage.monthSectionSpacing * 6)) / 7))
                        readonly property int dayCellHeight: Math.max(10, Math.floor((monthCard.height - (yearCalendarPage.monthCardPadding * 2) - monthTitleLabel.implicitHeight - weekdayGrid.implicitHeight - (yearCalendarPage.monthSectionSpacing * 2) - (dayGrid.spacing * 5)) / 6))

                        color: LV.Theme.accentTransparent
                        height: yearCalendarPage.mobileYearListMode ? Math.max(yearCalendarPage.mobileMonthCardMinHeight, monthBodyColumn.implicitHeight + (yearCalendarPage.monthCardPadding * 2)) : monthGrid.desktopCardHeight
                        radius: LV.Theme.radiusSm
                        width: yearCalendarPage.mobileYearListMode ? monthGrid.width : monthGrid.desktopCardWidth

                        Column {
                            id: monthBodyColumn

                            anchors.fill: parent
                            anchors.margins: yearCalendarPage.monthCardPadding
                            spacing: yearCalendarPage.monthSectionSpacing

                            LV.Label {
                                id: monthTitleLabel

                                color: yearCalendarPage.monthTitleColor
                                font.pixelSize: yearCalendarPage.monthTitlePixelSize
                                font.weight: Font.Medium
                                text: monthCard.monthModel && monthCard.monthModel.monthLabel ? String(monthCard.monthModel.monthLabel) : "Month"

                                TapHandler {
                                    gesturePolicy: TapHandler.DragThreshold

                                    onTapped: yearCalendarPage.requestOpenMonthFromMonthModel(monthCard.monthModel, "select-month")
                                }
                            }
                            Grid {
                                id: weekdayGrid

                                columns: 7
                                spacing: yearCalendarPage.monthSectionSpacing

                                Repeater {
                                    model: yearCalendarPage.weekdayLabels

                                    Item {
                                        id: weekdayCell

                                        required property var modelData
                                        height: yearCalendarPage.monthWeekdayPixelSize + yearCalendarPage.monthSectionSpacing
                                        width: monthCard.dayCellWidth

                                        LV.Label {
                                            anchors.centerIn: parent
                                            color: yearCalendarPage.weekdayTextColor
                                            font.pixelSize: yearCalendarPage.monthWeekdayPixelSize
                                            font.weight: Font.Medium
                                            text: yearCalendarPage.weekdaySymbol(weekdayCell.modelData)
                                        }
                                    }
                                }
                            }
                            Grid {
                                id: dayGrid

                                columns: 7
                                spacing: yearCalendarPage.monthSectionSpacing

                                Repeater {
                                    model: monthCard.monthModel && monthCard.monthModel.days ? monthCard.monthModel.days : []

                                    Item {
                                        id: dayCell

                                        required property var modelData
                                        readonly property var dayModel: dayCell.modelData
                                        readonly property bool hasValidDate: dayCell.dayModel && dayCell.dayModel.dateIso !== undefined && String(dayCell.dayModel.dateIso).trim().length > 0
                                        readonly property bool isToday: dayModel && dayModel.isToday === true
                                        height: monthCard.dayCellHeight
                                        width: monthCard.dayCellWidth

                                        Rectangle {
                                            anchors.centerIn: parent
                                            color: yearCalendarPage.todayBadgeColor
                                            height: Math.min(parent.width, parent.height)
                                            radius: Math.min(parent.width, parent.height) / 2
                                            visible: dayCell.isToday
                                            width: Math.min(parent.width, parent.height)
                                        }
                                        LV.Label {
                                            anchors.centerIn: parent
                                            color: yearCalendarPage.dayTextColor(dayCell.dayModel, dayCell.isToday)
                                            font.pixelSize: yearCalendarPage.monthDayPixelSize
                                            font.weight: Font.Medium
                                            text: dayCell.dayModel && dayCell.dayModel.day !== undefined ? String(dayCell.dayModel.day) : ""
                                        }
                                        TapHandler {
                                            enabled: dayCell.hasValidDate
                                            gesturePolicy: TapHandler.DragThreshold

                                            onTapped: yearCalendarPage.requestOpenMonthFromDayModel(dayCell.dayModel, "select-month-date")
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
