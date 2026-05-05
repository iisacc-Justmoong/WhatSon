pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: lineNumberRail

    property var rows: []
    property int activeSourceCursorPosition: 0
    property int activeSelectionEnd: activeSourceCursorPosition
    property int activeSelectionStart: activeSourceCursorPosition
    property color textColor: LV.Theme.captionColor
    property color activeIndicatorColor: LV.Theme.accentBlue
    readonly property real minimumRowHeight: Math.max(1, Number(LV.Theme.textBodyLineHeight) || 1)
    readonly property real activeIndicatorWidth: Math.max(LV.Theme.strokeThin, LV.Theme.gap2)
    readonly property real numberColumnWidth: Math.max(LV.Theme.gap16, Number(LV.Theme.textCaption) * 2)
    readonly property real numberRightPadding: LV.Theme.gap8
    readonly property real baselineLeftBlankWidth: Math.max(0, LV.Theme.buttonMinWidth - lineNumberRail.numberColumnWidth - lineNumberRail.numberRightPadding)
    readonly property real leftBlankWidth: lineNumberRail.baselineLeftBlankWidth / 2
    readonly property real preferredWidth: lineNumberRail.leftBlankWidth + lineNumberRail.numberColumnWidth + lineNumberRail.numberRightPadding

    objectName: "contentsLogicalLineNumberRail"

    function normalizedSourceOffset(value) {
        const offset = Number(value);
        return isFinite(offset) ? Math.max(0, Math.floor(offset)) : 0;
    }

    function rowContainsActiveSourceRange(row) {
        const rowStart = lineNumberRail.normalizedSourceOffset(row && row.sourceStart !== undefined ? row.sourceStart : 0);
        const rowEnd = Math.max(rowStart, lineNumberRail.normalizedSourceOffset(row && row.sourceEnd !== undefined ? row.sourceEnd : rowStart));
        const selectionStart = lineNumberRail.normalizedSourceOffset(lineNumberRail.activeSelectionStart);
        const selectionEnd = lineNumberRail.normalizedSourceOffset(lineNumberRail.activeSelectionEnd);

        if (selectionStart !== selectionEnd) {
            const activeStart = Math.min(selectionStart, selectionEnd);
            const activeEnd = Math.max(selectionStart, selectionEnd);
            if (rowStart === rowEnd)
                return activeStart <= rowStart && activeEnd >= rowEnd;
            return activeStart < rowEnd && activeEnd > rowStart;
        }

        const cursorPosition = lineNumberRail.normalizedSourceOffset(lineNumberRail.activeSourceCursorPosition);
        return cursorPosition >= rowStart && cursorPosition <= rowEnd;
    }

    Repeater {
        model: lineNumberRail.rows

        Item {
            id: lineNumberRailRow

            required property int index
            required property var modelData
            readonly property bool activeSourceRow: lineNumberRail.rowContainsActiveSourceRange(modelData)

            height: Math.max(
                        lineNumberRail.minimumRowHeight,
                        Number(modelData.height) || lineNumberRail.minimumRowHeight)
            width: lineNumberRail.width
            x: LV.Theme.gapNone
            y: Number(modelData.y) || LV.Theme.gapNone

            Rectangle {
                anchors.left: lineNumberRailRow.left
                anchors.top: lineNumberRailRow.top
                color: lineNumberRail.activeIndicatorColor
                height: lineNumberRailRow.height
                radius: width / 2
                visible: lineNumberRailRow.activeSourceRow
                width: lineNumberRail.activeIndicatorWidth
            }

            Text {
                anchors.left: lineNumberRailRow.left
                anchors.leftMargin: lineNumberRail.leftBlankWidth
                anchors.right: lineNumberRailRow.right
                anchors.rightMargin: lineNumberRail.numberRightPadding
                anchors.top: lineNumberRailRow.top
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
