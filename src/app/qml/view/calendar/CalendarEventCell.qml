pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: calendarEventCell

    readonly property string figmaNodeId: "227:9429"
    readonly property int backgroundDefault: 0
    readonly property int backgroundColored: 1
    property string label: ""
    property int backgroundType: calendarEventCell.backgroundDefault
    property color defaultBackgroundColor: LV.Theme.panelBackground08
    property color coloredBackgroundColor: LV.Theme.primary
    property color textColor: Qt.rgba(1.0, 1.0, 1.0, 0.8)
    property int cornerRadius: 4
    property int horizontalInset: 8
    property int verticalInset: 2
    property int labelPixelSize: 12
    property int labelWeight: Font.Medium

    color: calendarEventCell.backgroundType === calendarEventCell.backgroundColored
           ? calendarEventCell.coloredBackgroundColor
           : calendarEventCell.defaultBackgroundColor
    height: 16
    radius: calendarEventCell.cornerRadius

    LV.Label {
        anchors.left: parent.left
        anchors.leftMargin: calendarEventCell.horizontalInset
        anchors.right: parent.right
        anchors.rightMargin: calendarEventCell.horizontalInset
        anchors.verticalCenter: parent.verticalCenter
        color: calendarEventCell.textColor
        elide: Text.ElideRight
        font.pixelSize: calendarEventCell.labelPixelSize
        font.weight: calendarEventCell.labelWeight
        text: calendarEventCell.label
    }
}
