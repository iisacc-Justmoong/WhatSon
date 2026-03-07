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
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 0
        },
        {
            label: "Page",
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 1
        },
        {
            label: "Print",
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 2
        },
        {
            label: "Web",
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 3
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

    Text {
        color: Qt.rgba(1, 1, 1, 0.8)
        font.family: "Pretendard"
        font.pixelSize: 12
        font.weight: 500
        lineHeight: 12
        lineHeightMode: Text.FixedHeight
        text: "View"
    }
    LV.ComboBox {
        id: editorViewCombo

        arrow: editorViewContextMenu.opened ? LV.Stepper.Up : LV.Stepper.Down
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
