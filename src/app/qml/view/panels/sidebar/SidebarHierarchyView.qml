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
    property int activeToolbarIndex: defaultToolbarIndex
    readonly property bool createFolderContractAvailable: hierarchyViewModel && hierarchyViewModel.createFolder !== undefined
    readonly property bool createFolderEnabled: sidebarHierarchyView.createFolderContractAvailable && (hierarchyViewModel.createFolderEnabled === undefined || Boolean(hierarchyViewModel.createFolderEnabled))
    property int defaultToolbarIndex: 0
    readonly property bool deleteFolderContractAvailable: hierarchyViewModel && hierarchyViewModel.deleteSelectedFolder !== undefined
    readonly property bool deleteFolderEnabled: sidebarHierarchyView.deleteFolderContractAvailable && hierarchyViewModel.deleteFolderEnabled !== undefined && Boolean(hierarchyViewModel.deleteFolderEnabled)
    property int editingHierarchyIndex: -1
    property string editingHierarchyLabel: ""
    property bool footerVisible: true
    property string frameName: ""
    property string frameNodeId: ""
    property var hierarchyDragDropBridge: null
    property bool hierarchyEditable: false
    readonly property color hierarchyRenameFieldBackgroundColor: {
        const item = sidebarHierarchyView.activeHierarchyItem;
        if (item && item.rowBackgroundColor !== undefined)
            return item.rowBackgroundColor;
        return LV.Theme.accentBlueMuted;
    }
    readonly property real hierarchyRenameFieldHeight: {
        const item = sidebarHierarchyView.activeHierarchyItem;
        return Math.max(16, item ? (Number(item.rowHeight) || Number(item.height) || 20) : 20);
    }
    readonly property real hierarchyRenameFieldWidth: {
        const item = sidebarHierarchyView.activeHierarchyItem;
        if (!item)
            return 0;
        const chevronWidth = item.effectiveShowChevron ? (Number(item.chevronSize) || 0) + (Number(item.leadingSpacing) || 0) : 0;
        const usedWidth = (Number(item.leftPadding) || 0) + (Number(item.rightPadding) || 0) + (Number(item.iconSize) || 0) + (Number(item.leadingSpacing) || 0) + chevronWidth;
        return Math.max(0, (Number(sidebarHierarchyView.activeHierarchyItemRect.width) || 0) - usedWidth);
    }
    readonly property real hierarchyRenameFieldX: {
        const item = sidebarHierarchyView.activeHierarchyItem;
        if (!item)
            return 0;
        return (Number(sidebarHierarchyView.activeHierarchyItemRect.x) || 0) + (Number(item.leftPadding) || 0) + (Number(item.iconSize) || 0) + (Number(item.leadingSpacing) || 0);
    }
    readonly property real hierarchyRenameFieldY: {
        const rectY = Number(sidebarHierarchyView.activeHierarchyItemRect.y) || 0;
        const rectHeight = Number(sidebarHierarchyView.activeHierarchyItemRect.height) || 0;
        return rectY + Math.max(0, Math.floor((rectHeight - sidebarHierarchyView.hierarchyRenameFieldHeight) * 0.5));
    }
    property var hierarchyViewModel: null
    property int horizontalInset: 2
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.SidebarHierarchyView") : null
    readonly property bool renameContractAvailable: hierarchyViewModel && hierarchyViewModel.canRenameItem !== undefined && hierarchyViewModel.renameItem !== undefined
    readonly property bool renameEditingActive: sidebarHierarchyView.editingHierarchyIndex >= 0
    readonly property int searchHeaderTopGap: LV.Theme.gap4
    property color searchFieldBackgroundColor: LV.Theme.panelBackground10
    property bool searchFieldVisible: false
    property string searchText: ""
    readonly property int selectedFolderIndex: hierarchyViewModel && hierarchyViewModel.selectedIndex !== undefined ? hierarchyViewModel.selectedIndex : -1
    readonly property bool setItemExpandedContractAvailable: hierarchyViewModel && hierarchyViewModel.setItemExpanded !== undefined
    readonly property var standardHierarchyModel: sidebarHierarchyView.projectedHierarchyModel(hierarchyViewModel && hierarchyViewModel.hierarchyModel !== undefined ? hierarchyViewModel.hierarchyModel : [])
    readonly property int toolbarButtonSize: 20
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
    readonly property int verticalInset: 2
    readonly property bool viewOptionsEnabled: hierarchyViewModel && hierarchyViewModel.viewOptionsEnabled !== undefined ? Boolean(hierarchyViewModel.viewOptionsEnabled) : true

    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal toolbarIndexChangeRequested(int index)
    signal viewHookRequested

    function beginRenameSelectedHierarchyItem() {
        if (!sidebarHierarchyView.canRenameSelectedHierarchyItem() || !sidebarHierarchyView.activeHierarchyItem)
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
        return Boolean(sidebarHierarchyView.hierarchyViewModel.canRenameItem(Math.floor(numericIndex)));
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
        const renamed = Boolean(sidebarHierarchyView.hierarchyViewModel.renameItem(renameIndex, nextLabel));
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
    function deepestChildAt(item, x, y) {
        if (!item || item.childAt === undefined)
            return item;
        const child = item.childAt(x, y);
        if (!child)
            return item;
        const localPoint = child.mapFromItem(item, x, y);
        return sidebarHierarchyView.deepestChildAt(child, localPoint.x, localPoint.y);
    }
    function hierarchyItemAtPosition(x, y) {
        const deepest = sidebarHierarchyView.deepestChildAt(hierarchyTree, x, y);
        let candidate = deepest;
        while (candidate) {
            if (candidate.__isHierarchyItem === true)
                return candidate;
            candidate = candidate.parent;
        }
        return null;
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
    function noteDropIndexAtPosition(x, y) {
        const hierarchyItem = sidebarHierarchyView.hierarchyItemAtPosition(x, y);
        if (!hierarchyItem || hierarchyItem.itemId === undefined || hierarchyItem.itemId === null)
            return -1;
        const parsedIndex = Number(hierarchyItem.itemId);
        if (!isFinite(parsedIndex))
            return -1;
        return Math.max(-1, Math.floor(parsedIndex));
    }
    function noteIdFromDragSource(source) {
        if (!source || source.noteId === undefined || source.noteId === null)
            return "";
        return String(source.noteId).trim();
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
        projectedItem.label = "";
        projectedModel[editingIndex] = projectedItem;
        return projectedModel;
    }
    function requestCreateFolder() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.createFolderEnabled || !sidebarHierarchyView.hierarchyViewModel || sidebarHierarchyView.hierarchyViewModel.createFolder === undefined)
            return;
        const activeHierarchyItemId = Number(hierarchyTree.activeListItemId);
        if (sidebarHierarchyView.hierarchyViewModel.setSelectedIndex !== undefined && isFinite(activeHierarchyItemId) && activeHierarchyItemId >= 0)
            sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(Math.floor(activeHierarchyItemId));
        sidebarHierarchyView.hierarchyViewModel.createFolder();
        sidebarHierarchyView.requestViewHook("hierarchy.footer.create");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(true);
        });
    }
    function requestDeleteFolder() {
        if (sidebarHierarchyView.renameEditingActive)
            sidebarHierarchyView.cancelHierarchyRename();
        if (!sidebarHierarchyView.deleteFolderEnabled || !sidebarHierarchyView.hierarchyViewModel || sidebarHierarchyView.hierarchyViewModel.deleteSelectedFolder === undefined)
            return;
        sidebarHierarchyView.hierarchyViewModel.deleteSelectedFolder();
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
        if (sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.hierarchyViewModel.itemLabel !== undefined)
            return String(sidebarHierarchyView.hierarchyViewModel.itemLabel(selectedIndex));
        if (sidebarHierarchyView.activeHierarchyItem && sidebarHierarchyView.activeHierarchyItem.text !== undefined)
            return String(sidebarHierarchyView.activeHierarchyItem.text);
        const item = sidebarHierarchyView.standardHierarchyModel[selectedIndex];
        if (!item || item.label === undefined || item.label === null)
            return "";
        return String(item.label);
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
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(false);
        });
    }
    onSelectedFolderIndexChanged: {
        if (sidebarHierarchyView.renameEditingActive && sidebarHierarchyView.selectedFolderIndex !== sidebarHierarchyView.editingHierarchyIndex)
            sidebarHierarchyView.cancelHierarchyRename();
        syncSelectedHierarchyItem(true);
    }

    Connections {
        function onHierarchyModelChanged() {
            if (sidebarHierarchyView.renameEditingActive && !sidebarHierarchyView.canRenameIndex(sidebarHierarchyView.editingHierarchyIndex))
                sidebarHierarchyView.cancelHierarchyRename();
            Qt.callLater(function () {
                sidebarHierarchyView.syncSelectedHierarchyItem(false);
            });
        }

        ignoreUnknownSignals: true
        target: sidebarHierarchyView.hierarchyViewModel
    }
    LV.Hierarchy {
        id: hierarchyTree

        anchors.bottomMargin: sidebarHierarchyView.verticalInset + (sidebarHierarchyView.footerVisible ? hierarchyFooter.implicitHeight : 0)
        anchors.fill: parent
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        anchors.rightMargin: sidebarHierarchyView.horizontalInset
        anchors.topMargin: sidebarHierarchyView.verticalInset + (sidebarHierarchyView.searchFieldVisible ? sidebarHierarchyView.toolbarButtonSize + sidebarHierarchyView.searchHeaderTopGap + hierarchySearchHeader.implicitHeight : 0)
        editable: sidebarHierarchyView.hierarchyEditable
        keyboardListNavigationEnabled: false
        model: sidebarHierarchyView.standardHierarchyModel
        panelColor: sidebarHierarchyView.panelColor
        toolbarItems: []

        onListItemActivated: function (item, itemId, index) {
            if (!sidebarHierarchyView.hierarchyViewModel || sidebarHierarchyView.hierarchyViewModel.setSelectedIndex === undefined)
                return;
            sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(itemId);
        }
        onListItemExpanded: function (item, itemId, index, expanded) {
            if (!sidebarHierarchyView.setItemExpandedContractAvailable)
                return;
            sidebarHierarchyView.hierarchyViewModel.setItemExpanded(itemId, expanded);
        }
        onListItemMoved: function (item, itemId, itemKey, fromIndex, toIndex, depth) {
            if (!sidebarHierarchyView.hierarchyDragDropBridge || !sidebarHierarchyView.hierarchyDragDropBridge.reorderContractAvailable)
                return;
            if (!sidebarHierarchyView.hierarchyDragDropBridge.applyHierarchyReorder(hierarchyTree.model, itemKey))
                return;
            sidebarHierarchyView.requestViewHook("hierarchy.reorder");
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
        visible: sidebarHierarchyView.renameEditingActive && !!sidebarHierarchyView.activeHierarchyItem && sidebarHierarchyView.hierarchyRenameFieldWidth > 0
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
    PanelView.ListBarHeader {
        id: hierarchySearchHeader

        anchors.left: parent.left
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        anchors.right: parent.right
        anchors.rightMargin: sidebarHierarchyView.horizontalInset
        anchors.top: hierarchyToolbar.bottom
        anchors.topMargin: sidebarHierarchyView.searchHeaderTopGap
        inlineFieldBackgroundColor: sidebarHierarchyView.searchFieldBackgroundColor
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
        function updateAcceptance(drag) {
            const noteId = sidebarHierarchyView.noteIdFromDragSource(drag ? drag.source : null);
            const targetIndex = sidebarHierarchyView.noteDropIndexAtPosition(drag ? drag.x : 0, drag ? drag.y : 0);
            const accepted = targetIndex >= 0 && noteId.length > 0 && sidebarHierarchyView.hierarchyDragDropBridge && sidebarHierarchyView.hierarchyDragDropBridge.canAcceptNoteDrop(targetIndex, noteId);
            if (drag)
                drag.accepted = accepted;
        }

        anchors.fill: hierarchyTree
        enabled: sidebarHierarchyView.hierarchyDragDropBridge && sidebarHierarchyView.hierarchyDragDropBridge.noteDropContractAvailable
        keys: ["whatson.library.note"]

        onDropped: function (drop) {
            const noteId = sidebarHierarchyView.noteIdFromDragSource(drop ? drop.source : null);
            const targetIndex = sidebarHierarchyView.noteDropIndexAtPosition(drop ? drop.x : 0, drop ? drop.y : 0);
            if (targetIndex < 0 || noteId.length === 0)
                return;
            if (!sidebarHierarchyView.hierarchyDragDropBridge || !sidebarHierarchyView.hierarchyDragDropBridge.assignNoteToFolder(targetIndex, noteId))
                return;
            sidebarHierarchyView.requestViewHook("hierarchy.noteDrop");
        }
        onEntered: function (drag) {
            updateAcceptance(drag);
        }
        onPositionChanged: function (drag) {
            updateAcceptance(drag);
        }
    }
}
