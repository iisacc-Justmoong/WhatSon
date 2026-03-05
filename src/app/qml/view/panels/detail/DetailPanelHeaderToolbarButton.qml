import QtQuick
import LVRS 1.0 as LV

LV.IconButton {
    id: detailPanelHeaderToolbarButton

    property var buttonSpec: ({})
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanelHeaderToolbarButton") : null
    property bool selected: buttonSpec && buttonSpec.selected === true
    readonly property color selectedBackgroundColor: LV.Theme.accentBlueMuted

    signal stateClickRequested(int stateValue)
    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    backgroundColor: selected ? selectedBackgroundColor : "transparent"
    backgroundColorDisabled: selected ? selectedBackgroundColor : "transparent"
    backgroundColorHover: selected ? selectedBackgroundColor : "transparent"
    backgroundColorPressed: selected ? selectedBackgroundColor : "transparent"
    checkable: false
    cornerRadius: 4
    height: 20
    horizontalPadding: 2
    iconName: buttonSpec && buttonSpec.iconName !== undefined ? buttonSpec.iconName : ""
    iconSize: 16
    tone: selected ? LV.AbstractButton.Default : LV.AbstractButton.Borderless
    verticalPadding: 2
    width: 20

    onClicked: {
        const nextState = buttonSpec && buttonSpec.stateValue !== undefined ? Number(buttonSpec.stateValue) : NaN;
        if (!isFinite(nextState))
            return;
        stateClickRequested(nextState);
    }
}
