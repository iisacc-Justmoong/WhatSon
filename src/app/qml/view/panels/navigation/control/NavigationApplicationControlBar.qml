import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import ".." as NavigationShared

Item {
    id: applicationControlBar

    readonly property var compactNoteListMenuItems: [
        {
            "label": "Show Structure",
            "iconName": "toolwindowtodo",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Sort",
            "iconName": "sortByType",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Visibility",
            "iconName": "cwmPermissionView",
            "keyVisible": false,
            "showChevron": false
        }
    ]
    property var applicationControlMenuItems: [
        {
            "label": "Show Structure",
            "iconName": "generalprojectStructure",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Pin Window",
            "iconName": "pin",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Alerts",
            "iconName": "toolwindownotifications",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "iconName": "startTimer",
            "label": "Timer",
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
        },
        {
            "type": "divider"
        },
        {
            "label": "New File",
            "iconName": "addFile",
            "onTriggered": function () {
                applicationControlBar.requestViewHook("create-note");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "type": "divider"
        },
        {
            "label": "Preferences",
            "iconName": "audioToAudio",
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": applicationControlBar.detailPanelCollapsed ? "Show Detail Panel" : "Hide Detail Panel",
            "iconName": "columnIndex",
            "onTriggered": function () {
                applicationControlBar.requestViewHook(
                    applicationControlBar.detailPanelCollapsed ? "expand-detail-panel" : "collapse-detail-panel");
                applicationControlBar.toggleDetailPanelRequested();
            },
            "keyVisible": false,
            "showChevron": false
        }
    ]
    property bool compactMode: false
    property bool compactNoteListControlsVisible: false
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
            sourceComponent: {
                if (!applicationControlBar.compactMode)
                    return fullApplicationControlComponent;
                if (applicationControlBar.compactNoteListControlsVisible)
                    return compactNoteListApplicationControlComponent;
                return compactApplicationControlComponent;
            }
        }
    }
    Component {
        id: fullApplicationControlComponent

        LV.HStack {
            id: fullApplicationControlBar

            spacing: 12

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

                bottomPadding: LV.Theme.gap2
                iconName: "toolwindowtodo"
                leftPadding: LV.Theme.gap2
                rightPadding: LV.Theme.gap4
                spacing: LV.Theme.gapNone
                tone: LV.AbstractButton.Borderless
                topPadding: LV.Theme.gap2

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
    Component {
        id: compactNoteListApplicationControlComponent

        LV.HStack {
            id: compactNoteListApplicationControlBar

            spacing: LV.Theme.gap12

            LV.IconButton {
                iconName: "sortByType"
                tone: LV.AbstractButton.Borderless

                onClicked: applicationControlBar.requestViewHook("compact-note-list-sort")
            }
            LV.IconButton {
                iconName: "cwmPermissionView"
                tone: LV.AbstractButton.Borderless

                onClicked: applicationControlBar.requestViewHook("compact-note-list-visibility")
            }
            LV.IconMenuButton {
                id: noteListApplicationControlMenuButton

                bottomPadding: LV.Theme.gap2
                iconName: "toolwindowtodo"
                leftPadding: LV.Theme.gap2
                rightPadding: LV.Theme.gap4
                spacing: LV.Theme.gapNone
                tone: LV.AbstractButton.Borderless
                topPadding: LV.Theme.gap2

                onClicked: {
                    if (noteListApplicationControlContextMenu.opened) {
                        noteListApplicationControlContextMenu.close();
                        return;
                    }
                    noteListApplicationControlContextMenu.openFor(
                        noteListApplicationControlMenuButton,
                        noteListApplicationControlMenuButton.width,
                        noteListApplicationControlMenuButton.height + applicationControlBar.menuYOffset);
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
    LV.ContextMenu {
        id: noteListApplicationControlContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: applicationControlBar.menuItemWidth
        items: applicationControlBar.compactNoteListMenuItems
        modal: false
        parent: Controls.Overlay.overlay
    }
}
