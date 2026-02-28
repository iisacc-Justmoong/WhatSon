import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

Item {
    id: libraryView

    property int activeToolbarIndex: 0
    readonly property int contentWidth: Math.max(0, width - horizontalInset * 2)
    property var depthItems: []
    property int editingIndex: -1
    property string editingText: ""
    readonly property var folderModel: hierarchyViewModel ? hierarchyViewModel.itemModel : null
    readonly property int footerHeight: 24
    property var hierarchyViewModel: (typeof libraryHierarchyViewModel !== "undefined" && libraryHierarchyViewModel) ? libraryHierarchyViewModel : null
    readonly property int horizontalInset: (typeof LV.Theme.gap8 === "number" && isFinite(LV.Theme.gap8)) ? LV.Theme.gap8 : 8
    property color panelColor: LV.Theme.panelBackground04
    readonly property int rowHeight: 28
    readonly property int searchHeight: (typeof LV.Theme.gap18 === "number" && isFinite(LV.Theme.gap18)) ? LV.Theme.gap18 : 18
    readonly property int searchRadius: 5
    readonly property int selectedFolderIndex: hierarchyViewModel && hierarchyViewModel.selectedIndex !== undefined ? hierarchyViewModel.selectedIndex : -1
    property var toolbarIconNames: ["nodeslibraryFolder", "table", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property var toolbarItems: {
        if (libraryView.hierarchyViewModel && libraryView.hierarchyViewModel.toolbarItems !== undefined)
            return libraryView.hierarchyViewModel.toolbarItems;
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
        if (libraryView.editingIndex >= 0 && libraryView.editingIndex !== index)
            libraryView.commitRename();
        if (libraryView.hierarchyViewModel)
            libraryView.hierarchyViewModel.setSelectedIndex(index);
        libraryView.editingIndex = index;
        libraryView.editingText = currentLabel;
    }
    function cancelRename() {
        libraryView.editingIndex = -1;
        libraryView.editingText = "";
    }
    function commitRename() {
        if (libraryView.editingIndex < 0)
            return;
        if (libraryView.hierarchyViewModel)
            libraryView.hierarchyViewModel.renameItem(libraryView.editingIndex, libraryView.editingText);
        libraryView.editingIndex = -1;
        libraryView.editingText = "";
    }
    function toggleViewOptionsMenu() {
        if (viewOptionsContextMenu.opened) {
            viewOptionsContextMenu.close();
            return;
        }
        viewOptionsContextMenu.openFor(libraryFooter, 0, libraryFooter.height + verticalInset);
    }

    clip: true

    Component.onCompleted: {
        if (libraryView.hierarchyViewModel)
            libraryView.hierarchyViewModel.setDepthItems(libraryView.depthItems);
    }
    onDepthItemsChanged: {
        if (libraryView.hierarchyViewModel)
            libraryView.hierarchyViewModel.setDepthItems(libraryView.depthItems);
        libraryView.cancelRename();
    }
    onHierarchyViewModelChanged: libraryView.cancelRename()

    Rectangle {
        anchors.fill: parent
        color: libraryView.panelColor
    }
    Item {
        id: libraryContents

        anchors.bottom: libraryFooter.top
        anchors.left: parent.left
        anchors.leftMargin: libraryView.horizontalInset
        anchors.right: parent.right
        anchors.rightMargin: libraryView.horizontalInset
        anchors.top: parent.top
        anchors.topMargin: libraryView.verticalInset

        LV.VStack {
            anchors.fill: parent
            spacing: LV.Theme.gap2

            LV.HierarchyToolbar {
                id: hierarchyHeaderToolbar

                Layout.alignment: Qt.AlignLeft
                Layout.maximumWidth: Math.max(libraryView.toolbarMinWidth, libraryView.contentWidth)
                Layout.minimumWidth: libraryView.toolbarMinWidth
                Layout.preferredHeight: 20
                Layout.preferredWidth: Math.max(libraryView.toolbarMinWidth, libraryView.contentWidth)
                activeButtonId: libraryView.activeToolbarIndex
                backgroundColor: "transparent"
                backgroundOpacity: 1.0
                buttonItems: libraryView.toolbarItems
                horizontalPadding: 0
                spacing: LV.Theme.gap2
                verticalPadding: 0

                onActiveChanged: function (button, buttonId, index) {
                    if (index < 0 || index === libraryView.activeToolbarIndex)
                        return;
                    libraryView.activeToolbarIndex = index;
                    libraryView.toolbarIndexChangeRequested(index);
                }
            }
            Rectangle {
                id: searchBar

                Layout.fillWidth: true
                Layout.preferredHeight: libraryView.searchHeight
                color: LV.Theme.panelBackground10
                radius: libraryView.searchRadius

                TextInput {
                    id: searchBarTextInput

                    anchors.bottomMargin: LV.Theme.gap3
                    anchors.fill: parent
                    anchors.leftMargin: LV.Theme.gap7
                    anchors.rightMargin: clearSearchButton.width + LV.Theme.gap7 + LV.Theme.gap2
                    anchors.topMargin: LV.Theme.gap3
                    clip: true
                    color: LV.Theme.titleHeaderColor
                    font.pixelSize: LV.Theme.textBody
                    font.weight: LV.Theme.textBodyWeight
                    renderType: Text.NativeRendering
                    selectByMouse: true
                    selectionColor: LV.Theme.accentBlue
                    verticalAlignment: TextInput.AlignVCenter
                }
                Text {
                    anchors.left: searchBarTextInput.left
                    anchors.verticalCenter: parent.verticalCenter
                    color: LV.Theme.descriptionColor
                    font.pixelSize: LV.Theme.textBody
                    font.weight: LV.Theme.textBodyWeight
                    text: "Placeholder"
                    visible: searchBarTextInput.text.length === 0 && !searchBarTextInput.activeFocus
                }
                Rectangle {
                    id: clearSearchButton

                    anchors.right: parent.right
                    anchors.rightMargin: LV.Theme.gap5
                    anchors.verticalCenter: parent.verticalCenter
                    color: LV.Theme.panelBackground12
                    height: 12
                    radius: 6
                    width: 12

                    Text {
                        anchors.centerIn: parent
                        color: LV.Theme.descriptionColor
                        font.pixelSize: 9
                        text: "x"
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor

                        onClicked: searchBarTextInput.text = ""
                    }
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
                        model: libraryView.folderModel

                        delegate: Item {
                            required property bool accent
                            required property bool expanded
                            required property int indentLevel
                            required property int index
                            required property string label
                            readonly property int renameLeftInset: LV.Theme.gap8 + indentLevel * 13 + LV.Theme.iconSm + LV.Theme.gap2
                            readonly property int renameRightInset: showChevron ? (LV.Theme.iconSm + LV.Theme.gap8) : LV.Theme.gap8
                            required property bool showChevron

                            height: libraryView.rowHeight
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
                                label: index === libraryView.editingIndex ? "" : label
                                leadingSpacing: LV.Theme.gap2
                                rowBackgroundColor: index === libraryView.selectedFolderIndex ? LV.Theme.panelBackground12 : "transparent"
                                rowBackgroundColorHover: index === libraryView.selectedFolderIndex ? LV.Theme.panelBackground12 : "transparent"
                                rowBackgroundColorPressed: index === libraryView.selectedFolderIndex ? LV.Theme.panelBackground12 : "transparent"
                                rowHeight: libraryView.rowHeight
                                rowRightPadding: LV.Theme.gap8
                                showChevron: showChevron
                                textColorNormal: accent ? LV.Theme.accentBlue : LV.Theme.bodyColor
                            }
                            MouseArea {
                                anchors.fill: parent

                                onClicked: {
                                    if (libraryView.renameEnabled) {
                                        libraryView.beginRename(index, label);
                                        return;
                                    }
                                    if (libraryView.hierarchyViewModel)
                                        libraryView.hierarchyViewModel.setSelectedIndex(index);
                                }
                            }
                            Item {
                                id: renameOverlay

                                anchors.fill: parent
                                visible: index === libraryView.editingIndex
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
                                    text: libraryView.editingText
                                    verticalAlignment: TextInput.AlignVCenter

                                    Keys.onEscapePressed: function (event) {
                                        libraryView.cancelRename();
                                        event.accepted = true;
                                    }
                                    onAccepted: libraryView.commitRename()
                                    onActiveFocusChanged: {
                                        if (!activeFocus && index === libraryView.editingIndex)
                                            libraryView.commitRename();
                                    }
                                    onTextChanged: {
                                        if (index === libraryView.editingIndex && libraryView.editingText !== text)
                                            libraryView.editingText = text;
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
        id: libraryFooter

        anchors.bottom: parent.bottom
        anchors.bottomMargin: libraryView.verticalInset
        anchors.left: parent.left
        anchors.leftMargin: libraryView.horizontalInset
        button1: ({
                type: "icon",
                enabled: libraryView.createFolderEnabled,
                iconName: "addFile",
                iconSize: LV.Theme.iconSm,
                tone: LV.AbstractButton.Borderless,
                onClicked: function () {
                    if (!libraryView.createFolderEnabled)
                        return;
                    libraryView.commitRename();
                    if (libraryView.hierarchyViewModel)
                        libraryView.hierarchyViewModel.createFolder();
                    if (libraryView.renameEnabled && libraryView.hierarchyViewModel && libraryView.hierarchyViewModel.selectedIndex >= 0) {
                        var createdIndex = libraryView.hierarchyViewModel.selectedIndex;
                        libraryView.beginRename(createdIndex, libraryView.hierarchyViewModel.itemLabel(createdIndex));
                    }
                }
            })
        button2: ({
                type: "icon",
                enabled: libraryView.deleteFolderEnabled,
                iconName: "generaldelete",
                iconSize: LV.Theme.iconSm,
                tone: LV.AbstractButton.Borderless,
                onClicked: function () {
                    if (!libraryView.deleteFolderEnabled)
                        return;
                    libraryView.cancelRename();
                    if (libraryView.hierarchyViewModel)
                        libraryView.hierarchyViewModel.deleteSelectedFolder();
                }
            })
        button3: ({
                type: "menu",
                iconName: "settings",
                iconSize: LV.Theme.iconSm,
                tone: LV.AbstractButton.Default,
                onClicked: function () {
                    libraryView.toggleViewOptionsMenu();
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
        items: libraryView.viewOptionsMenuItems
        modal: false
        parent: Controls.Overlay.overlay
    }
}
