pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: yearCalendarPage

    readonly property var calendarSystemOptions: calendarVm && calendarVm.calendarSystemOptions ? calendarVm.calendarSystemOptions : []
    readonly property var calendarVm: yearCalendarViewModel
    readonly property var monthModels: calendarVm && calendarVm.monthModels ? calendarVm.monthModels : []
    readonly property bool mobileYearListMode: LV.Theme.mobileTarget
    readonly property int desktopYearGridColumnCount: 4
    readonly property int desktopYearGridRowCount: 3
    readonly property int mobileYearGridColumnCount: 1
    readonly property int yearGridColumnCount: yearCalendarPage.mobileYearListMode ? yearCalendarPage.mobileYearGridColumnCount : yearCalendarPage.desktopYearGridColumnCount
    readonly property real desktopResponsiveScale: yearCalendarPage.mobileYearListMode ? 1.0 : Math.min(1.2, Math.max(0.72, Math.min(yearCalendarPage.width / 1600, yearCalendarPage.height / 1020)))
    readonly property int yearGridSpacing: Math.max(4, Math.round(24 * yearCalendarPage.desktopResponsiveScale))
    readonly property int monthCardPadding: Math.max(4, Math.round(14 * yearCalendarPage.desktopResponsiveScale))
    readonly property int monthSectionSpacing: Math.max(2, Math.round(8 * yearCalendarPage.desktopResponsiveScale))
    readonly property int monthTitlePixelSize: Math.max(18, Math.round(44 * yearCalendarPage.desktopResponsiveScale / 2))
    readonly property int monthWeekdayPixelSize: Math.max(10, Math.round(13 * yearCalendarPage.desktopResponsiveScale))
    readonly property int monthDayPixelSize: Math.max(10, Math.round(16 * yearCalendarPage.desktopResponsiveScale))
    readonly property int monthCellWidthFloor: Math.max(10, Math.round(14 * yearCalendarPage.desktopResponsiveScale))
    readonly property color monthTitleColor: "#FF4B4B"
    readonly property color weekdayTextColor: LV.Theme.descriptionColor
    readonly property color activeDayColor: LV.Theme.titleHeaderColor
    readonly property color adjacentDayColor: Qt.rgba(1.0, 1.0, 1.0, 0.28)
    readonly property color todayBadgeColor: Qt.rgba(1.0, 1.0, 1.0, 0.22)
    readonly property int mobileMonthCardMinHeight: 260
    readonly property var weekdayLabels: calendarVm && calendarVm.weekdayLabels ? calendarVm.weekdayLabels : []
    property var yearCalendarViewModel: null

    signal viewHookRequested(string reason)

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

    color: "transparent"
    radius: LV.Theme.radiusMd

    Component.onCompleted: requestViewHook("page-open")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: yearCalendarPage.mobileYearListMode ? LV.Theme.gap12 : Math.max(10, Math.round(20 * yearCalendarPage.desktopResponsiveScale))
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
                LV.HStack {
                    spacing: LV.Theme.gap2
                    visible: yearCalendarPage.mobileYearListMode

                    Repeater {
                        model: yearCalendarPage.calendarSystemOptions

                        LV.LabelButton {
                            id: calendarSystemButton

                            required property var modelData
                            readonly property var optionModel: calendarSystemButton.modelData

                            text: optionModel && optionModel.label ? String(optionModel.label) : "System"
                            tone: yearCalendarPage.calendarVm && optionModel && Number(optionModel.value) === Number(yearCalendarPage.calendarVm.calendarSystem) ? LV.AbstractButton.Primary : LV.AbstractButton.Borderless

                            onClicked: {
                                if (yearCalendarPage.calendarVm && yearCalendarPage.calendarVm.setCalendarSystemByValue)
                                    yearCalendarPage.calendarVm.setCalendarSystemByValue(Number(optionModel.value));
                                yearCalendarPage.requestViewHook("calendar-system");
                            }
                        }
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

                property int desktopCardWidth: Math.max(
                                                   1,
                                                   Math.floor(
                                                       (width - (yearCalendarPage.yearGridSpacing * (yearCalendarPage.desktopYearGridColumnCount - 1)))
                                                       / yearCalendarPage.desktopYearGridColumnCount))
                property int desktopCardHeight: Math.max(
                                                    1,
                                                    Math.floor(
                                                        (monthsFlickable.height - (yearCalendarPage.yearGridSpacing * (yearCalendarPage.desktopYearGridRowCount - 1)))
                                                        / yearCalendarPage.desktopYearGridRowCount))

                columns: yearCalendarPage.yearGridColumnCount
                spacing: yearCalendarPage.yearGridSpacing
                width: monthsFlickable.width

                Repeater {
                    model: yearCalendarPage.monthModels

                    Rectangle {
                        id: monthCard

                        required property var modelData
                        readonly property var monthModel: monthCard.modelData
                        readonly property int dayCellWidth: Math.max(
                                                                yearCalendarPage.monthCellWidthFloor,
                                                                Math.floor(
                                                                    (monthCard.width
                                                                     - (yearCalendarPage.monthCardPadding * 2)
                                                                     - (yearCalendarPage.monthSectionSpacing * 6))
                                                                    / 7))
                        readonly property int dayCellHeight: Math.max(
                                                                 10,
                                                                 Math.floor(
                                                                     (monthCard.height
                                                                      - (yearCalendarPage.monthCardPadding * 2)
                                                                      - monthTitleLabel.implicitHeight
                                                                      - weekdayGrid.implicitHeight
                                                                      - (yearCalendarPage.monthSectionSpacing * 2)
                                                                      - (dayGrid.spacing * 5))
                                                                     / 6))

                        color: "transparent"
                        height: yearCalendarPage.mobileYearListMode
                                ? Math.max(yearCalendarPage.mobileMonthCardMinHeight, monthBodyColumn.implicitHeight + (yearCalendarPage.monthCardPadding * 2))
                                : monthGrid.desktopCardHeight
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
                                            font.weight: dayCell.dayModel && dayCell.dayModel.inCurrentMonth === true ? Font.DemiBold : Font.Normal
                                            text: dayCell.dayModel && dayCell.dayModel.day !== undefined ? String(dayCell.dayModel.day) : ""
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
