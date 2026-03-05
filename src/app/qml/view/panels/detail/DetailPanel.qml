import QtQuick

Item {
    id: detailPanel

    property int detailContentsHeight: 324
    property int detailContentsWidth: 165
    property int headerToolbarHeight: 20
    property int headerToolbarWidth: 145
    property int panelSpacing: 10
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanel") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        spacing: detailPanel.panelSpacing

        DetailPanelHeaderToolbar {
            height: detailPanel.headerToolbarHeight
            width: detailPanel.headerToolbarWidth
        }
        DetailContents {
            height: detailPanel.detailContentsHeight
            width: detailPanel.detailContentsWidth
        }
    }
}
