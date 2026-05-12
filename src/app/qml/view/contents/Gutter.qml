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
    property real fallbackLineHeight: LV.Theme.gap20
    property var lineMetricProvider: null
    property int lineMetricsRevision: 0
    property int currentLineIndex: -1
    property color lineNumberColor: LV.Theme.descriptionColor
    property color currentLineIndicatorColor: LV.Theme.accentBlue
    property bool showLineNumbers: true
    readonly property int minimumLineNumberDigitCount: 6
    readonly property bool hasSelectedSource: gutter.selectedNoteId.trim().length > 0
            && gutter.sourceFilePath.trim().length > 0
    readonly property bool hasCurrentLineIndicator: gutter.hasSelectedSource
            && gutter.showLineNumbers
            && gutter.currentLineIndex >= 0
            && gutter.currentLineIndex < Math.max(0, gutter.lineCount)
    readonly property int lineNumberDigitCount: Math.max(
            gutter.minimumLineNumberDigitCount,
            String(Math.max(1, gutter.lineCount)).length)

    clip: true
    implicitWidth: LV.Theme.gap12 + gutter.lineNumberDigitCount * LV.Theme.gap8
    objectName: "contentsGutter"

    function fallbackLineMetric(lineIndex) {
        const fallbackHeight = Math.max(1, Number(gutter.fallbackLineHeight) || 1);
        return {
            y: Math.max(0, Math.floor(Number(lineIndex) || 0)) * fallbackHeight,
            height: fallbackHeight
        };
    }

    function lineMetricAt(lineIndex) {
        gutter.lineMetricsRevision;
        const normalizedIndex = Math.max(0, Math.floor(Number(lineIndex) || 0));
        const fallbackMetric = gutter.fallbackLineMetric(normalizedIndex);
        if (!gutter.lineMetricProvider)
            return fallbackMetric;

        const providedMetric = gutter.lineMetricProvider(normalizedIndex);
        if (!providedMetric)
            return fallbackMetric;

        return {
            y: Math.max(0, gutter.numberOrFallback(providedMetric.y, fallbackMetric.y)),
            height: Math.max(1, gutter.numberOrFallback(providedMetric.height, fallbackMetric.height))
        };
    }

    function numberOrFallback(value, fallbackValue) {
        const numericValue = Number(value);
        return Number.isFinite(numericValue) ? numericValue : fallbackValue;
    }

    Repeater {
        model: gutter.hasSelectedSource && gutter.showLineNumbers
               ? Math.max(0, gutter.lineCount)
               : 0

        delegate: Item {
            id: lineNumberRow

            required property int index
            readonly property var lineMetric: gutter.lineMetricAt(index)
            readonly property bool currentLineActive: gutter.hasCurrentLineIndicator && index === gutter.currentLineIndex

            height: Math.max(1, Number(lineMetric.height) || gutter.fallbackLineHeight)
            visible: gutter.showLineNumbers && gutter.hasSelectedSource
            width: gutter.width
            y: Math.round((Number(lineMetric.y) || 0) - gutter.contentY)

            Text {
                color: gutter.lineNumberColor
                elide: Text.ElideLeft
                font.family: LV.Theme.fontBody
                font.pixelSize: LV.Theme.textCaption
                height: parent.height
                horizontalAlignment: Text.AlignRight
                text: String(lineNumberRow.index + 1)
                verticalAlignment: Text.AlignVCenter
                width: Math.max(0, parent.width - LV.Theme.gap12)
                x: LV.Theme.gapNone
            }

            Rectangle {
                color: gutter.currentLineIndicatorColor
                height: Math.max(LV.Theme.gap12, Math.min(parent.height, LV.Theme.gap16))
                objectName: "contentsGutterCurrentLineIndicator"
                opacity: 0.92
                radius: width / 2
                visible: lineNumberRow.currentLineActive
                width: Math.max(LV.Theme.gap2, LV.Theme.strokeThin * 2)
                x: Math.max(LV.Theme.gapNone, parent.width - width - LV.Theme.gap2)
                y: Math.max(LV.Theme.gapNone, (parent.height - height) / 2)
            }
        }
    }
}
