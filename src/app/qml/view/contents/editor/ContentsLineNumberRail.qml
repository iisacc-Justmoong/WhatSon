pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: lineNumberRail

    property var rows: []
    property color textColor: LV.Theme.captionColor
    readonly property real minimumRowHeight: Math.max(1, Number(LV.Theme.textBodyLineHeight) || 1)

    objectName: "contentsLogicalLineNumberRail"

    Repeater {
        model: lineNumberRail.rows

        Item {
            required property int index
            required property var modelData

            height: Math.max(
                        lineNumberRail.minimumRowHeight,
                        Number(modelData.height) || lineNumberRail.minimumRowHeight)
            width: lineNumberRail.width
            x: LV.Theme.gapNone
            y: Number(modelData.y) || LV.Theme.gapNone

            Text {
                anchors.right: parent.right
                anchors.rightMargin: LV.Theme.gap8
                anchors.top: parent.top
                color: lineNumberRail.textColor
                font.family: LV.Theme.fontBody
                font.pixelSize: LV.Theme.textCaption
                horizontalAlignment: Text.AlignRight
                text: String(Number(modelData.number) || index + 1)
                verticalAlignment: Text.AlignTop
            }
        }
    }
}
