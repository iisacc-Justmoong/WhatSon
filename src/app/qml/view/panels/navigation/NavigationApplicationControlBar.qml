import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

Item {
    id: applicationControlBar

    property var applicationControlMenuItems: [
        {
            "label": "Show Structure",
            "iconName": "columnIndex",
            "showChevron": false
        },
        {
            "label": "Sync Files",
            "iconName": "syncFiles",
            "showChevron": false
        },
        {
            "type": "divider"
        },
        {
            "label": "Todo List",
            "iconName": "toolWindowCheckDetails",
            "showChevron": false
        },
        {
            "label": "Daily",
            "iconName": "newUIlightThemeSelected",
            "showChevron": false
        },
        {
            "label": "Weekly",
            "iconName": "table",
            "showChevron": false
        },
        {
            "label": "Monthly",
            "iconName": "pnpm",
            "showChevron": false
        },
        {
            "label": "Yearly",
            "iconName": "runshowCurrentFrame",
            "showChevron": false
        },
        {
            "type": "divider"
        },
        {
            "label": "Add File",
            "iconName": "addFile",
            "onTriggered": function () {
                applicationControlBar.requestViewHook("create-note");
            },
            "showChevron": false
        },
        {
            "label": "Pin",
            "iconName": "pin",
            "showChevron": false
        },
        {
            "label": "Notifications",
            "iconName": "toolwindownotifications",
            "showChevron": false
        },
        {
            "label": "Timer",
            "iconName": "startTimer",
            "showChevron": false
        },
        {
            "type": "divider"
        },
        {
            "label": "Export",
            "iconName": "generalupload",
            "showChevron": false
        },
        {
            "label": "Print",
            "iconName": "generalprint",
            "showChevron": false
        },
        {
            "label": "Mail",
            "iconName": "mailer",
            "showChevron": false
        }
    ]
    property bool compactMode: false
    property int menuItemWidth: 176
    property int menuYOffset: 2
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationControlBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    implicitHeight: applicationControlRow.implicitHeight
    implicitWidth: applicationControlRow.implicitWidth

    LV.HStack {
        id: applicationControlRow

        anchors.fill: parent
        spacing: LV.Theme.gap12

        Loader {
            id: applicationControlLoader

            Layout.alignment: Qt.AlignVCenter
            sourceComponent: applicationControlBar.compactMode ? compactApplicationControlComponent : fullApplicationControlComponent
        }
    }
    Component {
        id: fullApplicationControlComponent

        LV.HStack {
            id: fullApplicationControlBar

            spacing: 12

            NavigationCalendarBar {
                id: calendarBar

                Layout.alignment: Qt.AlignVCenter
            }
            NavigationAddNewBar {
                id: addNewBar

                Layout.alignment: Qt.AlignVCenter
            }
            NavigationAppControlBar {
                id: appControlBar

                Layout.alignment: Qt.AlignVCenter
            }
            NavigationExportBar {
                id: exportBar

                Layout.alignment: Qt.AlignVCenter
            }
            NavigationPreferenceBar {
                id: preferenceBar

                Layout.alignment: Qt.AlignVCenter
            }
        }
    }
    Component {
        id: compactApplicationControlComponent

        LV.HStack {
            id: compactApplicationControlBar

            spacing: 12

            LV.IconMenuButton {
                id: applicationControlMenuButton

                iconName: "generalprojectStructure"

                onClicked: {
                    if (applicationControlContextMenu.opened) {
                        applicationControlContextMenu.close();
                        return;
                    }
                    applicationControlContextMenu.openFor(applicationControlMenuButton, 0, height + applicationControlBar.menuYOffset);
                }
            }
            LV.IconButton {
                id: compactPreferenceButton

                iconName: "audioToAudio"
            }
        }
    }
    LV.ContextMenu {
        id: applicationControlContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: applicationControlBar.menuItemWidth
        items: applicationControlBar.applicationControlMenuItems
        modal: false
        parent: Controls.Overlay.overlay
    }
}
