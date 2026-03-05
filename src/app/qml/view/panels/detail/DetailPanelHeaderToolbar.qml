import QtQuick
import LVRS 1.0 as LV

Item {
    id: detailPanelHeaderToolbar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailPanelHeaderToolbar") : null
    property var toolbarButtonSpecs: [
        {
            "iconName": "syncFilesModInfo",
            "selected": true
        },
        {
            "iconName": "statisticsPanel",
            "selected": false
        },
        {
            "iconName": "fileFormat",
            "selected": false
        },
        {
            "iconName": "toolWindowClock",
            "selected": false
        },
        {
            "iconName": "cwmPermissionView",
            "selected": false
        },
        {
            "iconName": "featureAnswer",
            "selected": false
        }
    ]

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    Row {
        anchors.fill: parent
        spacing: 5

        Repeater {
            model: detailPanelHeaderToolbar.toolbarButtonSpecs.length

            LV.IconButton {
                readonly property var buttonSpec: detailPanelHeaderToolbar.toolbarButtonSpecs[index]

                backgroundColor: buttonSpec.selected ? LV.Theme.panelBackground12 : "transparent"
                backgroundColorDisabled: buttonSpec.selected ? LV.Theme.panelBackground12 : "transparent"
                backgroundColorHover: buttonSpec.selected ? LV.Theme.panelBackground12 : "transparent"
                backgroundColorPressed: buttonSpec.selected ? LV.Theme.panelBackground12 : "transparent"
                checkable: false
                cornerRadius: 4
                height: 20
                horizontalPadding: 2
                iconName: buttonSpec.iconName
                iconSize: 16
                tone: buttonSpec.selected ? LV.AbstractButton.Default : LV.AbstractButton.Borderless
                verticalPadding: 2
                width: 20
            }
        }
    }
}
