import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "navigation" as NavigationView

Rectangle {
    id: navigationBar

    readonly property string activeNavigationModeName: navigationModeViewModel && navigationModeViewModel.activeModeName !== undefined ? navigationModeViewModel.activeModeName : "Control"
    readonly property int bottomInset: 2
    readonly property int compactHorizontalInset: Math.max(12, Math.min(24, Math.round(width * 0.04)))
    property bool compactMode: false
    property color compactSurfaceColor: LV.Theme.panelBackground06
    readonly property int compactTopInset: compactHorizontalInset
    property var editorViewModeViewModel: null
    readonly property int effectivePanelHeight: compactMode ? (compactTopInset + panelHeight) : panelHeight
    property var navigationModeViewModel: null
    property color panelColor: LV.Theme.panelBackground06
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("NavigationBarLayout") : null
    property int panelHeight: 24
    readonly property int sideInset: compactMode ? 8 : 4
    readonly property int topInset: 2

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
        anchors.leftMargin: navigationBar.compactMode ? navigationBar.compactHorizontalInset : 0
        anchors.right: parent.right
        anchors.rightMargin: navigationBar.compactMode ? navigationBar.compactHorizontalInset : 0
        anchors.top: parent.top
        anchors.topMargin: navigationBar.compactMode ? navigationBar.compactTopInset : 0
        color: navigationBar.compactMode ? navigationBar.compactSurfaceColor : navigationBar.panelColor
        radius: navigationBar.compactMode ? 32 : 0

        Item {
            id: navigationBarContents

            anchors.bottomMargin: navigationBar.bottomInset
            anchors.fill: parent
            anchors.leftMargin: navigationBar.sideInset
            anchors.rightMargin: navigationBar.sideInset
            anchors.topMargin: navigationBar.topInset

            LV.HStack {
                anchors.fill: parent
                spacing: 0

                NavigationView.NavigationPropertiesBar {
                    id: propertiesBar

                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: 20
                    compactMode: navigationBar.compactMode
                    editorViewModeViewModel: navigationBar.editorViewModeViewModel
                    navigationModeViewModel: navigationBar.navigationModeViewModel
                }
                Item {
                    Layout.fillWidth: true
                }
                Loader {
                    id: applicationBarLoader

                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: 20
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
    Component {
        id: applicationViewBarComponent

        NavigationView.NavigationApplicationViewBar {
            compactMode: navigationBar.compactMode
        }
    }
    Component {
        id: applicationEditBarComponent

        NavigationView.NavigationApplicationEditBar {
            compactMode: navigationBar.compactMode
        }
    }
    Component {
        id: applicationControlBarComponent

        NavigationView.NavigationApplicationControlBar {
            compactMode: navigationBar.compactMode
        }
    }
}
