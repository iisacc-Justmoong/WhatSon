pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

Item {
    id: detailContents

    readonly property string figmaNodeId: "155:4582"
    property var activeContentViewModel: null
    property string activeStateName: "properties"
    property var projectSelectionViewModel: null
    property var bookmarkSelectionViewModel: null
    property var progressSelectionViewModel: null
    readonly property var folderItems: detailContents.resolveHeaderStringList(detailContents.activeContentViewModel ? detailContents.activeContentViewModel.folderItems : [])
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailContents") : null
    readonly property var projectMenuItems: detailContents.resolveHierarchyMenuItems(detailContents.projectSelectionViewModel)
    readonly property int projectMenuSelectedIndex: detailContents.resolveHierarchyMenuSelectedIndex(detailContents.projectSelectionViewModel)
    readonly property string resolvedProjectSelectionText: detailContents.resolveHierarchySelectionText(detailContents.projectSelectionViewModel, "No project")
    readonly property var bookmarkMenuItems: detailContents.resolveHierarchyMenuItems(detailContents.bookmarkSelectionViewModel)
    readonly property int bookmarkMenuSelectedIndex: detailContents.resolveHierarchyMenuSelectedIndex(detailContents.bookmarkSelectionViewModel)
    readonly property string resolvedBookmarkSelectionText: detailContents.resolveHierarchySelectionText(detailContents.bookmarkSelectionViewModel, "No bookmark")
    readonly property var progressMenuItems: detailContents.resolveHierarchyMenuItems(detailContents.progressSelectionViewModel)
    readonly property int progressMenuSelectedIndex: detailContents.resolveHierarchyMenuSelectedIndex(detailContents.progressSelectionViewModel)
    readonly property string resolvedProgressSelectionText: detailContents.resolveHierarchySelectionText(detailContents.progressSelectionViewModel, "No progress")
    readonly property string resolvedActiveStateName: detailContents.normalizeStateName(detailContents.activeStateName)
    readonly property var tagItems: detailContents.resolveHeaderStringList(detailContents.activeContentViewModel ? detailContents.activeContentViewModel.tagItems : [])

    signal viewHookRequested

    function normalizeStateName(value) {
        const normalized = value === undefined || value === null ? "" : String(value).trim();
        switch (normalized) {
        case "properties":
            return "properties";
        case "fileStat":
            return "fileStat";
        case "insert":
            return "insert";
        case "fileHistory":
            return "fileHistory";
        case "layer":
            return "layer";
        case "help":
            return normalized;
        default:
            return "properties";
        }
    }
    function placeholderHeadingForState(stateName) {
        switch (stateName) {
        case "fileStat":
            return "Statistics";
        case "insert":
            return "Insert";
        case "fileHistory":
            return "History";
        case "layer":
            return "Layer";
        case "help":
            return "Help";
        default:
            return "Properties";
        }
    }
    function placeholderSummaryForState(stateName) {
        switch (stateName) {
        case "fileStat":
            return "Detail statistics content will be mounted in this dedicated panel form.";
        case "insert":
            return "Insert controls will be mounted in this dedicated panel form.";
        case "fileHistory":
            return "History controls will be mounted in this dedicated panel form.";
        case "layer":
            return "Layer controls will be mounted in this dedicated panel form.";
        case "help":
            return "Help content will be mounted in this dedicated panel form.";
        default:
            return "Properties content remains the default detail panel form.";
        }
    }
    function hierarchyEntries(hierarchyViewModel) {
        if (!hierarchyViewModel || hierarchyViewModel.hierarchyModel === undefined || hierarchyViewModel.hierarchyModel === null)
            return [];
        const sourceEntries = hierarchyViewModel.hierarchyModel;
        if (Array.isArray(sourceEntries))
            return sourceEntries;
        if (sourceEntries.length !== undefined)
            return Array.prototype.slice.call(sourceEntries);
        return [];
    }
    function resolveHierarchyMenuItems(hierarchyViewModel) {
        const sourceEntries = detailContents.hierarchyEntries(hierarchyViewModel);
        const selectedIndex = detailContents.resolveHierarchyMenuSelectedIndex(hierarchyViewModel);
        const resolvedItems = [];
        for (let index = 0; index < sourceEntries.length; ++index) {
            const entry = sourceEntries[index];
            const label = entry && entry.label !== undefined && entry.label !== null ? String(entry.label).trim() : "";
            const iconName = entry && entry.iconName !== undefined && entry.iconName !== null ? String(entry.iconName).trim() : "";
            const iconSource = entry && entry.iconSource !== undefined && entry.iconSource !== null ? entry.iconSource : "";
            if (label.length === 0)
                continue;
            resolvedItems.push({
                                   iconName: iconName,
                                   iconSource: iconSource,
                                   label: label,
                                   keyVisible: false,
                                   selected: index === selectedIndex
                               });
        }
        return resolvedItems;
    }
    function resolveHierarchyMenuSelectedIndex(hierarchyViewModel) {
        if (!hierarchyViewModel || hierarchyViewModel.selectedIndex === undefined)
            return -1;
        const selectedIndex = Number(hierarchyViewModel.selectedIndex);
        return isFinite(selectedIndex) ? selectedIndex : -1;
    }
    function resolveHierarchySelectionText(hierarchyViewModel, emptyText) {
        const fallbackText = emptyText !== undefined && emptyText !== null ? String(emptyText) : "";
        if (!hierarchyViewModel || hierarchyViewModel.itemLabel === undefined)
            return fallbackText;
        const selectedIndex = detailContents.resolveHierarchyMenuSelectedIndex(hierarchyViewModel);
        if (selectedIndex < 0)
            return fallbackText;
        const label = String(hierarchyViewModel.itemLabel(selectedIndex) || "").trim();
        return label.length > 0 ? label : fallbackText;
    }
    function resolveHeaderStringList(values) {
        if (!values)
            return [];
        if (Array.isArray(values))
            return values;
        if (values.length !== undefined)
            return Array.prototype.slice.call(values);
        return [];
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function applyHierarchySelection(hierarchyViewModel, index, hookReason) {
        if (!hierarchyViewModel)
            return;
        const normalizedIndex = Number(index);
        if (!isFinite(normalizedIndex) || normalizedIndex < 0)
            return;

        if (hookReason === "projects.comboSelect" && detailPanelViewModel && detailPanelViewModel.writeProjectSelection !== undefined) {
            if (!detailPanelViewModel.writeProjectSelection(normalizedIndex))
                return;
        } else if (hookReason === "bookmark.comboSelect" && detailPanelViewModel && detailPanelViewModel.writeBookmarkSelection !== undefined) {
            if (!detailPanelViewModel.writeBookmarkSelection(normalizedIndex))
                return;
        } else if (hookReason === "progress.comboSelect" && detailPanelViewModel && detailPanelViewModel.writeProgressSelection !== undefined) {
            if (!detailPanelViewModel.writeProgressSelection(normalizedIndex))
                return;
        }

        if (hierarchyViewModel.setSelectedIndex !== undefined)
            hierarchyViewModel.setSelectedIndex(normalizedIndex);
        else if (hierarchyViewModel.selectedIndex !== undefined)
            hierarchyViewModel.selectedIndex = normalizedIndex;
        detailContents.requestViewHook(hookReason);
    }

    objectName: "DetailContents"
    state: detailContents.resolvedActiveStateName

    component DetailSectionTitle: LV.Label {
        required property string figmaTextNodeId

        property color labelColor: LV.Theme.captionColor

        color: labelColor
        readonly property string figmaNodeId: figmaTextNodeId
        style: caption
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.NoWrap
    }

    component DetailListRow: Item {
        required property string rowLabel
        required property string rowObjectName

        objectName: rowObjectName
        implicitHeight: 20
        implicitWidth: 170

        Row {
            anchors.fill: parent
            anchors.bottomMargin: 2
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            anchors.topMargin: 2
            spacing: 1

            Image {
                fillMode: Image.PreserveAspectFit
                height: 16
                smooth: true
                source: LV.Theme.iconPath("nodesfolder")
                sourceSize.height: 16
                sourceSize.width: 16
                width: 16
            }
            DetailSectionTitle {
                figmaTextNodeId: ""
                labelColor: LV.Theme.bodyColor
                style: body
                text: rowLabel
            }
        }
    }

    component DetailComboSection: Item {
        id: comboSection
        required property string comboNodeId
        property string comboText: valueText
        required property string fieldObjectName
        required property string frameNodeId
        required property string labelNodeId
        required property string labelText
        property var menuItems: []
        property int menuSelectedIndex: -1
        required property string valueText
        readonly property bool menuAvailable: {
            if (!menuItems)
                return false;
            if (menuItems.length !== undefined)
                return Number(menuItems.length) > 0;
            if (menuItems.count !== undefined)
                return Number(menuItems.count) > 0;
            return false;
        }

        readonly property string figmaNodeId: frameNodeId
        implicitHeight: 33
        implicitWidth: 178
        objectName: fieldObjectName

        signal menuItemTriggered(int index)

        function toggleContextMenu() {
            if (!comboSection.menuAvailable)
                return;
            if (comboContextMenu.opened) {
                comboContextMenu.close();
                return;
            }
            comboContextMenu.openFor(comboField, 0, comboField.height + 2);
            detailContents.requestViewHook(comboSection.fieldObjectName + ".comboMenu");
        }

        Column {
            anchors.fill: parent
            spacing: 4

            DetailSectionTitle {
                figmaTextNodeId: comboSection.labelNodeId
                labelColor: LV.Theme.captionColor
                objectName: comboSection.fieldObjectName + "Label"
                text: comboSection.labelText
                width: parent.width
            }
            LV.ComboBox {
                id: comboField

                readonly property string figmaNodeId: comboSection.comboNodeId

                objectName: comboSection.fieldObjectName + "ComboBox"
                text: comboSection.comboText
                width: parent.width

                onClicked: {
                    if (comboSection.menuAvailable)
                        comboSection.toggleContextMenu();
                    else
                        detailContents.requestViewHook(comboSection.fieldObjectName + ".combo");
                }
            }
            LV.ContextMenu {
                id: comboContextMenu

                autoCloseOnTrigger: true
                closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
                items: comboSection.menuItems
                modal: false
                parent: Controls.Overlay.overlay
                selectedIndex: comboSection.menuSelectedIndex

                onItemTriggered: function (index) {
                    comboSection.menuItemTriggered(index);
                }
            }
        }
    }

    component DetailListSection: Item {
        id: listSection
        required property string footerActionPrefix
        required property string frameNodeId
        required property string itemsNodeId
        required property var listItems
        required property string listNodeId
        required property string listObjectName
        required property string titleNodeId
        required property string titleText

        readonly property string figmaNodeId: frameNodeId
        implicitHeight: 155
        implicitWidth: 178
        objectName: listObjectName

        Column {
            anchors.fill: parent
            spacing: 4

            DetailSectionTitle {
                figmaTextNodeId: listSection.titleNodeId
                objectName: listSection.listObjectName + "Label"
                text: listSection.titleText
                width: parent.width
            }
            Rectangle {
                id: smallList

                readonly property string figmaNodeId: listSection.listNodeId

                color: LV.Theme.panelBackground03
                height: 140
                objectName: listSection.listObjectName + "SmallList"
                width: parent.width

                Item {
                    readonly property string figmaNodeId: listSection.itemsNodeId

                    clip: true
                    height: 116
                    objectName: listSection.listObjectName + "Items"
                    width: parent.width

                    Column {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        spacing: 0

                        Repeater {
                            model: listSection.listItems.length

                            delegate: DetailListRow {
                                required property int index

                                rowLabel: String(listSection.listItems[index])
                                rowObjectName: listSection.listObjectName + "Item" + index
                            }
                        }
                    }
                }
                LV.ListFooter {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    button1: ({
                            type: "icon",
                            iconName: "addFile",
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            horizontalPadding: 2,
                            onClicked: function () {
                                detailContents.requestViewHook(listSection.footerActionPrefix + ".add");
                            },
                            verticalPadding: 2
                        })
                    button2: ({
                            type: "icon",
                            iconName: "trash",
                            iconSource: LV.Theme.iconPath("generaldelete"),
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            horizontalPadding: 2,
                            onClicked: function () {
                                detailContents.requestViewHook(listSection.footerActionPrefix + ".delete");
                            },
                            verticalPadding: 2
                        })
                    button3: ({
                            type: "menu",
                            iconName: "settings",
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            bottomPadding: 2,
                            leftPadding: 2,
                            onClicked: function () {
                                detailContents.requestViewHook(listSection.footerActionPrefix + ".settings");
                            },
                            rightPadding: 4,
                            topPadding: 2
                    })
                    height: 24
                    horizontalPadding: 2
                    objectName: listSection.listObjectName + "Footer"
                    spacing: 0
                    verticalPadding: 2
                    width: 78
                }
            }
        }
    }

    component DetailPlaceholderForm: Item {
        anchors.fill: parent
        objectName: detailContents.resolvedActiveStateName + "Form"

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            anchors.topMargin: 2
            spacing: 10

            DetailSectionTitle {
                figmaTextNodeId: ""
                labelColor: LV.Theme.accentWhite
                objectName: detailContents.resolvedActiveStateName + "Title"
                style: body
                text: detailContents.placeholderHeadingForState(detailContents.resolvedActiveStateName)
                width: parent.width
            }
            Rectangle {
                color: LV.Theme.panelBackground03
                height: 72
                radius: LV.Theme.radiusControl
                width: parent.width

                DetailSectionTitle {
                    anchors.fill: parent
                    anchors.bottomMargin: 8
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    anchors.topMargin: 8
                    figmaTextNodeId: ""
                    labelColor: LV.Theme.bodyColor
                    objectName: detailContents.resolvedActiveStateName + "Description"
                    style: body
                    text: detailContents.placeholderSummaryForState(detailContents.resolvedActiveStateName)
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    Flickable {
        id: propertiesFlickable

        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds
        clip: true
        contentHeight: formColumn.y + formColumn.implicitHeight + 2
        contentWidth: width
        interactive: contentHeight > height
        visible: detailContents.resolvedActiveStateName === "properties"

        Column {
            id: formColumn

            readonly property string figmaNodeId: "155:4583"

            x: 8
            y: 2
            objectName: "Form"
            spacing: 10
            width: Math.max(0, propertiesFlickable.width - 16)

            DetailComboSection {
                comboNodeId: "178:5496"
                comboText: detailContents.resolvedProjectSelectionText
                fieldObjectName: "Projects"
                frameNodeId: "178:5494"
                labelNodeId: "178:5495"
                labelText: "Projects"
                menuItems: detailContents.projectMenuItems
                menuSelectedIndex: detailContents.projectMenuSelectedIndex
                valueText: "No project"
                width: parent.width

                onMenuItemTriggered: function(index) {
                    detailContents.applyHierarchySelection(detailContents.projectSelectionViewModel, index, "projects.comboSelect");
                }
            }
            DetailComboSection {
                comboNodeId: "155:4586"
                comboText: detailContents.resolvedBookmarkSelectionText
                fieldObjectName: "Bookmark"
                frameNodeId: "155:4584"
                labelNodeId: "155:4585"
                labelText: "Bookmark"
                menuItems: detailContents.bookmarkMenuItems
                menuSelectedIndex: detailContents.bookmarkMenuSelectedIndex
                valueText: "No bookmark"
                width: parent.width

                onMenuItemTriggered: function(index) {
                    detailContents.applyHierarchySelection(detailContents.bookmarkSelectionViewModel, index, "bookmark.comboSelect");
                }
            }
            DetailListSection {
                footerActionPrefix: "folders"
                frameNodeId: "155:4587"
                itemsNodeId: "I155:4589;179:438"
                listItems: detailContents.folderItems
                listNodeId: "155:4589"
                listObjectName: "FoldersList"
                titleNodeId: "155:4588"
                titleText: "Folders"
                width: parent.width
            }
            DetailListSection {
                footerActionPrefix: "tags"
                frameNodeId: "155:4590"
                itemsNodeId: "I155:4592;179:438"
                listItems: detailContents.tagItems
                listNodeId: "155:4592"
                listObjectName: "TagsList"
                titleNodeId: "155:4591"
                titleText: "Tags"
                width: parent.width
            }
            DetailComboSection {
                comboNodeId: "178:5503"
                comboText: detailContents.resolvedProgressSelectionText
                fieldObjectName: "Progress"
                frameNodeId: "178:5501"
                labelNodeId: "178:5502"
                labelText: "Progress"
                menuItems: detailContents.progressMenuItems
                menuSelectedIndex: detailContents.progressMenuSelectedIndex
                valueText: "No progress"
                width: parent.width

                onMenuItemTriggered: function(index) {
                    detailContents.applyHierarchySelection(detailContents.progressSelectionViewModel, index, "progress.comboSelect");
                }
            }
        }
    }
    DetailPlaceholderForm {
        visible: detailContents.resolvedActiveStateName !== "properties"
    }

    states: [
        State {
            name: "properties"
        },
        State {
            name: "fileStat"
        },
        State {
            name: "insert"
        },
        State {
            name: "fileHistory"
        },
        State {
            name: "layer"
        },
        State {
            name: "help"
        }
    ]
}
