import QtQuick
import LVRS 1.0 as LV

LV.IconButton {
    id: detailPanelHeaderToolbarButton

    property var buttonSpec: ({})
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanelHeaderToolbarButton") : null
    property bool selected: buttonSpec && buttonSpec.selected === true

    signal stateClickRequested(int stateValue)
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    iconName: buttonSpec && buttonSpec.iconName !== undefined ? buttonSpec.iconName : ""
    iconSize: 16
    tone: selected ? LV.AbstractButton.Default : LV.AbstractButton.Borderless

    onClicked: {
        const nextState = buttonSpec && buttonSpec.stateValue !== undefined ? Number(buttonSpec.stateValue) : NaN;
        if (!isFinite(nextState)) {
            requestViewHook("detailToolbarButtonClick.invalidState");
            return;
        }
        requestViewHook("detailToolbarButtonClick.stateValue=" + nextState);
        stateClickRequested(nextState);
    }
}
