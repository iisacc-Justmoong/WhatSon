import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

LV.HStack {
    id: editorViewBar

    readonly property var activeEditorViewModeViewModel: editorViewModeViewModel && editorViewModeViewModel.activeViewModeViewModel !== undefined ? editorViewModeViewModel.activeViewModeViewModel : null
    readonly property string activeViewText: activeEditorViewModeViewModel && activeEditorViewModeViewModel.editorViewName !== undefined ? activeEditorViewModeViewModel.editorViewName : "Plain"
    readonly property var editorViewMenuItems: [
        {
            label: "Plain",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 0
        },
        {
            label: "Page",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 1
        },
        {
            label: "Print",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 2
        },
        {
            label: "Web",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 3
        },
        {
            label: "Presentation",
            keyVisible: false,
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 4
        }
    ]
    property var editorViewModeViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationEditorViewBar") : null

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
        editorViewContextMenu.openFor(editorViewCombo, 0, editorViewCombo.height + 2);
        editorViewBar.requestViewHook("open-editor-view-menu");
    }

    spacing: 8

    LV.Label {
        color: LV.Theme.bodyColor
        style: body
        text: "View"
    }
    LV.ComboBox {
        id: editorViewCombo

        text: editorViewBar.activeViewText
        tone: LV.ComboBox.Borderless

        onClicked: editorViewBar.toggleEditorViewMenu()
    }
    LV.ContextMenu {
        id: editorViewContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: 108
        items: editorViewBar.editorViewMenuItems
        modal: false
        parent: Controls.Overlay.overlay
        selectedIndex: editorViewModeViewModel && editorViewModeViewModel.activeViewMode !== undefined ? editorViewModeViewModel.activeViewMode : 0

        onItemTriggered: function (index) {
            if (editorViewBar.editorViewModeViewModel && editorViewBar.editorViewModeViewModel.requestViewModeChange !== undefined)
                editorViewBar.editorViewModeViewModel.requestViewModeChange(index);
            editorViewBar.requestViewHook("select-editor-view-mode");
        }
    }
}
