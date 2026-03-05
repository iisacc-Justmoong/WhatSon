import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: exportBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationExportBar") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    spacing: 2

    LV.IconButton {
        id: exportButton

        checkable: false
        height: 20
        iconName: "generalupload"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: printButton

        checkable: false
        height: 20
        iconName: "generalprint"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: mailingButton

        checkable: false
        height: 20
        iconName: "mailer"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
