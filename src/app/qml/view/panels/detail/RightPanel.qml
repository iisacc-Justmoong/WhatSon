import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: rightPanel

    readonly property string figmaNodeId: "155:4574"
    property int detailPanelDefaultWidth: LV.Theme.inputMinWidth + LV.Theme.gap14
    property int detailPanelMinWidth: LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + Math.round(LV.Theme.strokeThin)
    property color panelColor: "transparent"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.RightPanel") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    objectName: "RightPanel"
    implicitWidth: rightPanel.detailPanelDefaultWidth
    color: rightPanel.panelColor

    DetailPanel {
        anchors.fill: parent
    }
}
