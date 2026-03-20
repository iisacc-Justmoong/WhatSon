import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

LV.HStack {
    id: modeBar

    readonly property string activeModeText: activeNavigationModeViewModel && activeNavigationModeViewModel.modeName !== undefined ? activeNavigationModeViewModel.modeName : "Control"
    readonly property var activeNavigationModeViewModel: navigationModeViewModel && navigationModeViewModel.activeModeViewModel !== undefined ? navigationModeViewModel.activeModeViewModel : null
    readonly property var modeMenuItems: [
        {
            label: "View",
            keyVisible: false,
            selected: navigationModeViewModel && navigationModeViewModel.activeMode === 0
        },
        {
            label: "Edit",
            keyVisible: false,
            selected: navigationModeViewModel && navigationModeViewModel.activeMode === 1
        },
        {
            label: "Control",
            keyVisible: false,
            selected: navigationModeViewModel && navigationModeViewModel.activeMode === 2
        }
    ]
    property var navigationModeViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationModeBar") : null
    property bool showLabel: true

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function toggleModeMenu() {
        if (modeContextMenu.opened) {
            modeContextMenu.close();
            return;
        }
        modeContextMenu.openFor(modeCombo, 0, modeCombo.height + 2);
        modeBar.requestViewHook("open-navigation-mode-menu");
    }

    spacing: modeBar.showLabel ? 8 : 0

    LV.Label {
        color: LV.Theme.bodyColor
        style: body
        text: "Mode"
        visible: modeBar.showLabel
    }
    LV.ComboBox {
        id: modeCombo

        text: modeBar.activeModeText
        tone: LV.ComboBox.Tone.Primary

        onClicked: modeBar.toggleModeMenu()
    }
    LV.ContextMenu {
        id: modeContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: 132
        items: modeBar.modeMenuItems
        modal: false
        parent: Controls.Overlay.overlay
        selectedIndex: navigationModeViewModel && navigationModeViewModel.activeMode !== undefined ? navigationModeViewModel.activeMode : 2

        onItemTriggered: function (index) {
            if (modeBar.navigationModeViewModel && modeBar.navigationModeViewModel.requestModeChange !== undefined)
                modeBar.navigationModeViewModel.requestModeChange(index);
            modeBar.requestViewHook("select-navigation-mode");
        }
    }
}
