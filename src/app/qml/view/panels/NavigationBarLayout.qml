import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "navigation" as NavigationView
import "navigation/control" as NavigationControlMode
import "navigation/edit" as NavigationEditMode
import "navigation/view" as NavigationViewMode

Rectangle {
    id: navigationBar

    readonly property string activeNavigationModeName: navigationModeViewModel && navigationModeViewModel.activeModeName !== undefined ? navigationModeViewModel.activeModeName : "Control"
    readonly property int bottomInset: LV.Theme.gap2
    readonly property int compactHorizontalInset: LV.Theme.gapNone
    readonly property int compactLeftGroupSpacing: LV.Theme.gap4
    property bool compactNoteListControlsVisible: false
    property bool compactSettingsVisible: true
    readonly property int compactRightGroupSpacing: LV.Theme.gap12
    property bool compactAddFolderEnabled: true
    property bool compactAddFolderVisible: false
    property bool compactLeadingActionEnabled: true
    property string compactLeadingActionIconName: "generalchevronLeft"
    property bool compactLeadingActionVisible: false
    property bool compactMode: false
    property color compactSurfaceColor: LV.Theme.panelBackground10
    readonly property int compactTopInset: LV.Theme.gapNone
    property bool detailPanelCollapsed: false
    property var editorViewModeViewModel: null
    readonly property int effectivePanelHeight: panelHeight
    property var navigationModeViewModel: null
    property color panelColor: "transparent"
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("NavigationBarLayout") : null
    property int panelHeight: LV.Theme.gap24
    property bool sidebarCollapsed: false
    readonly property int sideInset: compactMode ? LV.Theme.gap8 : LV.Theme.gap4
    readonly property int topInset: LV.Theme.gap2

    signal compactAddFolderRequested
    signal compactLeadingActionRequested
    signal toggleDetailPanelRequested
    signal toggleSidebarRequested
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    Layout.fillWidth: true
    Layout.preferredHeight: navigationBar.effectivePanelHeight
    clip: true
    color: navigationBar.compactMode ? LV.Theme.accentTransparent : navigationBar.panelColor

    Rectangle {
        id: navigationBarSurface

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: navigationBar.compactMode ? navigationBar.compactHorizontalInset : LV.Theme.gapNone
        anchors.right: parent.right
        anchors.rightMargin: navigationBar.compactMode ? navigationBar.compactHorizontalInset : LV.Theme.gapNone
        anchors.top: parent.top
        anchors.topMargin: navigationBar.compactMode ? navigationBar.compactTopInset : LV.Theme.gapNone
        color: navigationBar.compactMode ? navigationBar.compactSurfaceColor : navigationBar.panelColor
        radius: navigationBar.compactMode ? LV.Theme.radiusXl * 2 : LV.Theme.gapNone

        Item {
            id: navigationBarContents

            anchors.bottomMargin: navigationBar.bottomInset
            anchors.fill: parent
            anchors.leftMargin: navigationBar.sideInset
            anchors.rightMargin: navigationBar.sideInset
            anchors.topMargin: navigationBar.topInset

            LV.HStack {
                anchors.fill: parent
                spacing: LV.Theme.gapNone
                visible: !navigationBar.compactMode

                NavigationView.NavigationPropertiesBar {
                    id: propertiesBar

                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: LV.Theme.gap20
                    compactMode: false
                    editorViewModeViewModel: navigationBar.editorViewModeViewModel
                    navigationModeViewModel: navigationBar.navigationModeViewModel
                    sidebarCollapsed: navigationBar.sidebarCollapsed

                    onToggleSidebarRequested: navigationBar.toggleSidebarRequested()
                }
                Item {
                    Layout.fillWidth: true
                }
                Loader {
                    id: applicationBarLoader

                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: LV.Theme.gap20
                    sourceComponent: {
                        switch (navigationBar.activeNavigationModeName) {
                        case "View":
                            return applicationViewBarComponent;
                        case "Edit":
                            return applicationEditBarComponent;
                        case "Control":
                        default:
                            return applicationControlBarComponent;
                        }
                    }
                }
            }
            LV.HStack {
                anchors.fill: parent
                spacing: LV.Theme.gapNone
                visible: navigationBar.compactMode

                LV.HStack {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: navigationBar.compactLeftGroupSpacing

                    LV.IconButton {
                        enabled: navigationBar.compactLeadingActionEnabled
                        horizontalPadding: LV.Theme.gap2
                        iconName: navigationBar.compactLeadingActionIconName
                        tone: LV.AbstractButton.Borderless
                        verticalPadding: LV.Theme.gap2
                        visible: navigationBar.compactLeadingActionVisible

                        onClicked: navigationBar.compactLeadingActionRequested()
                    }
                    LV.IconButton {
                        visible: navigationBar.compactSettingsVisible
                        iconName: "settings"
                        horizontalPadding: LV.Theme.gap2
                        tone: LV.AbstractButton.Borderless
                        verticalPadding: LV.Theme.gap2

                        onClicked: navigationBar.requestViewHook("compact-open-settings")
                    }
                    NavigationView.NavigationModeBar {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredHeight: LV.Theme.gap18
                        navigationModeViewModel: navigationBar.navigationModeViewModel
                        showLabel: false
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
                LV.HStack {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: navigationBar.compactRightGroupSpacing

                    LV.IconButton {
                        enabled: navigationBar.compactAddFolderEnabled
                        horizontalPadding: LV.Theme.gap2
                        iconName: "nodesnewFolder"
                        tone: LV.AbstractButton.Borderless
                        verticalPadding: LV.Theme.gap2
                        visible: navigationBar.compactAddFolderVisible

                        onClicked: {
                            navigationBar.requestViewHook("compact-create-folder");
                            navigationBar.compactAddFolderRequested();
                        }
                    }
                    Loader {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredHeight: LV.Theme.gap20
                        sourceComponent: {
                            switch (navigationBar.activeNavigationModeName) {
                            case "View":
                                return applicationViewBarComponent;
                            case "Edit":
                                return applicationEditBarComponent;
                            case "Control":
                            default:
                                return applicationControlBarComponent;
                            }
                        }
                    }
                }
            }
        }
    }
    Component {
        id: applicationViewBarComponent

        NavigationViewMode.NavigationApplicationViewBar {
            compactMode: navigationBar.compactMode
            detailPanelCollapsed: navigationBar.detailPanelCollapsed

            onToggleDetailPanelRequested: navigationBar.toggleDetailPanelRequested()
        }
    }
    Component {
        id: applicationEditBarComponent

        NavigationEditMode.NavigationApplicationEditBar {
            compactMode: navigationBar.compactMode
            detailPanelCollapsed: navigationBar.detailPanelCollapsed

            onToggleDetailPanelRequested: navigationBar.toggleDetailPanelRequested()
        }
    }
    Component {
        id: applicationControlBarComponent

        NavigationControlMode.NavigationApplicationControlBar {
            compactMode: navigationBar.compactMode
            compactNoteListControlsVisible: navigationBar.compactNoteListControlsVisible
            detailPanelCollapsed: navigationBar.detailPanelCollapsed

            onToggleDetailPanelRequested: navigationBar.toggleDetailPanelRequested()
        }
    }
}
