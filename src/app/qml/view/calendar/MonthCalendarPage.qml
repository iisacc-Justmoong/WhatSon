pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: monthCalendarPage

    readonly property var calendarSystemOptions: calendarVm && calendarVm.calendarSystemOptions ? calendarVm.calendarSystemOptions : []
    readonly property var calendarVm: monthCalendarViewModel
    readonly property var dayModels: calendarVm && calendarVm.dayModels ? calendarVm.dayModels : []
    readonly property var weekdayLabels: calendarVm && calendarVm.weekdayLabels ? calendarVm.weekdayLabels : []
    property var monthCalendarViewModel: null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (calendarVm && calendarVm.requestMonthView)
            calendarVm.requestMonthView(hookReason);
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
                            if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.shiftMonth)
                                monthCalendarPage.calendarVm.shiftMonth(-1);
                            monthCalendarPage.requestViewHook("previous-month");
                        }
                    }
                    LV.Label {
                        text: monthCalendarPage.calendarVm
                              ? String(monthCalendarPage.calendarVm.monthLabel) + " "
                                + String(monthCalendarPage.calendarVm.displayedYear)
                                + " · " + String(monthCalendarPage.calendarVm.calendarSystemName)
                              : "Month calendar"
                    }
                    LV.LabelButton {
                        text: "Next"

                        onClicked: {
                            if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.shiftMonth)
                                monthCalendarPage.calendarVm.shiftMonth(1);
                            monthCalendarPage.requestViewHook("next-month");
                        }
                    }
                }
                LV.HStack {
                    spacing: LV.Theme.gap2

                    Repeater {
                        model: monthCalendarPage.calendarSystemOptions

                        LV.LabelButton {
                            id: calendarSystemButton

                            required property var modelData
                            readonly property var optionModel: calendarSystemButton.modelData

                            text: optionModel && optionModel.label ? String(optionModel.label) : "System"
                            tone: monthCalendarPage.calendarVm && optionModel && Number(optionModel.value) === Number(monthCalendarPage.calendarVm.calendarSystem)
                                  ? LV.AbstractButton.Primary
                                  : LV.AbstractButton.Borderless

                            onClicked: {
                                if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.setCalendarSystemByValue)
                                    monthCalendarPage.calendarVm.setCalendarSystemByValue(Number(optionModel.value));
                                monthCalendarPage.requestViewHook("calendar-system");
                            }
                        }
                    }
                }
            }
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: LV.Theme.panelBackground09
            radius: LV.Theme.radiusSm

            Column {
                anchors.fill: parent
                anchors.margins: LV.Theme.gap4
                spacing: LV.Theme.gap2

                Grid {
                    columns: 7
                    spacing: LV.Theme.gap2

                    Repeater {
                        model: monthCalendarPage.weekdayLabels

                        Rectangle {
                            id: weekdayCell

                            required property var modelData
                            color: LV.Theme.accentTransparent
                            height: LV.Theme.gap12
                            width: Math.max(20, Math.floor((monthCalendarPage.width - LV.Theme.gap20) / 7))

                            LV.Label {
                                anchors.centerIn: parent
                                text: String(weekdayCell.modelData)
                            }
                        }
                    }
                }
                Grid {
                    columns: 7
                    spacing: LV.Theme.gap2

                    Repeater {
                        model: monthCalendarPage.dayModels

                        Rectangle {
                            id: dayCell

                            required property var modelData
                            readonly property var dayModel: dayCell.modelData
                            readonly property bool isCurrentMonth: dayModel && dayModel.inCurrentMonth === true
                            readonly property bool isToday: dayModel && dayModel.isToday === true
                            readonly property bool isSelectedDate: monthCalendarPage.calendarVm
                                                                     && dayModel
                                                                     && dayModel.dateIso !== undefined
                                                                     && String(dayModel.dateIso) === String(monthCalendarPage.calendarVm.selectedDateIso)
                            readonly property int entryCount: dayModel && dayModel.entryCount !== undefined
                                                              ? Number(dayModel.entryCount)
                                                              : 0

                            color: isToday
                                   ? LV.Theme.primary
                                   : isSelectedDate
                                     ? LV.Theme.panelBackground12
                                     : isCurrentMonth
                                       ? LV.Theme.panelBackground10
                                       : LV.Theme.panelBackground06
                            height: LV.Theme.gap12
                            radius: LV.Theme.radiusSm
                            width: Math.max(20, Math.floor((monthCalendarPage.width - LV.Theme.gap20) / 7))

                            MouseArea {
                                anchors.fill: parent

                                onClicked: {
                                    if (monthCalendarPage.calendarVm && monthCalendarPage.calendarVm.setSelectedDateIso
                                            && dayCell.dayModel && dayCell.dayModel.dateIso !== undefined)
                                        monthCalendarPage.calendarVm.setSelectedDateIso(String(dayCell.dayModel.dateIso));
                                    monthCalendarPage.requestViewHook("select-date");
                                }
                            }
                            LV.Label {
                                anchors.centerIn: parent
                                color: dayCell.isToday ? LV.Theme.panelBackground01 : dayCell.isCurrentMonth ? LV.Theme.titleHeaderColor : LV.Theme.descriptionColor
                                text: dayCell.dayModel && dayCell.dayModel.day !== undefined ? String(dayCell.dayModel.day) : ""
                            }
                            LV.Label {
                                anchors.bottom: parent.bottom
                                anchors.right: parent.right
                                anchors.bottomMargin: LV.Theme.gap2
                                anchors.rightMargin: LV.Theme.gap2
                                color: dayCell.isToday ? LV.Theme.panelBackground01 : LV.Theme.descriptionColor
                                text: dayCell.entryCount > 0 ? String(dayCell.entryCount) : ""
                                visible: dayCell.entryCount > 0
                            }
                        }
                    }
                }
            }
        }
    }
}
