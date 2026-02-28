import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "navigation" as NavigationView

Rectangle {
    id: navigationBar

    readonly property int bottomInset: 2
    readonly property int compactHorizontalInset: Math.max(12, Math.min(24, Math.round(width * 0.04)))
    property bool compactMode: false
    property color compactSurfaceColor: LV.Theme.panelBackground10
    readonly property int compactTopInset: compactHorizontalInset
    readonly property int effectivePanelHeight: compactMode ? (compactTopInset + panelHeight) : panelHeight
    property color panelColor: LV.Theme.panelBackground05
    property int panelHeight: 24
    readonly property int sideInset: compactMode ? 8 : 4
    readonly property int topInset: 2

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

                NavigationView.NavigationInformationBar {
                    id: informationBar

                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: 20
                    compactMode: navigationBar.compactMode
                }
                Item {
                    Layout.fillWidth: true
                }
                NavigationView.NavigationApplicationContentsBar {
                    id: applicationContentsBar

                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: 20
                    compactMode: navigationBar.compactMode
                }
            }
        }
    }
}
