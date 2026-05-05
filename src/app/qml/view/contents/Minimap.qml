pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: minimap

    property color lineColor: LV.Theme.captionColor
    property int rowCount: LV.Theme.strokeThin
    property var rowWidthRatios: []
    property bool scrollDragEnabled: true
    property real scrollPositionRatio: 0
    property real viewportRatio: 1

    signal scrollRatioRequested(real ratio)
    signal viewHookRequested(string reason)

    function clampedRatio(value) {
        const ratio = Number(value);
        if (!isFinite(ratio))
            return 0;
        return Math.max(0, Math.min(1, ratio));
    }

    function requestScrollRatioFromY(localY) {
        if (!minimap.scrollDragEnabled || minimap.height <= LV.Theme.strokeThin)
            return false;
        const ratio = minimap.clampedRatio((Number(localY) || 0) / minimap.height);
        minimap.scrollRatioRequested(ratio);
        return true;
    }

    function resolvedViewportIndicatorHeight() {
        const ratio = minimap.clampedRatio(minimap.viewportRatio);
        if (ratio >= 1)
            return minimap.height;
        return Math.max(LV.Theme.gap8, minimap.height * ratio);
    }

    function resolvedViewportIndicatorY() {
        const indicatorHeight = minimap.resolvedViewportIndicatorHeight();
        const travelRange = Math.max(0, minimap.height - indicatorHeight);
        return travelRange * minimap.clampedRatio(minimap.scrollPositionRatio);
    }

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        minimap.viewHookRequested(hookReason);
    }

    function resolvedRowWidth(index) {
        const ratios = minimap.rowWidthRatios || [];
        const ratio = ratios.length !== undefined && index >= 0 && index < ratios.length
                ? Number(ratios[index])
                : 1;
        const safeRatio = isFinite(ratio) && ratio > 0
                ? Math.max(0, Math.min(1, ratio))
                : 1;
        return Math.max(LV.Theme.strokeThin, minimap.width * safeRatio);
    }

    clip: true
    objectName: "figma-352-8626-Minimap"

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: LV.Theme.strokeThin

        Repeater {
            model: minimap.rowCount

            Rectangle {
                required property int index

                color: minimap.lineColor
                height: LV.Theme.strokeThin
                width: minimap.resolvedRowWidth(index)
            }
        }
    }

    Rectangle {
        id: minimapViewportIndicator

        anchors.left: parent.left
        anchors.right: parent.right
        color: minimap.lineColor
        height: minimap.resolvedViewportIndicatorHeight()
        objectName: "figma-352-8626-MinimapViewportIndicator"
        opacity: 0.18
        visible: minimap.scrollDragEnabled && minimap.viewportRatio < 1
        y: minimap.resolvedViewportIndicatorY()
        z: 1
    }

    MouseArea {
        id: minimapScrollDragSurface

        acceptedButtons: Qt.LeftButton
        anchors.fill: parent
        cursorShape: Qt.SizeVerCursor
        enabled: minimap.scrollDragEnabled
        objectName: "figma-352-8626-MinimapScrollDragSurface"
        preventStealing: true
        z: 2

        onPositionChanged: function (mouse) {
            if (pressed)
                minimap.requestScrollRatioFromY(mouse.y);
        }
        onPressed: function (mouse) {
            mouse.accepted = minimap.requestScrollRatioFromY(mouse.y);
        }
        onReleased: function (mouse) {
            minimap.requestScrollRatioFromY(mouse.y);
            mouse.accepted = true;
        }
    }
}
