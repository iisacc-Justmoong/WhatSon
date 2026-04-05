pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import ".." as NavigationShared

Item {
    id: applicationEditBar

    property bool compactMode: false
    property bool compactDetailPanelVisible: false
    property bool detailPanelCollapsed: false
    property int menuItemWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(196)))
    property int menuYOffset: LV.Theme.gap2
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationEditBar") : null
    readonly property var applicationEditMenuItems: applicationEditBar.buildApplicationEditMenuItems()

    signal toggleDetailPanelRequested
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested(hookReason);
    }
    function buildApplicationEditMenuItems() {
        const items = [
            {
                "label": "Agenda",
                "iconName": "toolWindowCheckDetails",
                "onTriggered": function () {
                    applicationEditBar.requestViewHook("edit-open-agenda");
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
            }
        ];
        return items;
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

            spacing: LV.Theme.gap12

            LV.IconMenuButton {
                id: applicationEditMenuButton

                bottomPadding: LV.Theme.gap2
                iconName: "toolwindowtodo"
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
            LV.IconButton {
                horizontalPadding: LV.Theme.gap2
                iconName: "columnIndex"
                rotation: 180
                tone: LV.AbstractButton.Borderless
                transformOrigin: Item.Center
                verticalPadding: LV.Theme.gap2
                visible: applicationEditBar.compactDetailPanelVisible

                onClicked: {
                    applicationEditBar.requestViewHook("open-detail-page");
                    applicationEditBar.toggleDetailPanelRequested();
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
