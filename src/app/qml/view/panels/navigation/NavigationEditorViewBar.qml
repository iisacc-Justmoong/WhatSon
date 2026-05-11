import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

LV.HStack {
    id: editorViewBar

    readonly property var activeEditorViewModeController: editorViewModeController && editorViewModeController.activeViewModeController !== undefined ? editorViewModeController.activeViewModeController : null
    readonly property string activeViewText: activeEditorViewModeController && activeEditorViewModeController.editorViewName !== undefined ? activeEditorViewModeController.editorViewName : "Plain"
    readonly property var editorViewMenuItems: [
        {
            iconName: "string",
            label: "Plain",
            keyVisible: false,
            selected: editorViewModeController && editorViewModeController.activeViewMode === 0
        },
        {
            iconName: "fileSet",
            label: "Page",
            keyVisible: false,
            selected: editorViewModeController && editorViewModeController.activeViewMode === 1
        },
        {
            iconName: "generalprint",
            label: "Print",
            keyVisible: false,
            selected: editorViewModeController && editorViewModeController.activeViewMode === 2
        },
        {
            iconName: "toolwindowweb",
            label: "Web",
            keyVisible: false,
            selected: editorViewModeController && editorViewModeController.activeViewMode === 3
        },
        {
            iconName: "procedure",
            label: "Presentation",
            keyVisible: false,
            selected: editorViewModeController && editorViewModeController.activeViewMode === 4
        }
    ]
    property int comboContextMenuWidth: LV.Theme.buttonMinWidth + LV.Theme.controlHeightMd + LV.Theme.gap5
    property int comboMenuYOffset: LV.Theme.gap2
    property int compactComboWidth: LV.Theme.buttonMinWidth - LV.Theme.gap3
    property var editorViewModeController: null
    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.NavigationEditorViewBar") : null
    property bool showLabel: true

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }
    function toggleEditorViewMenu() {
        if (editorViewContextMenu.opened) {
            editorViewContextMenu.close();
            return;
        }
        editorViewContextMenu.openFor(editorViewCombo, 0, editorViewCombo.height + editorViewBar.comboMenuYOffset);
        editorViewBar.requestViewHook("open-editor-view-menu");
    }

    spacing: editorViewBar.showLabel ? LV.Theme.gap8 : LV.Theme.gapNone

    LV.Label {
        color: LV.Theme.bodyColor
        style: body
        text: "View"
        visible: editorViewBar.showLabel
    }
    LV.ComboBox {
        id: editorViewCombo

        text: editorViewBar.activeViewText
        tone: LV.ComboBox.Borderless
        width: editorViewBar.showLabel ? implicitWidth : editorViewBar.compactComboWidth

        onClicked: editorViewBar.toggleEditorViewMenu()
    }
    LV.ContextMenu {
        id: editorViewContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: editorViewBar.comboContextMenuWidth
        items: editorViewBar.editorViewMenuItems
        modal: false
        parent: Controls.Overlay.overlay
        selectedIndex: editorViewBar.editorViewModeController && editorViewBar.editorViewModeController.activeViewMode !== undefined ? editorViewBar.editorViewModeController.activeViewMode : 0

        onItemTriggered: function (index) {
            if (editorViewBar.editorViewModeController && editorViewBar.editorViewModeController.requestViewModeChange !== undefined)
                editorViewBar.editorViewModeController.requestViewModeChange(index);
            editorViewBar.requestViewHook("select-editor-view-mode");
        }
    }
}
