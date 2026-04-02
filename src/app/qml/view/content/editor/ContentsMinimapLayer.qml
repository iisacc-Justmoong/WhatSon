pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: minimapLayer

    property var editorFlickable: null
    property var minimapBarWidthResolver: null
    property color minimapCurrentLineColor: "#9DA0A8"
    property real minimapCurrentLineHeight: 1
    property real minimapCurrentLineWidth: 0
    property real minimapCurrentLineY: 0
    property color minimapLineColor: "#4E5157"
    property real minimapSilhouetteHeight: 1
    property bool minimapScrollable: false
    property int minimapTrackInset: 8
    property int minimapTrackWidth: 36
    property color minimapViewportFillColor: "#149DA0A8"
    property real minimapViewportHeight: 0
    property real minimapViewportY: 0
    property var minimapVisualRowPaintHeightResolver: null
    property var minimapVisualRowPaintYResolver: null
    property var minimapVisualRows: []
    property var scrollToMinimapPositionHandler: null
    readonly property real trackHeight: minimapTrack.height
    readonly property real trackWidth: minimapTrack.width

    function resolveNumericResolverValue(resolver, fallbackValue, argument) {
        const numericFallbackValue = Number(fallbackValue);
        const safeFallbackValue = isFinite(numericFallbackValue) ? numericFallbackValue : 0;
        if (typeof resolver !== "function")
            return safeFallbackValue;
        const resolvedValue = argument === undefined ? resolver() : resolver(argument);
        const numericResolvedValue = Number(resolvedValue);
        return isFinite(numericResolvedValue) ? numericResolvedValue : safeFallbackValue;
    }
    function invokeScrollToMinimapPosition(localY) {
        const scrollHandler = minimapLayer.scrollToMinimapPositionHandler;
        if (typeof scrollHandler === "function")
            scrollHandler(localY);
    }
    function requestRepaint() {
        minimapCanvas.requestPaint();
    }

    onHeightChanged: requestRepaint()
    onMinimapVisualRowsChanged: requestRepaint()
    onWidthChanged: requestRepaint()

    Item {
        id: minimapTrack

        anchors.right: parent.right
        anchors.rightMargin: minimapLayer.minimapTrackInset
        anchors.top: parent.top
        anchors.topMargin: 8
        height: Math.min(
                    Math.max(1, parent.height - 16),
                    Math.max(1, Number(minimapLayer.minimapSilhouetteHeight) || 1))
        width: minimapLayer.minimapTrackWidth

        Canvas {
            id: minimapCanvas

            anchors.fill: parent
            renderTarget: Canvas.Image

            onPaint: {
                const context = getContext("2d");
                context.clearRect(0, 0, width, height);

                const rows = Array.isArray(minimapLayer.minimapVisualRows) ? minimapLayer.minimapVisualRows : [];
                for (let rowIndex = 0; rowIndex < rows.length; ++rowIndex) {
                    const row = rows[rowIndex];
                    const characterCount = Number(row.charCount) || 0;
                    const barY = minimapLayer.resolveNumericResolverValue(
                                minimapLayer.minimapVisualRowPaintYResolver,
                                0,
                                row);
                    const barHeight = minimapLayer.resolveNumericResolverValue(
                                minimapLayer.minimapVisualRowPaintHeightResolver,
                                1,
                                row);
                    if (barY > height || barY + barHeight < 0)
                        continue;
                    context.fillStyle = minimapLayer.minimapLineColor;
                    context.globalAlpha = characterCount > 0 ? 0.48 : 0.12;
                    const barWidth = minimapLayer.resolveNumericResolverValue(
                                minimapLayer.minimapBarWidthResolver,
                                1,
                                characterCount);
                    context.fillRect(0, barY, barWidth, barHeight);
                }
                context.globalAlpha = 1;
            }
        }
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            border.width: 0
            color: minimapLayer.minimapViewportFillColor
            height: Math.max(0, Number(minimapLayer.minimapViewportHeight) || 0)
            radius: 3
            visible: minimapLayer.minimapScrollable
            y: Math.max(0, Number(minimapLayer.minimapViewportY) || 0)
        }
        Rectangle {
            color: minimapLayer.minimapCurrentLineColor
            height: Math.max(
                        1,
                        Number(minimapLayer.minimapCurrentLineHeight) || 1)
            opacity: 0.8
            radius: 1
            width: Math.max(0, Number(minimapLayer.minimapCurrentLineWidth) || 0)
            x: 0
            y: Math.max(0, Number(minimapLayer.minimapCurrentLineY) || 0)
        }
        MouseArea {
            acceptedButtons: Qt.LeftButton
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true

            onPositionChanged: function (mouse) {
                if (!(pressedButtons & Qt.LeftButton) || !minimapLayer.scrollToMinimapPositionHandler)
                    return;
                minimapLayer.invokeScrollToMinimapPosition(mouse.y);
            }
            onPressed: function (mouse) {
                minimapLayer.invokeScrollToMinimapPosition(mouse.y);
            }
        }
        LV.WheelScrollGuard {
            anchors.fill: parent
            consumeInside: true
            targetFlickable: minimapLayer.editorFlickable
        }
    }
}
