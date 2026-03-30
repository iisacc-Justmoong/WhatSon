pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import ".." as NavigationShared

Item {
    id: applicationViewBar

    property var applicationViewMenuItems: [
        {
            "label": "Todo List",
            "iconName": "toolWindowCheckDetails",
            "onTriggered": function () {
                applicationViewBar.requestViewHook("view-open-todo-list");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Daily Calendar",
            "iconName": "newUIlightThemeSelected",
            "onTriggered": function () {
                applicationViewBar.requestViewHook("view-open-daily-calendar");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Weekly Calendar",
            "iconName": "table",
            "onTriggered": function () {
                applicationViewBar.requestViewHook("view-open-weekly-calendar");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Monthly Calendar",
            "iconName": "pnpm",
            "onTriggered": function () {
                applicationViewBar.requestViewHook("view-open-monthly-calendar");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Yearly Calendar",
            "iconName": "runshowCurrentFrame",
            "onTriggered": function () {
                applicationViewBar.requestViewHook("view-open-yearly-calendar");
            },
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
                applicationViewBar.requestViewHook("create-note");
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
            "onTriggered": function () {
                applicationViewBar.requestViewHook("view-open-preferences");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": applicationViewBar.detailPanelCollapsed ? "Show Detail Panel" : "Hide Detail Panel",
            "iconName": "columnIndex",
            "onTriggered": function () {
                applicationViewBar.requestViewHook(
                    applicationViewBar.detailPanelCollapsed ? "expand-detail-panel" : "collapse-detail-panel");
                applicationViewBar.toggleDetailPanelRequested();
            },
            "keyVisible": false,
            "showChevron": false
        }
    ]
    property bool compactMode: false
    property bool detailPanelCollapsed: false
    property int menuItemWidth: 196
    property int menuYOffset: 2
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationViewBar") : null

    signal toggleDetailPanelRequested
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested(hookReason);
    }

    implicitHeight: applicationViewRow.implicitHeight
    implicitWidth: applicationViewRow.implicitWidth

    LV.HStack {
        id: applicationViewRow

        anchors.fill: parent
        spacing: LV.Theme.gap12

        Loader {
            Layout.alignment: Qt.AlignVCenter
            sourceComponent: applicationViewBar.compactMode ? compactApplicationViewComponent : fullApplicationViewComponent
        }
    }
    Component {
        id: fullApplicationViewComponent

        LV.HStack {
            id: fullApplicationViewBar

            spacing: LV.Theme.gap12

            NavigationShared.NavigationApplicationCalendarBar {
                Layout.alignment: Qt.AlignVCenter

                onViewHookRequested: function (reason) {
                    applicationViewBar.requestViewHook(reason);
                }
            }
            NavigationShared.NavigationApplicationAddNewBar {
                Layout.alignment: Qt.AlignVCenter
            }
            NavigationShared.NavigationApplicationPreferenceBar {
                Layout.alignment: Qt.AlignVCenter
                detailPanelCollapsed: applicationViewBar.detailPanelCollapsed

                onToggleDetailPanelRequested: applicationViewBar.toggleDetailPanelRequested()
            }
        }
    }
    Component {
        id: compactApplicationViewComponent

        LV.HStack {
            id: compactApplicationViewBar

            spacing: LV.Theme.gapNone

            LV.IconMenuButton {
                id: applicationViewMenuButton

                bottomPadding: LV.Theme.gap2
                iconName: "toolWindowCheckDetails"
                leftPadding: LV.Theme.gap2
                rightPadding: LV.Theme.gap4
                spacing: LV.Theme.gapNone
                tone: LV.AbstractButton.Borderless
                topPadding: LV.Theme.gap2

                onClicked: {
                    if (applicationViewContextMenu.opened) {
                        applicationViewContextMenu.close();
                        return;
                    }
                    applicationViewContextMenu.openFor(
                        applicationViewMenuButton,
                        applicationViewMenuButton.width,
                        applicationViewMenuButton.height + applicationViewBar.menuYOffset);
                }
            }
        }
    }
    LV.ContextMenu {
        id: applicationViewContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: applicationViewBar.menuItemWidth
        items: applicationViewBar.applicationViewMenuItems
        modal: false
        parent: Controls.Overlay.overlay
    }
}
