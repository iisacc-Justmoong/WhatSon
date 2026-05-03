pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: gutter

    property int activeLineNumber: LV.Theme.gapNone
    property color cursorMarkerColor: LV.Theme.accentBlue
    property int iconRailX: LV.Theme.gap20
    property color lineNumberActiveColor: LV.Theme.captionColor
    property int lineNumberBaseOffset: LV.Theme.strokeThin
    property color lineNumberColor: LV.Theme.disabledColor
    property int lineNumberCount: LV.Theme.strokeThin
    property int lineNumberColumnLeft: LV.Theme.gap14
    property int lineNumberColumnTextWidth: LV.Theme.controlHeightSm
    property var lineNumberEntries: []
    property int markerFallbackHeight: LV.Theme.textBodyLineHeight
    property var markerEntries: []
    property color unsavedMarkerColor: LV.Theme.warning

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        gutter.viewHookRequested(hookReason);
    }

    clip: true
    objectName: "figma-155-5345-Gutter"

    Item {
        id: markerRail

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: LV.Theme.gap4
        anchors.top: parent.top
        width: LV.Theme.gap10

        Repeater {
            model: gutter.markerEntries

            Rectangle {
                required property var modelData

                readonly property string markerType: String(modelData.type || "")

                color: markerType === "cursor" ? gutter.cursorMarkerColor : gutter.unsavedMarkerColor
                height: Math.max(LV.Theme.strokeThin, Number(modelData.height) || gutter.markerFallbackHeight)
                radius: LV.Theme.radiusXs
                width: LV.Theme.gap4
                x: LV.Theme.gap2
                y: Number(modelData.y) || LV.Theme.gapNone
            }
        }
    }
    Item {
        id: iconRail

        anchors.bottom: parent.bottom
        anchors.top: parent.top
        width: LV.Theme.controlIndicatorSize
        x: gutter.iconRailX
    }
    Item {
        id: lineNumberLayer

        clip: true
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        width: gutter.lineNumberColumnTextWidth
        x: gutter.lineNumberColumnLeft

        Repeater {
            model: gutter.lineNumberEntries

            Text {
                required property var modelData

                readonly property real lineHeight: Math.max(
                        LV.Theme.textCaptionLineHeight,
                        Number(modelData.height) || LV.Theme.textCaptionLineHeight)
                readonly property int lineNumber: Number(modelData.lineNumber) || gutter.lineNumberBaseOffset

                color: lineNumber === gutter.activeLineNumber ? gutter.lineNumberActiveColor : gutter.lineNumberColor
                elide: Text.ElideNone
                font.family: LV.Theme.fontBody
                font.pixelSize: LV.Theme.textCaption
                font.weight: LV.Theme.textCaptionWeight
                height: lineHeight
                horizontalAlignment: Text.AlignRight
                text: String(lineNumber)
                verticalAlignment: Text.AlignTop
                width: lineNumberLayer.width
                y: Number(modelData.y) || LV.Theme.gapNone
            }
        }
    }
}
