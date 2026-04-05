pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: gutterLayer

    property color activeLineNumberColor: LV.Theme.accentGray
    property int currentCursorLineNumber: 1
    property int editorLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    property var effectiveGutterMarkers: []
    property color gutterColor: "transparent"
    property int gutterCommentMarkerOffset: LV.Theme.gap2
    property int gutterCommentRailLeft: LV.Theme.gap4
    property int gutterIconRailLeft: Math.max(0, Math.round(LV.Theme.scaleMetric(40)))
    property int gutterIconRailWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(18)))
    property color lineNumberColor: LV.Theme.descriptionColor
    property int lineNumberColumnLeft: Math.max(0, Math.round(LV.Theme.scaleMetric(14)))
    property int lineNumberColumnTextWidth: 0
    property var lineYResolver: null
    property var markerHeightResolver: null
    property var markerYResolver: null
    readonly property int gutterMarkerWidth: LV.Theme.gap4
    readonly property int lineNumberTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(11)))
    readonly property real viewportHeight: lineNumberViewport.height
    property var visibleLineNumbersModel: []

    function resolveNumericResolverValue(resolver, fallbackValue, argument) {
        const numericFallbackValue = Number(fallbackValue);
        const safeFallbackValue = isFinite(numericFallbackValue) ? numericFallbackValue : 0;
        if (typeof resolver !== "function")
            return safeFallbackValue;
        const resolvedValue = argument === undefined ? resolver() : resolver(argument);
        const numericResolvedValue = Number(resolvedValue);
        return isFinite(numericResolvedValue) ? numericResolvedValue : safeFallbackValue;
    }

    color: gutterColor

    Item {
        id: lineNumberViewport

        anchors.fill: parent
        clip: true

        Item {
            height: parent.height
            width: gutterLayer.gutterIconRailWidth
            x: gutterLayer.gutterIconRailLeft
        }
        Repeater {
            model: gutterLayer.effectiveGutterMarkers

            delegate: Rectangle {
                id: gutterMarker

                required property var modelData
                readonly property var markerSpec: gutterMarker.modelData || ({
                        "color": gutterLayer.activeLineNumberColor,
                        "lineSpan": 1,
                        "startLine": 1
                    })

                color: markerSpec.color
                height: gutterLayer.resolveNumericResolverValue(
                            gutterLayer.markerHeightResolver,
                            0,
                            markerSpec)
                radius: width / 2
                width: gutterLayer.gutterMarkerWidth
                x: gutterLayer.gutterCommentRailLeft + gutterLayer.gutterCommentMarkerOffset
                y: gutterLayer.resolveNumericResolverValue(
                       gutterLayer.markerYResolver,
                       0,
                       markerSpec)
            }
        }
        Repeater {
            model: gutterLayer.visibleLineNumbersModel

            delegate: LV.Label {
                required property var modelData

                readonly property int lineNumber: modelData && modelData.lineNumber !== undefined ? Number(modelData.lineNumber) || 0 : 0
                readonly property real resolvedY: modelData && modelData.y !== undefined ? Number(modelData.y) || 0 : 0

                color: lineNumber === gutterLayer.currentCursorLineNumber ? gutterLayer.activeLineNumberColor : gutterLayer.lineNumberColor
                font.family: LV.Theme.fontBody
                font.letterSpacing: 0
                font.pixelSize: gutterLayer.lineNumberTextSize
                font.weight: lineNumber === gutterLayer.currentCursorLineNumber ? Font.Medium : Font.Normal
                height: gutterLayer.editorLineHeight
                horizontalAlignment: Text.AlignRight
                style: caption
                text: String(lineNumber)
                verticalAlignment: Text.AlignVCenter
                width: gutterLayer.lineNumberColumnTextWidth
                x: gutterLayer.lineNumberColumnLeft
                y: resolvedY
            }
        }
    }
}
