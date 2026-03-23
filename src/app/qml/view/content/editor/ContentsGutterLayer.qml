import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: gutterLayer

    property color activeLineNumberColor: "#9DA0A8"
    property int currentCursorLineNumber: 1
    property int editorLineHeight: 12
    property var effectiveGutterMarkers: []
    property color gutterColor: LV.Theme.panelBackground02
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
                readonly property var markerSpec: modelData || ({
                        "color": gutterLayer.activeLineNumberColor,
                        "lineSpan": 1,
                        "startLine": 1
                    })
                required property var modelData

                color: markerSpec.color
                height: gutterLayer.markerHeightResolver ? Number(gutterLayer.markerHeightResolver(markerSpec)) || 0 : 0
                radius: width / 2
                width: 4
                x: gutterLayer.gutterCommentRailLeft + gutterLayer.gutterCommentMarkerOffset
                y: gutterLayer.markerYResolver ? Number(gutterLayer.markerYResolver(markerSpec)) || 0 : 0
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
                y: gutterLayer.lineYResolver ? Number(gutterLayer.lineYResolver(modelData)) || 0 : 0
            }
        }
    }
}
