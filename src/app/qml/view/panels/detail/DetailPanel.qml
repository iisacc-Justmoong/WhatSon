import QtQuick
import LVRS 1.0 as LV

Item {
    id: detailPanel

    readonly property var defaultToolbarItems: [
        {
            "figmaNodeId": "155:4576",
            "iconName": "config",
            "iconSource": LV.Theme.iconPath("configuration"),
            "objectName": "Properties",
            "selected": true,
            "stateValue": 0
        },
        {
            "figmaNodeId": "155:4577",
            "iconName": "chartBar",
            "objectName": "FileStat",
            "selected": false,
            "stateValue": 1
        },
        {
            "figmaNodeId": "155:4578",
            "iconName": "generaladd",
            "objectName": "Insert",
            "selected": false,
            "stateValue": 2
        },
        {
            "figmaNodeId": "155:4579",
            "iconName": "toolwindowdependencies",
            "objectName": "Layer",
            "selected": false,
            "stateValue": 4
        },
        {
            "figmaNodeId": "155:4580",
            "iconName": "toolWindowClock",
            "objectName": "FileHistory",
            "selected": false,
            "stateValue": 3
        },
        {
            "figmaNodeId": "155:4581",
            "iconName": "featureAnswer",
            "objectName": "Help",
            "selected": false,
            "stateValue": 5
        }
    ]
    readonly property int detailContentsHeight: Math.max(0, detailPanel.height - detailPanel.headerToolbarHeight - detailPanel.panelSpacing)
    readonly property int detailContentsWidth: detailPanel.width
    readonly property var registeredViewModelKeys: LV.ViewModels.keys
    readonly property var detailPanelVm: {
        const _ = detailPanel.registeredViewModelKeys;
        return LV.ViewModels.get("detailPanelViewModel");
    }
    property int headerToolbarHeight: 20
    property int headerToolbarWidth: 145
    property int panelSpacing: 10
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanel") : null
    readonly property var resolvedActiveContentViewModel: detailPanel.resolveActiveContentViewModel()
    readonly property var resolvedFileStatViewModel: detailPanel.resolveFileStatViewModel()
    readonly property var resolvedProjectSelectionViewModel: detailPanel.resolveProjectSelectionViewModel()
    readonly property var resolvedBookmarkSelectionViewModel: detailPanel.resolveBookmarkSelectionViewModel()
    readonly property var resolvedProgressSelectionViewModel: detailPanel.resolveProgressSelectionViewModel()
    readonly property string resolvedActiveStateName: detailPanel.resolveActiveStateName()
    readonly property var resolvedToolbarItems: detailPanel.resolveToolbarItems()

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function toolbarMetadataForStateValue(stateValue) {
        switch (Number(stateValue)) {
        case 0:
            return detailPanel.defaultToolbarItems[0];
        case 1:
            return detailPanel.defaultToolbarItems[1];
        case 2:
            return detailPanel.defaultToolbarItems[2];
        case 3:
            return detailPanel.defaultToolbarItems[4];
        case 4:
            return detailPanel.defaultToolbarItems[3];
        case 5:
            return detailPanel.defaultToolbarItems[5];
        default:
            return detailPanel.defaultToolbarItems[0];
        }
    }
    function resolveActiveContentViewModel() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.activeContentViewModel === undefined)
            return null;
        return detailPanel.detailPanelVm.activeContentViewModel;
    }
    function resolveFileStatViewModel() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.fileStatViewModel === undefined)
            return null;
        return detailPanel.detailPanelVm.fileStatViewModel;
    }
    function resolveProjectSelectionViewModel() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.projectSelectionViewModel === undefined)
            return null;
        return detailPanel.detailPanelVm.projectSelectionViewModel;
    }
    function resolveBookmarkSelectionViewModel() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.bookmarkSelectionViewModel === undefined)
            return null;
        return detailPanel.detailPanelVm.bookmarkSelectionViewModel;
    }
    function resolveProgressSelectionViewModel() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.progressSelectionViewModel === undefined)
            return null;
        return detailPanel.detailPanelVm.progressSelectionViewModel;
    }
    function resolveActiveStateName() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.activeStateName === undefined)
            return "properties";
        const stateName = String(detailPanel.detailPanelVm.activeStateName).trim();
        return stateName.length > 0 ? stateName : "properties";
    }
    function resolveToolbarItems() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.toolbarItems === undefined || detailPanel.detailPanelVm.toolbarItems === null)
            return detailPanel.defaultToolbarItems;
        const sourceItems = detailPanel.detailPanelVm.toolbarItems;
        const normalizedItems = Array.isArray(sourceItems)
            ? sourceItems
            : sourceItems.length !== undefined
                ? Array.prototype.slice.call(sourceItems)
                : [];
        if (normalizedItems.length <= 0)
            return detailPanel.defaultToolbarItems;
        const resolvedItems = [];
        for (var index = 0; index < normalizedItems.length; ++index) {
            const sourceItem = normalizedItems[index];
            const stateValue = sourceItem && sourceItem.stateValue !== undefined
                ? Number(sourceItem.stateValue)
                : NaN;
            const metadata = detailPanel.toolbarMetadataForStateValue(stateValue);
            resolvedItems.push({
                                   "figmaNodeId": metadata.figmaNodeId,
                                   "iconName": metadata.iconName,
                                   "iconSource": metadata.iconSource,
                                   "objectName": metadata.objectName,
                                   "selected": sourceItem && sourceItem.selected === true,
                                   "stateValue": isFinite(stateValue) ? stateValue : metadata.stateValue
                               });
        }
        return resolvedItems.length > 0 ? resolvedItems : detailPanel.defaultToolbarItems;
    }

    objectName: "DetailPanel"
    implicitWidth: detailPanel.detailContentsWidth > 0 ? detailPanel.detailContentsWidth : 194

    Column {
        anchors.fill: parent
        spacing: detailPanel.panelSpacing

        Item {
            width: parent.width
            height: detailPanel.headerToolbarHeight

            DetailPanelHeaderToolbar {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                height: detailPanel.headerToolbarHeight
                toolbarButtonSpecs: detailPanel.resolvedToolbarItems
                width: detailPanel.headerToolbarWidth

                onDetailStateChangeRequested: function (stateValue) {
                    detailPanel.requestViewHook("detailStateChangeRequested.stateValue=" + stateValue);
                    if (detailPanel.detailPanelVm)
                        detailPanel.detailPanelVm.requestStateChange(stateValue);
                }
            }
        }
        DetailContents {
            activeContentViewModel: detailPanel.resolvedActiveContentViewModel
            activeStateName: detailPanel.resolvedActiveStateName
            bookmarkSelectionViewModel: detailPanel.resolvedBookmarkSelectionViewModel
            detailPanelViewModel: detailPanel.detailPanelVm
            fileStatViewModel: detailPanel.resolvedFileStatViewModel
            height: detailPanel.detailContentsHeight
            progressSelectionViewModel: detailPanel.resolvedProgressSelectionViewModel
            projectSelectionViewModel: detailPanel.resolvedProjectSelectionViewModel
            width: detailPanel.detailContentsWidth
        }
    }
}
