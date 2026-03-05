import QtQuick
import LVRS 1.0 as LV
import "detail" as DetailView

Rectangle {
    id: detailPanel

    property color panelColor: LV.Theme.panelBackground06
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("DetailPanelLayout") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    color: detailPanel.panelColor

    DetailView.RightPanel {
        anchors.fill: parent
        panelColor: detailPanel.panelColor
    }
}
