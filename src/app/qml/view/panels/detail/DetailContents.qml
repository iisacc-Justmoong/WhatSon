import QtQuick

Item {
    id: detailContents

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailContents") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }
}
