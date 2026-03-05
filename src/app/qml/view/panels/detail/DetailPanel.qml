import QtQuick

Item {
    id: detailPanel

    property int detailContentsHeight: 324
    property int detailContentsWidth: 165
    property int headerToolbarHeight: 20
    property int headerToolbarWidth: 145
    property int panelSpacing: 10
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanel") : null

    signal viewHookRequested

    function requestViewHook() {
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
                if (detailPanel.detailPanelVm)
                    detailPanel.detailPanelVm.requestStateChange(stateValue);
            }
        }
        DetailContents {
            activeStateName: detailPanel.detailPanelVm ? detailPanel.detailPanelVm.activeStateName : "fileInfo"
            height: detailPanel.detailContentsHeight
            width: detailPanel.detailContentsWidth
        }
    }
}
