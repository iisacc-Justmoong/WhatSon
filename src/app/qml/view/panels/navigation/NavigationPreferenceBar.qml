import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: preferenceBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationPreferenceBar") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    spacing: 2

    LV.IconButton {
        id: preferenceButton

        checkable: false
        height: 20
        iconName: "audioToAudio"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: detailPanelControlButton

        checkable: false
        height: 20
        iconName: "columnIndex"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
