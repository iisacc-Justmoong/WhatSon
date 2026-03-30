pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: gutterLayer

    property color activeLineNumberColor: "#9DA0A8"
    property int currentCursorLineNumber: 1
    property int editorLineHeight: 12
    property var effectiveGutterMarkers: []
    property color gutterColor: "transparent"
    property int gutterCommentMarkerOffset: 2
    property int gutterCommentRailLeft: 4
    property int gutterIconRailLeft: 40
    property int gutterIconRailWidth: 18
    property color lineNumberColor: "#4E5157"
    property int lineNumberColumnLeft: 14
    property int lineNumberColumnTextWidth: 0
    property var lineYResolver: null
    property var markerHeightResolver: null
    property var markerYResolver: null
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
                width: 4
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
                required property int modelData

                color: modelData === gutterLayer.currentCursorLineNumber ? gutterLayer.activeLineNumberColor : gutterLayer.lineNumberColor
                font.family: LV.Theme.fontBody
                font.letterSpacing: 0
                font.pixelSize: 11
                font.weight: modelData === gutterLayer.currentCursorLineNumber ? Font.Medium : Font.Normal
                height: gutterLayer.editorLineHeight
                horizontalAlignment: Text.AlignRight
                style: caption
                text: String(modelData)
                verticalAlignment: Text.AlignVCenter
                width: gutterLayer.lineNumberColumnTextWidth
                x: gutterLayer.lineNumberColumnLeft
                y: gutterLayer.resolveNumericResolverValue(
                       gutterLayer.lineYResolver,
                       0,
                       modelData)
            }
        }
    }
}
