pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: minimap

    property color lineColor: LV.Theme.captionColor
    property int rowCount: LV.Theme.strokeThin

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        minimap.viewHookRequested(hookReason);
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
                width: parent.width
            }
        }
    }
}
