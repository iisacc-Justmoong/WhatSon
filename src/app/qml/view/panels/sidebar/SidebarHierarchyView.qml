import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

Item {
    id: sidebarHierarchyView

    property int activeToolbarIndex: defaultToolbarIndex
    readonly property int contentWidth: Math.max(0, width - horizontalInset * 2)
    readonly property bool createFolderEnabled: hierarchyViewModel && hierarchyViewModel.createFolderEnabled !== undefined ? hierarchyViewModel.createFolderEnabled : false
    property int defaultToolbarIndex: 0
    readonly property bool deleteFolderEnabled: hierarchyViewModel && hierarchyViewModel.deleteFolderEnabled !== undefined ? hierarchyViewModel.deleteFolderEnabled : false
    property alias editingIndex: sidebarController.editingIndex
    property alias editingText: sidebarController.editingText
    property bool folderDragActive: false
    property alias folderDropAsChild: sidebarController.folderDropAsChild
    property alias folderDropBefore: sidebarController.folderDropBefore
    property alias folderDropTargetIndex: sidebarController.folderDropTargetIndex
    // SOURCE-OF-TRUTH GUARD:
    // Hierarchy list must always come from per-domain ViewModel.itemModel (store-backed path).
    // Do not add any UI-side sample/fallback/depth injection model here.
    readonly property var folderModel: hierarchyViewModel ? hierarchyViewModel.itemModel : null
    readonly property int footerHeight: 24
    property string frameName: ""
    property string frameNodeId: ""
    readonly property int hierarchyChevronSlotWidth: ((typeof LV.Theme.iconSm === "number" && isFinite(LV.Theme.iconSm)) ? LV.Theme.iconSm : 16) + ((typeof LV.Theme.gap2 === "number" && isFinite(LV.Theme.gap2)) ? LV.Theme.gap2 : 2)
    readonly property int hierarchyIndentStep: 8
    readonly property int hierarchyItemBaseLeftPadding: (typeof LV.Theme.gap8 === "number" && isFinite(LV.Theme.gap8)) ? LV.Theme.gap8 : 8
    property var hierarchyViewModel: null
    property int horizontalInset: 2
    property alias noteDropTargetIndex: sidebarController.noteDropTargetIndex
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.SidebarHierarchyView") : null
    readonly property bool renameEnabled: hierarchyViewModel && hierarchyViewModel.renameEnabled !== undefined ? hierarchyViewModel.renameEnabled : false
    property alias rootDropHighlighted: sidebarController.rootDropHighlighted
    readonly property int searchHeight: (typeof LV.Theme.gap18 === "number" && isFinite(LV.Theme.gap18)) ? LV.Theme.gap18 : 18
    property string searchQuery: ""
    readonly property int searchRadius: 5
    readonly property int selectedFolderIndex: hierarchyViewModel && hierarchyViewModel.selectedIndex !== undefined ? hierarchyViewModel.selectedIndex : -1
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

    function activateHierarchyDelegate(delegate, index) {
        sidebarController.activateHierarchyDelegate(delegate, index);
    }
    function activateSelectedHierarchyItem(focusView) {
        sidebarController.activateSelectedHierarchyItem(focusView);
    }
    function assignNoteToFolder(index, noteId) {
        return sidebarController.assignNoteToFolder(index, noteId);
    }
    function beginRename(index, currentLabel) {
        sidebarController.beginRename(index, currentLabel);
    }
    function canAcceptFolderDrop(sourceIndex, targetIndex, asChild) {
        return sidebarController.canAcceptFolderDrop(sourceIndex, targetIndex, asChild);
    }
    function canAcceptFolderDropBefore(sourceIndex, targetIndex) {
        return sidebarController.canAcceptFolderDropBefore(sourceIndex, targetIndex);
    }
    function canAcceptNoteDrop(index, noteId) {
        return sidebarController.canAcceptNoteDrop(index, noteId);
    }
    function canMoveFolder(index) {
        return sidebarController.canMoveFolder(index);
    }
    function canMoveFolderToRoot(sourceIndex) {
        return sidebarController.canMoveFolderToRoot(sourceIndex);
    }
    function canRenameAtIndex(index) {
        return sidebarController.canRenameAtIndex(index);
    }
    function cancelRename() {
        sidebarController.cancelRename();
    }
    function clearHierarchySelection() {
        sidebarController.clearHierarchySelection();
    }
    function commitRename() {
        sidebarController.commitRename();
    }
    function draggedFolderIndex(event) {
        return sidebarController.draggedFolderIndex(event);
    }
    function draggedNoteId(event) {
        return sidebarController.draggedNoteId(event);
    }
    function matchesSearchText(label) {
        var query = sidebarHierarchyView.searchQuery.trim().toLowerCase();
        if (query.length === 0)
            return true;
        var target = (label === undefined || label === null) ? "" : String(label).toLowerCase();
        return target.indexOf(query) >= 0;
    }
    function moveFolder(sourceIndex, targetIndex, asChild) {
        return sidebarController.moveFolder(sourceIndex, targetIndex, asChild);
    }
    function moveFolderBefore(sourceIndex, targetIndex) {
        return sidebarController.moveFolderBefore(sourceIndex, targetIndex);
    }
    function moveFolderToRoot(sourceIndex) {
        return sidebarController.moveFolderToRoot(sourceIndex);
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function resetDropTargets() {
        sidebarController.resetDropTargets();
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

        if (sidebarHierarchyView.editingIndex >= 0) {
            sidebarHierarchyView.commitRename();
            event.accepted = true;
            return;
        }

        if (sidebarHierarchyView.selectedFolderIndex < 0)
            return;

        if (!sidebarHierarchyView.hierarchyViewModel || sidebarHierarchyView.hierarchyViewModel.itemLabel === undefined || !sidebarHierarchyView.canRenameAtIndex(sidebarHierarchyView.selectedFolderIndex))
            return;

        sidebarHierarchyView.beginRename(sidebarHierarchyView.selectedFolderIndex, sidebarHierarchyView.hierarchyViewModel.itemLabel(sidebarHierarchyView.selectedFolderIndex));
        event.accepted = true;
    }
    onHierarchyViewModelChanged: {
        sidebarHierarchyView.cancelRename();
        sidebarHierarchyView.folderDragActive = false;
        sidebarHierarchyView.resetDropTargets();
    }
    onSelectedFolderIndexChanged: {
        if (sidebarHierarchyView.editingIndex >= 0 && sidebarHierarchyView.editingIndex !== sidebarHierarchyView.selectedFolderIndex)
            sidebarHierarchyView.commitRename();
        if (sidebarHierarchyView.selectedFolderIndex < 0)
            return;
        sidebarHierarchyView.activateSelectedHierarchyItem(true);
    }

    SidebarHierarchyInteractionController {
        id: sidebarController

        folderRepeater: folderRepeater
        hierarchyList: hierarchyList
        hierarchyViewModel: sidebarHierarchyView.hierarchyViewModel
        hierarchyViewport: hierarchyViewport
        requestViewHook: function (reason) {
            sidebarHierarchyView.requestViewHook(reason);
        }
        selectedFolderIndex: sidebarHierarchyView.selectedFolderIndex
        viewRoot: sidebarHierarchyView
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
                    if (sidebarHierarchyView.editingIndex >= 0)
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
                contentHeight: Math.max(height, hierarchyList.implicitHeight)
                contentWidth: width
                interactive: contentHeight > height && !sidebarHierarchyView.folderDragActive

                HierarchyListCompat {
                    id: hierarchyList

                    autoSelectFirstItem: false
                    keyboardNavigationEnabled: false
                    manualActivationOnly: true
                    width: hierarchyViewport.width

                    onActiveChanged: function (item, itemId, index) {
                        if (sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.hierarchyViewModel.setSelectedIndex !== undefined)
                            sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(index);
                    }

                    Item {
                        id: rootFolderDropZone

                        height: 6
                        width: hierarchyViewport.width

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            color: LV.Theme.accentBlue
                            height: 2
                            opacity: 0.85
                            visible: sidebarHierarchyView.rootDropHighlighted
                        }
                        DropArea {
                            anchors.fill: parent
                            keys: ["whatson.hierarchy.folder"]

                            onDropped: function (drop) {
                                const sourceIndex = sidebarHierarchyView.draggedFolderIndex(drop);
                                const accepted = sidebarHierarchyView.moveFolderToRoot(sourceIndex);
                                sidebarHierarchyView.resetDropTargets();
                                if (!accepted)
                                    return;
                                sidebarController.notifyAcceptedInteraction("move-folder-to-root");
                            }
                            onEntered: function (drag) {
                                const sourceIndex = sidebarHierarchyView.draggedFolderIndex(drag);
                                sidebarHierarchyView.noteDropTargetIndex = -1;
                                sidebarHierarchyView.folderDropTargetIndex = -1;
                                sidebarHierarchyView.folderDropAsChild = false;
                                sidebarHierarchyView.folderDropBefore = false;
                                sidebarHierarchyView.rootDropHighlighted = sidebarHierarchyView.canMoveFolderToRoot(sourceIndex);
                            }
                            onExited: {
                                sidebarHierarchyView.rootDropHighlighted = false;
                            }
                        }
                    }
                    Repeater {
                        id: folderRepeater

                        model: sidebarHierarchyView.folderModel

                        delegate: LV.HierarchyItem {
                            id: hierarchyDelegate

                            readonly property bool folderBeforeDropHighlighted: sidebarHierarchyView.folderDropTargetIndex === index && sidebarHierarchyView.folderDropBefore
                            readonly property bool folderChildDropHighlighted: sidebarHierarchyView.folderDropTargetIndex === index && sidebarHierarchyView.folderDropAsChild && !sidebarHierarchyView.folderDropBefore
                            readonly property bool folderSiblingDropHighlighted: sidebarHierarchyView.folderDropTargetIndex === index && !sidebarHierarchyView.folderDropAsChild && !sidebarHierarchyView.folderDropBefore
                            required property int index
                            readonly property bool itemExpanded: model.expanded === undefined ? false : !!model.expanded
                            readonly property int itemIndentLevel: {
                                if (model.indentLevel === undefined || model.indentLevel === null)
                                    return 0;
                                var parsed = Number(model.indentLevel);
                                if (!isFinite(parsed))
                                    return 0;
                                return Math.max(0, Math.floor(parsed));
                            }
                            readonly property string itemKeyValue: {
                                if (model.itemKey === undefined || model.itemKey === null)
                                    return String(index);
                                var rawItemKey = String(model.itemKey).trim();
                                return rawItemKey.length > 0 ? rawItemKey : String(index);
                            }
                            readonly property string itemLabel: model.label === undefined || model.label === null ? "" : String(model.label)
                            readonly property bool itemShowChevron: model.showChevron === undefined ? false : !!model.showChevron
                            readonly property bool matchesSearch: sidebarHierarchyView.matchesSearchText(itemLabel)
                            required property var model
                            readonly property bool noteDropHighlighted: sidebarHierarchyView.noteDropTargetIndex === index
                            readonly property int renameLeftInset: sidebarHierarchyView.hierarchyItemBaseLeftPadding + itemIndentLevel * sidebarHierarchyView.hierarchyIndentStep + sidebarHierarchyView.hierarchyChevronSlotWidth
                            readonly property int renameRightInset: sidebarHierarchyView.hierarchyItemBaseLeftPadding
                            property int sourceIndex: index
                            readonly property bool visibleInView: matchesSearch && rowVisible

                            Drag.active: folderDragHandler.active
                            Drag.hotSpot.x: width * 0.5
                            Drag.hotSpot.y: height * 0.5
                            Drag.keys: ["whatson.hierarchy.folder"]
                            Drag.source: hierarchyDelegate
                            Drag.supportedActions: Qt.MoveAction
                            baseLeftPadding: sidebarHierarchyView.hierarchyItemBaseLeftPadding
                            dragPreviewActive: folderDragHandler.active
                            expanded: hierarchyDelegate.itemExpanded
                            height: visibleInView ? implicitHeight : 0
                            indentLevel: hierarchyDelegate.itemIndentLevel
                            indentStep: sidebarHierarchyView.hierarchyIndentStep
                            itemId: index
                            itemKey: hierarchyDelegate.itemKeyValue
                            label: index === sidebarHierarchyView.editingIndex ? "" : hierarchyDelegate.itemLabel
                            opacity: folderDragHandler.active ? 0.72 : 1
                            showChevron: hierarchyDelegate.itemShowChevron
                            visible: visibleInView
                            width: hierarchyViewport.width
                            z: folderDragHandler.active ? 20 : 0

                            Rectangle {
                                anchors.fill: parent
                                color: LV.Theme.accentBlueMuted
                                opacity: hierarchyDelegate.noteDropHighlighted ? 0.55 : 0.38
                                visible: hierarchyDelegate.noteDropHighlighted || hierarchyDelegate.folderChildDropHighlighted
                                z: 1
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                color: LV.Theme.accentBlue
                                height: 2
                                opacity: 0.85
                                visible: hierarchyDelegate.folderBeforeDropHighlighted
                                z: 2
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: LV.Theme.accentBlue
                                height: 2
                                opacity: 0.85
                                visible: hierarchyDelegate.folderSiblingDropHighlighted
                                z: 2
                            }
                            TapHandler {
                                acceptedButtons: Qt.LeftButton
                                gesturePolicy: TapHandler.DragThreshold

                                onDoubleTapped: {
                                    sidebarHierarchyView.activateHierarchyDelegate(hierarchyDelegate, index);
                                    sidebarHierarchyView.beginRename(index, hierarchyDelegate.itemLabel);
                                }
                            }
                            DragHandler {
                                id: folderDragHandler

                                acceptedButtons: Qt.LeftButton
                                dragThreshold: 4
                                enabled: sidebarHierarchyView.canMoveFolder(index) && sidebarHierarchyView.editingIndex !== index
                                grabPermissions: PointerHandler.CanTakeOverFromAnything
                                target: null

                                onActiveChanged: {
                                    sidebarHierarchyView.folderDragActive = active;
                                    if (active && sidebarHierarchyView.editingIndex >= 0)
                                        sidebarHierarchyView.commitRename();
                                    if (active)
                                        sidebarHierarchyView.activateHierarchyDelegate(hierarchyDelegate, index);
                                    if (!active)
                                        sidebarHierarchyView.resetDropTargets();
                                }
                            }
                            DropArea {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                height: 6
                                keys: ["whatson.hierarchy.folder"]

                                onDropped: function (drop) {
                                    const sourceIndex = sidebarHierarchyView.draggedFolderIndex(drop);
                                    const accepted = sidebarHierarchyView.moveFolderBefore(sourceIndex, index);
                                    sidebarHierarchyView.resetDropTargets();
                                    if (!accepted)
                                        return;
                                    sidebarController.notifyAcceptedInteraction("move-folder-before");
                                }
                                onEntered: function (drag) {
                                    const sourceIndex = sidebarHierarchyView.draggedFolderIndex(drag);
                                    sidebarHierarchyView.noteDropTargetIndex = -1;
                                    sidebarHierarchyView.rootDropHighlighted = false;
                                    sidebarHierarchyView.folderDropTargetIndex = sidebarHierarchyView.canAcceptFolderDropBefore(sourceIndex, index) ? index : -1;
                                    sidebarHierarchyView.folderDropAsChild = false;
                                    sidebarHierarchyView.folderDropBefore = true;
                                }
                                onExited: {
                                    if (sidebarHierarchyView.folderDropTargetIndex === index && sidebarHierarchyView.folderDropBefore)
                                        sidebarHierarchyView.folderDropTargetIndex = -1;
                                }
                            }
                            DropArea {
                                anchors.fill: parent
                                keys: ["whatson.library.note"]

                                onDropped: function (drop) {
                                    const noteId = sidebarHierarchyView.draggedNoteId(drop);
                                    const accepted = sidebarHierarchyView.assignNoteToFolder(index, noteId);
                                    sidebarHierarchyView.resetDropTargets();
                                    if (!accepted)
                                        return;
                                    sidebarController.notifyAcceptedInteraction("drop-note-to-folder");
                                }
                                onEntered: function (drag) {
                                    const noteId = sidebarHierarchyView.draggedNoteId(drag);
                                    sidebarHierarchyView.folderDropTargetIndex = -1;
                                    sidebarHierarchyView.folderDropBefore = false;
                                    sidebarHierarchyView.rootDropHighlighted = false;
                                    sidebarHierarchyView.noteDropTargetIndex = sidebarHierarchyView.canAcceptNoteDrop(index, noteId) ? index : -1;
                                }
                                onExited: {
                                    if (sidebarHierarchyView.noteDropTargetIndex === index)
                                        sidebarHierarchyView.noteDropTargetIndex = -1;
                                }
                            }
                            DropArea {
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 4
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.topMargin: 4
                                keys: ["whatson.hierarchy.folder"]

                                onDropped: function (drop) {
                                    const sourceIndex = sidebarHierarchyView.draggedFolderIndex(drop);
                                    const accepted = sidebarHierarchyView.moveFolder(sourceIndex, index, true);
                                    sidebarHierarchyView.resetDropTargets();
                                    if (!accepted)
                                        return;
                                    sidebarController.notifyAcceptedInteraction("move-folder-child");
                                }
                                onEntered: function (drag) {
                                    const sourceIndex = sidebarHierarchyView.draggedFolderIndex(drag);
                                    sidebarHierarchyView.noteDropTargetIndex = -1;
                                    sidebarHierarchyView.rootDropHighlighted = false;
                                    sidebarHierarchyView.folderDropTargetIndex = sidebarHierarchyView.canAcceptFolderDrop(sourceIndex, index, true) ? index : -1;
                                    sidebarHierarchyView.folderDropAsChild = true;
                                    sidebarHierarchyView.folderDropBefore = false;
                                }
                                onExited: {
                                    if (sidebarHierarchyView.folderDropTargetIndex === index && sidebarHierarchyView.folderDropAsChild)
                                        sidebarHierarchyView.folderDropTargetIndex = -1;
                                }
                            }
                            DropArea {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 6
                                keys: ["whatson.hierarchy.folder"]

                                onDropped: function (drop) {
                                    const sourceIndex = sidebarHierarchyView.draggedFolderIndex(drop);
                                    const accepted = sidebarHierarchyView.moveFolder(sourceIndex, index, false);
                                    sidebarHierarchyView.resetDropTargets();
                                    if (!accepted)
                                        return;
                                    sidebarController.notifyAcceptedInteraction("move-folder-sibling");
                                }
                                onEntered: function (drag) {
                                    const sourceIndex = sidebarHierarchyView.draggedFolderIndex(drag);
                                    sidebarHierarchyView.noteDropTargetIndex = -1;
                                    sidebarHierarchyView.rootDropHighlighted = false;
                                    sidebarHierarchyView.folderDropTargetIndex = sidebarHierarchyView.canAcceptFolderDrop(sourceIndex, index, false) ? index : -1;
                                    sidebarHierarchyView.folderDropAsChild = false;
                                    sidebarHierarchyView.folderDropBefore = false;
                                }
                                onExited: {
                                    if (sidebarHierarchyView.folderDropTargetIndex === index && !sidebarHierarchyView.folderDropAsChild && !sidebarHierarchyView.folderDropBefore)
                                        sidebarHierarchyView.folderDropTargetIndex = -1;
                                }
                            }
                            Item {
                                id: renameOverlay

                                anchors.fill: parent
                                visible: index === sidebarHierarchyView.editingIndex
                                z: 2

                                onVisibleChanged: {
                                    if (visible)
                                        Qt.callLater(function () {
                                            renameTextInput.forceActiveFocus();
                                            renameTextInput.selectAll();
                                        });
                                }

                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.leftMargin: renameLeftInset
                                    anchors.right: parent.right
                                    anchors.rightMargin: renameRightInset
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: LV.Theme.panelBackground10
                                    height: 20
                                    radius: 4
                                }
                                TextInput {
                                    id: renameTextInput

                                    anchors.left: parent.left
                                    anchors.leftMargin: renameLeftInset + LV.Theme.gap3
                                    anchors.right: parent.right
                                    anchors.rightMargin: renameRightInset + LV.Theme.gap3
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
                                        if (!activeFocus && index === sidebarHierarchyView.editingIndex)
                                            sidebarHierarchyView.commitRename();
                                    }
                                    onTextChanged: {
                                        if (index === sidebarHierarchyView.editingIndex && sidebarHierarchyView.editingText !== text)
                                            sidebarHierarchyView.editingText = text;
                                    }
                                }
                            }
                        }
                    }
                }
                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: hierarchyList.bottom
                    height: Math.max(0, hierarchyViewport.height - hierarchyList.implicitHeight)
                    visible: height > 0

                    TapHandler {
                        acceptedButtons: Qt.LeftButton

                        onTapped: {
                            sidebarHierarchyView.clearHierarchySelection();
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
                    if (sidebarHierarchyView.hierarchyViewModel)
                        sidebarHierarchyView.hierarchyViewModel.createFolder();
                    if (sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.hierarchyViewModel.selectedIndex >= 0) {
                        var createdIndex = sidebarHierarchyView.hierarchyViewModel.selectedIndex;
                        sidebarHierarchyView.activateSelectedHierarchyItem(true);
                        if (sidebarHierarchyView.canRenameAtIndex(createdIndex))
                            Qt.callLater(function () {
                                sidebarHierarchyView.beginRename(createdIndex, sidebarHierarchyView.hierarchyViewModel.itemLabel(createdIndex));
                            });
                    }
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
                    if (sidebarHierarchyView.hierarchyViewModel)
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
