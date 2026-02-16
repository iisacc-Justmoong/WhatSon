import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "components/layoutshell" as LayoutShell

LV.ApplicationWindow {
    id: window

    readonly property int baseDrawerHeight: 255
    readonly property int baseListViewWidth: 198
    readonly property int baseRightPanelWidth: 194
    readonly property int baseSidebarWidth: 216
    readonly property int bodyHeight: Math.max(0, height - statusBarHeight - navigationBarHeight)
    readonly property color canvasColor: "#1b1b1c"
    readonly property bool compactSidebar: width < 820
    readonly property color contentPanelColor: "#39445b"
    readonly property color contentsDisplayColor: "#495473"
    readonly property color drawerColor: "#665d47"
    readonly property int drawerHeight: Math.min(baseDrawerHeight, Math.max(120, bodyHeight))
    readonly property bool hideListView: width < 980
    readonly property bool hideRightPanel: width < 1180
    readonly property color listViewColor: "#3a5c57"
    readonly property int listViewWidth: hideListView ? 0 : baseListViewWidth
    readonly property color navigationBarColor: "#303743"
    readonly property int navigationBarHeight: 24
    readonly property color rightPanelColor: "#63556a"
    readonly property int rightPanelWidth: hideRightPanel ? 0 : baseRightPanelWidth
    readonly property color sidebarColor: "#3b4b63"
    readonly property int sidebarWidth: compactSidebar ? 120 : baseSidebarWidth
    readonly property color statusBarColor: "#2a3038"
    readonly property int statusBarHeight: 24

    autoAttachRuntimeEvents: false
    flags: Qt.Window | Qt.FramelessWindowHint
    height: 748
    minimumHeight: 420
    minimumWidth: 640
    navItems: []
    navigationEnabled: false
    visible: true
    width: 1265
    windowColor: "#141414"
    windowDragHandleEnabled: true
    windowDragHandleHeight: statusBarHeight + navigationBarHeight
    windowDragHandleTopMargin: 0

    Item {
        anchors.fill: parent
        clip: true

        Rectangle {
            anchors.fill: parent
            color: window.canvasColor
        }
        LV.VStack {
            anchors.fill: parent
            spacing: 0

            LayoutShell.StatusBarLayout {
                panelColor: window.statusBarColor
            }
            LayoutShell.NavigationBarLayout {
                panelColor: window.navigationBarColor
            }
            LayoutShell.BodyLayout {
                contentPanelColor: window.contentPanelColor
                contentsDisplayColor: window.contentsDisplayColor
                drawerColor: window.drawerColor
                drawerHeight: window.drawerHeight
                listViewColor: window.listViewColor
                listViewWidth: window.listViewWidth
                rightPanelColor: window.rightPanelColor
                rightPanelWidth: window.rightPanelWidth
                sidebarColor: window.sidebarColor
                sidebarWidth: window.sidebarWidth
            }
        }
    }
}
