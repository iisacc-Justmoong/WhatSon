import QtQuick

Item {
    id: detailPanelHeaderToolbar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanelHeaderToolbar") : null
    property var toolbarButtonSpecs: []

    signal detailStateChangeRequested(int stateValue)
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    Row {
        anchors.fill: parent
        spacing: 5

        Repeater {
            model: detailPanelHeaderToolbar.toolbarButtonSpecs.length

            DetailPanelHeaderToolbarButton {
                buttonSpec: detailPanelHeaderToolbar.toolbarButtonSpecs[index]

                onStateClickRequested: function (stateValue) {
                    detailPanelHeaderToolbar.requestViewHook("toolbarDelegateStateClick.stateValue=" + stateValue);
                    detailPanelHeaderToolbar.detailStateChangeRequested(stateValue);
                }
            }
        }
    }
}
