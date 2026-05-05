pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: minimap

    property color lineColor: LV.Theme.captionColor
    property int rowCount: LV.Theme.strokeThin
    property var rowWidthRatios: []
    property bool scrollDragEnabled: true
    property real scrollDragLastY: 0
    property real horizontalPadding: LV.Theme.gap8

    signal scrollDeltaRequested(real deltaY)
    signal viewHookRequested(string reason)

    function beginScrollDrag(localY) {
        if (!minimap.scrollDragEnabled)
            return false;
        minimap.scrollDragLastY = Number(localY) || 0;
        return true;
    }

    function requestScrollDeltaFromY(localY) {
        if (!minimap.scrollDragEnabled)
            return false;
        const currentY = Number(localY) || 0;
        const deltaY = currentY - minimap.scrollDragLastY;
        minimap.scrollDragLastY = currentY;
        if (deltaY !== 0)
            minimap.scrollDeltaRequested(deltaY);
        return true;
    }

    function finishScrollDrag(localY) {
        const updated = minimap.requestScrollDeltaFromY(localY);
        minimap.scrollDragLastY = 0;
        return updated;
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
        const availableWidth = Math.max(0, minimap.width - (minimap.horizontalPadding * 2));
        return Math.max(LV.Theme.strokeThin, availableWidth * safeRatio);
    }

    clip: true
    objectName: "figma-352-8626-Minimap"

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: minimap.horizontalPadding
        anchors.rightMargin: minimap.horizontalPadding
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

    MouseArea {
        id: minimapScrollDragSurface

        acceptedButtons: Qt.LeftButton
        anchors.fill: parent
        cursorShape: Qt.SizeVerCursor
        enabled: minimap.scrollDragEnabled
        objectName: "figma-352-8626-MinimapScrollDragSurface"
        preventStealing: true
        z: 1

        onPositionChanged: function (mouse) {
            if (minimapScrollDragSurface.pressed)
                minimap.requestScrollDeltaFromY(mouse.y);
        }
        onPressed: function (mouse) {
            mouse.accepted = minimap.beginScrollDrag(mouse.y);
        }
        onReleased: function (mouse) {
            minimap.finishScrollDrag(mouse.y);
            mouse.accepted = true;
        }
    }
}
