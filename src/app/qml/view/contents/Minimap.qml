pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: minimap

    property int rowCount: 0
    property var rowWidthRatios: []
    property color rowColor: LV.Theme.captionColor
    property real scrollProgress: 0
    property color viewportColor: LV.Theme.strokeSoft
    property real viewportRatio: 1

    clip: true
    implicitWidth: LV.Theme.gap20
    objectName: "contentsMinimap"

    function rowWidthRatio(rowIndex) {
        if (!rowWidthRatios || rowIndex < 0 || rowIndex >= rowWidthRatios.length)
            return 1;
        const ratio = Number(rowWidthRatios[rowIndex]);
        return isFinite(ratio) ? Math.max(0, Math.min(1, ratio)) : 1;
    }

    Repeater {
        model: Math.max(0, minimap.rowCount)

        delegate: Rectangle {
            required property int index

            color: minimap.rowColor
            height: Math.max(1, Math.round(LV.Theme.strokeThin))
            opacity: 0.48
            width: Math.max(Math.round(LV.Theme.strokeThin), minimap.width * minimap.rowWidthRatio(index))
            x: minimap.width - width
            y: index * (height + LV.Theme.gap2)
        }
    }

    Rectangle {
        color: minimap.viewportColor
        height: Math.max(LV.Theme.gap12, minimap.height * Math.max(0, Math.min(1, minimap.viewportRatio)))
        opacity: 0.36
        radius: Math.max(1, Math.round(LV.Theme.strokeThin))
        width: minimap.width
        y: Math.max(0, Math.min(minimap.height - height, minimap.scrollProgress * (minimap.height - height)))
    }
}
