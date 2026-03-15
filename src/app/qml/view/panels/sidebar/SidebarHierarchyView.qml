import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import WhatSon.App.Internal 1.0

Item {
    id: sidebarHierarchyView

    property int activeToolbarIndex: defaultToolbarIndex
    readonly property int contentWidth: Math.max(0, width - horizontalInset * 2)
    readonly property bool createFolderEnabled: hierarchyViewModel && hierarchyViewModel.createFolderEnabled !== undefined ? hierarchyViewModel.createFolderEnabled : false
    property int defaultToolbarIndex: 0
    readonly property bool deleteFolderEnabled: hierarchyViewModel && hierarchyViewModel.deleteFolderEnabled !== undefined ? hierarchyViewModel.deleteFolderEnabled : false
    property string editingItemKey: ""
    property string editingText: ""
    readonly property int footerHeight: 24
    property string frameName: ""
    property string frameNodeId: ""
    property var hierarchyViewModel: null
    property int horizontalInset: 2
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.SidebarHierarchyView") : null
    readonly property int searchHeight: (typeof LV.Theme.gap18 === "number" && isFinite(LV.Theme.gap18)) ? LV.Theme.gap18 : 18
    property string searchQuery: ""
    readonly property int selectedFolderIndex: hierarchyViewModel && hierarchyViewModel.selectedIndex !== undefined ? hierarchyViewModel.selectedIndex : -1
    readonly property string selectedItemKey: hierarchyAdapter.selectedItemKey
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
    readonly property int toolbarMinWidth: {
        var count = toolbarItems.length;
        if (count <= 0)
            return 20;
        return count * 20 + (count - 1) * ((typeof LV.Theme.gap2 === "number" && isFinite(LV.Theme.gap2)) ? LV.Theme.gap2 : 2);
    }
    readonly property int verticalInset: 2
    readonly property bool viewOptionsEnabled: hierarchyViewModel && hierarchyViewModel.viewOptionsEnabled !== undefined ? hierarchyViewModel.viewOptionsEnabled : true
    property var viewOptionsMenuItems: []

    signal toolbarIndexChangeRequested(int index)
    signal viewHookRequested

    function activateSelectedHierarchyItem(focusView) {
        if (selectedItemKey.length === 0)
            return;
        hierarchyList.activateByKey(selectedItemKey);
        if (focusView)
            sidebarHierarchyView.forceActiveFocus();
    }
    function beginRenameKey(itemKey, currentLabel) {
        const normalizedKey = itemKey === undefined || itemKey === null ? "" : String(itemKey).trim();
        if (normalizedKey.length === 0 || !hierarchyAdapter.canRenameKey(normalizedKey))
            return;
        hierarchyAdapter.activateKey(normalizedKey);
        editingItemKey = normalizedKey;
        editingText = currentLabel === undefined || currentLabel === null ? hierarchyAdapter.labelForKey(normalizedKey) : String(currentLabel);
    }
    function cancelRename() {
        editingItemKey = "";
        editingText = "";
    }
    function commitRename() {
        if (editingItemKey.length === 0)
            return;
        if (hierarchyAdapter.renameKey(editingItemKey, editingText))
            requestViewHook("rename-folder");
        cancelRename();
    }
    function draggedNoteId(event) {
        if (!event || !event.source || event.source.noteId === undefined || event.source.noteId === null)
            return "";
        return String(event.source.noteId).trim();
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function targetItemForKey(itemKey) {
        const normalizedKey = itemKey === undefined || itemKey === null ? "" : String(itemKey).trim();
        if (normalizedKey.length === 0)
            return null;
        return hierarchyList.resolveByKey(normalizedKey);
    }
    function toggleViewOptionsMenu() {
        if (viewOptionsContextMenu.opened) {
            viewOptionsContextMenu.close();
            return;
        }
        viewOptionsContextMenu.openFor(sidebarFooter, 0, sidebarFooter.height + verticalInset);
    }

    clip: true
    focus: true

    Keys.onPressed: function (event) {
        if (event.key !== Qt.Key_Return && event.key !== Qt.Key_Enter)
            return;

        if (editingItemKey.length > 0) {
            commitRename();
            event.accepted = true;
            return;
        }

        if (selectedItemKey.length === 0 || !hierarchyAdapter.canRenameKey(selectedItemKey))
            return;

        beginRenameKey(selectedItemKey, hierarchyAdapter.labelForKey(selectedItemKey));
        event.accepted = true;
    }
    onHierarchyViewModelChanged: {
        cancelRename();
        noteDropTargetKey = "";
    }
    onSelectedFolderIndexChanged: {
        if (editingItemKey.length > 0 && editingItemKey !== selectedItemKey)
            commitRename();
        activateSelectedHierarchyItem(true);
    }

    SidebarHierarchyLvrsAdapter {
        id: hierarchyAdapter

        hierarchyViewModel: sidebarHierarchyView.hierarchyViewModel
        searchQuery: sidebarHierarchyView.searchQuery

        onSelectedItemKeyChanged: {
            if (selectedItemKey.length > 0)
                hierarchyList.activateByKey(selectedItemKey);
        }
    }
    Rectangle {
        anchors.fill: parent
        color: sidebarHierarchyView.panelColor
    }
    Item {
        id: sidebarContents

        anchors.bottom: sidebarFooter.top
        anchors.left: parent.left
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        anchors.right: parent.right
        anchors.rightMargin: sidebarHierarchyView.horizontalInset
        anchors.top: parent.top
        anchors.topMargin: sidebarHierarchyView.verticalInset

        LV.VStack {
            anchors.fill: parent
            spacing: 2

            Item {
                Layout.fillWidth: true
                Layout.minimumWidth: sidebarHierarchyView.toolbarMinWidth
                Layout.preferredHeight: 20

                LV.HierarchyToolbar {
                    id: hierarchyHeaderToolbar

                    activeButtonId: sidebarHierarchyView.activeToolbarIndex
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    buttonItems: sidebarHierarchyView.toolbarItems
                    height: 20

                    onActiveChanged: function (button, buttonId, index) {
                        if (index < 0 || index === sidebarHierarchyView.activeToolbarIndex)
                            return;
                        sidebarHierarchyView.commitRename();
                        sidebarHierarchyView.toolbarIndexChangeRequested(index);
                    }
                }
            }
            LV.InputField {
                id: searchInput

                Layout.fillWidth: true
                Layout.preferredHeight: sidebarHierarchyView.searchHeight
                mode: searchMode
                selectByMouse: true
                text: sidebarHierarchyView.searchQuery

                onAccepted: function (text) {
                    sidebarHierarchyView.commitRename();
                    sidebarHierarchyView.searchQuery = text;
                }
                onTextEdited: function (text) {
                    if (sidebarHierarchyView.editingItemKey.length > 0)
                        sidebarHierarchyView.commitRename();
                    sidebarHierarchyView.searchQuery = text;
                }
            }
            Flickable {
                id: hierarchyViewport

                Layout.fillHeight: true
                Layout.fillWidth: true
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                contentHeight: hierarchyCanvas.height
                contentWidth: width
                interactive: contentHeight > height

                Item {
                    id: hierarchyCanvas

                    height: Math.max(hierarchyViewport.height, hierarchyList.implicitHeight)
                    width: hierarchyViewport.width

                    LV.HierarchyList {
                        id: hierarchyList

                        autoExpandAncestorsOnActivate: true
                        editable: hierarchyAdapter.editable
                        enabledRole: "enabled"
                        expandedRole: "expanded"
                        generatedIndentStep: 8
                        generatedItemWidth: hierarchyViewport.width
                        generatedRowHeight: 20
                        itemIdRole: "itemId"
                        itemKeyRole: "key"
                        keyboardNavigationEnabled: false
                        labelRole: "label"
                        model: hierarchyAdapter.nodes
                        showChevronRole: "showChevron"
                        width: hierarchyViewport.width

                        onActiveChanged: function (item, itemId, index) {
                            if (!item || item.itemKey === undefined || item.itemKey === null)
                                return;
                            hierarchyAdapter.activateKey(item.itemKey);
                        }
                        onEnsureVisibleRequested: function (y, itemHeight) {
                            if (y < hierarchyViewport.contentY)
                                hierarchyViewport.contentY = y;
                            else if (y + itemHeight > hierarchyViewport.contentY + hierarchyViewport.height)
                                hierarchyViewport.contentY = y + itemHeight - hierarchyViewport.height;
                        }
                        onItemMoved: function (item, itemId, itemKey, fromIndex, toIndex, depth) {
                            if (!hierarchyAdapter.commitEditableNodes(hierarchyList.model, hierarchyList.activeItemKey))
                                return;
                            sidebarHierarchyView.requestViewHook("move-folder-lvrs");
                        }
                    }
                    Item {
                        id: overlayLayer

                        anchors.fill: parent
                        z: 10

                        Repeater {
                            model: hierarchyAdapter.flatNodes

                            delegate: Item {
                                id: overlayDelegate

                                readonly property bool dragLocked: !!modelData.dragLocked
                                readonly property string itemKey: modelData.key === undefined || modelData.key === null ? "" : String(modelData.key)
                                required property var modelData
                                readonly property bool noteDropHighlighted: sidebarHierarchyView.noteDropTargetKey === itemKey
                                readonly property var targetItem: sidebarHierarchyView.targetItemForKey(itemKey)

                                height: targetItem ? targetItem.height : 0
                                visible: targetItem !== null && targetItem.visible !== false && height > 0
                                width: targetItem ? targetItem.width : hierarchyViewport.width
                                x: targetItem ? targetItem.x : 0
                                y: targetItem ? targetItem.y : 0

                                Rectangle {
                                    anchors.fill: parent
                                    color: LV.Theme.accentBlueMuted
                                    opacity: 0.4
                                    visible: overlayDelegate.noteDropHighlighted
                                }
                                MouseArea {
                                    acceptedButtons: Qt.LeftButton
                                    anchors.fill: parent
                                    enabled: overlayDelegate.dragLocked

                                    onClicked: {
                                        hierarchyAdapter.activateKey(overlayDelegate.itemKey);
                                    }
                                }
                                TapHandler {
                                    acceptedButtons: Qt.LeftButton
                                    enabled: !overlayDelegate.dragLocked && hierarchyAdapter.canRenameKey(overlayDelegate.itemKey)
                                    gesturePolicy: TapHandler.DragThreshold

                                    onDoubleTapped: {
                                        sidebarHierarchyView.beginRenameKey(overlayDelegate.itemKey, hierarchyAdapter.labelForKey(overlayDelegate.itemKey));
                                    }
                                }
                                DropArea {
                                    anchors.fill: parent
                                    enabled: hierarchyAdapter.noteDropEnabled
                                    keys: ["whatson.library.note"]

                                    onDropped: function (drop) {
                                        const noteId = sidebarHierarchyView.draggedNoteId(drop);
                                        const accepted = hierarchyAdapter.assignNoteToKey(overlayDelegate.itemKey, noteId);
                                        sidebarHierarchyView.noteDropTargetKey = "";
                                        if (!accepted)
                                            return;
                                        sidebarHierarchyView.requestViewHook("drop-note-to-folder");
                                    }
                                    onEntered: function (drag) {
                                        const noteId = sidebarHierarchyView.draggedNoteId(drag);
                                        sidebarHierarchyView.noteDropTargetKey = hierarchyAdapter.canAcceptNoteDrop(overlayDelegate.itemKey, noteId) ? overlayDelegate.itemKey : "";
                                    }
                                    onExited: {
                                        if (sidebarHierarchyView.noteDropTargetKey === overlayDelegate.itemKey)
                                            sidebarHierarchyView.noteDropTargetKey = "";
                                    }
                                }
                            }
                        }
                        Item {
                            id: renameOverlay

                            readonly property var targetItem: sidebarHierarchyView.targetItemForKey(sidebarHierarchyView.editingItemKey)

                            height: targetItem ? targetItem.height : 0
                            visible: sidebarHierarchyView.editingItemKey.length > 0 && targetItem !== null
                            width: targetItem ? targetItem.width : hierarchyViewport.width
                            x: targetItem ? targetItem.x : 0
                            y: targetItem ? targetItem.y : 0
                            z: 20

                            onVisibleChanged: {
                                if (visible)
                                    Qt.callLater(function () {
                                        renameTextInput.forceActiveFocus();
                                        renameTextInput.selectAll();
                                    });
                            }

                            Rectangle {
                                anchors.left: parent.left
                                anchors.leftMargin: LV.Theme.gap8 + (renameOverlay.targetItem && renameOverlay.targetItem.indentLevel !== undefined ? renameOverlay.targetItem.indentLevel * 8 : 0) + ((typeof LV.Theme.iconSm === "number" && isFinite(LV.Theme.iconSm)) ? LV.Theme.iconSm : 16) + ((typeof LV.Theme.gap2 === "number" && isFinite(LV.Theme.gap2)) ? LV.Theme.gap2 : 2)
                                anchors.right: parent.right
                                anchors.rightMargin: LV.Theme.gap8
                                anchors.verticalCenter: parent.verticalCenter
                                color: LV.Theme.panelBackground10
                                height: 20
                                radius: 4
                            }
                            TextInput {
                                id: renameTextInput

                                anchors.left: parent.left
                                anchors.leftMargin: LV.Theme.gap8 + (renameOverlay.targetItem && renameOverlay.targetItem.indentLevel !== undefined ? renameOverlay.targetItem.indentLevel * 8 : 0) + ((typeof LV.Theme.iconSm === "number" && isFinite(LV.Theme.iconSm)) ? LV.Theme.iconSm : 16) + ((typeof LV.Theme.gap2 === "number" && isFinite(LV.Theme.gap2)) ? LV.Theme.gap2 : 2) + LV.Theme.gap3
                                anchors.right: parent.right
                                anchors.rightMargin: LV.Theme.gap8 + LV.Theme.gap3
                                anchors.verticalCenter: parent.verticalCenter
                                clip: true
                                color: LV.Theme.titleHeaderColor
                                font.pixelSize: LV.Theme.textBody
                                font.weight: LV.Theme.textBodyWeight
                                height: 20
                                renderType: Text.NativeRendering
                                selectByMouse: true
                                selectionColor: LV.Theme.accentBlue
                                text: sidebarHierarchyView.editingText
                                verticalAlignment: TextInput.AlignVCenter

                                Keys.onEscapePressed: function (event) {
                                    sidebarHierarchyView.cancelRename();
                                    event.accepted = true;
                                }
                                Keys.onReturnPressed: function (event) {
                                    sidebarHierarchyView.commitRename();
                                    event.accepted = true;
                                }
                                onAccepted: sidebarHierarchyView.commitRename()
                                onActiveFocusChanged: {
                                    if (!activeFocus && sidebarHierarchyView.editingItemKey.length > 0)
                                        sidebarHierarchyView.commitRename();
                                }
                                onTextChanged: {
                                    if (sidebarHierarchyView.editingText !== text)
                                        sidebarHierarchyView.editingText = text;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    LV.ListFooter {
        id: sidebarFooter

        anchors.bottom: parent.bottom
        anchors.bottomMargin: sidebarHierarchyView.verticalInset
        anchors.left: parent.left
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        button1: ({
                type: "icon",
                enabled: sidebarHierarchyView.createFolderEnabled,
                iconName: "addFile",
                onClicked: function () {
                    if (!sidebarHierarchyView.createFolderEnabled)
                        return;
                    sidebarHierarchyView.commitRename();
                    if (sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.hierarchyViewModel.createFolder)
                        sidebarHierarchyView.hierarchyViewModel.createFolder();
                    Qt.callLater(function () {
                        if (sidebarHierarchyView.selectedItemKey.length > 0 && hierarchyAdapter.canRenameKey(sidebarHierarchyView.selectedItemKey))
                            sidebarHierarchyView.beginRenameKey(sidebarHierarchyView.selectedItemKey, hierarchyAdapter.labelForKey(sidebarHierarchyView.selectedItemKey));
                    });
                }
            })
        button2: ({
                type: "icon",
                enabled: sidebarHierarchyView.deleteFolderEnabled,
                iconName: "generaldelete",
                onClicked: function () {
                    if (!sidebarHierarchyView.deleteFolderEnabled)
                        return;
                    sidebarHierarchyView.cancelRename();
                    if (sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.hierarchyViewModel.deleteSelectedFolder)
                        sidebarHierarchyView.hierarchyViewModel.deleteSelectedFolder();
                }
            })
        button3: ({
                type: "menu",
                enabled: sidebarHierarchyView.viewOptionsEnabled,
                iconName: "settings",
                tone: LV.AbstractButton.Default,
                onClicked: function () {
                    if (!sidebarHierarchyView.viewOptionsEnabled)
                        return;
                    sidebarHierarchyView.toggleViewOptionsMenu();
                }
            })
        interactive: true
    }
    LV.ContextMenu {
        id: viewOptionsContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: 176
        items: sidebarHierarchyView.viewOptionsMenuItems
        modal: false
        parent: Controls.Overlay.overlay
    }
}
