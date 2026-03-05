import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: rightPanel

    property int detailPanelDefaultWidth: 194
    property int detailPanelMinWidth: 145
    property color panelColor: LV.Theme.panelBackground06
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.RightPanel") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    color: rightPanel.panelColor

    DetailPanel {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: rightPanel.resolvedDetailPanelWidth
    }
}
