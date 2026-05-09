pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: gutter

    property real contentY: 0
    property int lineCount: 0
    property real lineHeight: LV.Theme.gap20
    property color lineNumberColor: LV.Theme.descriptionColor
    property color separatorColor: LV.Theme.strokeSoft
    property bool showLineNumbers: true

    clip: true
    implicitWidth: LV.Theme.gap24 + LV.Theme.gap12
    objectName: "contentsGutter"

    Repeater {
        model: Math.max(0, gutter.lineCount)

        delegate: Text {
            required property int index

            color: gutter.lineNumberColor
            elide: Text.ElideLeft
            font.family: LV.Theme.fontBody
            font.pixelSize: LV.Theme.textCaption
            height: gutter.lineHeight
            horizontalAlignment: Text.AlignRight
            text: String(index + 1)
            visible: gutter.showLineNumbers
            width: Math.max(0, gutter.width - LV.Theme.gap8)
            y: Math.round(index * gutter.lineHeight - gutter.contentY)
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.top: parent.top
        color: gutter.separatorColor
        width: Math.max(1, Math.round(LV.Theme.strokeThin))
    }
}
