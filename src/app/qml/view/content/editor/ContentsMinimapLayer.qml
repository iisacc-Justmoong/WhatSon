import QtQuick
import LVRS 1.0 as LV

Item {
    id: minimapLayer

    property var editorFlickable: null
    property var minimapBarWidthResolver: null
    property var minimapContentHeightResolver: null
    property color minimapCurrentLineColor: "#9DA0A8"
    property var minimapCurrentLineHeightResolver: null
    property var minimapCurrentLineWidthResolver: null
    property var minimapCurrentLineYResolver: null
    property color minimapLineColor: "#4E5157"
    property var minimapSilhouetteHeightResolver: null
    property int minimapTrackInset: 8
    property int minimapTrackWidth: 36
    property color minimapViewportFillColor: "#149DA0A8"
    property var minimapViewportHeightResolver: null
    property var minimapViewportYResolver: null
    property var minimapVisualRowPaintHeightResolver: null
    property var minimapVisualRowPaintYResolver: null
    property var minimapVisualRows: []
    property var scrollToMinimapPositionHandler: null
    readonly property real trackHeight: minimapTrack.height
    readonly property real trackWidth: minimapTrack.width

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
        height: Math.min(Math.max(1, parent.height - 16), minimapLayer.minimapSilhouetteHeightResolver ? Number(minimapLayer.minimapSilhouetteHeightResolver()) || 1 : 1)
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
                    const barY = minimapLayer.minimapVisualRowPaintYResolver ? Number(minimapLayer.minimapVisualRowPaintYResolver(row)) || 0 : 0;
                    const barHeight = minimapLayer.minimapVisualRowPaintHeightResolver ? Number(minimapLayer.minimapVisualRowPaintHeightResolver(row)) || 1 : 1;
                    if (barY > height || barY + barHeight < 0)
                        continue;
                    context.fillStyle = minimapLayer.minimapLineColor;
                    context.globalAlpha = characterCount > 0 ? 0.48 : 0.12;
                    const barWidth = minimapLayer.minimapBarWidthResolver ? Number(minimapLayer.minimapBarWidthResolver(characterCount)) || 1 : 1;
                    context.fillRect(0, barY, barWidth, barHeight);
                }
                context.globalAlpha = 1;
            }
        }
        Rectangle {
            readonly property bool scrollable: minimapLayer.editorFlickable && minimapLayer.minimapContentHeightResolver && Number(minimapLayer.minimapContentHeightResolver()) > (Number(minimapLayer.editorFlickable.height) || 0)

            anchors.left: parent.left
            anchors.right: parent.right
            border.width: 0
            color: minimapLayer.minimapViewportFillColor
            height: minimapLayer.minimapViewportHeightResolver ? Number(minimapLayer.minimapViewportHeightResolver()) || 0 : 0
            radius: 3
            visible: scrollable
            y: minimapLayer.minimapViewportYResolver ? Number(minimapLayer.minimapViewportYResolver()) || 0 : 0
        }
        Rectangle {
            color: minimapLayer.minimapCurrentLineColor
            height: Math.max(1, minimapLayer.minimapCurrentLineHeightResolver ? Number(minimapLayer.minimapCurrentLineHeightResolver()) || 1 : 1)
            opacity: 0.8
            radius: 1
            width: minimapLayer.minimapCurrentLineWidthResolver ? Number(minimapLayer.minimapCurrentLineWidthResolver()) || 0 : 0
            x: 0
            y: minimapLayer.minimapCurrentLineYResolver ? Number(minimapLayer.minimapCurrentLineYResolver()) || 0 : 0
        }
        MouseArea {
            acceptedButtons: Qt.LeftButton
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true

            onPositionChanged: function (mouse) {
                if (!(pressedButtons & Qt.LeftButton) || !minimapLayer.scrollToMinimapPositionHandler)
                    return;
                minimapLayer.scrollToMinimapPositionHandler(mouse.y);
            }
            onPressed: function (mouse) {
                if (minimapLayer.scrollToMinimapPositionHandler)
                    minimapLayer.scrollToMinimapPositionHandler(mouse.y);
            }
        }
        LV.WheelScrollGuard {
            anchors.fill: parent
            consumeInside: true
            targetFlickable: minimapLayer.editorFlickable
        }
    }
}
