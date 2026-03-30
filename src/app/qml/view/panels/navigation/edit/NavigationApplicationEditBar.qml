pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import ".." as NavigationShared

Item {
    id: applicationEditBar

    property var applicationEditMenuItems: [
        {
            "label": "Todo List",
            "iconName": "toolWindowCheckDetails",
            "onTriggered": function () {
                applicationEditBar.requestViewHook("edit-open-todo-list");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Daily Calendar",
            "iconName": "newUIlightThemeSelected",
            "onTriggered": function () {
                applicationEditBar.requestViewHook("edit-open-daily-calendar");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Weekly Calendar",
            "iconName": "table",
            "onTriggered": function () {
                applicationEditBar.requestViewHook("edit-open-weekly-calendar");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Monthly Calendar",
            "iconName": "pnpm",
            "onTriggered": function () {
                applicationEditBar.requestViewHook("edit-open-monthly-calendar");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": "Yearly Calendar",
            "iconName": "runshowCurrentFrame",
            "onTriggered": function () {
                applicationEditBar.requestViewHook("edit-open-yearly-calendar");
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
                applicationEditBar.requestViewHook("create-note");
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
                applicationEditBar.requestViewHook("edit-open-preferences");
            },
            "keyVisible": false,
            "showChevron": false
        },
        {
            "label": applicationEditBar.detailPanelCollapsed ? "Show Detail Panel" : "Hide Detail Panel",
            "iconName": "columnIndex",
            "onTriggered": function () {
                applicationEditBar.requestViewHook(
                    applicationEditBar.detailPanelCollapsed ? "expand-detail-panel" : "collapse-detail-panel");
                applicationEditBar.toggleDetailPanelRequested();
            },
            "keyVisible": false,
            "showChevron": false
        }
    ]
    property bool compactMode: false
    property bool detailPanelCollapsed: false
    property int menuItemWidth: 196
    property int menuYOffset: 2
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationEditBar") : null

    signal toggleDetailPanelRequested
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested(hookReason);
    }

    implicitHeight: applicationEditRow.implicitHeight
    implicitWidth: applicationEditRow.implicitWidth

    LV.HStack {
        id: applicationEditRow

        anchors.fill: parent
        spacing: LV.Theme.gap12

        Loader {
            Layout.alignment: Qt.AlignVCenter
            sourceComponent: applicationEditBar.compactMode ? compactApplicationEditComponent : fullApplicationEditComponent
        }
    }
    Component {
        id: fullApplicationEditComponent

        LV.HStack {
            id: fullApplicationEditBar

            spacing: LV.Theme.gap12

            NavigationShared.NavigationApplicationCalendarBar {
                Layout.alignment: Qt.AlignVCenter

                onViewHookRequested: function (reason) {
                    applicationEditBar.requestViewHook(reason);
                }
            }
            NavigationShared.NavigationApplicationAddNewBar {
                Layout.alignment: Qt.AlignVCenter
            }
            NavigationShared.NavigationApplicationPreferenceBar {
                Layout.alignment: Qt.AlignVCenter
                detailPanelCollapsed: applicationEditBar.detailPanelCollapsed

                onToggleDetailPanelRequested: applicationEditBar.toggleDetailPanelRequested()
            }
        }
    }
    Component {
        id: compactApplicationEditComponent

        LV.HStack {
            id: compactApplicationEditBar

            spacing: LV.Theme.gapNone

            LV.IconMenuButton {
                id: applicationEditMenuButton

                bottomPadding: LV.Theme.gap2
                iconName: "toolWindowCheckDetails"
                leftPadding: LV.Theme.gap2
                rightPadding: LV.Theme.gap4
                spacing: LV.Theme.gapNone
                tone: LV.AbstractButton.Borderless
                topPadding: LV.Theme.gap2

                onClicked: {
                    if (applicationEditContextMenu.opened) {
                        applicationEditContextMenu.close();
                        return;
                    }
                    applicationEditContextMenu.openFor(
                        applicationEditMenuButton,
                        applicationEditMenuButton.width,
                        applicationEditMenuButton.height + applicationEditBar.menuYOffset);
                }
            }
        }
    }
    LV.ContextMenu {
        id: applicationEditContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: applicationEditBar.menuItemWidth
        items: applicationEditBar.applicationEditMenuItems
        modal: false
        parent: Controls.Overlay.overlay
    }
}
