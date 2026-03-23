import QtQuick
import LVRS 1.0 as LV
import "detail" as DetailView

Rectangle {
    id: detailPanel

    property color panelColor: "transparent"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("DetailPanelLayout") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    color: detailPanel.panelColor

    DetailView.RightPanel {
        anchors.fill: parent
        panelColor: detailPanel.panelColor
    }
}
