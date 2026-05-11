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
    property color lineNumberColor: LV.Theme.descriptionColor
    property bool showLineNumbers: true
    readonly property int minimumLineNumberDigitCount: 6
    readonly property bool hasSelectedSource: gutter.selectedNoteId.trim().length > 0
            && gutter.sourceFilePath.trim().length > 0
    readonly property int lineNumberDigitCount: Math.max(
            gutter.minimumLineNumberDigitCount,
            String(Math.max(1, gutter.lineCount)).length)

    clip: true
    implicitWidth: LV.Theme.gap12 + gutter.lineNumberDigitCount * LV.Theme.gap8
    objectName: "contentsGutter"

    function numberOrFallback(value, fallbackValue) {
        const numericValue = Number(value);
        return Number.isFinite(numericValue) ? numericValue : fallbackValue;
    }

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

    Repeater {
        model: gutter.hasSelectedSource && gutter.showLineNumbers
               ? Math.max(0, gutter.lineCount)
               : 0

        delegate: Text {
            required property int index
            readonly property var lineMetric: gutter.lineMetricAt(index)

            color: gutter.lineNumberColor
            elide: Text.ElideLeft
            font.family: LV.Theme.fontBody
            font.pixelSize: LV.Theme.textCaption
            height: Math.max(1, Number(lineMetric.height) || gutter.fallbackLineHeight)
            horizontalAlignment: Text.AlignRight
            text: String(index + 1)
            verticalAlignment: Text.AlignVCenter
            visible: gutter.showLineNumbers && gutter.hasSelectedSource
            width: Math.max(0, gutter.width - LV.Theme.gap8)
            y: Math.round((Number(lineMetric.y) || 0) - gutter.contentY)
        }
    }
}
