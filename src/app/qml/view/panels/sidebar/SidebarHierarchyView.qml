import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: sidebarHierarchyView

    property int activeToolbarIndex: defaultToolbarIndex
    readonly property bool createFolderContractAvailable: hierarchyViewModel && hierarchyViewModel.createFolder !== undefined
    readonly property bool createFolderEnabled: sidebarHierarchyView.createFolderContractAvailable && (hierarchyViewModel.createFolderEnabled === undefined || Boolean(hierarchyViewModel.createFolderEnabled))
    property int defaultToolbarIndex: 0
    readonly property bool deleteFolderContractAvailable: hierarchyViewModel && hierarchyViewModel.deleteSelectedFolder !== undefined
    readonly property bool deleteFolderEnabled: sidebarHierarchyView.deleteFolderContractAvailable && hierarchyViewModel.deleteFolderEnabled !== undefined && Boolean(hierarchyViewModel.deleteFolderEnabled)
    property string frameName: ""
    property string frameNodeId: ""
    property var hierarchyDragDropBridge: null
    property bool hierarchyEditable: false
    property var hierarchyViewModel: null
    property int horizontalInset: 2
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.SidebarHierarchyView") : null
    readonly property int selectedFolderIndex: hierarchyViewModel && hierarchyViewModel.selectedIndex !== undefined ? hierarchyViewModel.selectedIndex : -1
    readonly property var standardHierarchyModel: sidebarHierarchyView.normalizeHierarchyModel(hierarchyViewModel && hierarchyViewModel.hierarchyModel !== undefined ? hierarchyViewModel.hierarchyModel : [])
    readonly property int toolbarButtonSize: 20
    readonly property real toolbarButtonSpacing: sidebarHierarchyView.toolbarItems.length > 1 ? (sidebarHierarchyView.toolbarFrameWidth - sidebarHierarchyView.toolbarButtonSize * sidebarHierarchyView.toolbarItems.length) / (sidebarHierarchyView.toolbarItems.length - 1) : 0
    readonly property int toolbarFrameWidth: 200
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

    signal toolbarIndexChangeRequested(int index)
    signal viewHookRequested

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
    function requestCreateFolder() {
        if (!sidebarHierarchyView.createFolderEnabled || !sidebarHierarchyView.hierarchyViewModel || sidebarHierarchyView.hierarchyViewModel.createFolder === undefined)
            return;
        sidebarHierarchyView.hierarchyViewModel.createFolder();
        sidebarHierarchyView.requestViewHook("hierarchy.footer.create");
        Qt.callLater(function () {
            sidebarHierarchyView.syncSelectedHierarchyItem(true);
        });
    }
    function requestDeleteFolder() {
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
        if (!sidebarHierarchyView.viewOptionsEnabled)
            return;
        sidebarHierarchyView.requestViewHook("hierarchy.footer.options");
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

    onHierarchyViewModelChanged: Qt.callLater(function () {
        sidebarHierarchyView.syncSelectedHierarchyItem(false);
    })
    onSelectedFolderIndexChanged: syncSelectedHierarchyItem(true)

    Connections {
        function onHierarchyModelChanged() {
            Qt.callLater(function () {
                sidebarHierarchyView.syncSelectedHierarchyItem(false);
            });
        }

        ignoreUnknownSignals: true
        target: sidebarHierarchyView.hierarchyViewModel
    }
    LV.Hierarchy {
        id: hierarchyTree

        activeToolbarIndex: sidebarHierarchyView.activeToolbarIndex
        anchors.bottomMargin: sidebarHierarchyView.verticalInset + hierarchyFooter.implicitHeight
        anchors.fill: parent
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        anchors.rightMargin: sidebarHierarchyView.horizontalInset
        anchors.topMargin: sidebarHierarchyView.verticalInset
        editable: sidebarHierarchyView.hierarchyEditable
        keyboardListNavigationEnabled: false
        model: sidebarHierarchyView.standardHierarchyModel
        panelColor: sidebarHierarchyView.panelColor
        toolbarDistributeSpacing: false
        toolbarItems: sidebarHierarchyView.toolbarItems
        toolbarSpacing: sidebarHierarchyView.toolbarButtonSpacing

        onListItemActivated: function (item, itemId, index) {
            if (!sidebarHierarchyView.hierarchyViewModel || sidebarHierarchyView.hierarchyViewModel.setSelectedIndex === undefined)
                return;
            sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(itemId);
        }
        onListItemMoved: function (item, itemId, itemKey, fromIndex, toIndex, depth) {
            if (!sidebarHierarchyView.hierarchyDragDropBridge || !sidebarHierarchyView.hierarchyDragDropBridge.reorderContractAvailable)
                return;
            if (!sidebarHierarchyView.hierarchyDragDropBridge.applyHierarchyReorder(hierarchyTree.model, itemKey))
                return;
            sidebarHierarchyView.requestViewHook("hierarchy.reorder");
        }
        onToolbarActivated: function (button, buttonId, index) {
            if (index < 0 || index === sidebarHierarchyView.activeToolbarIndex)
                return;
            sidebarHierarchyView.toolbarIndexChangeRequested(index);
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
                spacing: -4,
                topPadding: 2,
                bottomPadding: 2
            })
        height: 24
        horizontalPadding: 2
        spacing: 0
        verticalPadding: 2
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
