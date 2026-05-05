pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Window
import LVRS 1.0 as LV
import ".." as PanelView

Rectangle {
    id: sidebarHierarchyView

    readonly property var activeHierarchyItem: hierarchyTree.activeListItem
    readonly property var bookmarkPaletteCanvasItem: bookmarkPaletteIconOverlay
    readonly property var hierarchyRenameFieldItem: hierarchyRenameField
    readonly property var hierarchyTreeItem: hierarchyTree
    readonly property var activeHierarchyItemRect: {
        const item = sidebarHierarchyView.activeHierarchyItem;
        if (!item || item.mapToItem === undefined)
            return ({
                    "height": 0,
                    "width": 0,
                    "x": 0,
                    "y": 0
                });
        const point = item.mapToItem(sidebarHierarchyView, 0, 0);
        return ({
                "height": Number(item.height) || 0,
                "width": Number(item.width) || 0,
                "x": Number(point.x) || 0,
                "y": Number(point.y) || 0
            });
    }
    property int activeToolbarIndex: defaultToolbarIndex
    property bool bookmarkPaletteVisualsEnabled: false
    readonly property bool createFolderEnabled: hierarchyInteractionBridge ? Boolean(hierarchyInteractionBridge.createFolderEnabled) : false
    property int defaultToolbarIndex: 0
    readonly property bool deleteFolderEnabled: hierarchyInteractionBridge ? Boolean(hierarchyInteractionBridge.deleteFolderEnabled) : false
    property var displayedHierarchyModel: []
    property string displayedHierarchyModelSignature: "[]"
    property int editingHierarchyIndex: -1
    readonly property var editingHierarchyItem: sidebarHierarchyView.resolveVisibleHierarchyItem(sidebarHierarchyView.editingHierarchyIndex)
    readonly property var editingHierarchyItemPresentation: {
        const snapshot = sidebarHierarchyView.editingHierarchyPresentation;
        const editingIndex = sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.editingHierarchyIndex, -1);
        if (editingIndex >= 0 && snapshot && sidebarHierarchyView.normalizedInteger(snapshot.index, -1) === editingIndex && (Number(snapshot.width) || 0) > 0 && (Number(snapshot.height) || 0) > 0) {
            return snapshot;
        }
        return sidebarHierarchyView.hierarchyItemPresentation(sidebarHierarchyView.editingHierarchyItem, editingIndex);
    }
    readonly property var editingHierarchyItemRect: {
        const presentation = sidebarHierarchyView.editingHierarchyItemPresentation;
        return ({
                "height": Number(presentation.height) || 0,
                "width": Number(presentation.width) || 0,
                "x": Number(presentation.x) || 0,
                "y": Number(presentation.y) || 0
            });
    }
    property string editingHierarchyLabel: ""
    property var editingHierarchyPresentation: ({
            "chevronSize": 0,
            "effectiveShowChevron": false,
            "height": 0,
            "iconSize": 0,
            "index": -1,
            "leadingSpacing": 0,
            "leftPadding": 0,
            "rightPadding": 0,
            "rowBackgroundColor": LV.Theme.accentBlueMuted,
            "rowHeight": 0,
            "width": 0,
            "x": 0,
            "y": 0
        })
    property bool footerVisible: true
    property int hierarchyActivationPendingSerial: 0
    readonly property int hierarchyCompactFooterHeight: LV.Theme.gap24
    readonly property int hierarchyCompactFooterWidth: LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap6
    readonly property int hierarchyCompactInset: LV.Theme.gap2
    readonly property int hierarchyCompactMenuItemWidth: LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap24 + LV.Theme.gap24
    readonly property bool hierarchyBulkExpansionEnabled: {
        if (!sidebarHierarchyView.hierarchyInteractionBridge)
            return false;
        const nodes = sidebarHierarchyView.standardHierarchyModel;
        for (let index = 0; index < nodes.length; ++index) {
            const node = nodes[index];
            if (node && node.showChevron !== undefined && Boolean(node.showChevron))
                return true;
        }
        return false;
    }
    readonly property bool hierarchyKineticViewportEnabled: LV.Theme.mobileTarget || (Window.window && Window.window.isMobilePlatform !== undefined ? Boolean(Window.window.isMobilePlatform) : false)
    readonly property int hierarchyListFlickDeceleration: hierarchyKineticViewportEnabled ? Math.max(1, Math.round(LV.Theme.scaleMetric(1800))) : Math.max(1, Math.round(LV.Theme.scaleMetric(3200)))
    readonly property int hierarchyListMaximumFlickVelocity: hierarchyKineticViewportEnabled ? Math.max(1, Math.round(LV.Theme.scaleMetric(12000))) : Math.max(1, Math.round(LV.Theme.scaleMetric(8000)))
    readonly property int hierarchyListReboundDuration: hierarchyKineticViewportEnabled ? 220 : 160
    property string hierarchyContextMenuKind: "options"
    readonly property var hierarchyContextMenuItems: sidebarHierarchyView.hierarchyContextMenuKind === "folder" ? sidebarHierarchyView.hierarchyFolderContextMenuItems : sidebarHierarchyView.hierarchyTreeContextMenuItems
    property int hierarchyContextMenuTargetIndex: -1
    readonly property bool hierarchyContextMenuTargetValid: sidebarHierarchyView.isFolderContextMenuTargetIndex(sidebarHierarchyView.hierarchyContextMenuTargetIndex)
    readonly property var hierarchyFolderContextMenuItems: [
        {
            "label": "New Folder",
            "iconName": "generaladd",
            "eventName": "hierarchy.folder.create",
            "enabled": sidebarHierarchyView.hierarchyContextMenuTargetValid && sidebarHierarchyView.createFolderEnabled,
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Delete Folder",
            "iconName": "generaldelete",
            "eventName": "hierarchy.folder.delete",
            "enabled": sidebarHierarchyView.hierarchyContextMenuTargetValid && sidebarHierarchyView.deleteFolderEnabled,
            "keyVisible": false,
            "showChevron": false
        }
    ]
    property var hierarchyDragDropBridge: null
    property bool hierarchyEditable: false
    property bool hierarchyExpansionActivationSuppressed: false
    property string hierarchyExpansionPointerArmedKey: ""
    property var hierarchyExpansionStateByKey: ({})
    property var hierarchyInteractionBridge: null
    readonly property var hierarchyFooterToolbarButtons: [
        {
            type: "icon",
            iconName: "generaladd",
            backgroundColor: LV.Theme.accentTransparent,
            backgroundColorDisabled: LV.Theme.accentTransparent,
            backgroundColorHover: LV.Theme.accentTransparent,
            backgroundColorPressed: LV.Theme.accentTransparent,
            enabled: sidebarHierarchyView.createFolderEnabled,
            eventName: "hierarchy.footer.create",
            horizontalPadding: sidebarHierarchyView.hierarchyCompactInset,
            verticalPadding: sidebarHierarchyView.hierarchyCompactInset
        },
        {
            type: "icon",
            iconName: "generaldelete",
            backgroundColor: LV.Theme.accentTransparent,
            backgroundColorDisabled: LV.Theme.accentTransparent,
            backgroundColorHover: LV.Theme.accentTransparent,
            backgroundColorPressed: LV.Theme.accentTransparent,
            enabled: sidebarHierarchyView.deleteFolderEnabled,
            eventName: "hierarchy.footer.delete",
            horizontalPadding: sidebarHierarchyView.hierarchyCompactInset,
            verticalPadding: sidebarHierarchyView.hierarchyCompactInset
        },
        {
            type: "menu",
            iconName: "generalmoreHorizontal",
            backgroundColor: LV.Theme.accentTransparent,
            backgroundColorDisabled: LV.Theme.accentTransparent,
            backgroundColorHover: LV.Theme.accentTransparent,
            backgroundColorPressed: LV.Theme.accentTransparent,
            enabled: sidebarHierarchyView.viewOptionsEnabled,
            eventName: "hierarchy.footer.options",
            leftPadding: sidebarHierarchyView.hierarchyCompactInset,
            rightPadding: LV.Theme.gap4,
            topPadding: sidebarHierarchyView.hierarchyCompactInset,
            bottomPadding: sidebarHierarchyView.hierarchyCompactInset
        }
    ]
    readonly property color hierarchyNoteDropHoverColor: {
        const item = sidebarHierarchyView.noteDropHoverItem;
        if (item && item.rowBackgroundColorActive !== undefined)
            return item.rowBackgroundColorActive;
        return LV.Theme.accentBlueMuted;
    }
    readonly property var hierarchyNoteDropHoverItemRect: {
        const item = sidebarHierarchyView.noteDropHoverItem;
        if (!item || item.mapToItem === undefined)
            return ({
                    "height": 0,
                    "width": 0,
                    "x": 0,
                    "y": 0
                });
        const point = item.mapToItem(sidebarHierarchyView, 0, 0);
        return ({
                "height": Number(item.height) || 0,
                "width": Number(item.width) || 0,
                "x": Number(point.x) || 0,
                "y": Number(point.y) || 0
            });
    }
    readonly property real hierarchyNoteDropHoverRadius: {
        const item = sidebarHierarchyView.noteDropHoverItem;
        if (item && item.resolvedCornerRadius !== undefined)
            return Number(item.resolvedCornerRadius) || 0;
        return LV.Theme.radiusControl;
    }
    property alias hierarchyPointerSelectionModifiers: hierarchySelectionController.pointerSelectionModifiers
    property alias hierarchyPointerSelectionModifiersCapturedAtMs: hierarchySelectionController.pointerSelectionModifiersCapturedAtMs
    readonly property bool hierarchyNoteDropSurfaceEnabled: sidebarHierarchyView.hierarchyDragDropBridge !== undefined && sidebarHierarchyView.hierarchyDragDropBridge !== null
    readonly property color hierarchyRenameFieldBackgroundColor: {
        const presentation = sidebarHierarchyView.editingHierarchyItemPresentation;
        if (presentation && presentation.rowBackgroundColor !== undefined)
            return presentation.rowBackgroundColor;
        return LV.Theme.accentBlueMuted;
    }
    readonly property real hierarchyRenameFieldHeight: {
        const presentation = sidebarHierarchyView.editingHierarchyItemPresentation;
        return Math.max(16, Number(presentation.rowHeight) || Number(presentation.height) || 20);
    }
    readonly property real hierarchyRenameFieldWidth: {
        const presentation = sidebarHierarchyView.editingHierarchyItemPresentation;
        const itemWidth = Number(presentation.width) || 0;
        if (itemWidth <= 0)
            return 0;
        const leadingSpacing = Number(presentation.leadingSpacing) || 0;
        const chevronWidth = Boolean(presentation.effectiveShowChevron) ? (Number(presentation.chevronSize) || 0) + leadingSpacing : 0;
        const usedWidth = (Number(presentation.leftPadding) || 0) + (Number(presentation.rightPadding) || 0) + (Number(presentation.iconSize) || 0) + leadingSpacing + chevronWidth;
        return Math.max(0, itemWidth - usedWidth);
    }
    readonly property real hierarchyRenameFieldX: {
        const presentation = sidebarHierarchyView.editingHierarchyItemPresentation;
        return (Number(presentation.x) || 0) + (Number(presentation.leftPadding) || 0) + (Number(presentation.iconSize) || 0) + (Number(presentation.leadingSpacing) || 0);
    }
    readonly property real hierarchyRenameFieldY: {
        const rectY = Number(sidebarHierarchyView.editingHierarchyItemRect.y) || 0;
        const rectHeight = Number(sidebarHierarchyView.editingHierarchyItemRect.height) || 0;
        return rectY + Math.max(0, Math.floor((rectHeight - sidebarHierarchyView.hierarchyRenameFieldHeight) * 0.5));
    }
    property alias hierarchySelectionAnchorIndex: hierarchySelectionController.selectionAnchorIndex
    property int hierarchySelectionVisualRevision: 0
    property var hierarchyController: null
    property string hierarchyViewOptionsTriggerQueuedAction: ""
    readonly property var hierarchyTreeContextMenuItems: [
        {
            "label": "Expand All",
            "iconName": "generalshow",
            "eventName": "hierarchy.expandAll",
            "enabled": sidebarHierarchyView.hierarchyBulkExpansionEnabled,
            "keyVisible": false,
            "onTriggered": function () {
                sidebarHierarchyView.handleHierarchyViewOptionsTrigger(0, "hierarchy.expandAll");
            },
            "showChevron": false
        },
        {
            "label": "Collapse All",
            "eventName": "hierarchy.collapseAll",
            "enabled": sidebarHierarchyView.hierarchyBulkExpansionEnabled,
            "keyVisible": false,
            "onTriggered": function () {
                sidebarHierarchyView.handleHierarchyViewOptionsTrigger(1, "hierarchy.collapseAll");
            },
            "showChevron": false
        }
    ]
    property int horizontalInset: LV.Theme.gap2
    property int noteDropHoverIndex: -1
    readonly property var noteDropHoverItem: sidebarHierarchyView.hierarchyItemForResolvedIndex(sidebarHierarchyView.noteDropHoverIndex)
    readonly property bool noteDropHoverVisible: sidebarHierarchyView.noteDropHoverIndex >= 0 && !!sidebarHierarchyView.noteDropHoverItem
    property color panelColor: "transparent"
    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("sidebar.SidebarHierarchyView") : null
    readonly property bool renameContractAvailable: hierarchyInteractionBridge ? Boolean(hierarchyInteractionBridge.renameContractAvailable) : false
    readonly property bool renameEditingActive: sidebarHierarchyView.editingHierarchyIndex >= 0
    readonly property bool renamePresentationAvailable: sidebarHierarchyView.hierarchyRenameFieldWidth > 0 && (Number(sidebarHierarchyView.editingHierarchyItemRect.height) || 0) > 0
    property color searchFieldBackgroundColor: "transparent"
    property bool searchFieldVisible: false
    property int searchHeaderMinHeight: LV.Theme.gap24
    property int searchHeaderTopGap: LV.Theme.gap4
    property int searchHeaderVerticalInset: LV.Theme.gap2
    property int searchListGap: LV.Theme.gapNone
    property string searchText: ""
    readonly property int selectedFolderIndex: hierarchyController ? hierarchyController.hierarchySelectedIndex : -1
    property alias selectedHierarchyIndices: hierarchySelectionController.selectedIndices
    readonly property var safeSelectedHierarchyIndices: Array.isArray(sidebarHierarchyView.selectedHierarchyIndices) ? sidebarHierarchyView.selectedHierarchyIndices : []
    readonly property var selectedHierarchyOverlayRects: {
        const revision = sidebarHierarchyView.hierarchySelectionVisualRevision;
        return sidebarHierarchyView.collectSelectedHierarchyOverlayRects() || [];
    }
    readonly property var standardHierarchyModel: sidebarHierarchyView.displayedHierarchyModel
    readonly property int toolbarButtonSize: LV.Theme.gap20
    readonly property real toolbarButtonSpacing: sidebarHierarchyView.toolbarItems.length > 1 ? (sidebarHierarchyView.toolbarFrameWidth - sidebarHierarchyView.toolbarButtonSize * sidebarHierarchyView.toolbarItems.length) / (sidebarHierarchyView.toolbarItems.length - 1) : 0
    property int toolbarFrameWidth: 200
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property var toolbarItems: {
        if (sidebarHierarchyView.hierarchyController && sidebarHierarchyView.hierarchyController.toolbarItems !== undefined)
            return sidebarHierarchyView.hierarchyController.toolbarItems;
        var items = [];
        for (var i = 0; i < toolbarIconNames.length; ++i) {
            items.push({
                "id": i,
                "iconName": toolbarIconNames[i],
                "selected": i === activeToolbarIndex
            });
        }
        return items;
    }
    property int verticalInset: LV.Theme.gap2
    readonly property bool viewOptionsEnabled: hierarchyInteractionBridge ? Boolean(hierarchyInteractionBridge.viewOptionsEnabled) : true

    signal hierarchyItemActivated(var item, int itemId, int index)
    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal toolbarIndexChangeRequested(int index)
    signal viewHookRequested

    function applyBookmarkPaletteVisuals() {
        bookmarkPaletteController.applyBookmarkPaletteVisuals();
    }
    function armHierarchyExpansionActivationSuppression(item, itemId, index) {
        // Expansion originated from the chevron slot; invalidate any pending activation callback
        // from the same pointer transaction before it can route to folder activation.
        sidebarHierarchyView.hierarchyActivationPendingSerial += 1;
        sidebarHierarchyView.hierarchyExpansionActivationSuppressed = true;
        hierarchyExpansionActivationBlockTimer.restart();
    }
    function beginRenameSelectedHierarchyItem() {
        return renameController.beginRenameSelectedHierarchyItem();
    }
    function bookmarkPaletteColorForLabel(label) {
        return bookmarkPaletteController.bookmarkPaletteColorForLabel(label);
    }
    function bookmarkPaletteColorTokenForLabel(label) {
        return bookmarkPaletteController.bookmarkPaletteColorTokenForLabel(label);
    }
    function canAcceptNoteDropAtPosition(x, y, noteIds, referenceItem) {
        return noteDropController.canAcceptNoteDropAtPosition(x, y, noteIds, referenceItem);
    }
    function canRenameIndex(index) {
        return renameController.canRenameIndex(index);
    }
    function canRenameSelectedHierarchyItem() {
        return renameController.canRenameSelectedHierarchyItem();
    }
    function cancelHierarchyRename() {
        return renameController.cancelHierarchyRename();
    }
    function captureHierarchyPointerSelectionModifiers(modifiers) {
        hierarchySelectionController.captureHierarchyPointerSelectionModifiers(modifiers);
    }
    function clearEditingHierarchyPresentation() {
        sidebarHierarchyView.editingHierarchyPresentation = ({
                "chevronSize": 0,
                "effectiveShowChevron": false,
                "height": 0,
                "iconSize": 0,
                "index": -1,
                "leadingSpacing": 0,
                "leftPadding": 0,
                "rightPadding": 0,
                "rowBackgroundColor": LV.Theme.accentBlueMuted,
                "rowHeight": 0,
                "width": 0,
                "x": 0,
                "y": 0
            });
    }
    function clearHierarchyPointerSelectionModifiers() {
        hierarchySelectionController.clearHierarchyPointerSelectionModifiers();
    }
    function clearNoteDropPreview() {
        noteDropController.clearNoteDropPreview();
    }
    function collectHierarchyItems() {
        return noteDropController.collectHierarchyItems();
    }
    function collectSelectedHierarchyOverlayRects() {
        const selectedIndices = sidebarHierarchyView.normalizeHierarchySelectionIndices(sidebarHierarchyView.selectedHierarchyIndices);
        const primaryIndex = sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.selectedFolderIndex, -1);
        const overlayRects = [];
        for (let index = 0; index < selectedIndices.length; ++index) {
            const resolvedIndex = selectedIndices[index];
            if (resolvedIndex < 0 || resolvedIndex === primaryIndex)
                continue;
            const hierarchyItem = sidebarHierarchyView.resolveVisibleHierarchyItem(resolvedIndex);
            if (!hierarchyItem || hierarchyItem.mapToItem === undefined)
                continue;
            const point = hierarchyItem.mapToItem(hierarchyTree, 0, 0);
            const width = Number(hierarchyItem.width) || 0;
            const height = Number(hierarchyItem.height) || 0;
            if (width <= 0 || height <= 0)
                continue;
            overlayRects.push({
                "height": height,
                "radius": Number(hierarchyItem.resolvedCornerRadius) || LV.Theme.radiusControl,
                "width": width,
                "x": Number(point.x) || 0,
                "y": Number(point.y) || 0
            });
        }
        return overlayRects;
    }
    function commitHierarchyRename() {
        return renameController.commitHierarchyRename();
    }
    function commitNoteDropAtPosition(x, y, noteIds, referenceItem) {
        return noteDropController.commitNoteDropAtPosition(x, y, noteIds, referenceItem);
    }
    function drawBookmarkGlyph(context, x, y, size, color) {
        bookmarkPaletteController.drawBookmarkGlyph(context, x, y, size, color);
    }
    function emitHierarchySelectionActivation(item, resolvedIndex) {
        hierarchySelectionController.emitHierarchySelectionActivation(item, resolvedIndex);
    }
    function handleHierarchyViewOptionsTrigger(index, eventName) {
        sidebarHierarchyView.requestHierarchyViewOptionsAction(index, eventName);
    }
    function handleHierarchyFooterButtonClicked(index, config) {
        const normalizedEventName = config && config.eventName !== undefined && config.eventName !== null ? String(config.eventName).trim() : "";
        const normalizedIndex = sidebarHierarchyView.normalizedInteger(index, -1);
        if (normalizedEventName === "hierarchy.footer.create" || normalizedIndex === 0) {
            sidebarHierarchyView.requestCreateFolder();
            return;
        }
        if (normalizedEventName === "hierarchy.footer.delete" || normalizedIndex === 1) {
            sidebarHierarchyView.requestDeleteFolder();
            return;
        }
        if (normalizedEventName === "hierarchy.footer.options" || normalizedIndex === 2)
            sidebarHierarchyView.requestViewOptions();
    }
    function hierarchyViewOptionsActionName(index, eventName) {
        const normalizedEventName = eventName === undefined || eventName === null ? "" : String(eventName).trim();
        if (normalizedEventName === "hierarchy.expandAll" || normalizedEventName === "hierarchy.collapseAll")
            return normalizedEventName;
        const normalizedIndex = sidebarHierarchyView.normalizedInteger(index, -1);
        if (normalizedIndex === 0)
            return "hierarchy.expandAll";
        if (normalizedIndex === 1)
            return "hierarchy.collapseAll";
        return "";
    }
    function hierarchyItemAtPosition(x, y) {
        return noteDropController.hierarchyItemAtPosition(x, y);
    }
    function handleHierarchyContextMenuTrigger(index, eventName) {
        const normalizedEventName = eventName === undefined || eventName === null ? "" : String(eventName).trim();
        const normalizedIndex = Number(index);
        if (sidebarHierarchyView.hierarchyContextMenuKind === "folder") {
            const targetIndex = sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.hierarchyContextMenuTargetIndex, -1);
            if (normalizedEventName === "hierarchy.folder.create" || normalizedIndex === 0)
                sidebarHierarchyView.requestCreateFolder(targetIndex, "hierarchy.contextMenu.create");
            else if (normalizedEventName === "hierarchy.folder.delete" || normalizedIndex === 1)
                sidebarHierarchyView.requestDeleteFolder(targetIndex, "hierarchy.contextMenu.delete");
            sidebarHierarchyView.hierarchyContextMenuTargetIndex = -1;
            sidebarHierarchyView.hierarchyContextMenuKind = "options";
            return;
        }
        sidebarHierarchyView.handleHierarchyViewOptionsTrigger(index, eventName);
    }
    function hierarchyItemContainsPoint(item, x, y) {
        return noteDropController.hierarchyItemContainsPoint(item, x, y);
    }
    function hierarchyItemForResolvedIndex(itemId) {
        return noteDropController.hierarchyItemForResolvedIndex(itemId);
    }
    function hierarchyExpansionStateScopeKey() {
        return "hierarchy:" + String(sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.activeToolbarIndex, sidebarHierarchyView.defaultToolbarIndex));
    }
    function hierarchyScopedExpansionKey(key) {
        const normalizedKey = key === undefined || key === null ? "" : String(key).trim();
        if (!normalizedKey.length)
            return "";
        return sidebarHierarchyView.hierarchyExpansionStateScopeKey() + ":" + normalizedKey;
    }
    function hierarchyItemExpansionKey(item, fallbackIndex) {
        if (!item)
            return "";
        const itemKey = item.itemKey !== undefined && item.itemKey !== null ? String(item.itemKey).trim() : "";
        if (itemKey.length > 0)
            return sidebarHierarchyView.hierarchyScopedExpansionKey(itemKey);
        const key = item.key !== undefined && item.key !== null ? String(item.key).trim() : "";
        if (key.length > 0)
            return sidebarHierarchyView.hierarchyScopedExpansionKey(key);
        const resolvedItemKey = item.resolvedItemKey !== undefined && item.resolvedItemKey !== null ? String(item.resolvedItemKey).trim() : "";
        if (resolvedItemKey.length > 0)
            return sidebarHierarchyView.hierarchyScopedExpansionKey(resolvedItemKey);
        const uuid = item.uuid !== undefined && item.uuid !== null ? String(item.uuid).trim() : "";
        if (uuid.length > 0)
            return sidebarHierarchyView.hierarchyScopedExpansionKey("folder:" + uuid);
        const id = item.id !== undefined && item.id !== null ? String(item.id).trim() : "";
        if (id.length > 0)
            return sidebarHierarchyView.hierarchyScopedExpansionKey(id);
        const itemId = sidebarHierarchyView.normalizedInteger(item.itemId !== undefined ? item.itemId : item.resolvedItemId, -1);
        if (itemId >= 0)
            return sidebarHierarchyView.hierarchyScopedExpansionKey(String(itemId));
        const normalizedFallbackIndex = sidebarHierarchyView.normalizedInteger(fallbackIndex, -1);
        return normalizedFallbackIndex >= 0 ? sidebarHierarchyView.hierarchyScopedExpansionKey(String(normalizedFallbackIndex)) : "";
    }
    function hierarchyExpansionStateContainsKey(key) {
        const normalizedKey = key === undefined || key === null ? "" : String(key).trim();
        if (!normalizedKey.length)
            return false;
        const state = sidebarHierarchyView.hierarchyExpansionStateByKey;
        if (!state)
            return false;
        for (const existingKey in state) {
            if (existingKey === normalizedKey)
                return true;
        }
        return false;
    }
    function hierarchyExpansionStateForKey(key, fallbackValue) {
        const normalizedKey = key === undefined || key === null ? "" : String(key).trim();
        if (!sidebarHierarchyView.hierarchyExpansionStateContainsKey(normalizedKey))
            return Boolean(fallbackValue);
        return Boolean(sidebarHierarchyView.hierarchyExpansionStateByKey[normalizedKey]);
    }
    function hierarchyModelIndexVisible(index) {
        const targetIndex = sidebarHierarchyView.normalizedInteger(index, -1);
        const nodes = sidebarHierarchyView.standardHierarchyModel;
        if (targetIndex <= 0 || targetIndex >= nodes.length)
            return targetIndex === 0;
        const targetItem = nodes[targetIndex];
        const targetDepth = sidebarHierarchyView.normalizedInteger(targetItem && targetItem.depth !== undefined ? targetItem.depth : 0, 0);
        if (targetDepth <= 0)
            return true;
        let requiredDepth = targetDepth;
        for (let modelIndex = targetIndex - 1; modelIndex >= 0 && requiredDepth > 0; --modelIndex) {
            const candidate = nodes[modelIndex];
            if (!candidate)
                continue;
            const candidateDepth = sidebarHierarchyView.normalizedInteger(candidate.depth !== undefined ? candidate.depth : 0, 0);
            if (candidateDepth >= requiredDepth)
                continue;
            if (candidate.showChevron !== undefined && Boolean(candidate.showChevron) && !Boolean(candidate.expanded))
                return false;
            requiredDepth = candidateDepth;
        }
        return true;
    }
    function hierarchyItemPresentation(item, itemId) {
        if (!item || item.mapToItem === undefined)
            return ({
                    "chevronSize": 0,
                    "effectiveShowChevron": false,
                    "height": 0,
                    "iconSize": 0,
                    "index": sidebarHierarchyView.normalizedInteger(itemId, -1),
                    "leadingSpacing": 0,
                    "leftPadding": 0,
                    "rightPadding": 0,
                    "rowBackgroundColor": LV.Theme.accentBlueMuted,
                    "rowHeight": 0,
                    "width": 0,
                    "x": 0,
                    "y": 0
                });
        const point = item.mapToItem(sidebarHierarchyView, 0, 0);
        return ({
                "chevronSize": Number(item.chevronSize) || 0,
                "effectiveShowChevron": Boolean(item.effectiveShowChevron),
                "height": Number(item.height) || 0,
                "iconSize": Number(item.iconSize) || 0,
                "index": sidebarHierarchyView.normalizedInteger(itemId, -1),
                "leadingSpacing": Number(item.leadingSpacing) || 0,
                "leftPadding": Number(item.leftPadding) || 0,
                "rightPadding": Number(item.rightPadding) || 0,
                "rowBackgroundColor": item.rowBackgroundColor !== undefined ? item.rowBackgroundColor : LV.Theme.accentBlueMuted,
                "rowHeight": Number(item.rowHeight) || Number(item.height) || 0,
                "width": Number(item.width) || 0,
                "x": Number(point.x) || 0,
                "y": Number(point.y) || 0
            });
    }
    function hierarchySelectionContainsIndex(index) {
        return hierarchySelectionController.hierarchySelectionContainsIndex(index);
    }
    function hierarchySelectionModifierPressed(modifiers) {
        return hierarchySelectionController.hierarchySelectionModifierPressed(modifiers);
    }
    function hierarchySelectionRangeIndices(anchorIndex, targetIndex) {
        return hierarchySelectionController.hierarchySelectionRangeIndices(anchorIndex, targetIndex);
    }
    function hierarchySelectionRangeModifierPressed(modifiers) {
        return hierarchySelectionController.hierarchySelectionRangeModifierPressed(modifiers);
    }
    function hierarchySelectionToggleModifierPressed(modifiers) {
        return hierarchySelectionController.hierarchySelectionToggleModifierPressed(modifiers);
    }
    function hierarchyChevronExpansionTargetAtPosition(x, y) {
        const targetX = Number(x) || 0;
        const targetY = Number(y) || 0;
        const items = sidebarHierarchyView.collectHierarchyItems();
        for (let index = items.length - 1; index >= 0; --index) {
            const item = items[index];
            if (!item || item.visible === false || item.mapToItem === undefined)
                continue;
            if (!sidebarHierarchyView.hierarchyItemContainsPoint(item, targetX, targetY))
                continue;
            if (item.canToggleExpanded !== undefined && !Boolean(item.canToggleExpanded))
                return null;
            if (item.effectiveShowChevron !== undefined && !Boolean(item.effectiveShowChevron))
                return null;
            const point = item.mapToItem(hierarchyTree, 0, 0);
            const itemX = Number(point.x) || 0;
            const itemWidth = Number(item.width) || 0;
            const chevronSize = Math.max(0, Number(item.chevronSize) || 0);
            if (itemWidth <= 0 || chevronSize <= 0)
                return null;
            const localX = targetX - itemX;
            if (localX < itemWidth - chevronSize - LV.Theme.gap2 || localX > itemWidth)
                return null;
            const resolvedIndex = sidebarHierarchyView.resolveHierarchyActivationIndex(item, item.itemId, item.flatIndex);
            const key = sidebarHierarchyView.hierarchyItemExpansionKey(item, resolvedIndex);
            return ({
                    "index": resolvedIndex,
                    "item": item,
                    "key": key
                });
        }
        return null;
    }
    function invalidateHierarchySelectionVisuals() {
        sidebarHierarchyView.hierarchySelectionVisualRevision += 1;
    }
    function leafHierarchyItemLabel(rawLabel) {
        return renameController.leafHierarchyItemLabel(rawLabel);
    }
    function hierarchyModelItemAt(index) {
        const resolvedIndex = sidebarHierarchyView.normalizedInteger(index, -1);
        if (resolvedIndex < 0 || resolvedIndex >= sidebarHierarchyView.standardHierarchyModel.length)
            return null;
        return sidebarHierarchyView.standardHierarchyModel[resolvedIndex];
    }
    function hierarchyModelItemKey(item) {
        if (!item)
            return "";
        const itemKey = item.itemKey !== undefined && item.itemKey !== null ? String(item.itemKey).trim() : "";
        if (itemKey.length > 0)
            return itemKey;
        const key = item.key !== undefined && item.key !== null ? String(item.key).trim() : "";
        if (key.length > 0)
            return key;
        return "";
    }
    function isFolderContextMenuTargetIndex(index) {
        const item = sidebarHierarchyView.hierarchyModelItemAt(index);
        if (!item)
            return false;
        const itemKey = sidebarHierarchyView.hierarchyModelItemKey(item);
        if (itemKey.startsWith("folder:"))
            return true;
        const uuid = item.uuid !== undefined && item.uuid !== null ? String(item.uuid).trim() : "";
        return uuid.length > 0;
    }
    function normalizeHierarchyModel(modelValue) {
        return renameController.normalizeHierarchyModel(modelValue);
    }
    function normalizeHierarchySelectionIndices(indices) {
        return hierarchySelectionController.normalizeHierarchySelectionIndices(indices);
    }
    function hierarchyModelSignature(modelValue) {
        const normalizedModelValue = modelValue === undefined || modelValue === null ? [] : modelValue;
        try {
            return JSON.stringify(normalizedModelValue);
        } catch (error) {
            return "";
        }
    }
    function normalizedInteger(value, fallbackValue) {
        const numericValue = Number(value);
        if (!isFinite(numericValue))
            return fallbackValue;
        return Math.floor(numericValue);
    }
    function normalizedKeyboardModifiers(modifiers) {
        return hierarchySelectionController.normalizedKeyboardModifiers(modifiers);
    }
    function normalizedNonNegativeInteger(value) {
        const normalized = sidebarHierarchyView.normalizedInteger(value, -1);
        return normalized >= 0 ? normalized : -1;
    }
    function noteDropIndexAtPosition(x, y, referenceItem) {
        return noteDropController.noteDropIndexAtPosition(x, y, referenceItem);
    }
    function noteDropTargetAtPosition(x, y, referenceItem) {
        return noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
    }
    function noteIdFromDragPayload(drag) {
        const noteIds = sidebarHierarchyView.noteIdsFromDragPayload(drag);
        return noteIds.length > 0 ? noteIds[0] : "";
    }
    function noteIdsFromDragPayload(drag) {
        return noteDropController.noteIdsFromDragPayload(drag);
    }
    function projectedHierarchyModel(modelValue) {
        return renameController.projectedHierarchyModel(modelValue);
    }
    function captureHierarchyExpansionState(modelValue) {
        const nodes = Array.isArray(modelValue) ? modelValue : [];
        const nextState = {};
        const currentState = sidebarHierarchyView.hierarchyExpansionStateByKey || {};
        for (const existingKey in currentState)
            nextState[existingKey] = Boolean(currentState[existingKey]);
        for (let index = 0; index < nodes.length; ++index) {
            const node = nodes[index];
            const key = sidebarHierarchyView.hierarchyItemExpansionKey(node, index);
            if (!key.length)
                continue;
            if (nextState[key] !== undefined)
                continue;
            nextState[key] = Boolean(node && node.expanded !== undefined ? node.expanded : false);
        }
        sidebarHierarchyView.hierarchyExpansionStateByKey = nextState;
    }
    function hierarchyModelWithPreservedExpansion(modelValue) {
        const nodes = Array.isArray(modelValue) ? modelValue : [];
        const preserved = [];
        for (let index = 0; index < nodes.length; ++index) {
            const node = nodes[index];
            const key = sidebarHierarchyView.hierarchyItemExpansionKey(node, index);
            if (!key.length || !sidebarHierarchyView.hierarchyExpansionStateContainsKey(key)) {
                preserved.push(node);
                continue;
            }
            const copy = {};
            for (const propertyName in node)
                copy[propertyName] = node[propertyName];
            copy.expanded = sidebarHierarchyView.hierarchyExpansionStateForKey(key, copy.expanded);
            preserved.push(copy);
        }
        return preserved;
    }
    function refreshEditingHierarchyPresentation(forceSelectionSync) {
        const editingIndex = sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.editingHierarchyIndex, -1);
        if (editingIndex < 0) {
            sidebarHierarchyView.clearEditingHierarchyPresentation();
            return null;
        }
        if (Boolean(forceSelectionSync) && sidebarHierarchyView.hierarchyModelIndexVisible(editingIndex))
            hierarchyTree.activateListItemById(editingIndex);
        const item = sidebarHierarchyView.resolveVisibleHierarchyItem(editingIndex);
        if (!item) {
            sidebarHierarchyView.clearEditingHierarchyPresentation();
            return null;
        }
        sidebarHierarchyView.editingHierarchyPresentation = sidebarHierarchyView.hierarchyItemPresentation(item, editingIndex);
    }
    function requestCollapseAllHierarchyItems(reason) {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.hierarchyBulkExpansionEnabled || !sidebarHierarchyView.hierarchyInteractionBridge)
            return false;
        const previousExpansionState = sidebarHierarchyView.hierarchyExpansionStateByKey;
        sidebarHierarchyView.setAllHierarchyExpansionStates(false);
        if (!sidebarHierarchyView.hierarchyInteractionBridge.setAllItemsExpanded(false)) {
            sidebarHierarchyView.hierarchyExpansionStateByKey = previousExpansionState;
            return false;
        }
        sidebarHierarchyView.requestViewHook(reason !== undefined ? reason : "hierarchy.contextMenu.collapseAll");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
        return true;
    }
    function openHierarchyFolderContextMenuAtPosition(x, y, referenceItem) {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
        const targetIndex = sidebarHierarchyView.normalizedInteger(target.index, -1);
        if (targetIndex < 0 || !sidebarHierarchyView.isFolderContextMenuTargetIndex(targetIndex)) {
            sidebarHierarchyView.hierarchyContextMenuTargetIndex = -1;
            sidebarHierarchyView.hierarchyContextMenuKind = "options";
            if (hierarchyViewOptionsMenu.opened)
                hierarchyViewOptionsMenu.close();
            return false;
        }
        sidebarHierarchyView.requestHierarchySelection(target.item, targetIndex, Qt.NoModifier);
        sidebarHierarchyView.hierarchyContextMenuTargetIndex = targetIndex;
        sidebarHierarchyView.hierarchyContextMenuKind = "folder";
        if (hierarchyViewOptionsMenu.opened)
            hierarchyViewOptionsMenu.close();
        hierarchyViewOptionsMenu.openFor(referenceItem ? referenceItem : hierarchyTree, Number(x) || 0, Number(y) || 0);
        sidebarHierarchyView.requestViewHook("hierarchy.contextMenu.open");
        return true;
    }
    function hierarchyContextMenuPointerTriggerAccepted(triggerKind) {
        const normalizedTrigger = triggerKind === undefined || triggerKind === null ? "" : String(triggerKind).trim().toLowerCase();
        if (normalizedTrigger === "rightclick" || normalizedTrigger === "right-click" || normalizedTrigger === "contextmenu" || normalizedTrigger === "context-menu") {
            return true;
        }
        if (normalizedTrigger === "longpress" || normalizedTrigger === "long-press" || normalizedTrigger === "pressandhold" || normalizedTrigger === "press-and-hold") {
            return sidebarHierarchyView.hierarchyKineticViewportEnabled;
        }
        return false;
    }
    function openHierarchyFolderContextMenuFromPointer(x, y, referenceItem, triggerKind) {
        if (!sidebarHierarchyView.hierarchyContextMenuPointerTriggerAccepted(triggerKind))
            return false;
        return sidebarHierarchyView.openHierarchyFolderContextMenuAtPosition(x, y, referenceItem);
    }
    function requestCreateFolder(targetIndex, reason) {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.createFolderEnabled || !sidebarHierarchyView.hierarchyController || !sidebarHierarchyView.hierarchyInteractionBridge)
            return false;
        const explicitTargetIndex = sidebarHierarchyView.normalizedInteger(targetIndex, -1);
        const activeHierarchyItemId = sidebarHierarchyView.normalizedInteger(hierarchyTree.activeListItemId, -1);
        const selectionIndex = explicitTargetIndex >= 0 ? explicitTargetIndex : activeHierarchyItemId;
        if (selectionIndex >= 0) {
            sidebarHierarchyView.hierarchyController.setHierarchySelectedIndex(selectionIndex);
            hierarchySelectionController.setSelectedHierarchyIndices([selectionIndex]);
            hierarchySelectionController.selectionAnchorIndex = selectionIndex;
        }
        sidebarHierarchyView.hierarchyInteractionBridge.createFolder();
        sidebarHierarchyView.requestViewHook(reason !== undefined ? reason : "hierarchy.footer.create");
        Qt.callLater(function () {
            sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
            sidebarHierarchyView.syncSelectedHierarchyItem(true);
            if (sidebarHierarchyView.canRenameSelectedHierarchyItem())
                sidebarHierarchyView.beginRenameSelectedHierarchyItem();
        });
        return true;
    }
    function requestDeleteFolder(targetIndex, reason) {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.hierarchyInteractionBridge)
            return false;
        const explicitTargetIndex = sidebarHierarchyView.normalizedInteger(targetIndex, -1);
        if (explicitTargetIndex >= 0 && sidebarHierarchyView.hierarchyController) {
            sidebarHierarchyView.hierarchyController.setHierarchySelectedIndex(explicitTargetIndex);
            hierarchySelectionController.setSelectedHierarchyIndices([explicitTargetIndex]);
            hierarchySelectionController.selectionAnchorIndex = explicitTargetIndex;
        }
        if (!sidebarHierarchyView.deleteFolderEnabled)
            return false;
        sidebarHierarchyView.hierarchyInteractionBridge.deleteSelectedFolder();
        sidebarHierarchyView.requestViewHook(reason !== undefined ? reason : "hierarchy.footer.delete");
        return true;
    }
    function requestExpandAllHierarchyItems(reason) {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.hierarchyBulkExpansionEnabled || !sidebarHierarchyView.hierarchyInteractionBridge)
            return false;
        const previousExpansionState = sidebarHierarchyView.hierarchyExpansionStateByKey;
        sidebarHierarchyView.setAllHierarchyExpansionStates(true);
        if (!sidebarHierarchyView.hierarchyInteractionBridge.setAllItemsExpanded(true)) {
            sidebarHierarchyView.hierarchyExpansionStateByKey = previousExpansionState;
            return false;
        }
        sidebarHierarchyView.requestViewHook(reason !== undefined ? reason : "hierarchy.contextMenu.expandAll");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
        return true;
    }
    function requestHierarchyViewOptionsAction(index, eventName) {
        const action = sidebarHierarchyView.hierarchyViewOptionsActionName(index, eventName);
        if (action.length <= 0)
            return false;
        if (sidebarHierarchyView.hierarchyViewOptionsTriggerQueuedAction === action)
            return true;
        sidebarHierarchyView.hierarchyViewOptionsTriggerQueuedAction = action;
        Qt.callLater(function () {
            if (sidebarHierarchyView.hierarchyViewOptionsTriggerQueuedAction === action)
                sidebarHierarchyView.hierarchyViewOptionsTriggerQueuedAction = "";
        });
        if (action === "hierarchy.expandAll")
            return sidebarHierarchyView.requestExpandAllHierarchyItems("hierarchy.contextMenu.expandAll");
        if (action === "hierarchy.collapseAll")
            return sidebarHierarchyView.requestCollapseAllHierarchyItems("hierarchy.contextMenu.collapseAll");
        return false;
    }
    function requestHierarchySelection(item, resolvedIndex, modifiers) {
        hierarchySelectionController.requestHierarchySelection(item, resolvedIndex, modifiers);
    }
    function requestHierarchyChevronExpansionAtPosition(x, y, expectedKey) {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.hierarchyInteractionBridge)
            return false;
        const target = sidebarHierarchyView.hierarchyChevronExpansionTargetAtPosition(x, y);
        if (!target || !target.item || target.index < 0 || !target.key || !target.key.length)
            return false;
        const normalizedExpectedKey = expectedKey === undefined || expectedKey === null ? "" : String(expectedKey).trim();
        if (normalizedExpectedKey.length > 0 && normalizedExpectedKey !== target.key)
            return false;
        if (normalizedExpectedKey.length > 0 && sidebarHierarchyView.hierarchyExpansionPointerArmedKey.length === 0)
            return false;
        if (sidebarHierarchyView.hierarchyExpansionPointerArmedKey.length > 0 && sidebarHierarchyView.hierarchyExpansionPointerArmedKey !== target.key)
            return false;
        const previousExpanded = Boolean(target.item.expanded);
        const nextExpanded = !previousExpanded;
        sidebarHierarchyView.hierarchyExpansionPointerArmedKey = "";
        hierarchyExpansionPointerArmTimer.stop();
        sidebarHierarchyView.rememberHierarchyExpansionState(target.key, nextExpanded);
        sidebarHierarchyView.armHierarchyExpansionActivationSuppression(target.item, target.item.itemId, target.index);
        if (!sidebarHierarchyView.hierarchyInteractionBridge.setItemExpanded(target.index, nextExpanded)) {
            sidebarHierarchyView.rememberHierarchyExpansionState(target.key, previousExpanded);
            if (target.item.expanded !== undefined)
                target.item.expanded = previousExpanded;
            return false;
        }
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
        return true;
    }
    function requestHierarchyControllerReload(reason) {
        const normalizedReason = reason === undefined || reason === null ? "" : String(reason).trim();
        if (normalizedReason === "hierarchy.nodes.changed")
            return;
        const hierarchyControllerObject = sidebarHierarchyView.hierarchyController;
        if (!hierarchyControllerObject || hierarchyControllerObject.requestControllerHook === undefined)
            return;
        hierarchyControllerObject.requestControllerHook();
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }
    function requestViewOptions() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.viewOptionsEnabled)
            return;
        sidebarHierarchyView.hierarchyContextMenuKind = "options";
        sidebarHierarchyView.hierarchyContextMenuTargetIndex = -1;
        if (hierarchyViewOptionsMenu.opened) {
            hierarchyViewOptionsMenu.close();
            return;
        }
        hierarchyViewOptionsMenu.openFor(hierarchyFooter, hierarchyFooter.width, hierarchyFooter.height + 2);
        sidebarHierarchyView.requestViewHook("hierarchy.footer.options.open");
    }
    function rememberHierarchyExpansionState(key, expanded) {
        const normalizedKey = key === undefined || key === null ? "" : String(key).trim();
        if (!normalizedKey.length)
            return;
        const nextState = {};
        const currentState = sidebarHierarchyView.hierarchyExpansionStateByKey || {};
        for (const existingKey in currentState)
            nextState[existingKey] = Boolean(currentState[existingKey]);
        nextState[normalizedKey] = Boolean(expanded);
        sidebarHierarchyView.hierarchyExpansionStateByKey = nextState;
    }
    function setAllHierarchyExpansionStates(expanded) {
        const nextState = {};
        const currentState = sidebarHierarchyView.hierarchyExpansionStateByKey || {};
        for (const existingKey in currentState)
            nextState[existingKey] = Boolean(currentState[existingKey]);
        const nodes = sidebarHierarchyView.standardHierarchyModel;
        for (let index = 0; index < nodes.length; ++index) {
            const node = nodes[index];
            if (!node || node.showChevron === undefined || !Boolean(node.showChevron))
                continue;
            const key = sidebarHierarchyView.hierarchyItemExpansionKey(node, index);
            if (key.length > 0)
                nextState[key] = Boolean(expanded);
        }
        sidebarHierarchyView.hierarchyExpansionStateByKey = nextState;
    }
    function armHierarchyUserExpansionAtPosition(x, y) {
        const target = sidebarHierarchyView.hierarchyChevronExpansionTargetAtPosition(x, y);
        if (!target || !target.key || !target.key.length)
            return false;
        sidebarHierarchyView.hierarchyExpansionPointerArmedKey = target.key;
        hierarchyExpansionPointerArmTimer.restart();
        return true;
    }
    function resolveHierarchyActivationIndex(item, itemId, index) {
        const resolvedModelItemId = item ? sidebarHierarchyView.normalizedNonNegativeInteger(item.itemId) : -1;
        if (resolvedModelItemId >= 0)
            return resolvedModelItemId;
        const resolvedItemPropertyId = item ? sidebarHierarchyView.normalizedNonNegativeInteger(item.resolvedItemId) : -1;
        if (resolvedItemPropertyId >= 0)
            return resolvedItemPropertyId;
        const resolvedItemId = sidebarHierarchyView.normalizedNonNegativeInteger(itemId);
        if (resolvedItemId >= 0)
            return resolvedItemId;
        const resolvedFlatIndex = item ? sidebarHierarchyView.normalizedNonNegativeInteger(item.flatIndex) : -1;
        if (resolvedFlatIndex >= 0)
            return resolvedFlatIndex;
        const resolvedIndex = sidebarHierarchyView.normalizedNonNegativeInteger(index);
        if (resolvedIndex >= 0)
            return resolvedIndex;
        return -1;
    }
    function resolveHierarchySelectionModifiers(modifiers) {
        return hierarchySelectionController.resolveHierarchySelectionModifiers(modifiers);
    }
    function resolveVisibleHierarchyItem(itemId) {
        const resolvedIndex = sidebarHierarchyView.normalizedInteger(itemId, -1);
        if (resolvedIndex < 0)
            return null;
        const activeItemId = sidebarHierarchyView.normalizedInteger(hierarchyTree.activeListItemId, -1);
        if (activeItemId === resolvedIndex && hierarchyTree.activeListItem)
            return hierarchyTree.activeListItem;
        return noteDropController.hierarchyItemForResolvedIndex(resolvedIndex);
    }
    function scheduleBookmarkPaletteVisualRefresh() {
        bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
    }
    function selectedHierarchyItemActivationKey() {
        const selectedIndex = sidebarHierarchyView.normalizedInteger(selectedFolderIndex, -1);
        if (selectedIndex < 0)
            return "";
        const item = sidebarHierarchyView.standardHierarchyModel[selectedIndex];
        if (!item)
            return "";
        const itemKey = item.itemKey !== undefined && item.itemKey !== null ? String(item.itemKey).trim() : "";
        if (itemKey.length)
            return itemKey;
        const key = item.key !== undefined && item.key !== null ? String(item.key).trim() : "";
        if (key.length)
            return key;
        return "";
    }
    function selectedHierarchyItemLabel() {
        return renameController.selectedHierarchyItemLabel();
    }
    function setSelectedHierarchyIndices(indices) {
        hierarchySelectionController.setSelectedHierarchyIndices(indices);
    }
    function syncDisplayedHierarchyModel(forceRefresh) {
        sidebarHierarchyView.captureHierarchyExpansionState(sidebarHierarchyView.displayedHierarchyModel);
        const projectedModel = sidebarHierarchyView.projectedHierarchyModel(hierarchyController ? hierarchyController.hierarchyNodes : []);
        const nextModel = sidebarHierarchyView.hierarchyModelWithPreservedExpansion(projectedModel);
        const nextSignature = sidebarHierarchyView.hierarchyModelSignature(nextModel);
        if (!Boolean(forceRefresh) && nextSignature === sidebarHierarchyView.displayedHierarchyModelSignature) {
            sidebarHierarchyView.captureHierarchyExpansionState(nextModel);
            return false;
        }
        sidebarHierarchyView.displayedHierarchyModel = nextModel;
        sidebarHierarchyView.displayedHierarchyModelSignature = nextSignature;
        sidebarHierarchyView.captureHierarchyExpansionState(nextModel);
        return true;
    }
    function shouldSuppressHierarchyActivation(item, itemId, index) {
        if (!sidebarHierarchyView.hierarchyExpansionActivationSuppressed && !hierarchyExpansionActivationBlockTimer.running)
            return false;
        return true;
    }
    function syncHierarchySelectionFromSelectedFolder() {
        hierarchySelectionController.syncHierarchySelectionFromSelectedFolder();
    }
    function syncSelectedHierarchyItem(focusView) {
        if (selectedFolderIndex < 0) {
            sidebarHierarchyView.clearEditingHierarchyPresentation();
            sidebarHierarchyView.invalidateHierarchySelectionVisuals();
            return;
        }
        if (!sidebarHierarchyView.hierarchyModelIndexVisible(selectedFolderIndex)) {
            sidebarHierarchyView.invalidateHierarchySelectionVisuals();
            return;
        }
        const selectedItemActivationKey = sidebarHierarchyView.selectedHierarchyItemActivationKey();
        if (selectedItemActivationKey.length > 0 && hierarchyTree.activateListItemByKey !== undefined)
            hierarchyTree.activateListItemByKey(selectedItemActivationKey);
        else
            hierarchyTree.activateListItemById(selectedFolderIndex);
        if (focusView)
            sidebarHierarchyView.forceActiveFocus();
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.refreshEditingHierarchyPresentation(false);
        sidebarHierarchyView.invalidateHierarchySelectionVisuals();
    }
    function updateNoteDropPreviewAtPosition(x, y, noteIds, referenceItem) {
        return noteDropController.updateNoteDropPreviewAtPosition(x, y, noteIds, referenceItem);
    }

    clip: true
    color: panelColor
    focus: true

    Component.onCompleted: {
        sidebarHierarchyView.syncDisplayedHierarchyModel(true);
        sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
        bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
        sidebarHierarchyView.requestHierarchyControllerReload("hierarchy.view.ready");
    }
    Keys.onEnterPressed: function (event) {
        if (sidebarHierarchyView.renameEditingActive)
            return;
        event.accepted = sidebarHierarchyView.beginRenameSelectedHierarchyItem();
    }
    Keys.onReturnPressed: function (event) {
        if (sidebarHierarchyView.renameEditingActive)
            return;
        event.accepted = sidebarHierarchyView.beginRenameSelectedHierarchyItem();
    }
    onBookmarkPaletteVisualsEnabledChanged: {
        bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
    }
    onHeightChanged: sidebarHierarchyView.invalidateHierarchySelectionVisuals()
    onHierarchyControllerChanged: {
        sidebarHierarchyView.cancelHierarchyRename();
        sidebarHierarchyView.clearNoteDropPreview();
        sidebarHierarchyView.syncDisplayedHierarchyModel(true);
        sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
        sidebarHierarchyView.requestHierarchyControllerReload("hierarchy.controller.changed");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
        bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
    }
    onSelectedFolderIndexChanged: {
        if (sidebarHierarchyView.renameEditingActive && sidebarHierarchyView.selectedFolderIndex !== sidebarHierarchyView.editingHierarchyIndex)
            sidebarHierarchyView.cancelHierarchyRename();
        if (sidebarHierarchyView.safeSelectedHierarchyIndices.length === 0 || !sidebarHierarchyView.hierarchySelectionContainsIndex(sidebarHierarchyView.selectedFolderIndex))
            sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
        syncSelectedHierarchyItem(true);
    }
    onWidthChanged: sidebarHierarchyView.invalidateHierarchySelectionVisuals()

    QtObject {
        id: hierarchySelectionController

        property var view: sidebarHierarchyView
        property int selectionAnchorIndex: -1
        property var selectedIndices: []
        property int pointerSelectionModifiers: Qt.NoModifier
        property double pointerSelectionModifiersCapturedAtMs: 0

        function normalizedKeyboardModifiers(modifiers) {
            const eventModifiers = modifiers === undefined || modifiers === null ? Qt.NoModifier : modifiers;
            const applicationModifiers = Qt.application && Qt.application.keyboardModifiers !== undefined ? Qt.application.keyboardModifiers : Qt.NoModifier;
            return eventModifiers | applicationModifiers;
        }

        function hierarchySelectionToggleModifierPressed(modifiers) {
            const normalizedModifiers = hierarchySelectionController.normalizedKeyboardModifiers(modifiers);
            const toggleMask = Qt.ControlModifier | Qt.MetaModifier;
            return Boolean(normalizedModifiers & toggleMask);
        }

        function hierarchySelectionRangeModifierPressed(modifiers) {
            const normalizedModifiers = hierarchySelectionController.normalizedKeyboardModifiers(modifiers);
            return Boolean(normalizedModifiers & Qt.ShiftModifier);
        }

        function hierarchySelectionModifierPressed(modifiers) {
            return hierarchySelectionController.hierarchySelectionRangeModifierPressed(modifiers) || hierarchySelectionController.hierarchySelectionToggleModifierPressed(modifiers);
        }

        function captureHierarchyPointerSelectionModifiers(modifiers) {
            const normalizedModifiers = hierarchySelectionController.normalizedKeyboardModifiers(modifiers);
            if (!hierarchySelectionController.hierarchySelectionModifierPressed(normalizedModifiers))
                return;
            hierarchySelectionController.pointerSelectionModifiers = normalizedModifiers;
            hierarchySelectionController.pointerSelectionModifiersCapturedAtMs = Date.now();
        }

        function clearHierarchyPointerSelectionModifiers() {
            hierarchySelectionController.pointerSelectionModifiers = Qt.NoModifier;
            hierarchySelectionController.pointerSelectionModifiersCapturedAtMs = 0;
        }

        function resolveHierarchySelectionModifiers(modifiers) {
            const normalizedModifiers = hierarchySelectionController.normalizedKeyboardModifiers(modifiers);
            if (hierarchySelectionController.hierarchySelectionModifierPressed(normalizedModifiers))
                return normalizedModifiers;
            const capturedAtMs = Number(hierarchySelectionController.pointerSelectionModifiersCapturedAtMs);
            const cacheAgeMs = Date.now() - capturedAtMs;
            const cacheFresh = capturedAtMs > 0 && isFinite(cacheAgeMs) && cacheAgeMs >= 0 && cacheAgeMs <= 800;
            const normalizedCachedModifiers = hierarchySelectionController.normalizedKeyboardModifiers(hierarchySelectionController.pointerSelectionModifiers);
            if (cacheFresh && hierarchySelectionController.hierarchySelectionModifierPressed(normalizedCachedModifiers))
                return normalizedCachedModifiers;
            return normalizedModifiers;
        }

        function normalizeHierarchySelectionIndices(indices) {
            if (!indices || indices.length === undefined || !hierarchySelectionController.view)
                return [];
            const normalized = [];
            for (let index = 0; index < indices.length; ++index) {
                const resolvedIndex = hierarchySelectionController.view.normalizedInteger(indices[index], -1);
                if (resolvedIndex < 0)
                    continue;
                if (normalized.indexOf(resolvedIndex) >= 0)
                    continue;
                normalized.push(resolvedIndex);
            }
            normalized.sort(function (left, right) {
                return left - right;
            });
            return normalized;
        }

        function setSelectedHierarchyIndices(indices) {
            hierarchySelectionController.selectedIndices = hierarchySelectionController.normalizeHierarchySelectionIndices(indices);
            if (hierarchySelectionController.view)
                hierarchySelectionController.view.invalidateHierarchySelectionVisuals();
        }

        function hierarchySelectionContainsIndex(index) {
            if (!hierarchySelectionController.view)
                return false;
            const resolvedIndex = hierarchySelectionController.view.normalizedInteger(index, -1);
            if (resolvedIndex < 0)
                return false;
            const normalizedSelection = hierarchySelectionController.normalizeHierarchySelectionIndices(hierarchySelectionController.selectedIndices);
            return normalizedSelection.indexOf(resolvedIndex) >= 0;
        }

        function hierarchySelectionRangeIndices(anchorIndex, targetIndex) {
            if (!hierarchySelectionController.view)
                return [];
            const normalizedAnchor = hierarchySelectionController.view.normalizedInteger(anchorIndex, -1);
            const normalizedTarget = hierarchySelectionController.view.normalizedInteger(targetIndex, -1);
            if (normalizedTarget < 0)
                return [];
            if (normalizedAnchor < 0)
                return [normalizedTarget];
            const begin = Math.min(normalizedAnchor, normalizedTarget);
            const end = Math.max(normalizedAnchor, normalizedTarget);
            const range = [];
            for (let index = begin; index <= end; ++index)
                range.push(index);
            return range;
        }

        function syncHierarchySelectionFromSelectedFolder() {
            if (!hierarchySelectionController.view)
                return;
            const selectedIndex = hierarchySelectionController.view.normalizedInteger(hierarchySelectionController.view.selectedFolderIndex, -1);
            if (selectedIndex < 0) {
                hierarchySelectionController.setSelectedHierarchyIndices([]);
                hierarchySelectionController.selectionAnchorIndex = -1;
                return;
            }
            hierarchySelectionController.setSelectedHierarchyIndices([selectedIndex]);
            hierarchySelectionController.selectionAnchorIndex = selectedIndex;
        }

        function emitHierarchySelectionActivation(item, resolvedIndex) {
            if (!hierarchySelectionController.view || !hierarchySelectionController.view.hierarchyController)
                return;
            const normalizedIndex = hierarchySelectionController.view.normalizedInteger(resolvedIndex, -1);
            if (normalizedIndex < 0)
                return;
            const activationItem = item ? item : hierarchySelectionController.view.resolveVisibleHierarchyItem(normalizedIndex);
            hierarchySelectionController.view.hierarchyController.setHierarchySelectedIndex(normalizedIndex);
            hierarchySelectionController.view.hierarchyItemActivated(activationItem, normalizedIndex, normalizedIndex);
        }

        function requestHierarchySelection(item, resolvedIndex, modifiers) {
            if (!hierarchySelectionController.view || !hierarchySelectionController.view.hierarchyController)
                return;
            const normalizedIndex = hierarchySelectionController.view.normalizedInteger(resolvedIndex, -1);
            if (normalizedIndex < 0)
                return;
            const normalizedModifiers = hierarchySelectionController.normalizedKeyboardModifiers(modifiers);
            if (hierarchySelectionController.hierarchySelectionRangeModifierPressed(normalizedModifiers)) {
                let anchorIndex = hierarchySelectionController.view.normalizedInteger(hierarchySelectionController.selectionAnchorIndex, -1);
                if (anchorIndex < 0)
                    anchorIndex = hierarchySelectionController.view.normalizedInteger(hierarchySelectionController.view.selectedFolderIndex, -1);
                if (anchorIndex < 0)
                    anchorIndex = normalizedIndex;
                const rangeSelection = hierarchySelectionController.hierarchySelectionRangeIndices(anchorIndex, normalizedIndex);
                if (hierarchySelectionController.hierarchySelectionToggleModifierPressed(normalizedModifiers)) {
                    const selectedIndices = hierarchySelectionController.normalizeHierarchySelectionIndices(hierarchySelectionController.selectedIndices);
                    for (let selectionIndex = 0; selectionIndex < rangeSelection.length; ++selectionIndex)
                        selectedIndices.push(rangeSelection[selectionIndex]);
                    hierarchySelectionController.setSelectedHierarchyIndices(selectedIndices);
                } else {
                    hierarchySelectionController.setSelectedHierarchyIndices(rangeSelection);
                }
                hierarchySelectionController.selectionAnchorIndex = anchorIndex;
                hierarchySelectionController.emitHierarchySelectionActivation(item, normalizedIndex);
                return;
            }
            if (hierarchySelectionController.hierarchySelectionToggleModifierPressed(normalizedModifiers)) {
                const selectedIndices = hierarchySelectionController.normalizeHierarchySelectionIndices(hierarchySelectionController.selectedIndices);
                const existingSelectionIndex = selectedIndices.indexOf(normalizedIndex);
                if (existingSelectionIndex < 0) {
                    selectedIndices.push(normalizedIndex);
                    hierarchySelectionController.setSelectedHierarchyIndices(selectedIndices);
                    hierarchySelectionController.selectionAnchorIndex = normalizedIndex;
                    hierarchySelectionController.emitHierarchySelectionActivation(item, normalizedIndex);
                    return;
                }
                if (selectedIndices.length <= 1) {
                    hierarchySelectionController.setSelectedHierarchyIndices([normalizedIndex]);
                    hierarchySelectionController.selectionAnchorIndex = normalizedIndex;
                    hierarchySelectionController.emitHierarchySelectionActivation(item, normalizedIndex);
                    return;
                }
                selectedIndices.splice(existingSelectionIndex, 1);
                hierarchySelectionController.setSelectedHierarchyIndices(selectedIndices);
                const committedIndex = hierarchySelectionController.view.normalizedInteger(hierarchySelectionController.view.selectedFolderIndex, -1);
                if (hierarchySelectionController.hierarchySelectionContainsIndex(committedIndex))
                    return;
                const fallbackIndex = selectedIndices.length > 0 ? selectedIndices[selectedIndices.length - 1] : -1;
                if (fallbackIndex >= 0)
                    hierarchySelectionController.emitHierarchySelectionActivation(hierarchySelectionController.view.resolveVisibleHierarchyItem(fallbackIndex), fallbackIndex);
                return;
            }
            hierarchySelectionController.setSelectedHierarchyIndices([normalizedIndex]);
            hierarchySelectionController.selectionAnchorIndex = normalizedIndex;
            hierarchySelectionController.emitHierarchySelectionActivation(item, normalizedIndex);
        }
    }
    QtObject {
        id: renameController

        required property var hierarchyInteractionBridge
        required property var hierarchyRenameField
        required property var hierarchyController
        required property var hostView
        required property var standardHierarchyModel

        hierarchyController: sidebarHierarchyView.hierarchyController
        hierarchyInteractionBridge: sidebarHierarchyView.hierarchyInteractionBridge
        hierarchyRenameField: sidebarHierarchyView.hierarchyRenameFieldItem
        hostView: sidebarHierarchyView
        standardHierarchyModel: sidebarHierarchyView.standardHierarchyModel

        function normalizedInteger(value, fallbackValue) {
            const numericValue = Number(value);
            if (!isFinite(numericValue))
                return fallbackValue;
            return Math.floor(numericValue);
        }

        function beginRenameSelectedHierarchyItem() {
            const renameIndex = renameController.normalizedInteger(hostView.selectedFolderIndex, -1);
            if (!renameController.canRenameIndex(renameIndex))
                return false;
            hostView.syncSelectedHierarchyItem(false);
            hostView.editingHierarchyIndex = renameIndex;
            hostView.refreshEditingHierarchyPresentation(true);
            hostView.editingHierarchyLabel = renameController.selectedHierarchyItemLabel();
            hostView.syncDisplayedHierarchyModel(true);
            hostView.requestViewHook("hierarchy.rename.begin");
            Qt.callLater(function () {
                if (!hostView.renameEditingActive || hostView.editingHierarchyIndex !== renameIndex || !hierarchyRenameField)
                    return;
                hostView.refreshEditingHierarchyPresentation(true);
                hierarchyRenameField.text = hostView.editingHierarchyLabel;
                hierarchyRenameField.forceInputFocus();
                hierarchyRenameField.selectAll();
            });
            return true;
        }

        function canRenameIndex(index) {
            const numericIndex = Number(index);
            if (!hostView.renameContractAvailable || !isFinite(numericIndex) || !hierarchyInteractionBridge)
                return false;
            return Boolean(hierarchyInteractionBridge.canRenameItem(Math.floor(numericIndex)));
        }

        function canRenameSelectedHierarchyItem() {
            return renameController.canRenameIndex(hostView.selectedFolderIndex);
        }

        function cancelHierarchyRename() {
            if (!hostView.renameEditingActive)
                return false;
            hostView.editingHierarchyIndex = -1;
            hostView.editingHierarchyLabel = "";
            hostView.clearEditingHierarchyPresentation();
            hostView.syncDisplayedHierarchyModel(true);
            Qt.callLater(function () {
                hostView.syncSelectedHierarchyItem(true);
            });
            return true;
        }

        function cloneHierarchyItem(sourceItem) {
            const clone = {};
            if (!sourceItem)
                return clone;
            for (const key in sourceItem)
                clone[key] = sourceItem[key];
            return clone;
        }

        function commitHierarchyRename() {
            if (!hostView.renameEditingActive)
                return false;
            const renameIndex = hostView.editingHierarchyIndex;
            const nextLabel = hierarchyRenameField ? String(hierarchyRenameField.text) : hostView.editingHierarchyLabel;
            hostView.editingHierarchyLabel = nextLabel;
            if (!renameController.canRenameIndex(renameIndex))
                return renameController.cancelHierarchyRename();
            const renamed = Boolean(hierarchyInteractionBridge && hierarchyInteractionBridge.renameItem(renameIndex, nextLabel));
            if (!renamed) {
                Qt.callLater(function () {
                    if (!hierarchyRenameField)
                        return;
                    hierarchyRenameField.forceInputFocus();
                    hierarchyRenameField.selectAll();
                });
                return false;
            }
            hostView.editingHierarchyIndex = -1;
            hostView.editingHierarchyLabel = "";
            hostView.clearEditingHierarchyPresentation();
            hostView.syncDisplayedHierarchyModel(true);
            hostView.requestViewHook("hierarchy.rename.commit");
            Qt.callLater(function () {
                hostView.syncSelectedHierarchyItem(true);
            });
            return true;
        }

        function decodedHierarchyPathSegments(rawPath) {
            const normalizedPath = rawPath === undefined || rawPath === null ? "" : String(rawPath).trim();
            if (!normalizedPath.length)
                return [];
            const segments = [];
            let currentSegment = "";
            function flushCurrentSegment() {
                const normalizedSegment = String(currentSegment).trim();
                currentSegment = "";
                if (normalizedSegment.length)
                    segments.push(normalizedSegment);
            }
            for (let index = 0; index < normalizedPath.length; ++index) {
                const character = normalizedPath.charAt(index);
                if (character === "\\") {
                    const hasNextCharacter = index + 1 < normalizedPath.length;
                    if (hasNextCharacter) {
                        const nextCharacter = normalizedPath.charAt(index + 1);
                        if (nextCharacter === "\\" || nextCharacter === "/") {
                            currentSegment += nextCharacter;
                            ++index;
                            continue;
                        }
                    }
                    flushCurrentSegment();
                    continue;
                }
                if (character === "/") {
                    flushCurrentSegment();
                    continue;
                }
                currentSegment += character;
            }
            flushCurrentSegment();
            return segments;
        }

        function leafHierarchyItemLabel(rawLabel, rawPath) {
            const pathSegments = renameController.decodedHierarchyPathSegments(rawPath);
            if (pathSegments.length > 0)
                return String(pathSegments[pathSegments.length - 1]);
            const normalizedLabel = rawLabel === undefined || rawLabel === null ? "" : String(rawLabel).trim();
            return normalizedLabel;
        }

        function normalizeHierarchyModel(modelValue) {
            if (modelValue === undefined || modelValue === null)
                return [];
            if (Array.isArray(modelValue))
                return modelValue.slice();
            if (modelValue.length !== undefined)
                return Array.from(modelValue);
            return [];
        }

        function projectedHierarchyModel(modelValue) {
            const normalizedModel = renameController.normalizeHierarchyModel(modelValue);
            if (!hostView.renameEditingActive)
                return normalizedModel;
            const editingIndex = renameController.normalizedInteger(hostView.editingHierarchyIndex, -1);
            if (editingIndex < 0 || editingIndex >= normalizedModel.length)
                return normalizedModel;
            const projectedModel = normalizedModel.slice();
            const projectedItem = renameController.cloneHierarchyItem(projectedModel[editingIndex]);
            projectedItem.label = " ";
            projectedModel[editingIndex] = projectedItem;
            return projectedModel;
        }

        function selectedHierarchyItemLabel() {
            const selectedIndex = renameController.normalizedInteger(hostView.selectedFolderIndex, -1);
            if (selectedIndex < 0)
                return "";
            const item = standardHierarchyModel[selectedIndex];
            if (item && item.label !== undefined && item.label !== null) {
                const projectedLabel = renameController.leafHierarchyItemLabel(item.label, item.id);
                if (projectedLabel.length)
                    return projectedLabel;
            }
            if (hierarchyController) {
                const modelLabel = renameController.leafHierarchyItemLabel(hierarchyController.hierarchyItemLabelAt(selectedIndex), item && item.id !== undefined && item.id !== null ? item.id : "");
                if (modelLabel.length)
                    return modelLabel;
            }
            if (hostView.activeHierarchyItem && hostView.activeHierarchyItem.text !== undefined)
                return renameController.leafHierarchyItemLabel(hostView.activeHierarchyItem.text, "");
            return "";
        }
    }
    QtObject {
        id: noteDropController

        required property var hierarchyDragDropBridge
        required property var hierarchyTree
        required property var hostView

        hierarchyDragDropBridge: sidebarHierarchyView.hierarchyDragDropBridge
        hierarchyTree: sidebarHierarchyView.hierarchyTreeItem
        hostView: sidebarHierarchyView

        function canAcceptNoteDropAtPosition(x, y, noteIds, referenceItem) {
            const normalizedNoteIds = noteDropController.normalizeNoteIds(noteIds);
            const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
            if (target.index < 0 || normalizedNoteIds.length === 0 || !hierarchyDragDropBridge)
                return false;
            if (hierarchyDragDropBridge.canAcceptNoteDropList !== undefined)
                return hierarchyDragDropBridge.canAcceptNoteDropList(target.index, normalizedNoteIds);
            return hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteIds[0]);
        }
        function clearNoteDropPreview() {
            hostView.noteDropHoverIndex = -1;
        }
        function collectHierarchyItems() {
            const items = [];
            function visitHierarchyDescendants(item) {
                if (!item || item.children === undefined || item.children === null)
                    return;
                const children = item.children;
                for (let i = 0; i < children.length; ++i) {
                    const child = children[i];
                    if (!child || child.visible === false)
                        continue;
                    if (child.__isHierarchyItem === true)
                        items.push(child);
                    visitHierarchyDescendants(child);
                }
            }
            visitHierarchyDescendants(hierarchyTree);
            return items;
        }
        function commitNoteDropAtPosition(x, y, noteIds, referenceItem) {
            const normalizedNoteIds = noteDropController.normalizeNoteIds(noteIds);
            const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
            if (target.index < 0 || normalizedNoteIds.length === 0) {
                noteDropController.clearNoteDropPreview();
                return false;
            }
            const committed = hierarchyDragDropBridge && (hierarchyDragDropBridge.assignNotesToFolder !== undefined ? hierarchyDragDropBridge.assignNotesToFolder(target.index, normalizedNoteIds) : hierarchyDragDropBridge.assignNoteToFolder(target.index, normalizedNoteIds[0]));
            if (!committed) {
                noteDropController.clearNoteDropPreview();
                return false;
            }
            noteDropController.clearNoteDropPreview();
            hostView.requestViewHook("hierarchy.noteDrop");
            return true;
        }
        function hierarchyItemAtPosition(x, y) {
            const targetX = Number(x) || 0;
            const targetY = Number(y) || 0;
            function visitHierarchyDescendants(item) {
                if (!item || item.children === undefined || item.children === null)
                    return null;
                const children = item.children;
                for (let i = children.length - 1; i >= 0; --i) {
                    const child = children[i];
                    if (!child || child.visible === false)
                        continue;
                    const matchedDescendant = visitHierarchyDescendants(child);
                    if (matchedDescendant)
                        return matchedDescendant;
                    if (child.__isHierarchyItem === true && noteDropController.hierarchyItemContainsPoint(child, targetX, targetY))
                        return child;
                }
                return null;
            }
            return visitHierarchyDescendants(hierarchyTree);
        }
        function hierarchyItemContainsPoint(item, x, y) {
            if (!item || item.mapToItem === undefined)
                return false;
            const mappedPoint = item.mapToItem(hierarchyTree, 0, 0);
            const itemX = Number(mappedPoint.x) || 0;
            const itemY = Number(mappedPoint.y) || 0;
            const itemWidth = Number(item.width) || 0;
            const itemHeight = Number(item.height) || 0;
            if (itemWidth <= 0 || itemHeight <= 0)
                return false;
            if (item.rowVisible !== undefined && !Boolean(item.rowVisible))
                return false;
            return x >= itemX && x <= itemX + itemWidth && y >= itemY && y <= itemY + itemHeight;
        }
        function hierarchyItemForResolvedIndex(itemId) {
            const numericIndex = Number(itemId);
            if (!isFinite(numericIndex))
                return null;
            const resolvedIndex = Math.max(-1, Math.floor(numericIndex));
            if (resolvedIndex < 0)
                return null;
            function visitHierarchyDescendants(item) {
                if (!item || item.children === undefined || item.children === null)
                    return null;
                const children = item.children;
                for (let i = children.length - 1; i >= 0; --i) {
                    const child = children[i];
                    if (!child || child.visible === false)
                        continue;
                    const matchedDescendant = visitHierarchyDescendants(child);
                    if (matchedDescendant)
                        return matchedDescendant;
                    if (child.__isHierarchyItem !== true)
                        continue;
                    const rawItemId = child.itemId !== undefined && child.itemId !== null ? child.itemId : child.resolvedItemId;
                    if (noteDropController.normalizedInteger(rawItemId, -1) === resolvedIndex)
                        return child;
                }
                return null;
            }
            return visitHierarchyDescendants(hierarchyTree);
        }
        function normalizeNoteIds(noteIds) {
            if (noteIds === undefined || noteIds === null)
                return [];

            var sourceIds = noteIds;
            if (typeof sourceIds === "string")
                sourceIds = sourceIds.split(/\r?\n/);
            else if (!Array.isArray(sourceIds) && sourceIds.length !== undefined)
                sourceIds = Array.prototype.slice.call(sourceIds);
            else if (!Array.isArray(sourceIds))
                sourceIds = [sourceIds];

            const normalized = [];
            for (let index = 0; index < sourceIds.length; ++index) {
                const normalizedNoteId = String(sourceIds[index] === undefined || sourceIds[index] === null ? "" : sourceIds[index]).trim();
                if (!normalizedNoteId.length || normalized.indexOf(normalizedNoteId) >= 0)
                    continue;
                normalized.push(normalizedNoteId);
            }
            return normalized;
        }
        function normalizedInteger(value, fallbackValue) {
            const numericValue = Number(value);
            if (!isFinite(numericValue))
                return fallbackValue;
            return Math.floor(numericValue);
        }
        function normalizedNonNegativeInteger(value) {
            const normalized = noteDropController.normalizedInteger(value, -1);
            return normalized >= 0 ? normalized : -1;
        }
        function noteDropIndexAtPosition(x, y, referenceItem) {
            const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
            return target.index;
        }
        function noteDropTargetAtPosition(x, y, referenceItem) {
            const localX = Number(x) || 0;
            const localY = Number(y) || 0;
            const hierarchyPoint = referenceItem && referenceItem !== hierarchyTree && hierarchyTree.mapFromItem !== undefined ? hierarchyTree.mapFromItem(referenceItem, localX, localY) : ({
                    "x": localX,
                    "y": localY
                });
            const hierarchyItem = noteDropController.hierarchyItemAtPosition(hierarchyPoint.x, hierarchyPoint.y);
            if (!hierarchyItem)
                return ({
                        "index": -1,
                        "item": null
                    });
            const rawItemId = hierarchyItem.itemId !== undefined && hierarchyItem.itemId !== null ? hierarchyItem.itemId : hierarchyItem.resolvedItemId;
            const parsedIndex = noteDropController.normalizedNonNegativeInteger(rawItemId);
            if (parsedIndex < 0)
                return ({
                        "index": -1,
                        "item": null
                    });
            return ({
                    "index": parsedIndex,
                    "item": hierarchyItem
                });
        }
        function noteIdFromDragPayload(drag) {
            const noteIds = noteDropController.noteIdsFromDragPayload(drag);
            return noteIds.length > 0 ? noteIds[0] : "";
        }
        function noteIdsFromDragPayload(drag) {
            if (!drag)
                return [];
            const source = drag.source;
            if (source && source.draggedNoteIds !== undefined && source.draggedNoteIds !== null) {
                const draggedNoteIds = noteDropController.normalizeNoteIds(source.draggedNoteIds);
                if (draggedNoteIds.length > 0)
                    return draggedNoteIds;
            }
            if (source && source.noteId !== undefined && source.noteId !== null) {
                const sourceNoteId = String(source.noteId).trim();
                if (sourceNoteId.length > 0)
                    return [sourceNoteId];
            }
            if (drag.getDataAsString !== undefined) {
                const mimeNoteIds = String(drag.getDataAsString("application/x-whatson-note-ids") || "").trim();
                if (mimeNoteIds.length > 0) {
                    try {
                        const parsedNoteIds = JSON.parse(mimeNoteIds);
                        const normalizedParsedNoteIds = noteDropController.normalizeNoteIds(parsedNoteIds);
                        if (normalizedParsedNoteIds.length > 0)
                            return normalizedParsedNoteIds;
                    } catch (error) {}
                }
                const mimeNoteId = String(drag.getDataAsString("application/x-whatson-note-id") || "").trim();
                if (mimeNoteId.length > 0)
                    return [mimeNoteId];
                const plainTextNoteIds = noteDropController.normalizeNoteIds(String(drag.getDataAsString("text/plain") || "").trim());
                if (plainTextNoteIds.length > 0)
                    return plainTextNoteIds;
            }
            return [];
        }
        function updateNoteDropPreviewAtPosition(x, y, noteIds, referenceItem) {
            const normalizedNoteIds = noteDropController.normalizeNoteIds(noteIds);
            const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
            if (target.index < 0 || normalizedNoteIds.length === 0 || !hierarchyDragDropBridge || !(hierarchyDragDropBridge.canAcceptNoteDropList !== undefined ? hierarchyDragDropBridge.canAcceptNoteDropList(target.index, normalizedNoteIds) : hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteIds[0]))) {
                noteDropController.clearNoteDropPreview();
                return false;
            }
            hostView.noteDropHoverIndex = target.index;
            return true;
        }
    }
    QtObject {
        id: bookmarkPaletteController

        required property var bookmarkCanvas
        required property var hostView
        required property var itemLocator

        bookmarkCanvas: sidebarHierarchyView.bookmarkPaletteCanvasItem
        hostView: sidebarHierarchyView
        itemLocator: noteDropController

        function resolveThemeColorToken(tokenName) {
            const normalizedToken = tokenName === undefined || tokenName === null ? "" : String(tokenName).trim();
            if (!normalizedToken.length)
                return LV.Theme.bodyColor;
            if (LV.Theme[normalizedToken] !== undefined)
                return LV.Theme[normalizedToken];
            const accentTokens = LV.Theme.accentPaletteTokens !== undefined && LV.Theme.accentPaletteTokens !== null ? LV.Theme.accentPaletteTokens : [];
            for (let index = 0; index < accentTokens.length; ++index) {
                const token = accentTokens[index];
                if (!token || token.name === undefined || token.color === undefined)
                    continue;
                if (String(token.name) === normalizedToken)
                    return token.color;
            }
            return LV.Theme.bodyColor;
        }

        function bookmarkPaletteColorTokenForLabel(label) {
            const normalizedLabel = label === undefined || label === null ? "" : String(label).trim().toLowerCase();
            switch (normalizedLabel) {
            case "red":
                return "accentRed";
            case "orange":
                return "accentLightOrangeVivid";
            case "amber":
                return "accentLightAmberVivid";
            case "yellow":
                return "accentYellow";
            case "green":
                return "accentGreen";
            case "teal":
                return "accentDimTeal";
            case "blue":
                return "accentBlue";
            case "indigo":
                return "accentLighterIndigo";
            case "purple":
                return "accentPurple";
            case "pink":
                return "accentLightRose";
            default:
                return "";
            }
        }

        function bookmarkPaletteColorForLabel(label) {
            const colorToken = bookmarkPaletteController.bookmarkPaletteColorTokenForLabel(label);
            return colorToken.length ? bookmarkPaletteController.resolveThemeColorToken(colorToken) : LV.Theme.bodyColor;
        }

        function applyBookmarkPaletteVisuals() {
            if (!hostView.bookmarkPaletteVisualsEnabled)
                return;
            const hierarchyItems = itemLocator.collectHierarchyItems();
            for (let index = 0; index < hierarchyItems.length; ++index) {
                const item = hierarchyItems[index];
                if (!item)
                    continue;
                const bookmarkColor = bookmarkPaletteController.bookmarkPaletteColorForLabel(item.text);
                item.textColorNormal = bookmarkColor;
                item.textColorDisabled = bookmarkColor;
            }
        }

        function drawBookmarkGlyph(context, x, y, size, color) {
            const left = x + size * 0.25;
            const right = x + size * 0.75;
            const top = y + size * 0.09375;
            const bottom = y + size * 0.875;
            const notchY = y + size * 0.65625;
            const centerX = x + size * 0.5;
            context.beginPath();
            context.moveTo(left, top);
            context.lineTo(right, top);
            context.lineTo(right, bottom);
            context.lineTo(centerX, notchY);
            context.lineTo(left, bottom);
            context.closePath();
            context.fillStyle = color;
            context.fill();
        }

        function scheduleBookmarkPaletteVisualRefresh() {
            Qt.callLater(function () {
                Qt.callLater(function () {
                    bookmarkPaletteController.applyBookmarkPaletteVisuals();
                    bookmarkCanvas.requestPaint();
                });
            });
        }
    }
    Timer {
        id: hierarchyExpansionActivationBlockTimer

        interval: 160
        repeat: false

        onTriggered: sidebarHierarchyView.hierarchyExpansionActivationSuppressed = false
    }
    Timer {
        id: hierarchyExpansionPointerArmTimer

        interval: 5000
        repeat: false

        onTriggered: sidebarHierarchyView.hierarchyExpansionPointerArmedKey = ""
    }
    Connections {
        function onHierarchyNodesChanged() {
            sidebarHierarchyView.syncDisplayedHierarchyModel(false);
            if (sidebarHierarchyView.renameEditingActive && !sidebarHierarchyView.canRenameIndex(sidebarHierarchyView.editingHierarchyIndex))
                sidebarHierarchyView.cancelHierarchyRename();
            sidebarHierarchyView.clearNoteDropPreview();
            Qt.callLater(function () {
                if (sidebarHierarchyView.safeSelectedHierarchyIndices.length === 0 || !sidebarHierarchyView.hierarchySelectionContainsIndex(sidebarHierarchyView.selectedFolderIndex))
                    sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
                sidebarHierarchyView.syncSelectedHierarchyItem(false);
                if (sidebarHierarchyView.renameEditingActive)
                    sidebarHierarchyView.refreshEditingHierarchyPresentation(true);
            });
            bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
        }

        target: sidebarHierarchyView.hierarchyController
    }
    LV.Hierarchy {
        id: hierarchyTree

        anchors.bottomMargin: sidebarHierarchyView.verticalInset + (sidebarHierarchyView.footerVisible ? hierarchyFooter.implicitHeight : 0)
        anchors.fill: parent
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        anchors.rightMargin: sidebarHierarchyView.horizontalInset
        anchors.topMargin: sidebarHierarchyView.verticalInset + (sidebarHierarchyView.searchFieldVisible ? sidebarHierarchyView.searchHeaderTopGap + hierarchySearchHeader.implicitHeight + sidebarHierarchyView.searchListGap : 0)
        editable: sidebarHierarchyView.hierarchyEditable
        keyboardListNavigationEnabled: false
        listFlickDeceleration: sidebarHierarchyView.hierarchyListFlickDeceleration
        listMaximumFlickVelocity: sidebarHierarchyView.hierarchyListMaximumFlickVelocity
        listOvershootEnabled: sidebarHierarchyView.hierarchyKineticViewportEnabled
        listReboundDuration: sidebarHierarchyView.hierarchyListReboundDuration
        model: sidebarHierarchyView.standardHierarchyModel
        panelColor: sidebarHierarchyView.panelColor
        toolbarItems: []

        onListItemActivated: function (item, itemId, index) {
            if (!sidebarHierarchyView.hierarchyController)
                return;
            const resolvedActivationIndex = sidebarHierarchyView.resolveHierarchyActivationIndex(item, itemId, index);
            if (resolvedActivationIndex < 0)
                return;
            const activationModifiers = sidebarHierarchyView.resolveHierarchySelectionModifiers(Qt.application.keyboardModifiers);
            const pendingSerial = sidebarHierarchyView.hierarchyActivationPendingSerial + 1;
            sidebarHierarchyView.hierarchyActivationPendingSerial = pendingSerial;
            Qt.callLater(function () {
                sidebarHierarchyView.clearHierarchyPointerSelectionModifiers();
                if (sidebarHierarchyView.hierarchyActivationPendingSerial !== pendingSerial)
                    return;
                if (!sidebarHierarchyView.hierarchyController)
                    return;
                if (sidebarHierarchyView.shouldSuppressHierarchyActivation(item, itemId, index))
                    return;
                sidebarHierarchyView.requestHierarchySelection(item, resolvedActivationIndex, activationModifiers);
            });
        }
        onListItemExpanded: function (item, itemId, index, expanded) {
            const resolvedExpansionIndex = sidebarHierarchyView.resolveHierarchyActivationIndex(item, itemId, index);
            const expansionKey = sidebarHierarchyView.hierarchyItemExpansionKey(item, resolvedExpansionIndex);
            const expansionStateKnown = sidebarHierarchyView.hierarchyExpansionStateContainsKey(expansionKey);
            const preservedExpanded = sidebarHierarchyView.hierarchyExpansionStateForKey(expansionKey, expanded);
            const userExpansionArmed = expansionKey.length > 0 && expansionKey === sidebarHierarchyView.hierarchyExpansionPointerArmedKey;
            if (!userExpansionArmed) {
                if (expansionStateKnown && preservedExpanded !== Boolean(expanded) && item && item.expanded !== undefined)
                    item.expanded = preservedExpanded;
                else if (!expansionStateKnown)
                    sidebarHierarchyView.rememberHierarchyExpansionState(expansionKey, expanded);
                return;
            }
            sidebarHierarchyView.hierarchyExpansionPointerArmedKey = "";
            hierarchyExpansionPointerArmTimer.stop();
            if (!sidebarHierarchyView.hierarchyInteractionBridge)
                return;
            if (resolvedExpansionIndex < 0)
                return;
            const previousExpanded = expansionStateKnown ? preservedExpanded : !Boolean(expanded);
            sidebarHierarchyView.rememberHierarchyExpansionState(expansionKey, expanded);
            sidebarHierarchyView.armHierarchyExpansionActivationSuppression(item, itemId, index);
            if (!sidebarHierarchyView.hierarchyInteractionBridge.setItemExpanded(resolvedExpansionIndex, expanded)) {
                sidebarHierarchyView.rememberHierarchyExpansionState(expansionKey, previousExpanded);
                if (item && item.expanded !== undefined)
                    item.expanded = previousExpanded;
                return;
            }
            Qt.callLater(function () {
                sidebarHierarchyView.syncSelectedHierarchyItem(false);
            });
        }
        onListItemMoved: function (item, itemId, itemKey, fromIndex, toIndex, depth) {
            if (!sidebarHierarchyView.hierarchyDragDropBridge || !sidebarHierarchyView.hierarchyDragDropBridge.reorderContractAvailable)
                return;
            if (!sidebarHierarchyView.hierarchyDragDropBridge.applyHierarchyReorder(hierarchyTree.model, itemKey))
                return;
            sidebarHierarchyView.requestViewHook("hierarchy.reorder");
        }

        TapHandler {
            id: hierarchyLeftPressTapHandler

            acceptedButtons: Qt.LeftButton
            acceptedModifiers: Qt.KeyboardModifierMask
            gesturePolicy: TapHandler.DragThreshold
            grabPermissions: PointerHandler.ApprovesTakeOverByAnything
            target: null

            onPressedChanged: {
                if (!pressed)
                    return;
                const pressModifiers = point && point.modifiers !== undefined ? point.modifiers : Qt.NoModifier;
                sidebarHierarchyView.captureHierarchyPointerSelectionModifiers(pressModifiers);
                sidebarHierarchyView.armHierarchyUserExpansionAtPosition(point && point.position !== undefined ? point.position.x : 0, point && point.position !== undefined ? point.position.y : 0);
            }
            onTapped: function (eventPoint, button) {
                if (button !== Qt.LeftButton)
                    return;
                const armedKey = sidebarHierarchyView.hierarchyExpansionPointerArmedKey;
                if (!armedKey.length)
                    return;
                const tapX = eventPoint && eventPoint.position !== undefined ? eventPoint.position.x : 0;
                const tapY = eventPoint && eventPoint.position !== undefined ? eventPoint.position.y : 0;
                Qt.callLater(function () {
                    sidebarHierarchyView.requestHierarchyChevronExpansionAtPosition(tapX, tapY, armedKey);
                });
            }
        }
        TapHandler {
            id: hierarchyContextMenuTapHandler

            acceptedButtons: Qt.RightButton
            acceptedModifiers: Qt.KeyboardModifierMask
            gesturePolicy: TapHandler.DragThreshold
            grabPermissions: PointerHandler.ApprovesTakeOverByAnything
            target: null

            onTapped: function (eventPoint, button) {
                if (button !== Qt.RightButton)
                    return;
                sidebarHierarchyView.openHierarchyFolderContextMenuFromPointer(eventPoint && eventPoint.position !== undefined ? eventPoint.position.x : 0, eventPoint && eventPoint.position !== undefined ? eventPoint.position.y : 0, hierarchyTree, "rightClick");
            }
            onLongPressed: {
                sidebarHierarchyView.openHierarchyFolderContextMenuFromPointer(hierarchyContextMenuTapHandler.point && hierarchyContextMenuTapHandler.point.position !== undefined ? hierarchyContextMenuTapHandler.point.position.x : 0, hierarchyContextMenuTapHandler.point && hierarchyContextMenuTapHandler.point.position !== undefined ? hierarchyContextMenuTapHandler.point.position.y : 0, hierarchyTree, "longPress");
            }
        }
    }
    Item {
        id: hierarchySelectionOverlayLayer

        anchors.fill: hierarchyTree
        visible: sidebarHierarchyView.selectedHierarchyOverlayRects.length > 0
        z: 0.9

        Repeater {
            model: sidebarHierarchyView.selectedHierarchyOverlayRects

            delegate: Rectangle {
                required property var modelData

                border.color: Qt.alpha(LV.Theme.accentBlue, 0.55)
                border.width: Math.max(1, Math.round(LV.Theme.strokeThin))
                color: Qt.alpha(LV.Theme.accentBlue, 0.18)
                height: Number(modelData.height) || 0
                radius: Number(modelData.radius) || LV.Theme.radiusControl
                width: Number(modelData.width) || 0
                x: Number(modelData.x) || 0
                y: Number(modelData.y) || 0
            }
        }
    }
    Canvas {
        id: bookmarkPaletteIconOverlay

        anchors.fill: hierarchyTree
        visible: false
        z: 1

        onPaint: {
            const ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
        }
    }
    LV.InputField {
        id: hierarchyRenameField

        backgroundColor: sidebarHierarchyView.hierarchyRenameFieldBackgroundColor
        backgroundColorDisabled: sidebarHierarchyView.hierarchyRenameFieldBackgroundColor
        backgroundColorFocused: sidebarHierarchyView.hierarchyRenameFieldBackgroundColor
        backgroundColorHover: sidebarHierarchyView.hierarchyRenameFieldBackgroundColor
        backgroundColorPressed: sidebarHierarchyView.hierarchyRenameFieldBackgroundColor
        centeredTextHeight: 16
        clearButtonVisible: false
        cornerRadius: 0
        enabled: sidebarHierarchyView.renameEditingActive
        fieldMinHeight: 16
        height: sidebarHierarchyView.hierarchyRenameFieldHeight
        insetHorizontal: 0
        insetVertical: LV.Theme.gapNone
        placeholderText: ""
        selectByMouse: true
        sideSpacing: 0
        style: inlineStyle
        textColor: LV.Theme.bodyColor
        textColorDisabled: LV.Theme.disabledColor
        visible: sidebarHierarchyView.renameEditingActive && sidebarHierarchyView.renamePresentationAvailable
        width: sidebarHierarchyView.hierarchyRenameFieldWidth
        x: sidebarHierarchyView.hierarchyRenameFieldX
        y: sidebarHierarchyView.hierarchyRenameFieldY
        z: 3

        onAccepted: {
            sidebarHierarchyView.commitHierarchyRename();
        }
        onTextEdited: function (text) {
            sidebarHierarchyView.editingHierarchyLabel = text;
        }
    }
    Rectangle {
        id: noteDropHoverOverlay

        border.color: Qt.alpha(sidebarHierarchyView.hierarchyNoteDropHoverColor, 0.9)
        border.width: Math.max(1, Math.round(LV.Theme.strokeThin))
        color: Qt.alpha(sidebarHierarchyView.hierarchyNoteDropHoverColor, 0.34)
        height: Number(sidebarHierarchyView.hierarchyNoteDropHoverItemRect.height) || 0
        radius: sidebarHierarchyView.hierarchyNoteDropHoverRadius
        visible: sidebarHierarchyView.noteDropHoverVisible
        width: Number(sidebarHierarchyView.hierarchyNoteDropHoverItemRect.width) || 0
        x: Number(sidebarHierarchyView.hierarchyNoteDropHoverItemRect.x) || 0
        y: Number(sidebarHierarchyView.hierarchyNoteDropHoverItemRect.y) || 0
        z: 1

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            running: noteDropHoverOverlay.visible

            NumberAnimation {
                duration: 110
                easing.type: Easing.OutQuad
                from: 0.78
                to: 1.0
            }
            NumberAnimation {
                duration: 180
                easing.type: Easing.InOutQuad
                from: 1.0
                to: 0.84
            }
        }
    }
    PanelView.ListBarHeader {
        id: hierarchySearchHeader

        anchors.left: parent.left
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        anchors.right: parent.right
        anchors.rightMargin: sidebarHierarchyView.horizontalInset
        anchors.top: hierarchyToolbar.bottom
        anchors.topMargin: sidebarHierarchyView.searchHeaderTopGap
        frameMinHeight: sidebarHierarchyView.searchHeaderMinHeight
        inlineFieldBackgroundColor: sidebarHierarchyView.searchFieldBackgroundColor
        outerHorizontalInset: LV.Theme.gapNone
        outerVerticalInset: sidebarHierarchyView.searchHeaderVerticalInset
        searchFieldShapeStyle: hierarchySearchHeader.shapeRoundRect
        searchText: sidebarHierarchyView.searchText
        sortActionVisible: false
        visibilityActionVisible: false
        visible: sidebarHierarchyView.searchFieldVisible
        z: 2

        onSearchSubmitted: function (text) {
            sidebarHierarchyView.searchText = text;
            sidebarHierarchyView.searchSubmitted(text);
        }
        onSearchTextEdited: function (text) {
            sidebarHierarchyView.searchText = text;
            sidebarHierarchyView.searchTextEdited(text);
        }
    }
    Row {
        id: hierarchyToolbar

        anchors.left: parent.left
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        anchors.top: parent.top
        anchors.topMargin: sidebarHierarchyView.verticalInset
        height: sidebarHierarchyView.toolbarButtonSize
        spacing: sidebarHierarchyView.toolbarButtonSpacing
        width: sidebarHierarchyView.toolbarFrameWidth
        z: 2

        Repeater {
            model: sidebarHierarchyView.toolbarItems

            delegate: LV.IconButton {
                readonly property bool activeToolbar: index === sidebarHierarchyView.activeToolbarIndex
                required property int index
                readonly property var toolbarItem: sidebarHierarchyView.toolbarItems[index]

                backgroundColorHover: activeToolbar ? LV.Theme.panelBackground12 : "transparent"
                backgroundColorPressed: activeToolbar ? LV.Theme.panelBackground12 : "transparent"
                enabled: toolbarItem && toolbarItem.enabled !== undefined ? Boolean(toolbarItem.enabled) : true
                height: sidebarHierarchyView.toolbarButtonSize
                horizontalPadding: sidebarHierarchyView.hierarchyCompactInset
                iconName: toolbarItem && toolbarItem.iconName !== undefined ? String(toolbarItem.iconName) : ""
                tone: activeToolbar ? LV.AbstractButton.Default : LV.AbstractButton.Borderless
                verticalPadding: sidebarHierarchyView.hierarchyCompactInset
                visible: toolbarItem && toolbarItem.visible !== undefined ? Boolean(toolbarItem.visible) : true
                width: sidebarHierarchyView.toolbarButtonSize

                onClicked: {
                    if (sidebarHierarchyView.renameEditingActive)
                        sidebarHierarchyView.cancelHierarchyRename();
                    if (index < 0 || index === sidebarHierarchyView.activeToolbarIndex)
                        return;
                    sidebarHierarchyView.toolbarIndexChangeRequested(index);
                }
            }
        }
    }
    Shortcut {
        context: Qt.WindowShortcut
        enabled: sidebarHierarchyView.renameEditingActive && hierarchyRenameField.focused
        sequence: "Escape"

        onActivated: {
            sidebarHierarchyView.cancelHierarchyRename();
        }
    }
    LV.ContextMenu {
        id: hierarchyViewOptionsMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: sidebarHierarchyView.hierarchyCompactMenuItemWidth
        items: sidebarHierarchyView.hierarchyContextMenuItems
        modal: false
        parent: Controls.Overlay.overlay

        onItemEventTriggered: function (eventName, payload, index, item) {
            sidebarHierarchyView.handleHierarchyContextMenuTrigger(index, eventName);
        }
        onItemTriggered: function (index) {
            sidebarHierarchyView.handleHierarchyContextMenuTrigger(index, "");
        }
    }
    LV.ListFooter {
        id: hierarchyFooter

        anchors.bottom: parent.bottom
        anchors.bottomMargin: sidebarHierarchyView.verticalInset
        anchors.left: parent.left
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        button1: sidebarHierarchyView.hierarchyFooterToolbarButtons[0]
        button2: sidebarHierarchyView.hierarchyFooterToolbarButtons[1]
        button3: sidebarHierarchyView.hierarchyFooterToolbarButtons[2]
        height: sidebarHierarchyView.hierarchyCompactFooterHeight
        horizontalPadding: sidebarHierarchyView.hierarchyCompactInset
        spacing: LV.Theme.gapNone
        verticalPadding: sidebarHierarchyView.hierarchyCompactInset
        visible: sidebarHierarchyView.footerVisible
        width: sidebarHierarchyView.hierarchyCompactFooterWidth
        z: 2

        onButtonClicked: function (index, config) {
            sidebarHierarchyView.handleHierarchyFooterButtonClicked(index, config);
        }
    }
    DropArea {
        id: noteDropSurface

        function updateAcceptance(drag) {
            const noteIds = sidebarHierarchyView.noteIdsFromDragPayload(drag);
            const accepted = sidebarHierarchyView.updateNoteDropPreviewAtPosition(drag ? drag.x : 0, drag ? drag.y : 0, noteIds, noteDropSurface);
            if (drag)
                drag.accepted = accepted;
        }

        anchors.fill: hierarchyTree
        enabled: sidebarHierarchyView.hierarchyNoteDropSurfaceEnabled

        onDropped: function (drop) {
            const noteIds = sidebarHierarchyView.noteIdsFromDragPayload(drop);
            const committed = sidebarHierarchyView.commitNoteDropAtPosition(drop ? drop.x : 0, drop ? drop.y : 0, noteIds, noteDropSurface);
            if (drop)
                drop.accepted = committed;
        }
        onEntered: function (drag) {
            updateAcceptance(drag);
        }
        onExited: {
            sidebarHierarchyView.clearNoteDropPreview();
        }
        onPositionChanged: function (drag) {
            updateAcceptance(drag);
        }
    }
}
