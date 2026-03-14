import QtQuick

Item {
    id: detailContents

    property var activeContentViewModel: null
    property string activeStateName: "fileInfo"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailContents") : null
    readonly property string resolvedActiveStateName: detailContents.normalizeStateName(detailContents.activeStateName)

    signal viewHookRequested

    function normalizeStateName(value) {
        const normalized = value === undefined || value === null ? "" : String(value).trim();
        switch (normalized) {
        case "fileInfo":
        case "fileStat":
        case "fileFormat":
        case "fileHistory":
        case "appearance":
        case "help":
            return normalized;
        default:
            return "fileInfo";
        }
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    state: detailContents.resolvedActiveStateName

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
