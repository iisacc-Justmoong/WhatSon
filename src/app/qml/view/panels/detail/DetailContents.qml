import QtQuick

Item {
    id: detailContents

    property int activeState: DetailPanel.FileInfo
    readonly property string activeStateName: {
        switch (detailContents.activeState) {
        case DetailPanel.FileInfo:
            return "fileInfo";
        case DetailPanel.FileStat:
            return "fileStat";
        case DetailPanel.FileFormat:
            return "fileFormat";
        case DetailPanel.FileHistory:
            return "fileHistory";
        case DetailPanel.Appearance:
            return "appearance";
        case DetailPanel.Help:
            return "help";
        default:
            return "fileInfo";
        }
    }
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
