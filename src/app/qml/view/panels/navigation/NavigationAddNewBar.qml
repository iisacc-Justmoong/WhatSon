import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: addNewBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationAddNewBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    spacing: 0

    LV.IconMenuButton {
        id: newFileButton

        checkable: false
        iconName: "addFile"
        tone: LV.AbstractButton.Borderless

        onClicked: addNewBar.requestViewHook("create-note")
    }
}
