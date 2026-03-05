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

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        spacing: detailPanel.panelSpacing

        DetailPanelHeaderToolbar {
            height: detailPanel.headerToolbarHeight
            toolbarButtonSpecs: detailPanel.detailPanelVm ? detailPanel.detailPanelVm.toolbarItems : []
            width: detailPanel.headerToolbarWidth

            onDetailStateChangeRequested: function (stateValue) {
                detailPanel.requestViewHook("detailStateChangeRequested.stateValue=" + stateValue);
                if (detailPanel.detailPanelVm)
                    detailPanel.detailPanelVm.requestStateChange(stateValue);
            }
        }
        DetailContents {
            activeContentViewModel: detailPanel.activeDetailContentVm
            activeStateName: detailPanel.detailPanelVm ? detailPanel.detailPanelVm.activeStateName : "fileInfo"
            height: detailPanel.detailContentsHeight
            width: detailPanel.detailContentsWidth
        }
    }
}
