pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import "." as ViewNavigation
import ".." as NavigationShared

Item {
    id: applicationViewBar

    property bool compactMode: false
    property bool compactDetailPanelVisible: false
    property bool detailPanelCollapsed: false
    property int menuItemWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(196)))
    property int menuYOffset: LV.Theme.gap2
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationApplicationViewBar") : null
    readonly property var applicationViewMenuItems: applicationViewBar.buildApplicationViewMenuItems()

    signal toggleDetailPanelRequested
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested(hookReason);
    }
    function buildApplicationViewMenuItems() {
        const items = [
            {
                "label": "Read Only",
                "iconName": "readerMode",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-toggle-read-only");
                },
                "keyVisible": false,
                "showChevron": false
            },
            {
                "label": "Wrap Text",
                "iconName": "textAutoGenerate",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-toggle-wrap-text");
                },
                "keyVisible": false,
                "showChevron": false
            },
            {
                "label": "Center View",
                "iconName": "singleRecordView",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-option-center-view");
                },
                "keyVisible": false,
                "showChevron": false
            },
            {
                "label": "Text To Speech",
                "iconName": "textToSpeech",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-open-text-to-speech-options");
                },
                "keyVisible": false,
                "showChevron": false
            },
            {
                "label": "Paper Options",
                "iconName": "fileFormat",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-open-paper-options");
                },
                "keyVisible": false,
                "showChevron": false
            },
            {
                "type": "divider"
            },
            {
                "label": "Center View Mode",
                "iconName": "singleRecordView",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-mode-center-view");
                },
                "keyVisible": false,
                "showChevron": false
            },
            {
                "label": "Focus Mode",
                "iconName": "imagefitContent",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-mode-focus");
                },
                "keyVisible": false,
                "showChevron": false
            },
            {
                "label": "Presentation",
                "iconName": "runshowCurrentFrame",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-mode-presentation");
                },
                "keyVisible": false,
                "showChevron": false
            },
            {
                "type": "divider"
            },
            {
                "label": "Agenda",
                "iconName": "validator",
                "onTriggered": function () {
                    applicationViewBar.requestViewHook("view-open-agenda");
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
            }
        ];
        return items;
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

            ViewNavigation.NavigationApplicationViewOptionBar {
                id: viewOptionBar

                Layout.alignment: Qt.AlignVCenter

                onViewHookRequested: function (reason) {
                    applicationViewBar.requestViewHook(reason);
                }
            }
            ViewNavigation.NavigationApplicationViewModeBar {
                id: modeBar

                Layout.alignment: Qt.AlignVCenter

                onViewHookRequested: function (reason) {
                    applicationViewBar.requestViewHook(reason);
                }
            }
            ViewNavigation.NavigationApplicationViewCalendarBar {
                id: calendarBar

                Layout.alignment: Qt.AlignVCenter

                onViewHookRequested: function (reason) {
                    applicationViewBar.requestViewHook(reason);
                }
            }
            NavigationShared.NavigationApplicationAddNewBar {
                id: addNewBar

                Layout.alignment: Qt.AlignVCenter
            }
            NavigationShared.NavigationApplicationPreferenceBar {
                id: preferenceBar

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

            spacing: LV.Theme.gap12

            LV.IconMenuButton {
                id: applicationViewMenuButton

                bottomPadding: LV.Theme.gap2
                iconName: "toolwindowtodo"
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
            LV.IconButton {
                horizontalPadding: LV.Theme.gap2
                iconName: "columnIndex"
                rotation: 180
                tone: LV.AbstractButton.Borderless
                transformOrigin: Item.Center
                verticalPadding: LV.Theme.gap2
                visible: applicationViewBar.compactDetailPanelVisible

                onClicked: {
                    applicationViewBar.requestViewHook("open-detail-page");
                    applicationViewBar.toggleDetailPanelRequested();
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
