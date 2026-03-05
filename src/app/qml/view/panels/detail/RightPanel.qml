import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: rightPanel

    property int detailPanelHeight: 354
    property int detailPanelWidth: 165
    property color panelColor: LV.Theme.panelBackground06
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.RightPanel") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    color: rightPanel.panelColor

    DetailPanel {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        height: rightPanel.detailPanelHeight
        width: rightPanel.detailPanelWidth
    }
}
