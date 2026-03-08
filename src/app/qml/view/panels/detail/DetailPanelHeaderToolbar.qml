import QtQuick

Item {
    id: detailPanelHeaderToolbar

    readonly property int buttonSpacing: 5
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

    implicitHeight: 20
    implicitWidth: 145

    Row {
        anchors.fill: parent
        spacing: detailPanelHeaderToolbar.buttonSpacing

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
