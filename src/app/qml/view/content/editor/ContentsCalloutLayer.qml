pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: calloutLayer

    property var renderedCallouts: []
    property var sourceOffsetYResolver: null
    property var blockFocusHandler: null
    readonly property color calloutColor: "#262728"
    readonly property color dividerColor: "#D9D9D9"
    readonly property color textColor: "#FFFFFF"
    readonly property int dividerMinimumHeight: 14
    readonly property int dividerWidth: 1
    readonly property int framePadding: 4
    readonly property int frameSpacing: 12
    function normalizedList(value) {
        if (value === undefined || value === null)
            return [];
        if (Array.isArray(value))
            return value;
        const length = Number(value.length);
        if (isFinite(length) && length >= 0) {
            const normalized = [];
            for (let index = 0; index < Math.floor(length); ++index)
                normalized.push(value[index]);
            return normalized;
        }
        return [];
    }
    readonly property var calloutEntries: calloutLayer.normalizedList(renderedCallouts)
    readonly property int calloutCount: calloutLayer.calloutEntries.length

    function fallbackCalloutY(index) {
        return Math.max(0, index) * Math.max(1, LV.Theme.gap20);
    }
    function calloutYForEntry(calloutEntry, index) {
        if (!calloutLayer.sourceOffsetYResolver || typeof calloutLayer.sourceOffsetYResolver !== "function")
            return calloutLayer.fallbackCalloutY(index);
        const safeEntry = calloutEntry && typeof calloutEntry === "object" ? calloutEntry : ({});
        const sourceStart = Number(safeEntry.sourceStart);
        const resolvedY = Number(calloutLayer.sourceOffsetYResolver(isFinite(sourceStart) ? sourceStart : 0));
        if (!isFinite(resolvedY))
            return calloutLayer.fallbackCalloutY(index);
        return Math.max(0, resolvedY);
    }
    readonly property real contentBottomY: {
        let maxBottom = 0;
        if (!calloutRepeater)
            return maxBottom;
        for (let index = 0; index < calloutRepeater.count; ++index) {
            const item = calloutRepeater.itemAt(index);
            if (!item)
                continue;
            const itemY = Number(item.y) || 0;
            const itemHeight = Math.max(0, Number(item.implicitHeight) || Number(item.height) || 0);
            maxBottom = Math.max(maxBottom, itemY + itemHeight);
        }
        return maxBottom;
    }

    implicitHeight: contentBottomY

    Repeater {
        id: calloutRepeater

        model: calloutLayer.calloutEntries

        delegate: Rectangle {
            id: calloutFrame

            readonly property var calloutEntry: modelData && typeof modelData === "object" ? modelData : ({})
            readonly property string calloutText: calloutEntry.text !== undefined ? String(calloutEntry.text) : ""
            readonly property int focusSourceOffset: Math.max(0, Number(calloutEntry.focusSourceOffset) || 0)

            color: calloutLayer.calloutColor
            implicitHeight: Math.max(calloutLayer.dividerMinimumHeight, calloutTextLabel.implicitHeight)
                            + calloutLayer.framePadding * 2
            width: calloutLayer.width
            y: calloutLayer.calloutYForEntry(calloutFrame.calloutEntry, index)
            z: 1

            Rectangle {
                id: calloutDivider

                anchors.left: parent.left
                anchors.leftMargin: calloutLayer.framePadding
                anchors.top: parent.top
                anchors.topMargin: calloutLayer.framePadding
                color: calloutLayer.dividerColor
                height: Math.max(calloutLayer.dividerMinimumHeight, calloutTextLabel.implicitHeight)
                radius: 1
                width: calloutLayer.dividerWidth
            }
            LV.Label {
                id: calloutTextLabel

                anchors.left: calloutDivider.right
                anchors.leftMargin: calloutLayer.frameSpacing
                anchors.right: parent.right
                anchors.rightMargin: calloutLayer.framePadding
                anchors.top: parent.top
                anchors.topMargin: calloutLayer.framePadding
                color: calloutLayer.textColor
                font.family: "Pretendard"
                font.pixelSize: 12
                font.weight: Font.Medium
                lineHeight: 1.0
                style: body
                text: calloutFrame.calloutText
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton

                onTapped: {
                    if (calloutLayer.blockFocusHandler !== undefined
                            && calloutLayer.blockFocusHandler !== null) {
                        calloutLayer.blockFocusHandler(calloutFrame.focusSourceOffset);
                    }
                }
            }
        }
    }
}
