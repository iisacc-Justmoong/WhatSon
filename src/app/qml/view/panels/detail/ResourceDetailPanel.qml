import QtQuick

Item {
    id: resourceDetailPanel

    property var resourceDetailPanelController: null

    signal viewHookRequested

    function requestViewHook(reason) {
        viewHookRequested();
    }

    objectName: "ResourceDetailPanel"
}
