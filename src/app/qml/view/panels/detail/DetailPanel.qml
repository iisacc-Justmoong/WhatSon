import QtQuick

Item {
    id: detailPanel

    readonly property int detailContentsHeight: Math.max(0, detailPanel.height - detailPanel.headerToolbarHeight - detailPanel.panelSpacing)
    readonly property int detailContentsWidth: detailPanel.width
    readonly property var detailPanelVm: detailPanelViewModel
    property int headerToolbarHeight: 20
    property int headerToolbarWidth: 145
    property int panelSpacing: 10
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanel") : null
    readonly property var resolvedActiveContentViewModel: detailPanel.resolveActiveContentViewModel()
    readonly property string resolvedActiveStateName: detailPanel.resolveActiveStateName()
    readonly property var resolvedToolbarItems: detailPanel.resolveToolbarItems()

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function resolveActiveContentViewModel() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.activeContentViewModel === undefined)
            return null;

    }
    function resolveActiveStateName() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.activeStateName === undefined)
            return "fileInfo";
        const stateName = String(detailPanel.detailPanelVm.activeStateName).trim();
        return stateName.length > 0 ? stateName : "fileInfo";
    }
    function resolveToolbarItems() {
        if (!detailPanel.detailPanelVm || detailPanel.detailPanelVm.toolbarItems === undefined || detailPanel.detailPanelVm.toolbarItems === null)
            return [];

    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        spacing: detailPanel.panelSpacing

        DetailPanelHeaderToolbar {
            height: detailPanel.headerToolbarHeight
            toolbarButtonSpecs: detailPanel.resolvedToolbarItems
            width: detailPanel.headerToolbarWidth

            onDetailStateChangeRequested: function (stateValue) {
                detailPanel.requestViewHook("detailStateChangeRequested.stateValue=" + stateValue);
                if (detailPanel.detailPanelVm)
                    detailPanel.detailPanelVm.requestStateChange(stateValue);
            }
        }
        DetailContents {
            activeContentViewModel: detailPanel.resolvedActiveContentViewModel
            activeStateName: detailPanel.resolvedActiveStateName
            height: detailPanel.detailContentsHeight
            width: detailPanel.detailContentsWidth
        }
    }
}
