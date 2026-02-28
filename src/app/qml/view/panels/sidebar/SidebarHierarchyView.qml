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
    property var depthItems: []
    property int editingIndex: -1
    property string editingText: ""
    readonly property var folderModel: hierarchyViewModel ? hierarchyViewModel.itemModel : null
    readonly property int footerHeight: 24
    property string frameName: ""
    property string frameNodeId: ""
    property var hierarchyViewModel: (typeof libraryHierarchyViewModel !== "undefined" && libraryHierarchyViewModel) ? libraryHierarchyViewModel : null
    readonly property int horizontalInset: (typeof LV.Theme.gap8 === "number" && isFinite(LV.Theme.gap8)) ? LV.Theme.gap8 : 8
    property color panelColor: LV.Theme.panelBackground04
    readonly property bool renameEnabled: hierarchyViewModel && hierarchyViewModel.renameEnabled !== undefined ? hierarchyViewModel.renameEnabled : false
    readonly property int rowHeight: 28
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
    property var viewOptionsMenuItems: []

    signal toolbarIndexChangeRequested(int index)

    function beginRename(index, currentLabel) {
        if (index < 0)
            return;
        if (sidebarHierarchyView.editingIndex >= 0 && sidebarHierarchyView.editingIndex !== index)
            sidebarHierarchyView.commitRename();
        if (sidebarHierarchyView.hierarchyViewModel)
            sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(index);
        sidebarHierarchyView.editingIndex = index;
        sidebarHierarchyView.editingText = currentLabel;
    }
    function cancelRename() {
        sidebarHierarchyView.editingIndex = -1;
        sidebarHierarchyView.editingText = "";
    }
    function commitRename() {
        if (sidebarHierarchyView.editingIndex < 0)
            return;
        if (sidebarHierarchyView.hierarchyViewModel)
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
    function toggleViewOptionsMenu() {
        if (viewOptionsContextMenu.opened) {
            viewOptionsContextMenu.close();
            return;
        }
        viewOptionsContextMenu.openFor(sidebarFooter, 0, sidebarFooter.height + verticalInset);
    }

    clip: true

    Component.onCompleted: {
        if (sidebarHierarchyView.hierarchyViewModel)
            sidebarHierarchyView.hierarchyViewModel.setDepthItems(sidebarHierarchyView.depthItems);
    }
    onDepthItemsChanged: {
        if (sidebarHierarchyView.hierarchyViewModel)
            sidebarHierarchyView.hierarchyViewModel.setDepthItems(sidebarHierarchyView.depthItems);
        sidebarHierarchyView.cancelRename();
    }
    onHierarchyViewModelChanged: sidebarHierarchyView.cancelRename()

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
                backgroundColor: "transparent"
                backgroundOpacity: 1.0
                buttonItems: sidebarHierarchyView.toolbarItems
                horizontalPadding: 0
                spacing: LV.Theme.gap2
                verticalPadding: 0

                onActiveChanged: function (button, buttonId, index) {
                    if (index < 0 || index === sidebarHierarchyView.activeToolbarIndex)
                        return;
                    sidebarHierarchyView.activeToolbarIndex = index;
                    sidebarHierarchyView.toolbarIndexChangeRequested(index);
                }
            }
            LV.InputField {
                id: searchInput

                Layout.fillWidth: true
                Layout.preferredHeight: sidebarHierarchyView.searchHeight
                backgroundColor: LV.Theme.panelBackground10
                backgroundColorDisabled: LV.Theme.panelBackground10
                backgroundColorFocused: LV.Theme.panelBackground10
                backgroundColorHover: LV.Theme.panelBackground10
                backgroundColorPressed: LV.Theme.panelBackground10
                clearButtonVisible: true
                cornerRadius: sidebarHierarchyView.searchRadius
                fieldMinHeight: sidebarHierarchyView.searchHeight
                insetHorizontal: LV.Theme.gap7
                insetVertical: LV.Theme.gap3
                mode: searchMode
                placeholderText: "Search"
                selectByMouse: true
                sideSpacing: LV.Theme.gap5
                text: sidebarHierarchyView.searchQuery

                onAccepted: function (text) {
                    sidebarHierarchyView.searchQuery = text;
                }
                onTextEdited: function (text) {
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
                    rowSpacing: 0
                    width: hierarchyViewport.width

                    Repeater {
                        model: sidebarHierarchyView.folderModel

                        delegate: Item {
                            required property bool accent
                            required property bool expanded
                            required property int indentLevel
                            required property int index
                            required property string label
                            readonly property bool matchesSearch: sidebarHierarchyView.matchesSearchText(label)
                            readonly property int renameLeftInset: LV.Theme.gap8 + indentLevel * 13 + LV.Theme.iconSm + LV.Theme.gap2
                            readonly property int renameRightInset: showChevron ? (LV.Theme.iconSm + LV.Theme.gap8) : LV.Theme.gap8
                            required property bool showChevron

                            height: matchesSearch ? sidebarHierarchyView.rowHeight : 0
                            visible: matchesSearch
                            width: hierarchyViewport.width

                            LV.HierarchyItem {
                                anchors.fill: parent
                                baseLeftPadding: LV.Theme.gap8
                                chevronColor: LV.Theme.darkGrey10
                                chevronSize: LV.Theme.iconSm
                                expanded: expanded
                                iconPlaceholderColor: LV.Theme.accentGrayLight
                                iconSize: LV.Theme.iconSm
                                indentLevel: indentLevel
                                indentStep: 13
                                itemWidth: hierarchyViewport.width
                                label: index === sidebarHierarchyView.editingIndex ? "" : label
                                leadingSpacing: LV.Theme.gap2
                                rowBackgroundColor: index === sidebarHierarchyView.selectedFolderIndex ? LV.Theme.panelBackground12 : "transparent"
                                rowBackgroundColorHover: index === sidebarHierarchyView.selectedFolderIndex ? LV.Theme.panelBackground12 : "transparent"
                                rowBackgroundColorPressed: index === sidebarHierarchyView.selectedFolderIndex ? LV.Theme.panelBackground12 : "transparent"
                                rowHeight: sidebarHierarchyView.rowHeight
                                rowRightPadding: LV.Theme.gap8
                                showChevron: showChevron
                                textColorNormal: accent ? LV.Theme.accentBlue : LV.Theme.bodyColor
                            }
                            MouseArea {
                                anchors.fill: parent

                                onClicked: {
                                    if (sidebarHierarchyView.renameEnabled) {
                                        sidebarHierarchyView.beginRename(index, label);
                                        return;
                                    }
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
                iconSize: LV.Theme.iconSm,
                tone: LV.AbstractButton.Borderless,
                onClicked: function () {
                    if (!sidebarHierarchyView.createFolderEnabled)
                        return;
                    sidebarHierarchyView.commitRename();
                    if (sidebarHierarchyView.hierarchyViewModel)
                        sidebarHierarchyView.hierarchyViewModel.createFolder();
                    if (sidebarHierarchyView.renameEnabled && sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.hierarchyViewModel.selectedIndex >= 0) {
                        var createdIndex = sidebarHierarchyView.hierarchyViewModel.selectedIndex;
                        sidebarHierarchyView.beginRename(createdIndex, sidebarHierarchyView.hierarchyViewModel.itemLabel(createdIndex));
                    }
                }
            })
        button2: ({
                type: "icon",
                enabled: sidebarHierarchyView.deleteFolderEnabled,
                iconName: "generaldelete",
                iconSize: LV.Theme.iconSm,
                tone: LV.AbstractButton.Borderless,
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
                iconName: "settings",
                iconSize: LV.Theme.iconSm,
                tone: LV.AbstractButton.Default,
                onClicked: function () {
                    sidebarHierarchyView.toggleViewOptionsMenu();
                }
            })
        horizontalPadding: LV.Theme.gap2
        interactive: true
        spacing: LV.Theme.gapNone
        verticalPadding: LV.Theme.gap2
    }
    LV.ContextMenu {
        id: viewOptionsContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        dismissOnGlobalContextRequest: true
        dismissOnGlobalPress: true
        itemWidth: 176
        items: sidebarHierarchyView.viewOptionsMenuItems
        modal: false
        parent: Controls.Overlay.overlay
    }
}
