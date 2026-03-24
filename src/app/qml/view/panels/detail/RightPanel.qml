import QtQuick
import LVRS 1.0 as LV

Rectangle {
    id: rightPanel

    readonly property string figmaNodeId: "155:4574"
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

    objectName: "RightPanel"
    implicitWidth: LV.Theme.scaleMetric(rightPanel.detailPanelDefaultWidth)
    color: rightPanel.panelColor

    DetailPanel {
        anchors.fill: parent
    }
}
