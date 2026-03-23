import QtQuick
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
    readonly property var editingHierarchyItem: sidebarHierarchyView.hierarchyItemForResolvedIndex(sidebarHierarchyView.editingHierarchyIndex)
    readonly property var editingHierarchyItemRect: {
        const item = sidebarHierarchyView.editingHierarchyItem;
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
    readonly property bool createFolderEnabled: hierarchyViewModel ? Boolean(hierarchyViewModel.hierarchyCreateEnabled) : false
    property int defaultToolbarIndex: 0
    readonly property bool deleteFolderEnabled: hierarchyViewModel ? Boolean(hierarchyViewModel.hierarchyDeleteEnabled) : false
    property bool bookmarkPaletteVisualsEnabled: false
    property int editingHierarchyIndex: -1
    property string editingHierarchyLabel: ""
    property bool footerVisible: true
    property var hierarchyDragDropBridge: null
    property bool hierarchyEditable: false
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
        const item = sidebarHierarchyView.editingHierarchyItem;
        if (item && item.rowBackgroundColor !== undefined)
            return item.rowBackgroundColor;
        return LV.Theme.accentBlueMuted;
    }
    readonly property real hierarchyRenameFieldHeight: {
        const item = sidebarHierarchyView.editingHierarchyItem;
        return Math.max(16, item ? (Number(item.rowHeight) || Number(item.height) || 20) : 20);
    }
    readonly property real hierarchyRenameFieldWidth: {
        const item = sidebarHierarchyView.editingHierarchyItem;
        if (!item)
            return 0;
        const chevronWidth = item.effectiveShowChevron ? (Number(item.chevronSize) || 0) + (Number(item.leadingSpacing) || 0) : 0;
        const usedWidth = (Number(item.leftPadding) || 0) + (Number(item.rightPadding) || 0) + (Number(item.iconSize) || 0) + (Number(item.leadingSpacing) || 0) + chevronWidth;
        return Math.max(0, (Number(sidebarHierarchyView.editingHierarchyItemRect.width) || 0) - usedWidth);
    }
    readonly property real hierarchyRenameFieldX: {
        const item = sidebarHierarchyView.editingHierarchyItem;
        if (!item)
            return 0;
        return (Number(sidebarHierarchyView.editingHierarchyItemRect.x) || 0) + (Number(item.leftPadding) || 0) + (Number(item.iconSize) || 0) + (Number(item.leadingSpacing) || 0);
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
    readonly property bool renameContractAvailable: !!hierarchyViewModel
    readonly property bool renameEditingActive: sidebarHierarchyView.editingHierarchyIndex >= 0
    property int searchHeaderMinHeight: LV.Theme.gap24
    property int searchHeaderTopGap: LV.Theme.gap4
    property int searchListGap: LV.Theme.gapNone
    property int searchHeaderVerticalInset: LV.Theme.gap2
    property color searchFieldBackgroundColor: LV.Theme.panelBackground10
    property bool searchFieldVisible: false
    property string searchText: ""
    readonly property int selectedFolderIndex: hierarchyViewModel ? hierarchyViewModel.hierarchySelectedIndex : -1
    readonly property var standardHierarchyModel: sidebarHierarchyView.projectedHierarchyModel(hierarchyViewModel ? hierarchyViewModel.hierarchyNodes : [])
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
    readonly property bool viewOptionsEnabled: hierarchyViewModel ? Boolean(hierarchyViewModel.hierarchyViewOptionsEnabled) : true

    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal hierarchyItemActivated(var item, int itemId, int index)
    signal toolbarIndexChangeRequested(int index)
    signal viewHookRequested

    function beginRenameSelectedHierarchyItem() {
        if (!sidebarHierarchyView.canRenameSelectedHierarchyItem())
            return false;
        sidebarHierarchyView.editingHierarchyIndex = sidebarHierarchyView.selectedFolderIndex;
        sidebarHierarchyView.editingHierarchyLabel = sidebarHierarchyView.selectedHierarchyItemLabel();
        sidebarHierarchyView.requestViewHook("hierarchy.rename.begin");
        Qt.callLater(function () {
            if (!sidebarHierarchyView.renameEditingActive || !hierarchyRenameField)
                return;
            hierarchyRenameField.text = sidebarHierarchyView.editingHierarchyLabel;
            hierarchyRenameField.forceInputFocus();
            hierarchyRenameField.selectAll();
        });
        return true;
    }
    function canRenameIndex(index) {
        const numericIndex = Number(index);
        if (!sidebarHierarchyView.renameContractAvailable || !isFinite(numericIndex))
            return false;
        return Boolean(sidebarHierarchyView.hierarchyViewModel.canRenameHierarchyItemAt(Math.floor(numericIndex)));
    }
    function canRenameSelectedHierarchyItem() {
        return sidebarHierarchyView.canRenameIndex(sidebarHierarchyView.selectedFolderIndex);
    }
    function cancelHierarchyRename() {
        if (!sidebarHierarchyView.renameEditingActive)
            return false;
        sidebarHierarchyView.editingHierarchyIndex = -1;
        sidebarHierarchyView.editingHierarchyLabel = "";
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(true);
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
        if (!sidebarHierarchyView.renameEditingActive)
            return false;
        const renameIndex = sidebarHierarchyView.editingHierarchyIndex;
        const nextLabel = hierarchyRenameField ? String(hierarchyRenameField.text) : sidebarHierarchyView.editingHierarchyLabel;
        sidebarHierarchyView.editingHierarchyLabel = nextLabel;
        if (!sidebarHierarchyView.canRenameIndex(renameIndex))
            return sidebarHierarchyView.cancelHierarchyRename();
        const renamed = Boolean(sidebarHierarchyView.hierarchyViewModel.renameHierarchyItemAt(renameIndex, nextLabel));
        if (!renamed) {
            Qt.callLater(function () {
                if (!hierarchyRenameField)
                    return;
                hierarchyRenameField.forceInputFocus();
                hierarchyRenameField.selectAll();
            });
            return false;
        }
        sidebarHierarchyView.editingHierarchyIndex = -1;
        sidebarHierarchyView.editingHierarchyLabel = "";
        sidebarHierarchyView.requestViewHook("hierarchy.rename.commit");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(true);
        });
        return true;
    }
    function clearNoteDropPreview() {
        sidebarHierarchyView.noteDropHoverIndex = -1;
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
    function resolveThemeColorToken(tokenName) {
        const normalizedToken = tokenName === undefined || tokenName === null ? "" : String(tokenName).trim();
        if (!normalizedToken.length)
            return LV.Theme.bodyColor;
        if (LV.Theme[normalizedToken] !== undefined)
            return LV.Theme[normalizedToken];
        const accentTokens = LV.Theme.accentPaletteTokens !== undefined && LV.Theme.accentPaletteTokens !== null
            ? LV.Theme.accentPaletteTokens
            : [];
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
        const colorToken = sidebarHierarchyView.bookmarkPaletteColorTokenForLabel(label);
        return colorToken.length ? sidebarHierarchyView.resolveThemeColorToken(colorToken) : LV.Theme.bodyColor;
    }
    function applyBookmarkPaletteVisuals() {
        if (!sidebarHierarchyView.bookmarkPaletteVisualsEnabled)
            return;
        const hierarchyItems = sidebarHierarchyView.collectHierarchyItems();
        for (let index = 0; index < hierarchyItems.length; ++index) {
            const item = hierarchyItems[index];
            if (!item)
                continue;
            const bookmarkColor = sidebarHierarchyView.bookmarkPaletteColorForLabel(item.text);
            item.textColorNormal = bookmarkColor;
            item.textColorDisabled = bookmarkColor;
            item.iconName = "";
            item.iconSource = "";
            item.iconGlyph = "";
            item.iconPlaceholderVisible = false;
            item.iconPlaceholderColor = "transparent";
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
                sidebarHierarchyView.applyBookmarkPaletteVisuals();
                bookmarkPaletteIconOverlay.requestPaint();
            });
        });
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
                if (child.__isHierarchyItem === true && sidebarHierarchyView.hierarchyItemContainsPoint(child, targetX, targetY))
                    return child;
            }
            return null;
        }
        return visitHierarchyDescendants(hierarchyTree);
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
                const rawItemId = child.itemId !== undefined && child.itemId !== null
                    ? child.itemId
                    : child.resolvedItemId;
                if (Math.floor(Number(rawItemId) || -1) === resolvedIndex)
                    return child;
            }
            return null;
        }
        return visitHierarchyDescendants(hierarchyTree);
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
    function noteDropTargetAtPosition(x, y, referenceItem) {
        const localX = Number(x) || 0;
        const localY = Number(y) || 0;
        const hierarchyPoint = referenceItem && referenceItem !== hierarchyTree && hierarchyTree.mapFromItem !== undefined
            ? hierarchyTree.mapFromItem(referenceItem, localX, localY)
            : ({
                    "x": localX,
                    "y": localY
                });
        const hierarchyItem = sidebarHierarchyView.hierarchyItemAtPosition(hierarchyPoint.x, hierarchyPoint.y);
        if (!hierarchyItem)
            return ({
                    "index": -1,
                    "item": null
                });
        const rawItemId = hierarchyItem.itemId !== undefined && hierarchyItem.itemId !== null
            ? hierarchyItem.itemId
            : hierarchyItem.resolvedItemId;
        const parsedIndex = Number(rawItemId);
        if (!isFinite(parsedIndex))
            return ({
                    "index": -1,
                    "item": null
                });
        return ({
                "index": Math.max(-1, Math.floor(parsedIndex)),
                "item": hierarchyItem
            });
    }
    function noteDropIndexAtPosition(x, y, referenceItem) {
        return sidebarHierarchyView.noteDropTargetAtPosition(x, y, referenceItem).index;
    }
    function canAcceptNoteDropAtPosition(x, y, noteId, referenceItem) {
        const normalizedNoteId = noteId !== undefined && noteId !== null ? String(noteId).trim() : "";
        const target = sidebarHierarchyView.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0 || normalizedNoteId.length === 0 || !sidebarHierarchyView.hierarchyDragDropBridge)
            return false;
        return sidebarHierarchyView.hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteId);
    }
    function commitNoteDropAtPosition(x, y, noteId, referenceItem) {
        const normalizedNoteId = noteId !== undefined && noteId !== null ? String(noteId).trim() : "";
        const target = sidebarHierarchyView.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0 || normalizedNoteId.length === 0) {
            sidebarHierarchyView.clearNoteDropPreview();
            return false;
        }
        if (!sidebarHierarchyView.hierarchyDragDropBridge || !sidebarHierarchyView.hierarchyDragDropBridge.assignNoteToFolder(target.index, normalizedNoteId)) {
            sidebarHierarchyView.clearNoteDropPreview();
            return false;
        }
        sidebarHierarchyView.clearNoteDropPreview();
        sidebarHierarchyView.requestViewHook("hierarchy.noteDrop");
        return true;
    }
    function noteIdFromDragPayload(drag) {
        if (!drag)
            return "";
        const source = drag.source;
        if (source && source.noteId !== undefined && source.noteId !== null) {
            const sourceNoteId = String(source.noteId).trim();
            if (sourceNoteId.length > 0)
                return sourceNoteId;
        }
        if (drag.getDataAsString !== undefined) {
            const mimeNoteId = String(drag.getDataAsString("application/x-whatson-note-id") || "").trim();
            if (mimeNoteId.length > 0)
                return mimeNoteId;
            const plainTextNoteId = String(drag.getDataAsString("text/plain") || "").trim();
            if (plainTextNoteId.length > 0)
                return plainTextNoteId;
        }
        return "";
    }
    function updateNoteDropPreviewAtPosition(x, y, noteId, referenceItem) {
        const normalizedNoteId = noteId !== undefined && noteId !== null ? String(noteId).trim() : "";
        const target = sidebarHierarchyView.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0
                || normalizedNoteId.length === 0
                || !sidebarHierarchyView.hierarchyDragDropBridge
                || !sidebarHierarchyView.hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteId)) {
            sidebarHierarchyView.clearNoteDropPreview();
            return false;
        }
        sidebarHierarchyView.noteDropHoverIndex = target.index;
        return true;
    }
    function leafHierarchyItemLabel(rawLabel) {
        const normalizedLabel = rawLabel === undefined || rawLabel === null ? "" : String(rawLabel).trim();
        if (!normalizedLabel.length)
            return "";
        const segments = normalizedLabel.split("/");
        for (let index = segments.length - 1; index >= 0; --index) {
            const segment = String(segments[index]).trim();
            if (segment.length)
                return segment;
        }
        return normalizedLabel;
    }
    function projectedHierarchyModel(modelValue) {
        const normalizedModel = sidebarHierarchyView.normalizeHierarchyModel(modelValue);
        if (!sidebarHierarchyView.renameEditingActive)
            return normalizedModel;
        const editingIndex = Math.floor(Number(sidebarHierarchyView.editingHierarchyIndex) || -1);
        if (editingIndex < 0 || editingIndex >= normalizedModel.length)
            return normalizedModel;
        const projectedModel = normalizedModel.slice();
        const projectedItem = sidebarHierarchyView.cloneHierarchyItem(projectedModel[editingIndex]);
        projectedItem.label = " ";
        projectedItem.key = "";
        projectedItem.itemKey = "";
        projectedModel[editingIndex] = projectedItem;
        return projectedModel;
    }
    function requestCreateFolder() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.createFolderEnabled || !sidebarHierarchyView.hierarchyViewModel)
            return;
        const activeHierarchyItemId = Number(hierarchyTree.activeListItemId);
        if (isFinite(activeHierarchyItemId) && activeHierarchyItemId >= 0)
            sidebarHierarchyView.hierarchyViewModel.setHierarchySelectedIndex(Math.floor(activeHierarchyItemId));
        sidebarHierarchyView.hierarchyViewModel.createHierarchyItem();
        sidebarHierarchyView.requestViewHook("hierarchy.footer.create");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(true);
        });
    }
    function requestDeleteFolder() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.deleteFolderEnabled || !sidebarHierarchyView.hierarchyViewModel)
            return;
        sidebarHierarchyView.hierarchyViewModel.deleteSelectedHierarchyItem();
        sidebarHierarchyView.requestViewHook("hierarchy.footer.delete");
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
        sidebarHierarchyView.requestViewHook("hierarchy.footer.options");
    }
    function selectedHierarchyItemLabel() {
        const selectedIndex = Math.floor(Number(sidebarHierarchyView.selectedFolderIndex) || -1);
        if (selectedIndex < 0)
            return "";
        const item = sidebarHierarchyView.standardHierarchyModel[selectedIndex];
        if (item && item.label !== undefined && item.label !== null) {
            const projectedLabel = sidebarHierarchyView.leafHierarchyItemLabel(item.label);
            if (projectedLabel.length)
                return projectedLabel;
        }
        if (sidebarHierarchyView.hierarchyViewModel) {
            const modelLabel = sidebarHierarchyView.leafHierarchyItemLabel(
                        sidebarHierarchyView.hierarchyViewModel.hierarchyItemLabelAt(selectedIndex));
            if (modelLabel.length)
                return modelLabel;
        }
        if (sidebarHierarchyView.activeHierarchyItem && sidebarHierarchyView.activeHierarchyItem.text !== undefined)
            return sidebarHierarchyView.leafHierarchyItemLabel(sidebarHierarchyView.activeHierarchyItem.text);
        return "";
    }
    function syncSelectedHierarchyItem(focusView) {
        if (selectedFolderIndex < 0)
            return;
        hierarchyTree.activateListItemById(selectedFolderIndex);
        if (focusView)
            sidebarHierarchyView.forceActiveFocus();
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
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
        sidebarHierarchyView.scheduleBookmarkPaletteVisualRefresh();
    }
    onBookmarkPaletteVisualsEnabledChanged: {
        sidebarHierarchyView.scheduleBookmarkPaletteVisualRefresh();
    }
    onSelectedFolderIndexChanged: {
        if (sidebarHierarchyView.renameEditingActive && sidebarHierarchyView.selectedFolderIndex !== sidebarHierarchyView.editingHierarchyIndex)
            sidebarHierarchyView.cancelHierarchyRename();
        syncSelectedHierarchyItem(true);
    }
    Component.onCompleted: {
        sidebarHierarchyView.scheduleBookmarkPaletteVisualRefresh();
    }

    Connections {
        function onHierarchyNodesChanged() {
            if (sidebarHierarchyView.renameEditingActive && !sidebarHierarchyView.canRenameIndex(sidebarHierarchyView.editingHierarchyIndex))
                sidebarHierarchyView.cancelHierarchyRename();
            sidebarHierarchyView.clearNoteDropPreview();
            Qt.callLater(function () {
                sidebarHierarchyView.syncSelectedHierarchyItem(false);
            });
            sidebarHierarchyView.scheduleBookmarkPaletteVisualRefresh();
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
            sidebarHierarchyView.hierarchyViewModel.setHierarchySelectedIndex(itemId);
            sidebarHierarchyView.hierarchyItemActivated(item, itemId, index);
        }
        onListItemExpanded: function (item, itemId, index, expanded) {
            if (!sidebarHierarchyView.hierarchyViewModel)
                return;
            sidebarHierarchyView.hierarchyViewModel.setHierarchyItemExpandedState(itemId, expanded);
        }
        onListItemMoved: function (item, itemId, itemKey, fromIndex, toIndex, depth) {
            if (!sidebarHierarchyView.hierarchyDragDropBridge || !sidebarHierarchyView.hierarchyDragDropBridge.reorderContractAvailable)
                return;
            if (!sidebarHierarchyView.hierarchyDragDropBridge.applyHierarchyReorder(hierarchyTree.model, itemKey))
                return;
            sidebarHierarchyView.requestViewHook("hierarchy.reorder");
        }
    }
    Canvas {
        id: bookmarkPaletteIconOverlay

        anchors.fill: hierarchyTree
        visible: sidebarHierarchyView.bookmarkPaletteVisualsEnabled
        z: 1

        onPaint: {
            const ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            if (!sidebarHierarchyView.bookmarkPaletteVisualsEnabled)
                return;
            const hierarchyItems = sidebarHierarchyView.collectHierarchyItems();
            for (let index = 0; index < hierarchyItems.length; ++index) {
                const item = hierarchyItems[index];
                if (!item || item.mapToItem === undefined)
                    continue;
                const bookmarkColor = sidebarHierarchyView.bookmarkPaletteColorForLabel(item.text);
                const point = item.mapToItem(bookmarkPaletteIconOverlay, 0, 0);
                const iconSize = Math.max(0, Number(item.iconSize) || 0);
                const itemHeight = Math.max(0, Number(item.height) || 0);
                const leftPadding = Math.max(0, Number(item.leftPadding) || 0);
                const iconX = (Number(point.x) || 0) + leftPadding;
                const iconY = (Number(point.y) || 0) + Math.max(0, Math.floor((itemHeight - iconSize) * 0.5));
                sidebarHierarchyView.drawBookmarkGlyph(ctx, iconX, iconY, iconSize, bookmarkColor);
            }
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
        visible: sidebarHierarchyView.renameEditingActive && !!sidebarHierarchyView.editingHierarchyItem && sidebarHierarchyView.hierarchyRenameFieldWidth > 0
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
