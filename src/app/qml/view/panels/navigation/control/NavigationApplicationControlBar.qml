import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import ".." as NavigationShared

Item {
    id: applicationControlBar

    property var applicationControlMenuItems: [
        {
            "label": "Show Structure",
            "iconName": "columnIndex",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Sync Files",
            "iconName": "syncFiles",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "type": "divider"
        },
        {
            "label": "Todo List",
            "iconName": "toolWindowCheckDetails",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Daily",
            "iconName": "newUIlightThemeSelected",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Weekly",
            "iconName": "table",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Monthly",
            "iconName": "pnpm",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Yearly",
            "iconName": "runshowCurrentFrame",
            "keyVisible": false,
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
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Pin",
            "iconName": "pin",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Notifications",
            "iconName": "toolwindownotifications",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Timer",
            "iconName": "startTimer",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "type": "divider"
        },
        {
            "label": "Export",
            "iconName": "generalupload",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Print",
            "iconName": "generalprint",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Mail",
            "iconName": "mailer",
            "keyVisible": false,
            "showChevron": false
        }
    ]
    property bool compactMode: false
    property int menuItemWidth: 176
    property int menuYOffset: 2
    property bool detailPanelCollapsed: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationControlBar") : null

    signal toggleDetailPanelRequested
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
            NavigationAppControlBar {
                id: appControlBar

                Layout.alignment: Qt.AlignVCenter
            }
            NavigationExportBar {
                id: exportBar

                Layout.alignment: Qt.AlignVCenter
            }
            NavigationShared.NavigationAddNewBar {
                id: addNewBar

                Layout.alignment: Qt.AlignVCenter
            }
            NavigationShared.NavigationPreferenceBar {
                id: preferenceBar

                Layout.alignment: Qt.AlignVCenter
                detailPanelCollapsed: applicationControlBar.detailPanelCollapsed

                onToggleDetailPanelRequested: applicationControlBar.toggleDetailPanelRequested()
            }
        }
    }
    Component {
        id: compactApplicationControlComponent

        LV.HStack {
            id: compactApplicationControlBar

            spacing: 0

            LV.IconMenuButton {
                id: applicationControlMenuButton

                iconName: "generalprojectStructure"

                onClicked: {
                    if (applicationControlContextMenu.opened) {
                        applicationControlContextMenu.close();
                        return;
                    }
                    applicationControlContextMenu.openFor(
                        applicationControlMenuButton,
                        applicationControlMenuButton.width,
                        applicationControlMenuButton.height + applicationControlBar.menuYOffset);
                }
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
