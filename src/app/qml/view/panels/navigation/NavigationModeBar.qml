import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

LV.HStack {
    id: modeBar

    readonly property string activeModeText: activeNavigationModeController && activeNavigationModeController.modeName !== undefined ? activeNavigationModeController.modeName : "View"
    readonly property var activeNavigationModeController: navigationModeController && navigationModeController.activeModeController !== undefined ? navigationModeController.activeModeController : null
    property int comboContextMenuWidth: LV.Theme.buttonMinWidth + LV.Theme.gap24 + LV.Theme.gap8
    property int comboMenuYOffset: LV.Theme.gap2
    property int compactComboWidth: LV.Theme.buttonMinWidth - LV.Theme.gap3
    readonly property var modeMenuItems: [
        {
            iconName: "generalshow",
            label: "View",
            keyVisible: false,
            selected: navigationModeController && navigationModeController.activeMode === 0
        },
        {
            iconName: "renameColumn",
            label: "Edit",
            keyVisible: false,
            selected: navigationModeController && navigationModeController.activeMode === 1
        },
        {
            iconName: "abstractClass",
            label: "Control",
            keyVisible: false,
            selected: navigationModeController && navigationModeController.activeMode === 2
        }
    ]
    property var navigationModeController: null
    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.NavigationModeBar") : null
    property bool showLabel: true

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }
    function toggleModeMenu() {
        if (modeContextMenu.opened) {
            modeContextMenu.close();
            return;
        }
        modeContextMenu.openFor(modeCombo, 0, modeCombo.height + modeBar.comboMenuYOffset);
        modeBar.requestViewHook("open-navigation-mode-menu");
    }

    spacing: modeBar.showLabel ? LV.Theme.gap8 : LV.Theme.gapNone

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
        width: modeBar.showLabel ? implicitWidth : modeBar.compactComboWidth

        onClicked: modeBar.toggleModeMenu()
    }
    LV.ContextMenu {
        id: modeContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: modeBar.comboContextMenuWidth
        items: modeBar.modeMenuItems
        modal: false
        parent: Controls.Overlay.overlay
        selectedIndex: modeBar.navigationModeController && modeBar.navigationModeController.activeMode !== undefined ? modeBar.navigationModeController.activeMode : 0

        onItemTriggered: function (index) {
            if (modeBar.navigationModeController && modeBar.navigationModeController.requestModeChange !== undefined)
                modeBar.navigationModeController.requestModeChange(index);
            modeBar.requestViewHook("select-navigation-mode");
        }
    }
}
