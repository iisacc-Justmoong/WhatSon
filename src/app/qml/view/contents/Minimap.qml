pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: minimap

    property string documentText: ""
    property real sourceContentY: 0
    property real sourceContentHeight: height
    property real sourceViewportHeight: height
    property real sourceContentWidth: width
    property real sourceFontPixelSize: LV.Theme.textBody
    property string sourceFontFamily: LV.Theme.fontBody
    property int sourceFontWeight: LV.Theme.textBodyWeight
    property real sourceFontLetterSpacing: LV.Theme.textBodyLetterSpacing
    property color textColor: LV.Theme.bodyColor
    property color viewportColor: LV.Theme.strokeSoft
    property color viewportBorderColor: LV.Theme.captionColor
    property color scrollbarThumbColor: LV.Theme.accentBlue
    property var scrollTarget: null
    readonly property real minimapSizeFactor: 0.85
    readonly property real normalizedSourceContentWidth: Math.max(1, Number(minimap.sourceContentWidth) || 1)
    readonly property real normalizedSourceContentHeight: Math.max(1, Number(minimap.sourceContentHeight) || 1)
    readonly property real normalizedSourceViewportHeight: Math.max(1, Number(minimap.sourceViewportHeight) || 1)
    readonly property bool viewportOverlayVisible: minimap.documentText.length > 0
            && (minimapPointer.containsMouse || minimapPointer.pressed)
    readonly property real maximumMinimapScale: minimap.minimapSizeFactor
            * Math.max(0.01, LV.Theme.strokeThin / Math.max(1, LV.Theme.gap8))
    readonly property real minimapScale: Math.max(
            0.01,
            Math.min(
                minimap.maximumMinimapScale,
                Math.max(0.01, minimap.width / minimap.normalizedSourceContentWidth),
                Math.max(0.01, minimap.height / minimap.normalizedSourceContentHeight)))
    readonly property real scaledDocumentHeight: minimap.normalizedSourceContentHeight * minimap.minimapScale
    readonly property real viewportThumbHeight: Math.max(
            LV.Theme.gap12,
            Math.min(minimap.height, minimap.normalizedSourceViewportHeight * minimap.minimapScale))
    readonly property real viewportThumbY: Math.max(
            0,
            Math.min(
                Math.max(0, minimap.height - minimap.viewportThumbHeight),
                Math.max(0, Number(minimap.sourceContentY) || 0) * minimap.minimapScale))

    clip: true
    implicitWidth: Math.max(LV.Theme.gap20 * 5, LV.Theme.gap12 * 8) * minimap.minimapSizeFactor
    objectName: "contentsMinimap"

    function requestScrollAtMinimapY(minimapY) {
        if (!minimap.scrollTarget)
            return false;

        const centeredMinimapY = Math.max(0, Number(minimapY) || 0) - minimap.viewportThumbHeight / 2;
        const requestedContentY = centeredMinimapY / minimap.minimapScale;
        if (typeof minimap.scrollTarget === "function")
            return Boolean(minimap.scrollTarget(requestedContentY));
        return false;
    }

    TextEdit {
        id: minimapPreview

        activeFocusOnPress: false
        color: minimap.textColor
        font.family: minimap.sourceFontFamily
        font.letterSpacing: minimap.sourceFontLetterSpacing
        font.pixelSize: minimap.sourceFontPixelSize
        font.weight: minimap.sourceFontWeight
        height: Math.max(minimap.normalizedSourceContentHeight, minimap.height / minimap.minimapScale)
        objectName: "contentsMinimapPreview"
        opacity: 0.72
        readOnly: true
        renderType: TextEdit.QtRendering
        selectByMouse: false
        selectedTextColor: minimap.textColor
        selectionColor: minimap.viewportColor
        scale: minimap.minimapScale
        text: minimap.documentText
        textFormat: TextEdit.RichText
        transformOrigin: Item.TopLeft
        visible: minimap.documentText.length > 0
        width: minimap.normalizedSourceContentWidth
        wrapMode: TextEdit.Wrap
        x: LV.Theme.gapNone
        y: LV.Theme.gapNone
    }

    Rectangle {
        id: viewportThumb

        border.color: minimap.viewportBorderColor
        border.width: Math.max(1, Math.round(LV.Theme.strokeThin))
        color: minimap.viewportColor
        height: minimap.viewportThumbHeight
        objectName: "contentsMinimapViewportThumb"
        opacity: 0.32
        radius: LV.Theme.gapNone
        visible: minimap.viewportOverlayVisible
        width: minimap.width
        x: LV.Theme.gapNone
        y: minimap.viewportThumbY
    }

    Rectangle {
        color: minimap.scrollbarThumbColor
        height: viewportThumb.height
        opacity: 0.86
        radius: LV.Theme.gapNone
        visible: minimap.viewportOverlayVisible
        width: Math.max(LV.Theme.strokeThin, LV.Theme.gap2)
        x: minimap.width - width
        y: viewportThumb.y
    }

    MouseArea {
        id: minimapPointer

        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true

        onPositionChanged: function(mouse) {
            if (pressed)
                minimap.requestScrollAtMinimapY(mouse.y);
        }

        onPressed: function(mouse) {
            minimap.requestScrollAtMinimapY(mouse.y);
        }
    }
}
