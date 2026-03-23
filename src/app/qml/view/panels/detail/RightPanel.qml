import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: rightPanel

    property int detailPanelDefaultWidth: 194
    property int detailPanelMinWidth: 145
    property color panelColor: "transparent"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.RightPanel") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
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
