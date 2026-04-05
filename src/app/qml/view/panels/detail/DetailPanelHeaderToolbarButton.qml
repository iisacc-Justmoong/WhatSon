import QtQuick
import LVRS 1.0 as LV

LV.IconButton {
    id: detailPanelHeaderToolbarButton

    property var buttonSpec: ({})
    readonly property string figmaNodeId: buttonSpec && buttonSpec.figmaNodeId !== undefined ? String(buttonSpec.figmaNodeId) : ""
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanelHeaderToolbarButton") : null
    readonly property string resolvedObjectName: buttonSpec && buttonSpec.objectName !== undefined ? String(buttonSpec.objectName) : ""
    property bool selected: buttonSpec && buttonSpec.selected === true

    signal stateClickRequested(int stateValue)
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    objectName: detailPanelHeaderToolbarButton.resolvedObjectName
    iconName: buttonSpec && buttonSpec.iconName !== undefined ? buttonSpec.iconName : ""
    iconSource: buttonSpec && buttonSpec.iconSource !== undefined ? buttonSpec.iconSource : ""
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
