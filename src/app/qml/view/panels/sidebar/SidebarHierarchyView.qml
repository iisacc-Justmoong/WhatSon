pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import ".." as PanelView

Rectangle {
    id: sidebarHierarchyView

    readonly property var activeHierarchyItem: hierarchyTree.activeListItem
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
    readonly property var editingHierarchyItem: sidebarHierarchyView.resolveVisibleHierarchyItem(sidebarHierarchyView.editingHierarchyIndex)
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
    readonly property var editingHierarchyItemPresentation: {
        const snapshot = sidebarHierarchyView.editingHierarchyPresentation;
        const editingIndex = sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.editingHierarchyIndex, -1);
        if (editingIndex >= 0
                && snapshot
                && sidebarHierarchyView.normalizedInteger(snapshot.index, -1) === editingIndex
                && (Number(snapshot.width) || 0) > 0
                && (Number(snapshot.height) || 0) > 0) {
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
    property int activeToolbarIndex: defaultToolbarIndex
    readonly property bool createFolderEnabled: hierarchyInteractionBridge ? Boolean(hierarchyInteractionBridge.createFolderEnabled) : false
    property int defaultToolbarIndex: 0
    readonly property bool deleteFolderEnabled: hierarchyInteractionBridge ? Boolean(hierarchyInteractionBridge.deleteFolderEnabled) : false
    property bool bookmarkPaletteVisualsEnabled: false
    property int editingHierarchyIndex: -1
    property string editingHierarchyLabel: ""
    property bool footerVisible: true
    property var hierarchyDragDropBridge: null
    property var hierarchyInteractionBridge: null
    property bool hierarchyEditable: false
    property alias hierarchySelectionAnchorIndex: hierarchySelectionController.selectionAnchorIndex
    property int hierarchySelectionVisualRevision: 0
    property alias hierarchyPointerSelectionModifiersCapturedAtMs: hierarchySelectionController.pointerSelectionModifiersCapturedAtMs
    property alias hierarchyPointerSelectionModifiers: hierarchySelectionController.pointerSelectionModifiers
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
    property var hierarchyViewModel: null
    property int horizontalInset: LV.Theme.gap2
    property color panelColor: "transparent"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.SidebarHierarchyView") : null
    property int noteDropHoverIndex: -1
    readonly property var noteDropHoverItem: sidebarHierarchyView.hierarchyItemForResolvedIndex(sidebarHierarchyView.noteDropHoverIndex)
    readonly property bool noteDropHoverVisible: sidebarHierarchyView.noteDropHoverIndex >= 0 && !!sidebarHierarchyView.noteDropHoverItem
    readonly property bool renameContractAvailable: hierarchyInteractionBridge ? Boolean(hierarchyInteractionBridge.renameContractAvailable) : false
    readonly property bool renameEditingActive: sidebarHierarchyView.editingHierarchyIndex >= 0
    readonly property bool renamePresentationAvailable: sidebarHierarchyView.hierarchyRenameFieldWidth > 0 && (Number(sidebarHierarchyView.editingHierarchyItemRect.height) || 0) > 0
    property int searchHeaderMinHeight: LV.Theme.gap24
    property int searchHeaderTopGap: LV.Theme.gap4
    property int searchListGap: LV.Theme.gapNone
    property int searchHeaderVerticalInset: LV.Theme.gap2
    property color searchFieldBackgroundColor: "transparent"
    property bool searchFieldVisible: false
    property string searchText: ""
    property alias selectedHierarchyIndices: hierarchySelectionController.selectedIndices
    readonly property int selectedFolderIndex: hierarchyViewModel ? hierarchyViewModel.hierarchySelectedIndex : -1
    readonly property var selectedHierarchyOverlayRects: {
        const revision = sidebarHierarchyView.hierarchySelectionVisualRevision;
        return sidebarHierarchyView.collectSelectedHierarchyOverlayRects();
    }
    readonly property var standardHierarchyModel: sidebarHierarchyView.projectedHierarchyModel(hierarchyViewModel ? hierarchyViewModel.hierarchyNodes : [])
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
    readonly property var hierarchyViewOptionsMenuItems: [
        {
            "label": "Expand All",
            "iconName": "generalshow",
            "eventName": "hierarchy.expandAll",
            "enabled": sidebarHierarchyView.hierarchyBulkExpansionEnabled,
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Collapse All",
            "eventName": "hierarchy.collapseAll",
            "enabled": sidebarHierarchyView.hierarchyBulkExpansionEnabled,
            "keyVisible": false,
            "showChevron": false
        }
    ]
    property bool hierarchyExpansionActivationSuppressed: false
    property int hierarchyActivationPendingSerial: 0
    readonly property int toolbarButtonSize: LV.Theme.gap20
    readonly property real toolbarButtonSpacing: sidebarHierarchyView.toolbarItems.length > 1 ? (sidebarHierarchyView.toolbarFrameWidth - sidebarHierarchyView.toolbarButtonSize * sidebarHierarchyView.toolbarItems.length) / (sidebarHierarchyView.toolbarItems.length - 1) : 0
    property int toolbarFrameWidth: 200
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property var toolbarItems: {
        if (sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.hierarchyViewModel.toolbarItems !== undefined)
            return sidebarHierarchyView.hierarchyViewModel.toolbarItems;
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

    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal hierarchyItemActivated(var item, int itemId, int index)
    signal toolbarIndexChangeRequested(int index)
    signal viewHookRequested

    SidebarHierarchySelectionController {
        id: hierarchySelectionController

        view: sidebarHierarchyView
    }

    function beginRenameSelectedHierarchyItem() {
        return renameController.beginRenameSelectedHierarchyItem();
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

    function commitHierarchyRename() {
        return renameController.commitHierarchyRename();
    }

    function clearNoteDropPreview() {
        noteDropController.clearNoteDropPreview();
    }

    function collectHierarchyItems() {
        return noteDropController.collectHierarchyItems();
    }

    function hierarchyItemContainsPoint(item, x, y) {
        return noteDropController.hierarchyItemContainsPoint(item, x, y);
    }

    function normalizedInteger(value, fallbackValue) {
        const numericValue = Number(value);
        if (!isFinite(numericValue))
            return fallbackValue;
        return Math.floor(numericValue);
    }

    function normalizedNonNegativeInteger(value) {
        const normalized = sidebarHierarchyView.normalizedInteger(value, -1);
        return normalized >= 0 ? normalized : -1;
    }

    function normalizedKeyboardModifiers(modifiers) {
        return hierarchySelectionController.normalizedKeyboardModifiers(modifiers);
    }

    function hierarchySelectionToggleModifierPressed(modifiers) {
        return hierarchySelectionController.hierarchySelectionToggleModifierPressed(modifiers);
    }

    function hierarchySelectionRangeModifierPressed(modifiers) {
        return hierarchySelectionController.hierarchySelectionRangeModifierPressed(modifiers);
    }
    function hierarchySelectionModifierPressed(modifiers) {
        return hierarchySelectionController.hierarchySelectionModifierPressed(modifiers);
    }
    function captureHierarchyPointerSelectionModifiers(modifiers) {
        hierarchySelectionController.captureHierarchyPointerSelectionModifiers(modifiers);
    }
    function clearHierarchyPointerSelectionModifiers() {
        hierarchySelectionController.clearHierarchyPointerSelectionModifiers();
    }
    function resolveHierarchySelectionModifiers(modifiers) {
        return hierarchySelectionController.resolveHierarchySelectionModifiers(modifiers);
    }

    function normalizeHierarchySelectionIndices(indices) {
        return hierarchySelectionController.normalizeHierarchySelectionIndices(indices);
    }

    function setSelectedHierarchyIndices(indices) {
        hierarchySelectionController.setSelectedHierarchyIndices(indices);
    }

    function hierarchySelectionContainsIndex(index) {
        return hierarchySelectionController.hierarchySelectionContainsIndex(index);
    }

    function hierarchySelectionRangeIndices(anchorIndex, targetIndex) {
        return hierarchySelectionController.hierarchySelectionRangeIndices(anchorIndex, targetIndex);
    }

    function syncHierarchySelectionFromSelectedFolder() {
        hierarchySelectionController.syncHierarchySelectionFromSelectedFolder();
    }

    function invalidateHierarchySelectionVisuals() {
        sidebarHierarchyView.hierarchySelectionVisualRevision += 1;
    }

    function collectSelectedHierarchyOverlayRects() {
        const selectedIndices = sidebarHierarchyView.normalizeHierarchySelectionIndices(
                    sidebarHierarchyView.selectedHierarchyIndices);
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

    function emitHierarchySelectionActivation(item, resolvedIndex) {
        hierarchySelectionController.emitHierarchySelectionActivation(item, resolvedIndex);
    }

    function requestHierarchySelection(item, resolvedIndex, modifiers) {
        hierarchySelectionController.requestHierarchySelection(item, resolvedIndex, modifiers);
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

    function armHierarchyExpansionActivationSuppression(item, itemId, index) {
        sidebarHierarchyView.hierarchyExpansionActivationSuppressed = true;
        hierarchyExpansionActivationBlockTimer.restart();
    }

    function shouldSuppressHierarchyActivation(item, itemId, index) {
        if (!sidebarHierarchyView.hierarchyExpansionActivationSuppressed && !hierarchyExpansionActivationBlockTimer.running)
            return false;
        const selectedIndex = sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.selectedFolderIndex, -1);
        const resolvedIndex = sidebarHierarchyView.resolveHierarchyActivationIndex(item, itemId, index);
        if (selectedIndex >= 0 && resolvedIndex >= 0 && resolvedIndex === selectedIndex)
            return false;
        return true;
    }

    function hierarchyItemAtPosition(x, y) {
        return noteDropController.hierarchyItemAtPosition(x, y);
    }

    function hierarchyItemForResolvedIndex(itemId) {
        return noteDropController.hierarchyItemForResolvedIndex(itemId);
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

    function resolveVisibleHierarchyItem(itemId) {
        const resolvedIndex = sidebarHierarchyView.normalizedInteger(itemId, -1);
        if (resolvedIndex < 0)
            return null;
        const activeItemId = sidebarHierarchyView.normalizedInteger(hierarchyTree.activeListItemId, -1);
        if (activeItemId === resolvedIndex && hierarchyTree.activeListItem)
            return hierarchyTree.activeListItem;
        return noteDropController.hierarchyItemForResolvedIndex(resolvedIndex);
    }

    function refreshEditingHierarchyPresentation(forceSelectionSync) {
        const editingIndex = sidebarHierarchyView.normalizedInteger(sidebarHierarchyView.editingHierarchyIndex, -1);
        if (editingIndex < 0) {
            sidebarHierarchyView.clearEditingHierarchyPresentation();
            return null;
        }
        if (Boolean(forceSelectionSync))
            hierarchyTree.activateListItemById(editingIndex);
        const item = sidebarHierarchyView.resolveVisibleHierarchyItem(editingIndex);
        if (!item) {
            sidebarHierarchyView.clearEditingHierarchyPresentation();
            return null;
        }
        sidebarHierarchyView.editingHierarchyPresentation = sidebarHierarchyView.hierarchyItemPresentation(item, editingIndex);
        return item;
    }

    function normalizeHierarchyModel(modelValue) {
        return renameController.normalizeHierarchyModel(modelValue);
    }

    function projectedHierarchyModel(modelValue) {
        return renameController.projectedHierarchyModel(modelValue);
    }

    function leafHierarchyItemLabel(rawLabel) {
        return renameController.leafHierarchyItemLabel(rawLabel);
    }

    function selectedHierarchyItemLabel() {
        return renameController.selectedHierarchyItemLabel();
    }

    function bookmarkPaletteColorTokenForLabel(label) {
        return bookmarkPaletteController.bookmarkPaletteColorTokenForLabel(label);
    }

    function bookmarkPaletteColorForLabel(label) {
        return bookmarkPaletteController.bookmarkPaletteColorForLabel(label);
    }

    function applyBookmarkPaletteVisuals() {
        bookmarkPaletteController.applyBookmarkPaletteVisuals();
    }

    function drawBookmarkGlyph(context, x, y, size, color) {
        bookmarkPaletteController.drawBookmarkGlyph(context, x, y, size, color);
    }

    function scheduleBookmarkPaletteVisualRefresh() {
        bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
    }

    function noteDropTargetAtPosition(x, y, referenceItem) {
        return noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
    }

    function noteDropIndexAtPosition(x, y, referenceItem) {
        return noteDropController.noteDropIndexAtPosition(x, y, referenceItem);
    }

    function canAcceptNoteDropAtPosition(x, y, noteId, referenceItem) {
        return noteDropController.canAcceptNoteDropAtPosition(x, y, noteId, referenceItem);
    }

    function commitNoteDropAtPosition(x, y, noteId, referenceItem) {
        return noteDropController.commitNoteDropAtPosition(x, y, noteId, referenceItem);
    }

    function noteIdFromDragPayload(drag) {
        return noteDropController.noteIdFromDragPayload(drag);
    }

    function updateNoteDropPreviewAtPosition(x, y, noteId, referenceItem) {
        return noteDropController.updateNoteDropPreviewAtPosition(x, y, noteId, referenceItem);
    }

    function requestCreateFolder() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.createFolderEnabled || !sidebarHierarchyView.hierarchyViewModel)
            return;
        const activeHierarchyItemId = Number(hierarchyTree.activeListItemId);
        if (isFinite(activeHierarchyItemId) && activeHierarchyItemId >= 0)
            sidebarHierarchyView.hierarchyViewModel.setHierarchySelectedIndex(Math.floor(activeHierarchyItemId));
        sidebarHierarchyView.hierarchyInteractionBridge.createFolder();
        sidebarHierarchyView.requestViewHook("hierarchy.footer.create");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(true);
        });
    }

    function requestDeleteFolder() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.deleteFolderEnabled || !sidebarHierarchyView.hierarchyInteractionBridge)
            return;
        sidebarHierarchyView.hierarchyInteractionBridge.deleteSelectedFolder();
        sidebarHierarchyView.requestViewHook("hierarchy.footer.delete");
    }

    function requestHierarchyViewModelReload(reason) {
        const normalizedReason = reason === undefined || reason === null ? "" : String(reason).trim();
        if (normalizedReason === "hierarchy.nodes.changed")
            return;
        const hierarchyViewModelObject = sidebarHierarchyView.hierarchyViewModel;
        if (!hierarchyViewModelObject || hierarchyViewModelObject.requestViewModelHook === undefined)
            return;
        hierarchyViewModelObject.requestViewModelHook();
    }

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    function requestViewOptions() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.viewOptionsEnabled)
            return;
        if (hierarchyViewOptionsMenu.opened) {
            hierarchyViewOptionsMenu.close();
            return;
        }
        hierarchyViewOptionsMenu.openFor(hierarchyFooter, hierarchyFooter.width, hierarchyFooter.height + 2);
        sidebarHierarchyView.requestViewHook("hierarchy.footer.options.open");
    }

    function handleHierarchyViewOptionsTrigger(index, eventName) {
        const normalizedEventName = eventName === undefined || eventName === null ? "" : String(eventName).trim();
        const normalizedIndex = Number(index);
        if (normalizedEventName === "hierarchy.expandAll" || normalizedIndex === 0)
            sidebarHierarchyView.requestExpandAllHierarchyItems();
        else if (normalizedEventName === "hierarchy.collapseAll" || normalizedIndex === 1)
            sidebarHierarchyView.requestCollapseAllHierarchyItems();
    }

    function requestExpandAllHierarchyItems() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.hierarchyBulkExpansionEnabled || !sidebarHierarchyView.hierarchyInteractionBridge)
            return false;
        if (!sidebarHierarchyView.hierarchyInteractionBridge.setAllItemsExpanded(true))
            return false;
        sidebarHierarchyView.requestViewHook("hierarchy.footer.expandAll");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
        return true;
    }

    function requestCollapseAllHierarchyItems() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.hierarchyBulkExpansionEnabled || !sidebarHierarchyView.hierarchyInteractionBridge)
            return false;
        if (!sidebarHierarchyView.hierarchyInteractionBridge.setAllItemsExpanded(false))
            return false;
        sidebarHierarchyView.requestViewHook("hierarchy.footer.collapseAll");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
        return true;
    }

    function syncSelectedHierarchyItem(focusView) {
        if (selectedFolderIndex < 0) {
            sidebarHierarchyView.clearEditingHierarchyPresentation();
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

    SidebarHierarchyRenameController {
        id: renameController

        hierarchyInteractionBridge: sidebarHierarchyView.hierarchyInteractionBridge
        hierarchyRenameField: hierarchyRenameField
        hierarchyViewModel: sidebarHierarchyView.hierarchyViewModel
        hostView: sidebarHierarchyView
        standardHierarchyModel: sidebarHierarchyView.standardHierarchyModel
    }
    SidebarHierarchyNoteDropController {
        id: noteDropController

        hierarchyDragDropBridge: sidebarHierarchyView.hierarchyDragDropBridge
        hierarchyTree: hierarchyTree
        hostView: sidebarHierarchyView
    }
    SidebarHierarchyBookmarkPaletteController {
        id: bookmarkPaletteController

        bookmarkCanvas: bookmarkPaletteIconOverlay
        hostView: sidebarHierarchyView
        itemLocator: noteDropController
    }
    Timer {
        id: hierarchyExpansionActivationBlockTimer

        interval: 160
        repeat: false
        onTriggered: sidebarHierarchyView.hierarchyExpansionActivationSuppressed = false
    }

    clip: true
    color: panelColor
    focus: true

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
    onHierarchyViewModelChanged: {
        sidebarHierarchyView.cancelHierarchyRename();
        sidebarHierarchyView.clearNoteDropPreview();
        sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
        sidebarHierarchyView.requestHierarchyViewModelReload("hierarchy.viewModel.changed");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
        bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
    }
    onBookmarkPaletteVisualsEnabledChanged: {
        bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
    }
    onWidthChanged: sidebarHierarchyView.invalidateHierarchySelectionVisuals()
    onHeightChanged: sidebarHierarchyView.invalidateHierarchySelectionVisuals()
    onSelectedFolderIndexChanged: {
        if (sidebarHierarchyView.renameEditingActive && sidebarHierarchyView.selectedFolderIndex !== sidebarHierarchyView.editingHierarchyIndex)
            sidebarHierarchyView.cancelHierarchyRename();
        if (sidebarHierarchyView.selectedHierarchyIndices.length === 0 || !sidebarHierarchyView.hierarchySelectionContainsIndex(
                    sidebarHierarchyView.selectedFolderIndex))
            sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
        syncSelectedHierarchyItem(true);
    }
    Component.onCompleted: {
        sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
        bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
        sidebarHierarchyView.requestHierarchyViewModelReload("hierarchy.view.ready");
    }

    Connections {
        function onHierarchyNodesChanged() {
            if (sidebarHierarchyView.renameEditingActive && !sidebarHierarchyView.canRenameIndex(sidebarHierarchyView.editingHierarchyIndex))
                sidebarHierarchyView.cancelHierarchyRename();
            sidebarHierarchyView.clearNoteDropPreview();
            Qt.callLater(function () {
                if (sidebarHierarchyView.selectedHierarchyIndices.length === 0 || !sidebarHierarchyView.hierarchySelectionContainsIndex(
                            sidebarHierarchyView.selectedFolderIndex))
                    sidebarHierarchyView.syncHierarchySelectionFromSelectedFolder();
                sidebarHierarchyView.syncSelectedHierarchyItem(false);
                if (sidebarHierarchyView.renameEditingActive)
                    sidebarHierarchyView.refreshEditingHierarchyPresentation(true);
            });
            bookmarkPaletteController.scheduleBookmarkPaletteVisualRefresh();
        }
        target: sidebarHierarchyView.hierarchyViewModel
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
        model: sidebarHierarchyView.standardHierarchyModel
        panelColor: sidebarHierarchyView.panelColor
        toolbarItems: []

        onListItemActivated: function (item, itemId, index) {
            if (!sidebarHierarchyView.hierarchyViewModel)
                return;
            const resolvedActivationIndex = sidebarHierarchyView.resolveHierarchyActivationIndex(item, itemId, index);
            if (resolvedActivationIndex < 0)
                return;
            const activationModifiers = sidebarHierarchyView.resolveHierarchySelectionModifiers(
                        Qt.application.keyboardModifiers);
            const pendingSerial = sidebarHierarchyView.hierarchyActivationPendingSerial + 1;
            sidebarHierarchyView.hierarchyActivationPendingSerial = pendingSerial;
            Qt.callLater(function () {
                sidebarHierarchyView.clearHierarchyPointerSelectionModifiers();
                if (sidebarHierarchyView.hierarchyActivationPendingSerial !== pendingSerial)
                    return;
                if (!sidebarHierarchyView.hierarchyViewModel)
                    return;
                if (sidebarHierarchyView.shouldSuppressHierarchyActivation(item, itemId, index))
                    return;
                sidebarHierarchyView.requestHierarchySelection(item, resolvedActivationIndex, activationModifiers);
            });
        }
        onListItemExpanded: function (item, itemId, index, expanded) {
            const resolvedExpansionIndex = sidebarHierarchyView.resolveHierarchyActivationIndex(item, itemId, index);
            sidebarHierarchyView.armHierarchyExpansionActivationSuppression(item, itemId, index);
            if (!sidebarHierarchyView.hierarchyInteractionBridge)
                return;
            if (resolvedExpansionIndex < 0)
                return;
            if (!sidebarHierarchyView.hierarchyInteractionBridge.setItemExpanded(resolvedExpansionIndex, expanded))
                return;
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
            acceptedButtons: Qt.LeftButton
            acceptedModifiers: Qt.KeyboardModifierMask
            gesturePolicy: TapHandler.DragThreshold
            target: null

            onPressedChanged: {
                if (!pressed)
                    return;
                const pressModifiers = point && point.modifiers !== undefined
                        ? point.modifiers
                        : Qt.application.keyboardModifiers;
                sidebarHierarchyView.captureHierarchyPointerSelectionModifiers(pressModifiers);
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

                color: Qt.alpha(LV.Theme.accentBlue, 0.18)
                height: Number(modelData.height) || 0
                radius: Number(modelData.radius) || LV.Theme.radiusControl
                width: Number(modelData.width) || 0
                x: Number(modelData.x) || 0
                y: Number(modelData.y) || 0

                border.color: Qt.alpha(LV.Theme.accentBlue, 0.55)
                border.width: Math.max(1, Math.round(LV.Theme.strokeThin))
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
        insetVertical: 0
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

        visible: sidebarHierarchyView.noteDropHoverVisible
        x: Number(sidebarHierarchyView.hierarchyNoteDropHoverItemRect.x) || 0
        y: Number(sidebarHierarchyView.hierarchyNoteDropHoverItemRect.y) || 0
        width: Number(sidebarHierarchyView.hierarchyNoteDropHoverItemRect.width) || 0
        height: Number(sidebarHierarchyView.hierarchyNoteDropHoverItemRect.height) || 0
        radius: sidebarHierarchyView.hierarchyNoteDropHoverRadius
        color: Qt.alpha(sidebarHierarchyView.hierarchyNoteDropHoverColor, 0.34)
        border.color: Qt.alpha(sidebarHierarchyView.hierarchyNoteDropHoverColor, 0.9)
        border.width: Math.max(1, Math.round(LV.Theme.strokeThin))
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
                horizontalPadding: 2
                iconName: toolbarItem && toolbarItem.iconName !== undefined ? String(toolbarItem.iconName) : ""
                tone: activeToolbar ? LV.AbstractButton.Default : LV.AbstractButton.Borderless
                verticalPadding: 2
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
        itemWidth: 144
        items: sidebarHierarchyView.hierarchyViewOptionsMenuItems
        modal: false
        parent: Controls.Overlay.overlay

        onItemTriggered: function (index) {
            sidebarHierarchyView.handleHierarchyViewOptionsTrigger(index, "");
        }
        onItemEventTriggered: function (eventName, payload, index, item) {
            sidebarHierarchyView.handleHierarchyViewOptionsTrigger(index, eventName);
        }
    }
    LV.ListFooter {
        id: hierarchyFooter

        anchors.bottom: parent.bottom
        anchors.bottomMargin: sidebarHierarchyView.verticalInset
        anchors.left: parent.left
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        button1: ({
                type: "icon",
                iconName: "generaladd",
                backgroundColor: "transparent",
                backgroundColorDisabled: "transparent",
                backgroundColorHover: "transparent",
                backgroundColorPressed: "transparent",
                enabled: sidebarHierarchyView.createFolderEnabled,
                horizontalPadding: 2,
                onClicked: function () {
                    sidebarHierarchyView.requestCreateFolder();
                },
                verticalPadding: 2
            })
        button2: ({
                type: "icon",
                iconName: "generaldelete",
                backgroundColor: "transparent",
                backgroundColorDisabled: "transparent",
                backgroundColorHover: "transparent",
                backgroundColorPressed: "transparent",
                enabled: sidebarHierarchyView.deleteFolderEnabled,
                horizontalPadding: 2,
                onClicked: function () {
                    sidebarHierarchyView.requestDeleteFolder();
                },
                verticalPadding: 2
            })
        button3: ({
                type: "menu",
                iconName: "generalsettings",
                backgroundColor: "transparent",
                backgroundColorDisabled: "transparent",
                backgroundColorHover: "transparent",
                backgroundColorPressed: "transparent",
                enabled: sidebarHierarchyView.viewOptionsEnabled,
                leftPadding: 2,
                onClicked: function () {
                    sidebarHierarchyView.requestViewOptions();
                },
                rightPadding: 4,
                topPadding: 2,
                bottomPadding: 2
            })
        height: 24
        horizontalPadding: 2
        spacing: 0
        verticalPadding: 2
        visible: sidebarHierarchyView.footerVisible
        width: 78
        z: 2
    }
    DropArea {
        id: noteDropSurface

        function updateAcceptance(drag) {
            const noteId = sidebarHierarchyView.noteIdFromDragPayload(drag);
            const accepted = sidebarHierarchyView.updateNoteDropPreviewAtPosition(
                        drag ? drag.x : 0,
                        drag ? drag.y : 0,
                        noteId,
                        noteDropSurface);
            if (drag)
                drag.accepted = accepted;
        }

        anchors.fill: hierarchyTree
        enabled: sidebarHierarchyView.hierarchyDragDropBridge && sidebarHierarchyView.hierarchyDragDropBridge.noteDropContractAvailable

        onDropped: function (drop) {
            const noteId = sidebarHierarchyView.noteIdFromDragPayload(drop);
            if (!sidebarHierarchyView.commitNoteDropAtPosition(
                        drop ? drop.x : 0,
                        drop ? drop.y : 0,
                        noteId,
                        noteDropSurface))
                return;
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
