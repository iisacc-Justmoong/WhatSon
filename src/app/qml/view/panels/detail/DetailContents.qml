pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import "." as DetailView

Item {
    id: detailContents

    readonly property string figmaNodeId: "155:4582"
    property var activeContentViewModel: null
    property string activeStateName: "properties"
    property bool noteContextLinked: false
    property var detailPanelViewModel: null
    property var fileStatViewModel: null
    property var projectSelectionViewModel: null
    property var bookmarkSelectionViewModel: null
    property var progressSelectionViewModel: null
    property bool folderCreationEditing: false
    property string folderCreationText: ""
    property string metadataPickerKind: ""
    property var metadataPickerAnchorItem: null
    readonly property int activeFolderIndex: detailContents.resolveMetadataActiveIndex(
                                                 detailContents.activeContentViewModel ? detailContents.activeContentViewModel.activeFolderIndex : -1,
                                                 detailContents.folderItems)
    readonly property int activeTagIndex: detailContents.resolveMetadataActiveIndex(
                                              detailContents.activeContentViewModel ? detailContents.activeContentViewModel.activeTagIndex : -1,
                                              detailContents.tagItems)
    readonly property var folderItems: detailContents.resolveHeaderStringList(detailContents.activeContentViewModel ? detailContents.activeContentViewModel.folderItems : [])
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailContents") : null
    readonly property var registeredViewModelKeys: LV.ViewModels.keys
    readonly property var libraryHierarchySourceViewModel: {
        const _ = detailContents.registeredViewModelKeys;
        return LV.ViewModels.get("libraryHierarchyViewModel");
    }
    readonly property var tagsHierarchySourceViewModel: {
        const _ = detailContents.registeredViewModelKeys;
        return LV.ViewModels.get("tagsHierarchyViewModel");
    }
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
    readonly property string resolvedPanelStateName: detailContents.noteContextLinked ? detailContents.resolvedActiveStateName : "detached"
    readonly property int scaledGap2: Math.max(0, Math.round(LV.Theme.scaleMetric(2)))
    readonly property int scaledGap4: Math.max(0, Math.round(LV.Theme.scaleMetric(4)))
    readonly property int scaledGap6: Math.max(0, Math.round(LV.Theme.scaleMetric(6)))
    readonly property int scaledGap8: Math.max(0, Math.round(LV.Theme.scaleMetric(8)))
    readonly property int scaledGap10: Math.max(0, Math.round(LV.Theme.scaleMetric(10)))
    readonly property int scaledGap16: Math.max(0, Math.round(LV.Theme.scaleMetric(16)))
    readonly property int scaledCompactFrameWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(178)))
    readonly property int scaledCompactRowHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(20)))
    readonly property int scaledCompactRowWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(170)))
    readonly property int scaledCompactListHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(140)))
    readonly property int scaledCompactFooterHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(24)))
    readonly property int scaledCompactFooterWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(78)))
    readonly property int scaledCompactListViewportHeight: Math.max(0, detailContents.scaledCompactListHeight - detailContents.scaledCompactFooterHeight)
    readonly property int scaledInlineInputLeadingInset: Math.max(0, Math.round(LV.Theme.scaleMetric(25)))
    readonly property int scaledPlaceholderCardHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(72)))
    readonly property var tagItems: detailContents.resolveHeaderStringList(detailContents.activeContentViewModel ? detailContents.activeContentViewModel.tagItems : [])
    readonly property var metadataPickerItems: detailContents.resolveMetadataPickerItems(detailContents.metadataPickerKind)
    readonly property bool metadataPickerManualFallbackEnabled: detailContents.metadataPickerKind === "folders"

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
    function sourceHierarchyViewModelForListKind(listKind) {
        if (listKind === "folders")
            return detailContents.libraryHierarchySourceViewModel;
        if (listKind === "tags")
            return detailContents.tagsHierarchySourceViewModel;
        return null;
    }
    function fallbackHierarchyIconNameForListKind(listKind) {
        return listKind === "tags" ? "vcscurrentBranch" : "nodesfolder";
    }
    function resolveMetadataPickerItems(listKind) {
        const normalizedKind = listKind === undefined || listKind === null ? "" : String(listKind).trim();
        const sourceViewModel = detailContents.sourceHierarchyViewModelForListKind(normalizedKind);
        const sourceEntries = detailContents.hierarchyEntries(sourceViewModel);
        const resolvedItems = [];

        for (let index = 0; index < sourceEntries.length; ++index) {
            const entry = sourceEntries[index];
            if (!entry)
                continue;

            const key = entry.key !== undefined && entry.key !== null ? String(entry.key).trim() : "";
            if (normalizedKind === "folders" && key.startsWith("bucket:"))
                continue;

            const label = entry.label !== undefined && entry.label !== null ? String(entry.label).trim() : "";
            const id = entry.id !== undefined && entry.id !== null ? String(entry.id).trim() : "";
            if (label.length === 0 || (normalizedKind === "folders" && id.length === 0))
                continue;

            resolvedItems.push({
                                   accent: entry.accent === true,
                                   count: entry.count !== undefined ? Number(entry.count) || 0 : 0,
                                   depth: entry.depth !== undefined ? Number(entry.depth) || 0 : 0,
                                   expanded: true,
                                   iconName: entry.iconName !== undefined && entry.iconName !== null && String(entry.iconName).trim().length > 0
                                       ? String(entry.iconName).trim()
                                       : detailContents.fallbackHierarchyIconNameForListKind(normalizedKind),
                                   iconSource: entry.iconSource !== undefined && entry.iconSource !== null
                                       ? entry.iconSource
                                       : "",
                                   id: id,
                                   itemId: resolvedItems.length,
                                   key: key.length > 0 ? key : String(normalizedKind) + ":" + String(index),
                                   label: label,
                                   showChevron: false,
                                   sourceIndex: index,
                                   value: normalizedKind === "folders" ? id : (id.length > 0 ? id : label)
                               });
        }

        return resolvedItems;
    }
    function resolveMetadataActiveIndex(value, items) {
        const resolvedItems = detailContents.resolveHeaderStringList(items);
        const itemCount = resolvedItems.length;
        const normalizedValue = Number(value);
        if (!isFinite(normalizedValue) || normalizedValue < 0 || itemCount <= 0)
            return -1;
        if (normalizedValue >= itemCount)
            return itemCount - 1;
        return normalizedValue;
    }
    function beginFolderCreation() {
        detailContents.closeMetadataPicker();
        if (detailContents.folderCreationEditing)
            return;
        detailContents.folderCreationText = "";
        detailContents.folderCreationEditing = true;
        detailContents.requestViewHook("folders.add.begin");
    }
    function cancelFolderCreation() {
        if (!detailContents.folderCreationEditing && detailContents.folderCreationText.length === 0)
            return;
        detailContents.folderCreationEditing = false;
        detailContents.folderCreationText = "";
        detailContents.requestViewHook("folders.add.cancel");
    }
    function commitFolderCreation(rawText) {
        if (!detailContents.folderCreationEditing || !detailPanelViewModel || detailPanelViewModel.assignFolderByName === undefined)
            return;

        const normalizedText = rawText === undefined || rawText === null
            ? detailContents.folderCreationText.trim()
            : String(rawText).trim();
        if (normalizedText.length === 0) {
            detailContents.cancelFolderCreation();
            return;
        }
        if (!detailPanelViewModel.assignFolderByName(normalizedText))
            return;

        detailContents.folderCreationEditing = false;
        detailContents.folderCreationText = "";
        detailContents.requestViewHook("folders.add.commit");
    }
    function resetMetadataPickerState() {
        detailContents.metadataPickerKind = "";
        detailContents.metadataPickerAnchorItem = null;
    }
    function closeMetadataPicker() {
        if (metadataHierarchyPicker.opened)
            metadataHierarchyPicker.close();
        else
            detailContents.resetMetadataPickerState();
    }
    function openMetadataPicker(listKind, anchorItem) {
        const normalizedKind = listKind === undefined || listKind === null ? "" : String(listKind).trim();
        if (normalizedKind.length === 0)
            return;

        if (detailContents.folderCreationEditing)
            detailContents.cancelFolderCreation();
        detailContents.metadataPickerKind = normalizedKind;
        detailContents.metadataPickerAnchorItem = anchorItem ? anchorItem : null;
        metadataHierarchyPicker.openForAnchor(detailContents.metadataPickerAnchorItem, normalizedKind + ".picker.open");
    }
    function applyMetadataPickerEntry(entry) {
        if (!detailPanelViewModel)
            return;

        const resolvedEntry = entry || {};
        const resolvedValue = resolvedEntry.value !== undefined && resolvedEntry.value !== null
            ? String(resolvedEntry.value).trim()
            : "";
        if (resolvedValue.length === 0)
            return;

        if (detailContents.metadataPickerKind === "folders") {
            if (detailPanelViewModel.assignFolderByName === undefined || !detailPanelViewModel.assignFolderByName(resolvedValue))
                return;
            detailContents.requestViewHook("folders.add.commit");
        } else if (detailContents.metadataPickerKind === "tags") {
            if (detailPanelViewModel.assignTagByName === undefined || !detailPanelViewModel.assignTagByName(resolvedValue))
                return;
            detailContents.requestViewHook("tags.add.commit");
        }

        detailContents.closeMetadataPicker();
    }
    function requestMetadataManualFallback(listKind) {
        const normalizedKind = listKind === undefined || listKind === null ? "" : String(listKind).trim();
        if (normalizedKind !== "folders")
            return;
        detailContents.beginFolderCreation();
    }
    function requestMetadataAdd(listKind, anchorItem) {
        detailContents.openMetadataPicker(listKind, anchorItem);
    }
    function setMetadataListActiveIndex(listKind, index) {
        if (!detailContents.activeContentViewModel)
            return;

        const normalizedIndex = Number(index);
        if (!isFinite(normalizedIndex))
            return;

        if (listKind === "folders" && detailContents.activeContentViewModel.setActiveFolderIndex !== undefined) {
            detailContents.activeContentViewModel.setActiveFolderIndex(normalizedIndex);
            detailContents.requestViewHook("folders.select");
        } else if (listKind === "tags" && detailContents.activeContentViewModel.setActiveTagIndex !== undefined) {
            detailContents.activeContentViewModel.setActiveTagIndex(normalizedIndex);
            detailContents.requestViewHook("tags.select");
        }
    }
    function deleteActiveMetadataItem(listKind) {
        if (!detailPanelViewModel)
            return;

        if (listKind === "folders" && detailPanelViewModel.removeActiveFolder !== undefined) {
            if (!detailPanelViewModel.removeActiveFolder())
                return;
            detailContents.requestViewHook("folders.delete");
        } else if (listKind === "tags" && detailPanelViewModel.removeActiveTag !== undefined) {
            if (!detailPanelViewModel.removeActiveTag())
                return;
            detailContents.requestViewHook("tags.delete");
        }
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
        const currentIndex = detailContents.resolveHierarchyMenuSelectedIndex(hierarchyViewModel);
        if (currentIndex === normalizedIndex)
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
    state: detailContents.resolvedPanelStateName

    onResolvedPanelStateNameChanged: {
        if (detailContents.resolvedPanelStateName !== "properties") {
            detailContents.closeMetadataPicker();
            detailContents.cancelFolderCreation();
        }
    }

    component DetailSectionTitle: LV.Label {
        required property string figmaTextNodeId

        property color labelColor: LV.Theme.captionColor

        color: labelColor
        readonly property string figmaNodeId: figmaTextNodeId
        style: caption
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.NoWrap
    }

    component DetailListRow: LV.HierarchyItem {
        required property string rowLabel
        required property string rowObjectName
        property string rowIconName: ""
        property url rowIconSource: ""
        property bool rowSelected: false

        activatable: false
        generatedByTreeModel: false
        hasChildItems: false
        iconName: rowIconName
        iconSource: rowIconSource
        implicitHeight: detailContents.scaledCompactRowHeight
        implicitWidth: detailContents.scaledCompactRowWidth
        itemWidth: width
        label: rowLabel
        objectName: rowObjectName
        rowHeight: detailContents.scaledCompactRowHeight
        rowBackgroundColorInactive: rowSelected ? LV.Theme.accentBlueMuted : "transparent"
        selected: rowSelected
        showChevron: false
        textColorNormal: LV.Theme.bodyColor
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
        implicitHeight: comboSectionColumn.implicitHeight
        implicitWidth: Math.max(detailContents.scaledCompactFrameWidth, comboSectionColumn.implicitWidth)
        objectName: fieldObjectName

        signal menuItemTriggered(int index)

        function toggleContextMenu() {
            if (!comboSection.menuAvailable)
                return;
            if (comboContextMenu.opened) {
                comboContextMenu.close();
                return;
            }
            comboContextMenu.openFor(comboField, 0, comboField.height + detailContents.scaledGap2);
            detailContents.requestViewHook(comboSection.fieldObjectName + ".comboMenu");
        }

        Column {
            id: comboSectionColumn

            anchors.fill: parent
            spacing: detailContents.scaledGap4

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
        property int activeIndex: -1
        property bool addEnabled: true
        property bool deleteEnabled: false
        required property string footerActionPrefix
        required property string frameNodeId
        property bool inlineInputVisible: false
        property string inlineInputObjectName: listSection.listObjectName + "InlineInput"
        property string inlineInputPlaceholderText: ""
        property string inlineInputRowIconName: listSection.rowIconName
        property string inlineInputText: ""
        required property string itemsNodeId
        required property var listItems
        required property string listNodeId
        required property string listObjectName
        property string rowIconName: ""
        property url rowIconSource: ""
        property bool settingsEnabled: true
        readonly property var selectedIndices: metadataSelectionController.selectedIndices
        required property string titleNodeId
        required property string titleText

        readonly property string figmaNodeId: frameNodeId
        implicitHeight: listSectionColumn.implicitHeight
        implicitWidth: Math.max(detailContents.scaledCompactFrameWidth, listSectionColumn.implicitWidth)
        objectName: listObjectName

        signal itemTriggered(int index)
        signal inlineInputAccepted(string text)
        signal inlineInputCanceled()
        signal inlineInputTextEdited(string text)

        function requestSelection(index, modifiers) {
            metadataSelectionController.requestSelection(index, modifiers);
        }

        function selectionContainsIndex(index) {
            return metadataSelectionController.selectionContainsIndex(index);
        }

        Component.onCompleted: metadataSelectionController.reconcileSelection()
        onActiveIndexChanged: metadataSelectionController.reconcileSelection()
        onListItemsChanged: metadataSelectionController.reconcileSelection()
        onInlineInputVisibleChanged: {
            if (!listSection.inlineInputVisible)
                return;
            Qt.callLater(function() {
                inlineInputField.forceInputFocus();
                inlineInputField.selectAll();
            });
        }

        Column {
            id: listSectionColumn

            anchors.fill: parent
            spacing: detailContents.scaledGap4

            DetailSectionTitle {
                figmaTextNodeId: listSection.titleNodeId
                objectName: listSection.listObjectName + "Label"
                text: listSection.titleText
                width: parent.width
            }
            DetailView.DetailMetadataSelectionController {
                id: metadataSelectionController

                section: listSection
            }
            Rectangle {
                id: smallList

                readonly property string figmaNodeId: listSection.listNodeId

                color: LV.Theme.panelBackground03
                height: detailContents.scaledCompactListViewportHeight + listFooter.height
                objectName: listSection.listObjectName + "SmallList"
                width: parent.width

                Item {
                    readonly property string figmaNodeId: listSection.itemsNodeId

                    clip: true
                    height: detailContents.scaledCompactListViewportHeight
                    objectName: listSection.listObjectName + "Items"
                    width: parent.width

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        spacing: 0

                        Item {
                            visible: listSection.inlineInputVisible
                            width: parent.width
                            height: visible ? detailContents.scaledCompactRowHeight : 0

                            DetailListRow {
                                anchors.fill: parent
                                enabled: false
                                rowIconName: listSection.inlineInputRowIconName
                                rowLabel: ""
                                rowObjectName: listSection.inlineInputObjectName + "Row"
                                rowSelected: true
                            }
                            LV.InputField {
                                id: inlineInputField

                                anchors.fill: parent
                                anchors.leftMargin: detailContents.scaledInlineInputLeadingInset
                                anchors.rightMargin: detailContents.scaledGap6
                                backgroundColor: LV.Theme.accentTransparent
                                backgroundColorDisabled: LV.Theme.accentTransparent
                                backgroundColorFocused: LV.Theme.accentTransparent
                                backgroundColorHover: LV.Theme.accentTransparent
                                backgroundColorPressed: LV.Theme.accentTransparent
                                clearButtonVisible: false
                                fieldMinHeight: detailContents.scaledCompactRowHeight
                                insetHorizontal: 0
                                insetVertical: 0
                                objectName: listSection.inlineInputObjectName
                                placeholderText: listSection.inlineInputPlaceholderText
                                selectByMouse: true
                                sideSpacing: 0
                                style: inlineStyle
                                text: listSection.inlineInputText
                                textColor: LV.Theme.bodyColor
                                textColorDisabled: LV.Theme.disabledColor
                                visible: listSection.inlineInputVisible

                                Keys.onEscapePressed: function(event) {
                                    event.accepted = true;
                                    listSection.inlineInputCanceled();
                                }
                                onAccepted: function(text) {
                                    const nextText = typeof text === "string" ? text : inlineInputField.text;
                                    listSection.inlineInputAccepted(nextText);
                                }
                                onActiveFocusChanged: {
                                    if (inlineInputField.activeFocus)
                                        return;
                                    Qt.callLater(function() {
                                        if (listSection.inlineInputVisible
                                                && !inlineInputField.activeFocus
                                                && !inlineInputField.inputItem.activeFocus)
                                            listSection.inlineInputCanceled();
                                    });
                                }
                                onTextEdited: function(text) {
                                    const nextText = typeof text === "string" ? text : inlineInputField.text;
                                    listSection.inlineInputTextEdited(nextText);
                                }
                            }
                        }

                        Repeater {
                            model: listSection.listItems.length

                            delegate: DetailListRow {
                                required property int index

                                rowIconName: listSection.rowIconName
                                rowIconSource: listSection.rowIconSource
                                rowLabel: String(listSection.listItems[index])
                                rowObjectName: listSection.listObjectName + "Item" + index
                                rowSelected: listSection.selectionContainsIndex(index)
                                width: parent ? parent.width : listSection.width

                                TapHandler {
                                    acceptedButtons: Qt.LeftButton
                                    acceptedModifiers: Qt.KeyboardModifierMask
                                    gesturePolicy: TapHandler.DragThreshold
                                    target: null

                                    onPressedChanged: {
                                        if (!pressed)
                                            return;
                                        const pressModifiers = point && point.modifiers !== undefined ? point.modifiers : Qt.application.keyboardModifiers;
                                        metadataSelectionController.capturePointerSelectionModifiers(pressModifiers);
                                    }
                                }
                                onClicked: function() {
                                    const resolvedModifiers = metadataSelectionController.resolveSelectionModifiers(Qt.application.keyboardModifiers);
                                    listSection.requestSelection(index, resolvedModifiers);
                                    metadataSelectionController.clearPointerSelectionModifiers();
                                }
                            }
                        }
                    }
                }
                LV.ListFooter {
                    id: listFooter

                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    button1: ({
                            type: "icon",
                            iconName: "addFile",
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            enabled: listSection.addEnabled,
                            horizontalPadding: detailContents.scaledGap2,
                            onClicked: function () {
                                detailContents.requestMetadataAdd(listSection.footerActionPrefix, listFooter);
                            },
                            verticalPadding: detailContents.scaledGap2
                        })
                    button2: ({
                            type: "icon",
                            iconName: "trash",
                            iconSource: LV.Theme.iconPath("generaldelete"),
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            enabled: listSection.deleteEnabled,
                            horizontalPadding: detailContents.scaledGap2,
                            onClicked: function () {
                                detailContents.deleteActiveMetadataItem(listSection.footerActionPrefix);
                            },
                            verticalPadding: detailContents.scaledGap2
                        })
                    button3: ({
                            type: "menu",
                            iconName: "settings",
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            enabled: listSection.settingsEnabled,
                            bottomPadding: detailContents.scaledGap2,
                            leftPadding: detailContents.scaledGap2,
                            onClicked: function () {
                                detailContents.requestViewHook(listSection.footerActionPrefix + ".settings");
                            },
                            rightPadding: detailContents.scaledGap4,
                            topPadding: detailContents.scaledGap2
                    })
                    height: detailContents.scaledCompactFooterHeight
                    horizontalPadding: detailContents.scaledGap2
                    objectName: listSection.listObjectName + "Footer"
                    spacing: 0
                    verticalPadding: detailContents.scaledGap2
                    width: detailContents.scaledCompactFooterWidth
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
            anchors.leftMargin: detailContents.scaledGap8
            anchors.rightMargin: detailContents.scaledGap8
            anchors.topMargin: detailContents.scaledGap2
            spacing: detailContents.scaledGap10

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
                height: detailContents.scaledPlaceholderCardHeight
                radius: LV.Theme.radiusControl
                width: parent.width

                DetailSectionTitle {
                    anchors.fill: parent
                    anchors.bottomMargin: detailContents.scaledGap8
                    anchors.leftMargin: detailContents.scaledGap8
                    anchors.rightMargin: detailContents.scaledGap8
                    anchors.topMargin: detailContents.scaledGap8
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
        contentHeight: formColumn.y + formColumn.implicitHeight + detailContents.scaledGap2
        contentWidth: width
        interactive: contentHeight > height
        visible: detailContents.resolvedPanelStateName === "properties"

        Column {
            id: formColumn

            readonly property string figmaNodeId: "155:4583"

            x: detailContents.scaledGap8
            y: detailContents.scaledGap2
            objectName: "Form"
            spacing: detailContents.scaledGap10
            width: Math.max(0, propertiesFlickable.width - detailContents.scaledGap16)

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
                activeIndex: detailContents.activeFolderIndex
                addEnabled: !detailContents.folderCreationEditing
                deleteEnabled: !detailContents.folderCreationEditing && detailContents.activeFolderIndex >= 0
                footerActionPrefix: "folders"
                frameNodeId: "155:4587"
                inlineInputObjectName: "FoldersListInlineInput"
                inlineInputPlaceholderText: "Assign or create folder"
                inlineInputText: detailContents.folderCreationText
                inlineInputVisible: detailContents.folderCreationEditing
                inlineInputRowIconName: "nodesnewFolder"
                itemsNodeId: "I155:4589;179:438"
                listItems: detailContents.folderItems
                listNodeId: "155:4589"
                listObjectName: "FoldersList"
                rowIconName: "nodesfolder"
                settingsEnabled: !detailContents.folderCreationEditing
                titleNodeId: "155:4588"
                titleText: "Folders"
                width: parent.width

                onItemTriggered: function(index) {
                    detailContents.setMetadataListActiveIndex("folders", index);
                }
                onInlineInputAccepted: function(text) {
                    detailContents.commitFolderCreation(text);
                }
                onInlineInputCanceled: {
                    detailContents.cancelFolderCreation();
                }
                onInlineInputTextEdited: function(text) {
                    detailContents.folderCreationText = text;
                }
            }
            DetailListSection {
                activeIndex: detailContents.activeTagIndex
                deleteEnabled: detailContents.activeTagIndex >= 0
                footerActionPrefix: "tags"
                frameNodeId: "155:4590"
                itemsNodeId: "I155:4592;179:438"
                listItems: detailContents.tagItems
                listNodeId: "155:4592"
                listObjectName: "TagsList"
                rowIconName: "vcscurrentBranch"
                titleNodeId: "155:4591"
                titleText: "Tags"
                width: parent.width

                onItemTriggered: function(index) {
                    detailContents.setMetadataListActiveIndex("tags", index);
                }
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
    DetailView.DetailMetadataHierarchyPicker {
        id: metadataHierarchyPicker

        emptyStateText: detailContents.metadataPickerKind === "folders"
            ? "No existing folders are available in the library hierarchy."
            : "No existing tags are available in the tags hierarchy."
        hierarchyItems: detailContents.metadataPickerItems
        manualFallbackEnabled: detailContents.metadataPickerManualFallbackEnabled
        manualFallbackText: "Create custom folder path"

        onClosed: detailContents.resetMetadataPickerState()
        onEntryChosen: function(entry) {
            detailContents.applyMetadataPickerEntry(entry);
        }
        onManualFallbackRequested: {
            detailContents.requestMetadataManualFallback(detailContents.metadataPickerKind);
        }
        onViewHookRequested: function(reason) {
            detailContents.requestViewHook(reason);
        }
    }
    DetailFileStatForm {
        anchors.fill: parent
        fileStatViewModel: detailContents.resolvedPanelStateName === "fileStat" ? detailContents.fileStatViewModel : null
        visible: detailContents.resolvedPanelStateName === "fileStat"
    }
    DetailPlaceholderForm {
        visible: detailContents.resolvedPanelStateName !== "detached"
            && detailContents.resolvedPanelStateName !== "properties"
            && detailContents.resolvedPanelStateName !== "fileStat"
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
        },
        State {
            name: "detached"
        }
    ]
}
