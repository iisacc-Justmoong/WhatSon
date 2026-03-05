import QtQuick

Item {
    id: detailContents

    property string activeStateName: "fileInfo"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailContents") : null

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    state: detailContents.activeStateName
    states: [
        State {
            name: "fileInfo"
        },
        State {
            name: "fileStat"
        },
        State {
            name: "fileFormat"
        },
        State {
            name: "fileHistory"
        },
        State {
            name: "appearance"
        },
        State {
            name: "help"
        }
    ]
}
