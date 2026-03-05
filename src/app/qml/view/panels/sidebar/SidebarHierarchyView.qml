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
    property int editingIndex: -1
    property string editingText: ""
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
    readonly property int horizontalInset: (typeof LV.Theme.gap8 === "number" && isFinite(LV.Theme.gap8)) ? LV.Theme.gap8 : 8
    property color panelColor: LV.Theme.panelBackground04
    readonly property bool renameEnabled: hierarchyViewModel && hierarchyViewModel.renameEnabled !== undefined ? hierarchyViewModel.renameEnabled : false
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
    readonly property int verticalInset: (typeof LV.Theme.gap2 === "number" && isFinite(LV.Theme.gap2)) ? LV.Theme.gap2 : 2
    readonly property bool viewOptionsEnabled: hierarchyViewModel && hierarchyViewModel.viewOptionsEnabled !== undefined ? hierarchyViewModel.viewOptionsEnabled : true
    property var viewOptionsMenuItems: []

    signal toolbarIndexChangeRequested(int index)
    signal viewHookRequested

    function beginRename(index, currentLabel) {
        if (index < 0)
            return;
        if (!sidebarHierarchyView.canRenameAtIndex(index))
            return;
        if (sidebarHierarchyView.editingIndex >= 0 && sidebarHierarchyView.editingIndex !== index)
            sidebarHierarchyView.commitRename();
        if (sidebarHierarchyView.hierarchyViewModel)
            sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(index);
        sidebarHierarchyView.editingIndex = index;
        sidebarHierarchyView.editingText = currentLabel;
    }
    function canRenameAtIndex(index) {
        if (index < 0 || !sidebarHierarchyView.hierarchyViewModel)
            return false;
        if (sidebarHierarchyView.hierarchyViewModel.canRenameItem !== undefined)
            return sidebarHierarchyView.hierarchyViewModel.canRenameItem(index);
        return false;
    }
    function cancelRename() {
        sidebarHierarchyView.editingIndex = -1;
        sidebarHierarchyView.editingText = "";
    }
    function commitRename() {
        if (sidebarHierarchyView.editingIndex < 0)
            return;
        if (sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.canRenameAtIndex(sidebarHierarchyView.editingIndex))
            sidebarHierarchyView.hierarchyViewModel.renameItem(sidebarHierarchyView.editingIndex, sidebarHierarchyView.editingText);
        sidebarHierarchyView.editingIndex = -1;
        sidebarHierarchyView.editingText = "";
    }
    function matchesSearchText(label) {
        var query = sidebarHierarchyView.searchQuery.trim().toLowerCase();
        if (query.length === 0)
            return true;
        var target = (label === undefined || label === null) ? "" : String(label).toLowerCase();
        return target.indexOf(query) >= 0;
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
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
    }
    onSelectedFolderIndexChanged: {
        if (sidebarHierarchyView.editingIndex >= 0 && sidebarHierarchyView.editingIndex !== sidebarHierarchyView.selectedFolderIndex)
            sidebarHierarchyView.commitRename();
        if (sidebarHierarchyView.selectedFolderIndex < 0)
            return;
        if (hierarchyList && hierarchyList.activateById !== undefined)
            hierarchyList.activateById(sidebarHierarchyView.selectedFolderIndex);
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
            spacing: LV.Theme.gap2

            LV.HierarchyToolbar {
                id: hierarchyHeaderToolbar

                Layout.alignment: Qt.AlignLeft
                Layout.maximumWidth: Math.max(sidebarHierarchyView.toolbarMinWidth, sidebarHierarchyView.contentWidth)
                Layout.minimumWidth: sidebarHierarchyView.toolbarMinWidth
                Layout.preferredHeight: 20
                Layout.preferredWidth: Math.max(sidebarHierarchyView.toolbarMinWidth, sidebarHierarchyView.contentWidth)
                activeButtonId: sidebarHierarchyView.activeToolbarIndex
                buttonItems: sidebarHierarchyView.toolbarItems
                spacing: LV.Theme.gap2

                onActiveChanged: function (button, buttonId, index) {
                    if (index < 0 || index === sidebarHierarchyView.activeToolbarIndex)
                        return;
                    sidebarHierarchyView.commitRename();
                    sidebarHierarchyView.toolbarIndexChangeRequested(index);
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
                contentHeight: hierarchyList.implicitHeight
                contentWidth: width
                interactive: contentHeight > height

                LV.HierarchyList {
                    id: hierarchyList

                    keyboardNavigationEnabled: false
                    width: hierarchyViewport.width

                    Repeater {
                        model: sidebarHierarchyView.folderModel

                        delegate: LV.HierarchyItem {
                            id: hierarchyDelegate

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
                            readonly property string itemLabel: model.label === undefined || model.label === null ? "" : String(model.label)
                            readonly property bool itemShowChevron: model.showChevron === undefined ? false : !!model.showChevron
                            readonly property bool matchesSearch: sidebarHierarchyView.matchesSearchText(itemLabel)
                            required property var model
                            readonly property int renameLeftInset: sidebarHierarchyView.hierarchyItemBaseLeftPadding + itemIndentLevel * sidebarHierarchyView.hierarchyIndentStep + sidebarHierarchyView.hierarchyChevronSlotWidth
                            readonly property int renameRightInset: sidebarHierarchyView.hierarchyItemBaseLeftPadding
                            readonly property int selectionAreaRightMargin: itemShowChevron ? sidebarHierarchyView.hierarchyChevronSlotWidth + sidebarHierarchyView.hierarchyItemBaseLeftPadding : 0
                            readonly property bool visibleInView: matchesSearch && rowVisible

                            baseLeftPadding: sidebarHierarchyView.hierarchyItemBaseLeftPadding
                            expanded: hierarchyDelegate.itemExpanded
                            height: visibleInView ? implicitHeight : 0
                            indentLevel: hierarchyDelegate.itemIndentLevel
                            indentStep: sidebarHierarchyView.hierarchyIndentStep
                            itemId: index
                            label: index === sidebarHierarchyView.editingIndex ? "" : hierarchyDelegate.itemLabel
                            showChevron: hierarchyDelegate.itemShowChevron
                            visible: visibleInView
                            width: hierarchyViewport.width

                            MouseArea {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.rightMargin: selectionAreaRightMargin
                                anchors.top: parent.top

                                onClicked: {
                                    if (sidebarHierarchyView.editingIndex >= 0)
                                        sidebarHierarchyView.commitRename();
                                    sidebarHierarchyView.forceActiveFocus();
                                    if (hierarchyList && hierarchyList.requestActivate !== undefined)
                                        hierarchyList.requestActivate(hierarchyDelegate);
                                    if (sidebarHierarchyView.hierarchyViewModel)
                                        sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(index);
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
                        if (sidebarHierarchyView.canRenameAtIndex(createdIndex))
                            sidebarHierarchyView.beginRename(createdIndex, sidebarHierarchyView.hierarchyViewModel.itemLabel(createdIndex));
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
