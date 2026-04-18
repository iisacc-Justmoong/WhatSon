import QtQuick

Item {
    id: resourceDetailPanel

    property var resourceDetailPanelViewModel: null

    signal viewHookRequested

    function requestViewHook(reason) {
        viewHookRequested();
    }

    objectName: "ResourceDetailPanel"
}
