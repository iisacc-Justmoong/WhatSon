import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import LVRS 1.0 as LV
import "view/panels" as BodyPanelView

LV.ApplicationWindow {
    id: applicationWindow

    readonly property bool adaptiveCompactMode: matchesMedia("mobile") || matchesMedia("compact")
    readonly property int adaptiveStatusBarHeight: adaptiveCompactMode ? 0 : statusBarHeight
    readonly property int baseDrawerHeight: 255
    readonly property int baseListViewWidth: 198
    readonly property int baseRightPanelWidth: 194
    readonly property int baseSidebarWidth: minSidebarWidth
    readonly property int bodyHeight: Math.max(0, height - adaptiveStatusBarHeight - navigationBarHeight)
    readonly property color bodySplitterColor: "#445066"
    readonly property int bodySplitterThickness: 1
    readonly property color canvasColor: "#1b1b1c"
    readonly property color contentPanelColor: "#39445b"
    readonly property color contentsDisplayColor: "#495473"
    readonly property int desktopMinimumBodyWidth: minSidebarWidth + minListViewWidth + minContentWidth + minRightPanelWidth + bodySplitterThickness * 3
    readonly property color drawerColor: "#665d47"
    readonly property int drawerHeight: Math.max(minDrawerHeight, Math.min(preferredDrawerHeight, Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness)))
    readonly property bool hideListView: false
    readonly property bool hideRightPanel: false
    readonly property int hierarchyHorizontalInset: (typeof LV.Theme.gap8 === "number" && isFinite(LV.Theme.gap8)) ? LV.Theme.gap8 : 8
    readonly property int hierarchyToolbarButtonSize: (typeof LV.Theme.gap20 === "number" && isFinite(LV.Theme.gap20)) ? LV.Theme.gap20 : 20
    readonly property int hierarchyToolbarCount: hierarchyToolbarIconNames.length
    readonly property var hierarchyToolbarIconNames: (typeof sidebarHierarchyStore !== "undefined" && sidebarHierarchyStore && sidebarHierarchyStore.toolbarIconNames) ? sidebarHierarchyStore.toolbarIconNames : []
    readonly property int hierarchyToolbarSpacing: (typeof LV.Theme.gap2 === "number" && isFinite(LV.Theme.gap2)) ? LV.Theme.gap2 : 2
    readonly property int hierarchyToolbarWidth: hierarchyToolbarCount > 0 ? hierarchyToolbarCount * hierarchyToolbarButtonSize + (hierarchyToolbarCount - 1) * hierarchyToolbarSpacing : hierarchyToolbarButtonSize
    readonly property color listViewColor: "#3a5c57"
    readonly property int listViewWidth: hideListView ? 0 : Math.max(minListViewWidth, preferredListViewWidth)
    readonly property int minContentWidth: 320
    readonly property int minDisplayHeight: 160
    readonly property int minDrawerHeight: 120
    readonly property int minListViewWidth: 132
    readonly property int minRightPanelWidth: 132
    readonly property int minSidebarWidth: {
        var toolbarWidth = (typeof hierarchyToolbarWidth === "number" && isFinite(hierarchyToolbarWidth)) ? hierarchyToolbarWidth : 152;
        return Math.max(152, toolbarWidth + hierarchyHorizontalInset * 2);
    }
    readonly property color navigationBarColor: LV.Theme.panelBackground05
    readonly property int navigationBarHeight: 24
    property bool onboardingVisible: false
    property int preferredDrawerHeight: baseDrawerHeight
    property int preferredListViewWidth: baseListViewWidth
    property int preferredRightPanelWidth: baseRightPanelWidth
    property int preferredSidebarWidth: minSidebarWidth
    readonly property color rightPanelColor: "#63556a"
    readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)
    readonly property color sidebarColor: "#3b4b63"
    readonly property int sidebarWidth: Math.max(minSidebarWidth, preferredSidebarWidth)
    readonly property color statusBarColor: "#262728"
    readonly property int statusBarHeight: 36

    function clampPreferredSizes() {
        preferredSidebarWidth = Math.max(minSidebarWidth, preferredSidebarWidth);
        preferredListViewWidth = Math.max(minListViewWidth, preferredListViewWidth);
        preferredRightPanelWidth = Math.max(minRightPanelWidth, preferredRightPanelWidth);

        var maxDrawerHeight = Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness);
        preferredDrawerHeight = Math.max(minDrawerHeight, Math.min(maxDrawerHeight, preferredDrawerHeight));
    }

    autoAttachRuntimeEvents: false
    globalEventListenersEnabled: true
    height: 748
    minimumHeight: 420
    minimumWidth: adaptiveCompactMode ? 360 : desktopMinimumBodyWidth
    navItems: []
    navigationEnabled: false
    visible: true
    width: 1265
    windowColor: canvasColor
    windowDragHandleEnabled: false
    windowDragHandleHeight: navigationBarHeight
    windowDragHandleTopMargin: statusBarHeight

    Component.onCompleted: {
        clampPreferredSizes();
        if (applicationWindow.onboardingVisible)
            onboardingSubWindow.show();
    }
    onBodyHeightChanged: clampPreferredSizes()

    Item {
        anchors.fill: parent
        clip: true

        Rectangle {
            anchors.fill: parent
            color: applicationWindow.canvasColor
        }
        LV.VStack {
            id: vStack

            anchors.fill: parent
            spacing: 0

            BodyPanelView.StatusBarLayout {
                id: statusBar

                Layout.preferredHeight: visible ? panelHeight : 0
                compactMode: false
                panelColor: applicationWindow.statusBarColor
                panelHeight: applicationWindow.statusBarHeight
                visible: !applicationWindow.adaptiveCompactMode

                onWindowMoveRequested: {
                    applicationWindow.requestWindowMove();
                }
            }
            BodyPanelView.NavigationBarLayout {
                id: navigationBar

                compactMode: applicationWindow.adaptiveCompactMode
                panelColor: applicationWindow.navigationBarColor
                panelHeight: applicationWindow.navigationBarHeight
            }
            BodyPanelView.BodyLayout {
                id: hStack

                Layout.fillHeight: true
                Layout.fillWidth: true
                compactCanvasColor: applicationWindow.canvasColor
                compactMode: applicationWindow.adaptiveCompactMode
                contentPanelColor: applicationWindow.contentPanelColor
                contentsDisplayColor: applicationWindow.contentsDisplayColor
                drawerColor: applicationWindow.drawerColor
                drawerHeight: applicationWindow.drawerHeight
                listViewColor: applicationWindow.listViewColor
                listViewWidth: applicationWindow.listViewWidth
                minContentWidth: applicationWindow.minContentWidth
                minDisplayHeight: applicationWindow.minDisplayHeight
                minDrawerHeight: applicationWindow.minDrawerHeight
                minListViewWidth: applicationWindow.minListViewWidth
                minRightPanelWidth: applicationWindow.minRightPanelWidth
                minSidebarWidth: applicationWindow.minSidebarWidth
                rightPanelColor: applicationWindow.rightPanelColor
                rightPanelWidth: applicationWindow.rightPanelWidth
                sidebarColor: applicationWindow.sidebarColor
                sidebarWidth: applicationWindow.sidebarWidth
                splitterColor: applicationWindow.bodySplitterColor
                splitterThickness: applicationWindow.bodySplitterThickness

                onDrawerHeightDragRequested: function (value) {
                    if (value !== applicationWindow.preferredDrawerHeight)
                        applicationWindow.preferredDrawerHeight = value;
                }
                onListViewWidthDragRequested: function (value) {
                    if (value !== applicationWindow.preferredListViewWidth)
                        applicationWindow.preferredListViewWidth = value;
                }
                onRightPanelWidthDragRequested: function (value) {
                    if (value !== applicationWindow.preferredRightPanelWidth)
                        applicationWindow.preferredRightPanelWidth = value;
                }
                onSidebarWidthDragRequested: function (value) {
                    if (value !== applicationWindow.preferredSidebarWidth)
                        applicationWindow.preferredSidebarWidth = value;
                }
            }
            BodyPanelView.StatusBarLayout {
                id: mobileStatusBar

                Layout.preferredHeight: visible ? effectivePanelHeight : 0
                compactMode: true
                panelColor: applicationWindow.statusBarColor
                panelHeight: applicationWindow.statusBarHeight
                visible: applicationWindow.adaptiveCompactMode
            }
        }
    }
    Window {
        id: onboardingSubWindow

        color: applicationWindow.windowColor
        flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
        height: 420
        minimumHeight: 360
        minimumWidth: 520
        modality: Qt.ApplicationModal
        title: "WhatSon Onboarding"
        transientParent: applicationWindow
        visible: false
        width: 560
        x: applicationWindow.x + Math.round((applicationWindow.width - width) / 2)
        y: applicationWindow.y + Math.round((applicationWindow.height - height) / 2)

        onVisibleChanged: {
            if (visible) {
                x = applicationWindow.x + Math.round((applicationWindow.width - width) / 2);
                y = applicationWindow.y + Math.round((applicationWindow.height - height) / 2);
            }
        }

        Onboarding {
            anchors.fill: parent

            onCreateFileRequested: {
                applicationWindow.onboardingVisible = false;
                onboardingSubWindow.close();
            }
            onSelectFileRequested: {
                applicationWindow.onboardingVisible = false;
                onboardingSubWindow.close();
            }
        }
    }
}
