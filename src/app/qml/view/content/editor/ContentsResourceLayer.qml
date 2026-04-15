pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: resourceLayer

    property var renderedResources: []
    property var sourceOffsetYResolver: null
    property var blockFocusHandler: null
    property color borderColor: LV.Theme.panelBackground08
    property color cardColor: LV.Theme.panelBackground03
    property bool enableCardFocus: true

    function normalizedList(value) {
        if (value === undefined || value === null)
            return [];
        if (Array.isArray(value))
            return value;

        const explicitLength = Number(value.length);
        if (isFinite(explicitLength) && explicitLength >= 0) {
            const normalized = [];
            for (let index = 0; index < Math.floor(explicitLength); ++index)
                normalized.push(value[index]);
            return normalized;
        }

        const explicitCount = Number(value.count);
        if (isFinite(explicitCount) && explicitCount >= 0) {
            const normalized = [];
            for (let index = 0; index < Math.floor(explicitCount); ++index)
                normalized.push(value[index]);
            return normalized;
        }

        const indexedKeys = Object.keys(value).filter(function (key) {
            return /^\d+$/.test(key);
        }).sort(function (lhs, rhs) {
            return Number(lhs) - Number(rhs);
        });
        if (indexedKeys.length > 0) {
            const normalized = [];
            for (let index = 0; index < indexedKeys.length; ++index)
                normalized.push(value[indexedKeys[index]]);
            return normalized;
        }

        return [];
    }
    readonly property var resourceEntries: resourceLayer.normalizedList(renderedResources)
    readonly property int resourceCount: resourceLayer.resourceEntries.length

    function fallbackResourceY(index) {
        return Math.max(0, index) * Math.max(1, LV.Theme.gap20);
    }
    function resourceYForEntry(resourceEntry, index) {
        if (!resourceLayer.sourceOffsetYResolver || typeof resourceLayer.sourceOffsetYResolver !== "function")
            return resourceLayer.fallbackResourceY(index);
        const safeEntry = resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({});
        const sourceStart = Number(safeEntry.sourceStart);
        const resolvedY = Number(resourceLayer.sourceOffsetYResolver(isFinite(sourceStart) ? sourceStart : 0));
        if (!isFinite(resolvedY))
            return resourceLayer.fallbackResourceY(index);
        return Math.max(0, resolvedY);
    }
    readonly property real contentBottomY: {
        let maxBottom = 0;
        if (!resourceRepeater)
            return maxBottom;
        for (let index = 0; index < resourceRepeater.count; ++index) {
            const item = resourceRepeater.itemAt(index);
            if (!item)
                continue;
            const itemY = Number(item.y) || 0;
            const itemHeight = Math.max(0, Number(item.implicitHeight) || Number(item.height) || 0);
            maxBottom = Math.max(maxBottom, itemY + itemHeight);
        }
        return maxBottom;
    }

    implicitHeight: contentBottomY
    height: implicitHeight

    Repeater {
        id: resourceRepeater

        model: resourceLayer.resourceEntries

        delegate: ContentsResourceRenderCard {
            id: resourceCard

            readonly property var normalizedResourceEntry: modelData && typeof modelData === "object" ? modelData : ({})
            readonly property int focusSourceOffset: Math.max(
                                                         0,
                                                         Number(normalizedResourceEntry.focusSourceOffset)
                                                         || Number(normalizedResourceEntry.sourceEnd)
                                                         || 0)

            borderColor: resourceLayer.borderColor
            cardColor: resourceLayer.cardColor
            inlinePresentation: true
            resourceEntry: normalizedResourceEntry
            width: resourceLayer.width
            y: resourceLayer.resourceYForEntry(normalizedResourceEntry, index)
            z: 1

            TapHandler {
                acceptedButtons: Qt.LeftButton

                onTapped: {
                    if (resourceLayer.enableCardFocus
                            && resourceLayer.blockFocusHandler !== undefined
                            && resourceLayer.blockFocusHandler !== null) {
                        resourceLayer.blockFocusHandler(resourceCard.focusSourceOffset);
                    }
                }
            }
        }
    }
}
