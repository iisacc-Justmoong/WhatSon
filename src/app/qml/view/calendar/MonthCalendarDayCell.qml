pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: monthCalendarDayCell

    readonly property string figmaNodeId: "227:9427"
    property int dayNumber: 0
    property bool disable: false
    property bool selected: false
    property bool today: false
    property int maxVisibleEntries: 8
    property var entryCells: []
    readonly property int dayCellPadding: 8
    readonly property int dayLabelHeight: 12
    readonly property int dayLabelGap: 10
    readonly property int eventRowHeight: 16
    readonly property int eventRowSpacing: 2
    readonly property int entryCapacity: Math.max(
                                            0,
                                            Math.floor(
                                                (monthCalendarDayCell.height
                                                 - (monthCalendarDayCell.dayCellPadding * 2)
                                                 - monthCalendarDayCell.dayLabelHeight
                                                 - monthCalendarDayCell.dayLabelGap
                                                 + monthCalendarDayCell.eventRowSpacing)
                                                / (monthCalendarDayCell.eventRowHeight
                                                   + monthCalendarDayCell.eventRowSpacing)))
    readonly property bool needsOverflowIndicator: monthCalendarDayCell.entryCells.length > monthCalendarDayCell.entryCapacity
                                                  && monthCalendarDayCell.entryCapacity > 0
    readonly property int clippedEntryCount: Math.max(0, Math.min(monthCalendarDayCell.entryCells.length, monthCalendarDayCell.maxVisibleEntries, monthCalendarDayCell.needsOverflowIndicator ? monthCalendarDayCell.entryCapacity - 1 : monthCalendarDayCell.entryCapacity))

    signal clicked

    border.color: monthCalendarDayCell.today ? LV.Theme.strokeSoft : "transparent"
    border.width: monthCalendarDayCell.today ? Math.max(1, LV.Theme.strokeThin) : 0
    clip: true
    color: LV.Theme.panelBackground04
    opacity: monthCalendarDayCell.disable ? 0.5 : 1.0

    MouseArea {
        anchors.fill: parent

        onClicked: monthCalendarDayCell.clicked()
    }
    Column {
        anchors.fill: parent
        anchors.margins: monthCalendarDayCell.dayCellPadding
        spacing: monthCalendarDayCell.dayLabelGap

        LV.Label {
            color: LV.Theme.titleHeaderColor
            font.pixelSize: 12
            font.weight: Font.Medium
            height: monthCalendarDayCell.dayLabelHeight
            text: String(monthCalendarDayCell.dayNumber)
        }
        Column {
            spacing: monthCalendarDayCell.eventRowSpacing
            width: parent.width

            Repeater {
                model: monthCalendarDayCell.clippedEntryCount

                CalendarEventCell {
                    id: eventCell

                    required property int index
                    readonly property var eventCellModel: monthCalendarDayCell.entryCells[eventCell.index]

                    backgroundType: eventCell.eventCellModel && eventCell.eventCellModel.backgroundType !== undefined
                                    ? Number(eventCell.eventCellModel.backgroundType)
                                    : eventCell.backgroundDefault
                    coloredBackgroundColor: eventCell.eventCellModel && eventCell.eventCellModel.backgroundColor !== undefined
                                            ? eventCell.eventCellModel.backgroundColor
                                            : LV.Theme.primary
                    cornerRadius: 4
                    height: monthCalendarDayCell.eventRowHeight
                    label: eventCell.eventCellModel && eventCell.eventCellModel.label !== undefined
                           ? String(eventCell.eventCellModel.label)
                           : ""
                    width: parent.width
                }
            }
            LV.Label {
                color: LV.Theme.descriptionColor
                font.pixelSize: 12
                text: "+" + String(monthCalendarDayCell.entryCells.length - monthCalendarDayCell.clippedEntryCount)
                      + " more"
                visible: monthCalendarDayCell.needsOverflowIndicator
            }
        }
    }
}
