import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "view/top" as TopPanelView
import "view/body" as BodyPanelView

LV.ApplicationWindow {
    id: window

    readonly property int baseDrawerHeight: 255
    readonly property int baseListViewWidth: 198
    readonly property int baseRightPanelWidth: 194
    readonly property int baseSidebarWidth: 216
    readonly property int bodyHeight: Math.max(0, height - statusBarHeight - navigationBarHeight)
    readonly property color bodySplitterColor: "#445066"
    readonly property int bodySplitterThickness: 1
    readonly property color canvasColor: "#1b1b1c"
    readonly property color contentPanelColor: "#39445b"
    readonly property color contentsDisplayColor: "#495473"
    readonly property color drawerColor: "#665d47"
    readonly property int drawerHeight: Math.max(minDrawerHeight, Math.min(preferredDrawerHeight, Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness)))
    readonly property bool hideListView: false
    readonly property bool hideRightPanel: false
    readonly property int hierarchyHorizontalInset: LV.Theme.gap8
    readonly property int hierarchyToolbarButtonSize: LV.Theme.gap20
    readonly property int hierarchyToolbarCount: hierarchyToolbarIconNames.length
    readonly property var hierarchyToolbarIconNames: (typeof sidebarHierarchyStore !== "undefined" && sidebarHierarchyStore && sidebarHierarchyStore.toolbarIconNames) ? sidebarHierarchyStore.toolbarIconNames : []
    readonly property int hierarchyToolbarSpacing: LV.Theme.gap2
    readonly property int hierarchyToolbarWidth: hierarchyToolbarCount > 0 ? hierarchyToolbarCount * hierarchyToolbarButtonSize + (hierarchyToolbarCount - 1) * hierarchyToolbarSpacing : hierarchyToolbarButtonSize
    readonly property color listViewColor: "#3a5c57"
    readonly property int listViewWidth: hideListView ? 0 : Math.max(minListViewWidth, preferredListViewWidth)
    readonly property int minContentWidth: 320
    readonly property int minDisplayHeight: 160
    readonly property int minDrawerHeight: 120
    readonly property int minListViewWidth: 132
    readonly property int minRightPanelWidth: 132
    readonly property int minSidebarWidth: Math.max(152, hierarchyToolbarWidth + hierarchyHorizontalInset * 2)
    readonly property color navigationBarColor: "#303743"
    readonly property int navigationBarHeight: 36
    property int preferredDrawerHeight: baseDrawerHeight
    property int preferredListViewWidth: baseListViewWidth
    property int preferredRightPanelWidth: baseRightPanelWidth
    property int preferredSidebarWidth: baseSidebarWidth
    readonly property color rightPanelColor: "#63556a"
    readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)
    readonly property color sidebarColor: "#3b4b63"
    readonly property int sidebarWidth: Math.max(minSidebarWidth, preferredSidebarWidth)
    readonly property color statusBarColor: "#2a3038"
    readonly property int statusBarHeight: 36

    function clampPreferredSizes() {
        preferredSidebarWidth = Math.max(minSidebarWidth, preferredSidebarWidth);
        preferredListViewWidth = Math.max(minListViewWidth, preferredListViewWidth);
        preferredRightPanelWidth = Math.max(minRightPanelWidth, preferredRightPanelWidth);

        var maxDrawerHeight = Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness);
        preferredDrawerHeight = Math.max(minDrawerHeight, Math.min(maxDrawerHeight, preferredDrawerHeight));
    }

    autoAttachRuntimeEvents: false
    height: 748
    minimumHeight: 420
    minimumWidth: isMobilePlatform ? 360 : desktopMinimumBodyWidth
    navItems: []
    navigationEnabled: false
    visible: true
    width: 1265
    windowColor: "#141414"
    windowDragHandleEnabled: !isMobilePlatform
    windowDragHandleHeight: statusBarHeight + navigationBarHeight
    windowDragHandleTopMargin: 0

    Component.onCompleted: clampPreferredSizes()
    onBodyHeightChanged: clampPreferredSizes()

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

            TopPanelView.StatusBarLayout {
                panelColor: window.statusBarColor
                panelHeight: window.statusBarHeight
            }
            TopPanelView.NavigationBarLayout {
                panelColor: window.navigationBarColor
                panelHeight: window.navigationBarHeight
            }
            Loader {
                Layout.fillHeight: true
                Layout.fillWidth: true
                sourceComponent: window.isMobilePlatform ? mobileBodyComponent : desktopBodyComponent
            }
        }
    }
    Component {
        id: mobileBodyComponent

        BodyPanelView.ContentViewLayout {
            displayColor: window.contentsDisplayColor
            drawerColor: window.contentsDisplayColor
            drawerHeight: 0
            minDisplayHeight: 0
            minDrawerHeight: 0
            panelColor: window.contentPanelColor
            splitterHandleThickness: 0
            splitterThickness: 0
        }
    }
    Component {
        id: desktopBodyComponent

        BodyPanelView.BodyLayout {
            contentPanelColor: window.contentPanelColor
            contentsDisplayColor: window.contentsDisplayColor
            drawerColor: window.drawerColor
            drawerHeight: window.drawerHeight
            listViewColor: window.listViewColor
            listViewWidth: window.listViewWidth
            minContentWidth: window.minContentWidth
            minDisplayHeight: window.minDisplayHeight
            minDrawerHeight: window.minDrawerHeight
            minListViewWidth: window.minListViewWidth
            minRightPanelWidth: window.minRightPanelWidth
            minSidebarWidth: window.minSidebarWidth
            rightPanelColor: window.rightPanelColor
            rightPanelWidth: window.rightPanelWidth
            sidebarColor: window.sidebarColor
            sidebarWidth: window.sidebarWidth
            splitterColor: window.bodySplitterColor
            splitterThickness: window.bodySplitterThickness

            onDrawerHeightDragRequested: function (value) {
                if (value !== window.preferredDrawerHeight)
                    window.preferredDrawerHeight = value;
            }
            onListViewWidthDragRequested: function (value) {
                if (value !== window.preferredListViewWidth)
                    window.preferredListViewWidth = value;
            }
            onRightPanelWidthDragRequested: function (value) {
                if (value !== window.preferredRightPanelWidth)
                    window.preferredRightPanelWidth = value;
            }
            onSidebarWidthDragRequested: function (value) {
                if (value !== window.preferredSidebarWidth)
                    window.preferredSidebarWidth = value;
            }
        }
    }
}
