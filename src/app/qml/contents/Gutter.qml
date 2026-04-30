pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: gutter

    property int activeLineNumber: LV.Theme.controlHeightMd - LV.Theme.gap5
    property color changedMarkerColor: LV.Theme.warning
    property color conflictMarkerColor: LV.Theme.success
    property color gutterColor: LV.Theme.panelBackground02
    property color lineNumberActiveColor: LV.Theme.captionColor
    property color lineNumberColor: LV.Theme.disabledColor
    property int lineNumberCount: LV.Theme.controlHeightMd + LV.Theme.gap7

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        gutter.viewHookRequested(hookReason);
    }

    clip: true
    color: gutter.gutterColor
    objectName: "figma-155-5345-Gutter"

    Item {
        id: markerRail

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: LV.Theme.gap4
        anchors.top: parent.top
        width: LV.Theme.gap10

        Rectangle {
            color: gutter.changedMarkerColor
            height: LV.Theme.gap20 + LV.Theme.gap20
            radius: LV.Theme.radiusXs
            width: LV.Theme.gap4
            x: LV.Theme.gap2
            y: LV.Theme.inputWidthMd + LV.Theme.controlHeightMd + LV.Theme.gap5
        }
        Rectangle {
            color: gutter.conflictMarkerColor
            height: LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap24
            radius: LV.Theme.radiusXs
            width: LV.Theme.gap4
            x: LV.Theme.gap2
            y: LV.Theme.dialogMaxWidth + LV.Theme.gap20 + LV.Theme.gap3
        }
    }
    Item {
        id: iconRail

        anchors.bottom: parent.bottom
        anchors.top: parent.top
        width: LV.Theme.controlIndicatorSize
        x: LV.Theme.gap20 + LV.Theme.gap20
    }
    Column {
        id: lineNumberColumn

        clip: true
        spacing: LV.Theme.gapNone
        width: LV.Theme.controlHeightSm
        x: LV.Theme.gap14
        y: LV.Theme.gap3

        Repeater {
            model: gutter.lineNumberCount

            Text {
                required property int index

                readonly property int lineNumber: index + Math.round(LV.Theme.strokeThin)

                color: lineNumber === gutter.activeLineNumber ? gutter.lineNumberActiveColor : gutter.lineNumberColor
                elide: Text.ElideNone
                font.family: LV.Theme.fontBody
                font.pixelSize: LV.Theme.textCaption
                font.weight: LV.Theme.textCaptionWeight
                height: LV.Theme.textCaptionLineHeight
                horizontalAlignment: Text.AlignRight
                text: String(lineNumber)
                verticalAlignment: Text.AlignTop
                width: lineNumberColumn.width
            }
        }
    }
}
