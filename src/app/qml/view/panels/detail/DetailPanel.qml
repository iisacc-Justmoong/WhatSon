import QtQuick

Item {
    id: detailPanel

    enum DetailContentState {
        FileInfo,
        FileStat,
        FileFormat,
        FileHistory,
        Appearance,
        Help
    }

    property int activeDetailContentState: DetailPanel.FileInfo
    property int detailContentsHeight: 324
    property int detailContentsWidth: 165
    property int headerToolbarHeight: 20
    property int headerToolbarWidth: 145
    property int panelSpacing: 10
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanel") : null
    readonly property var toolbarButtonSpecs: [
        {
            "iconName": "syncFilesModInfo",
            "stateValue": DetailPanel.FileInfo
        },
        {
            "iconName": "statisticsPanel",
            "stateValue": DetailPanel.FileStat
        },
        {
            "iconName": "fileFormat",
            "stateValue": DetailPanel.FileFormat
        },
        {
            "iconName": "toolWindowClock",
            "stateValue": DetailPanel.FileHistory
        },
        {
            "iconName": "cwmPermissionView",
            "stateValue": DetailPanel.Appearance
        },
        {
            "iconName": "featureAnswer",
            "stateValue": DetailPanel.Help
        }
    ]

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        spacing: detailPanel.panelSpacing

        DetailPanelHeaderToolbar {
            activeState: detailPanel.activeDetailContentState
            height: detailPanel.headerToolbarHeight
            toolbarButtonSpecs: detailPanel.toolbarButtonSpecs
            width: detailPanel.headerToolbarWidth

            onDetailStateChangeRequested: function (stateValue) {
                if (stateValue < DetailPanel.FileInfo || stateValue > DetailPanel.Help)
                    return;
                if (stateValue === detailPanel.activeDetailContentState)
                    return;
                detailPanel.activeDetailContentState = stateValue;
            }
        }
        DetailContents {
            activeState: detailPanel.activeDetailContentState
            height: detailPanel.detailContentsHeight
            width: detailPanel.detailContentsWidth
        }
    }
}
