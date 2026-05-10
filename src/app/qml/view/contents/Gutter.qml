pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: gutter

    property real contentY: 0
    property int parsedLineCount: 0
    property int lineCount: parsedLineCount
    property string sourceFilePath: ""
    property string selectedNoteId: ""
    property string selectedNoteDirectoryPath: ""
    property real lineHeight: LV.Theme.gap20
    property color lineNumberColor: LV.Theme.descriptionColor
    property color separatorColor: LV.Theme.strokeSoft
    property bool showLineNumbers: true
    readonly property bool hasSelectedSource: gutter.selectedNoteId.trim().length > 0
            && gutter.sourceFilePath.trim().length > 0
    readonly property int lineNumberDigitCount: Math.max(
            2,
            String(Math.max(1, gutter.lineCount)).length)

    clip: true
    implicitWidth: LV.Theme.gap12 + gutter.lineNumberDigitCount * LV.Theme.gap8
    objectName: "contentsGutter"

    Repeater {
        model: gutter.hasSelectedSource && gutter.showLineNumbers
               ? Math.max(0, gutter.lineCount)
               : 0

        delegate: Text {
            required property int index

            color: gutter.lineNumberColor
            elide: Text.ElideLeft
            font.family: LV.Theme.fontBody
            font.pixelSize: LV.Theme.textCaption
            height: gutter.lineHeight
            horizontalAlignment: Text.AlignRight
            text: String(index + 1)
            visible: gutter.showLineNumbers && gutter.hasSelectedSource
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
