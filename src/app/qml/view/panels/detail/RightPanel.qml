import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: rightPanel

    property color panelColor: LV.Theme.panelBackground06
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.RightPanel") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    color: rightPanel.panelColor

    DetailPanel {
        anchors.fill: parent
    }
}
