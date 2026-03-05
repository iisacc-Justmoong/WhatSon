import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: yearCalendarPage

    readonly property var calendarSystemOptions: calendarVm && calendarVm.calendarSystemOptions ? calendarVm.calendarSystemOptions : []
    readonly property var calendarVm: yearCalendarViewModel
    readonly property var monthModels: calendarVm && calendarVm.monthModels ? calendarVm.monthModels : []
    readonly property var weekdayLabels: calendarVm && calendarVm.weekdayLabels ? calendarVm.weekdayLabels : []
    property var yearCalendarViewModel: null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        var hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarVm && calendarVm.requestYearView)
            calendarVm.requestYearView(hookReason);
        viewHookRequested(hookReason);
    }

    color: LV.Theme.panelBackground07
    radius: LV.Theme.radiusMd

    Component.onCompleted: requestViewHook("page-open")

    LV.VStack {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap12
        spacing: LV.Theme.gap12

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: headerColumn.implicitHeight + LV.Theme.gap8
            color: LV.Theme.panelBackground10
            radius: LV.Theme.radiusSm

            Column {
                id: headerColumn

                anchors.fill: parent
                anchors.margins: LV.Theme.gap4
                spacing: LV.Theme.gap4

                LV.HStack {
                    spacing: LV.Theme.gap4

                    LV.LabelButton {
                        text: "Prev"

                        onClicked: {
                            if (yearCalendarPage.calendarVm && yearCalendarPage.calendarVm.shiftYear)
                                yearCalendarPage.calendarVm.shiftYear(-1);
                            yearCalendarPage.requestViewHook("previous-year");
                        }
                    }
                    LV.Label {
                        text: yearCalendarPage.calendarVm ? String(yearCalendarPage.calendarVm.displayedYear) + " · " + String(yearCalendarPage.calendarVm.calendarSystemName) : "Year calendar"
                    }
                    LV.LabelButton {
                        text: "Next"

                        onClicked: {
                            if (yearCalendarPage.calendarVm && yearCalendarPage.calendarVm.shiftYear)
                                yearCalendarPage.calendarVm.shiftYear(1);
                            yearCalendarPage.requestViewHook("next-year");
                        }
                    }
                }
                LV.HStack {
                    spacing: LV.Theme.gap2

                    Repeater {
                        model: yearCalendarPage.calendarSystemOptions

                        LV.LabelButton {
                            readonly property var optionModel: modelData

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
            clip: true
            contentHeight: monthGrid.implicitHeight
            contentWidth: width

            Grid {
                id: monthGrid

                property int cellWidth: Math.max(220, Math.floor((width - (columns - 1) * spacing) / columns))

                columns: Math.max(1, Math.floor(width / 300))
                spacing: LV.Theme.gap8
                width: monthsFlickable.width

                Repeater {
                    model: yearCalendarPage.monthModels

                    Rectangle {
                        id: monthCard

                        readonly property var monthModel: modelData

                        color: LV.Theme.panelBackground09
                        height: monthColumn.implicitHeight + LV.Theme.gap8
                        radius: LV.Theme.radiusSm
                        width: monthGrid.cellWidth

                        Column {
                            id: monthColumn

                            anchors.fill: parent
                            anchors.margins: LV.Theme.gap4
                            spacing: LV.Theme.gap2

                            LV.Label {
                                text: monthCard.monthModel && monthCard.monthModel.monthLabel ? String(monthCard.monthModel.monthLabel) : "Month"
                            }
                            Grid {
                                columns: 7
                                spacing: LV.Theme.gap1

                                Repeater {
                                    model: yearCalendarPage.weekdayLabels

                                    Rectangle {
                                        color: LV.Theme.accentTransparent
                                        height: LV.Theme.gap12
                                        width: Math.max(20, Math.floor((monthCard.width - LV.Theme.gap10) / 7))

                                        LV.Label {
                                            anchors.centerIn: parent
                                            text: String(modelData)
                                        }
                                    }
                                }
                            }
                            Grid {
                                columns: 7
                                spacing: LV.Theme.gap1

                                Repeater {
                                    model: monthCard.monthModel && monthCard.monthModel.days ? monthCard.monthModel.days : []

                                    Rectangle {
                                        id: dayCell

                                        readonly property var dayModel: modelData
                                        readonly property bool isCurrentMonth: dayModel && dayModel.inCurrentMonth === true
                                        readonly property bool isToday: dayModel && dayModel.isToday === true

                                        color: isToday ? LV.Theme.primary : isCurrentMonth ? LV.Theme.panelBackground10 : LV.Theme.panelBackground06
                                        height: LV.Theme.gap12
                                        radius: LV.Theme.radiusSm
                                        width: Math.max(20, Math.floor((monthCard.width - LV.Theme.gap10) / 7))

                                        LV.Label {
                                            anchors.centerIn: parent
                                            color: dayCell.isToday ? LV.Theme.panelBackground01 : dayCell.isCurrentMonth ? LV.Theme.titleHeaderColor : LV.Theme.descriptionColor
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
