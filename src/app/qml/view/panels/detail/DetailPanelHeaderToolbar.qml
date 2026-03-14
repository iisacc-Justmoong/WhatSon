import QtQuick

Item {
    id: detailPanelHeaderToolbar

    readonly property int buttonSpacing: 5
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanelHeaderToolbar") : null
    readonly property var resolvedToolbarButtonSpecs: detailPanelHeaderToolbar.normalizeToolbarButtonSpecs(detailPanelHeaderToolbar.toolbarButtonSpecs)
    property var toolbarButtonSpecs: []

    signal detailStateChangeRequested(int stateValue)
    signal viewHookRequested

    function normalizeToolbarButtonSpecs(value) {
        if (value === undefined || value === null)
            return [];
        if (Array.isArray(value))
            return value;
        if (value.length !== undefined)
            return Array.prototype.slice.call(value);
        return [];
    }
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
            model: detailPanelHeaderToolbar.resolvedToolbarButtonSpecs.length

            DetailPanelHeaderToolbarButton {
                buttonSpec: detailPanelHeaderToolbar.resolvedToolbarButtonSpecs[index]

                onStateClickRequested: function (stateValue) {
                    detailPanelHeaderToolbar.requestViewHook("toolbarDelegateStateClick.stateValue=" + stateValue);
                    detailPanelHeaderToolbar.detailStateChangeRequested(stateValue);
                }
            }
        }
    }
}
