import QtQuick

Item {
    id: detailPanelHeaderToolbar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanelHeaderToolbar") : null
    property var toolbarButtonSpecs: []

    signal detailStateChangeRequested(int stateValue)
    signal viewHookRequested

    function requestViewHook() {
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
                    detailPanelHeaderToolbar.detailStateChangeRequested(stateValue);
                }
            }
        }
    }
}
