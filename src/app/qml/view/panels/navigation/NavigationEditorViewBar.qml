import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

LV.HStack {
    id: editorViewBar

    readonly property var activeEditorViewModeViewModel: editorViewModeViewModel && editorViewModeViewModel.activeViewModeViewModel !== undefined ? editorViewModeViewModel.activeViewModeViewModel : null
    readonly property string activeViewText: activeEditorViewModeViewModel && activeEditorViewModeViewModel.editorViewName !== undefined ? activeEditorViewModeViewModel.editorViewName : "Plain"
    readonly property var editorViewMenuItems: [
        {
            iconName: "string",
            label: "Plain",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 0
        },
        {
            iconName: "fileSet",
            label: "Page",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 1
        },
        {
            iconName: "generalprint",
            label: "Print",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 2
        },
        {
            iconName: "toolwindowweb",
            label: "Web",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 3
        },
        {
            iconName: "procedure",
            label: "Presentation",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 4
        }
    ]
    property int comboContextMenuWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(141)))
    property int comboMenuYOffset: LV.Theme.gap2
    property int compactComboWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(97)))
    property var editorViewModeViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationEditorViewBar") : null
    property bool showLabel: true

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
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
        selectedIndex: editorViewBar.editorViewModeViewModel
                       && editorViewBar.editorViewModeViewModel.activeViewMode !== undefined
                       ? editorViewBar.editorViewModeViewModel.activeViewMode
                       : 0

        onItemTriggered: function (index) {
            if (editorViewBar.editorViewModeViewModel && editorViewBar.editorViewModeViewModel.requestViewModeChange !== undefined)
                editorViewBar.editorViewModeViewModel.requestViewModeChange(index);
            editorViewBar.requestViewHook("select-editor-view-mode");
        }
    }
}
