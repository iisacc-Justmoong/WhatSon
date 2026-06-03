pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

LV.ContextMenu {
    id: picker

    property string emptyStateText: ""
    property string manualFallbackText: ""
    property var hierarchyItems: []
    property bool manualFallbackEnabled: false
    property var anchorItem: null
    readonly property int popupHorizontalMargin: LV.Theme.gap12
    readonly property int popupVerticalMargin: LV.Theme.gap12
    readonly property int popupRowHeight: LV.Theme.gap20
    readonly property int popupSpacing: LV.Theme.gap2
    readonly property int popupWidth: LV.Theme.inputWidthMd + LV.Theme.gap18
    readonly property int popupViewportHeight: {
        const rowCount = hierarchyItems.length + (manualFallbackEnabled ? 1 : 0);
        const minimumHeight = popupRowHeight;
        const contentHeight = rowCount > 0 ? rowCount * popupRowHeight + Math.max(0, rowCount - 1) * popupSpacing : minimumHeight;
        const overlayHeight = parent ? parent.height : contentHeight + topPadding + bottomPadding;
        const availableHeight = Math.max(minimumHeight, overlayHeight - popupVerticalMargin * 2 - topPadding - bottomPadding);
        return Math.max(minimumHeight, Math.min(contentHeight, availableHeight));
    }
    readonly property int popupHeight: popupViewportHeight + topPadding + bottomPadding
    readonly property int popupContentWidth: explicitContentWidth > 0 ? explicitContentWidth : Math.max(0, popupWidth - leftPadding - rightPadding)

    signal entryChosen(var entry)
    signal manualFallbackRequested
    signal viewHookRequested(string reason)

    function clampPosition(value, minimum, maximum) {
        return Math.max(minimum, Math.min(maximum, value));
    }

    function syncPopupSize() {
        const nextWidth = popupWidth;
        const nextHeight = popupHeight;
        requestedPopupWidth = nextWidth;
        if (Math.abs(Number(width) - nextWidth) >= 0.01)
            width = nextWidth;
        if (Math.abs(Number(height) - nextHeight) >= 0.01)
            height = nextHeight;
    }

    function openForAnchor(item, reason) {
        anchorItem = item ? item : null;
        if (reason !== undefined)
            viewHookRequested(String(reason));

        syncPopupSize();
        const target = anchorItem && anchorItem.mapToItem !== undefined ? anchorItem : null;
        if (target) {
            picker.openFor(target, Number(target.width) || 0, (Number(target.height) || 0) + LV.Theme.gap2);
            return;
        }

        picker.openAt(popupHorizontalMargin, popupVerticalMargin);
    }

    autoCloseOnTrigger: false
    closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
    focus: true
    height: popupHeight
    items: []
    modal: false
    padding: LV.Theme.gap4
    parent: Controls.Overlay.overlay
    requestedPopupWidth: popupWidth
    width: popupWidth

    onClosed: anchorItem = null
    onPopupHeightChanged: {
        syncPopupSize();
    }
    onPopupWidthChanged: {
        syncPopupSize();
    }

    contentItem: Item {
        implicitWidth: picker.popupContentWidth
        implicitHeight: picker.popupViewportHeight

        Flickable {
            id: listViewport

            anchors.fill: parent
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            contentHeight: listColumn.implicitHeight
            contentWidth: width
            flickableDirection: Flickable.VerticalFlick
            interactive: contentHeight > height

            Column {
                id: listColumn

                width: listViewport.width
                spacing: picker.popupSpacing

                Repeater {
                    model: picker.hierarchyItems

                    delegate: LV.HierarchyItem {
                        required property var modelData
                        readonly property var entry: modelData

                        activatable: true
                        count: entry.count !== undefined && Number(entry.count) > 0 ? Number(entry.count) : -1
                        generatedByTreeModel: false
                        hasChildItems: false
                        iconName: entry.iconName !== undefined && entry.iconName !== null ? String(entry.iconName) : ""
                        iconSource: entry.iconSource !== undefined && entry.iconSource !== null ? entry.iconSource : ""
                        indentLevel: entry.depth !== undefined ? Number(entry.depth) || 0 : 0
                        itemWidth: listColumn.width
                        label: entry.label !== undefined && entry.label !== null ? String(entry.label) : ""
                        rowHeight: picker.popupRowHeight
                        showChevron: false
                        textColorNormal: LV.Theme.bodyColor

                        onClicked: {
                            picker.entryChosen(entry);
                            picker.close();
                        }
                    }
                }

                LV.HierarchyItem {
                    id: manualFallbackRow

                    activatable: true
                    generatedByTreeModel: false
                    hasChildItems: false
                    iconName: "nodesnewFolder"
                    itemWidth: listColumn.width
                    label: picker.manualFallbackText
                    rowHeight: picker.popupRowHeight
                    showChevron: false
                    textColorNormal: LV.Theme.bodyColor
                    visible: picker.manualFallbackEnabled

                    onClicked: {
                        picker.manualFallbackRequested();
                        picker.close();
                    }
                }
            }

            Controls.ScrollBar.vertical: Controls.ScrollBar {
                policy: Controls.ScrollBar.AsNeeded
            }
        }

        LV.Label {
            anchors.centerIn: parent
            color: LV.Theme.captionColor
            style: caption
            text: picker.emptyStateText
            visible: picker.hierarchyItems.length === 0 && !picker.manualFallbackEnabled
        }
    }
}
