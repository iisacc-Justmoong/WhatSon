import QtQuick
import LVRS 1.0 as LV

Item {
    id: detailPanelHeaderToolbar

    property int activeState: -1
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

            LV.IconButton {
                readonly property var buttonSpec: detailPanelHeaderToolbar.toolbarButtonSpecs[index]
                readonly property bool selected: buttonSpec.stateValue === detailPanelHeaderToolbar.activeState

                backgroundColor: selected ? LV.Theme.panelBackground12 : "transparent"
                backgroundColorDisabled: selected ? LV.Theme.panelBackground12 : "transparent"
                backgroundColorHover: selected ? LV.Theme.panelBackground12 : "transparent"
                backgroundColorPressed: selected ? LV.Theme.panelBackground12 : "transparent"
                checkable: false
                cornerRadius: 4
                height: 20
                horizontalPadding: 2
                iconName: buttonSpec.iconName
                iconSize: 16
                tone: selected ? LV.AbstractButton.Default : LV.AbstractButton.Borderless
                verticalPadding: 2
                width: 20

                onClicked: detailPanelHeaderToolbar.detailStateChangeRequested(buttonSpec.stateValue)
            }
        }
    }
}
