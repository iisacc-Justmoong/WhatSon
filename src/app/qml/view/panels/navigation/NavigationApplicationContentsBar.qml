import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.HStack {
    id: applicationContentsBar

    property var applicationContentsMenuItems: [
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

    spacing: 12

    Loader {
        id: applicationContentsLoader

        Layout.alignment: Qt.AlignVCenter
        sourceComponent: applicationContentsBar.compactMode ? compactApplicationContentsComponent : fullApplicationContentsComponent
    }
    Component {
        id: fullApplicationContentsComponent

        LV.HStack {
            id: fullApplicationContentsBar

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
        id: compactApplicationContentsComponent

        LV.HStack {
            id: compactApplicationContentsBar

            spacing: 12

            LV.IconMenuButton {
                id: applicationContentsMenuButton

                height: 20
                iconName: "generalprojectStructure"
                iconSize: 16
                tone: LV.AbstractButton.Default
                width: 40

                onClicked: {
                    if (applicationContentsContextMenu.opened) {
                        applicationContentsContextMenu.close();
                        return;
                    }
                    applicationContentsContextMenu.openFor(applicationContentsMenuButton, 0, height + applicationContentsBar.menuYOffset);
                }
            }
            LV.IconButton {
                id: compactPreferenceButton

                height: 20
                iconName: "audioToAudio"
                iconSize: 16
                tone: LV.AbstractButton.Borderless
                width: 20
            }
        }
    }
    LV.ContextMenu {
        id: applicationContentsContextMenu

        autoCloseOnTrigger: true
        dismissOnGlobalContextRequest: true
        dismissOnGlobalPress: true
        itemWidth: applicationContentsBar.menuItemWidth
        items: applicationContentsBar.applicationContentsMenuItems
    }
}
