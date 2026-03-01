import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import LVRS 1.0 as LV
import "view/panels" as BodyPanelView

LV.ApplicationWindow {
    id: applicationWindow

    readonly property string activeMainLayout: useMobileMainLayout ? "mobile" : "desktop"
    readonly property bool adaptiveCompactMode: runtimeMobilePlatform || mediaCompactMode || mobileLayoutByMediaFallback
    readonly property int adaptiveStatusBarHeight: adaptiveCompactMode ? 0 : statusBarHeight
    readonly property int baseDrawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property int baseListViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property int baseRightPanelWidth: LV.Theme.inputWidthMd - LV.Theme.gap12
    readonly property int baseSidebarWidth: minSidebarWidth
    readonly property int bodyHeight: Math.max(0, height - adaptiveStatusBarHeight - navigationBarHeight)
    readonly property color bodySplitterColor: LV.Theme.panelBackground10
    readonly property int bodySplitterThickness: Math.max(1, Math.round(LV.Theme.strokeThin))
    readonly property color canvasColor: LV.Theme.panelBackground01
    readonly property color contentPanelColor: LV.Theme.panelBackground07
    readonly property color contentsDisplayColor: LV.Theme.panelBackground09
    readonly property int desktopMinimumBodyWidth: minSidebarWidth + minListViewWidth + minContentWidth + minRightPanelWidth + bodySplitterThickness * 3
    readonly property color drawerColor: LV.Theme.panelBackground11
    readonly property int drawerHeight: Math.max(minDrawerHeight, Math.min(preferredDrawerHeight, Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness)))
    readonly property bool hideListView: false
    readonly property bool hideRightPanel: false
    property int hierarchyActiveToolbarIndex: 0
    readonly property int hierarchyHorizontalInset: LV.Theme.gap8
    readonly property int hierarchyToolbarButtonSize: LV.Theme.gap20
    readonly property int hierarchyToolbarCount: hierarchyToolbarIconNames.length
    readonly property var hierarchyToolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property int hierarchyToolbarSpacing: LV.Theme.gap2
    readonly property int hierarchyToolbarWidth: hierarchyToolbarCount > 0 ? hierarchyToolbarCount * hierarchyToolbarButtonSize + (hierarchyToolbarCount - 1) * hierarchyToolbarSpacing : hierarchyToolbarButtonSize
    readonly property color listViewColor: LV.Theme.panelBackground08
    readonly property int listViewWidth: hideListView ? 0 : Math.max(minListViewWidth, preferredListViewWidth)
    readonly property int minContentWidth: LV.Theme.dialogMaxWidth - LV.Theme.gap20 * 2
    readonly property int minDisplayHeight: LV.Theme.gap20 * 8
    readonly property int minDrawerHeight: LV.Theme.gap20 * 6
    readonly property int minListViewWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    readonly property int minRightPanelWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    readonly property int minSidebarWidth: {
        var toolbarWidth = (typeof hierarchyToolbarWidth === "number" && isFinite(hierarchyToolbarWidth)) ? hierarchyToolbarWidth : (LV.Theme.gap20 * 7 + LV.Theme.gap12);
        return Math.max(LV.Theme.gap20 * 7 + LV.Theme.gap12, toolbarWidth + hierarchyHorizontalInset * 2);
    }
    readonly property color navigationBarColor: LV.Theme.panelBackground05
    readonly property int navigationBarHeight: LV.Theme.gap24
    readonly property int onboardingDefaultHeight: LV.Theme.gap20 * 21
    readonly property int onboardingDefaultWidth: LV.Theme.gap20 * 28
    readonly property int onboardingMinHeight: LV.Theme.controlHeightMd * 10
    readonly property int onboardingMinWidth: LV.Theme.gap20 * 26
    property bool onboardingVisible: false
    property int preferredDrawerHeight: baseDrawerHeight
    property int preferredListViewWidth: baseListViewWidth
    property int preferredRightPanelWidth: baseRightPanelWidth
    property int preferredSidebarWidth: minSidebarWidth
    readonly property color rightPanelColor: LV.Theme.panelBackground08
    readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)
    readonly property color sidebarColor: LV.Theme.panelBackground04
    readonly property int sidebarWidth: Math.max(minSidebarWidth, preferredSidebarWidth)
    readonly property color statusBarColor: LV.Theme.panelBackground06
    readonly property int statusBarHeight: LV.Theme.controlHeightMd
    readonly property int windowDefaultHeight: LV.Theme.gap24 * 31 + LV.Theme.gap4
    readonly property int windowDefaultWidth: LV.Theme.controlHeightMd * 35 + LV.Theme.gap5
    readonly property int windowMinHeight: LV.Theme.gap20 * 21
    readonly property int windowMobileMinWidth: LV.Theme.controlHeightMd * 10

    function clampPreferredSizes() {
        preferredSidebarWidth = Math.max(minSidebarWidth, preferredSidebarWidth);
        preferredListViewWidth = Math.max(minListViewWidth, preferredListViewWidth);
        preferredRightPanelWidth = Math.max(minRightPanelWidth, preferredRightPanelWidth);

        var maxDrawerHeight = Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness);
        preferredDrawerHeight = Math.max(minDrawerHeight, Math.min(maxDrawerHeight, preferredDrawerHeight));
    }
    function reportLayoutBranch(source) {
        console.log("[whatson:debug][main.layout][" + source + "] os=" + runtimePlatformOs + " runtimeDesktop=" + runtimeDesktopPlatform + " runtimeMobile=" + runtimeMobilePlatform + " mediaMobile=" + mediaMobileMode + " mediaCompact=" + mediaCompactMode + " mobileByPlatform=" + mobileLayoutByPlatform + " mobileByMediaFallback=" + mobileLayoutByMediaFallback + " useMobileMainLayout=" + useMobileMainLayout + " adaptiveCompactMode=" + adaptiveCompactMode + " selectedLayout=" + activeMainLayout);
    }

    autoAttachRuntimeEvents: true
    globalEventListenersEnabled: true
    height: windowDefaultHeight
    minimumHeight: windowMinHeight
    minimumWidth: adaptiveCompactMode ? windowMobileMinWidth : desktopMinimumBodyWidth
    navItems: []
    navigationEnabled: false
    visible: true
    width: windowDefaultWidth
    windowColor: canvasColor
    windowDragHandleEnabled: false
    windowDragHandleHeight: navigationBarHeight
    windowDragHandleTopMargin: statusBarHeight

    Component.onCompleted: {
        clampPreferredSizes();
        reportLayoutBranch("completed");
        if (applicationWindow.onboardingVisible)
            onboardingSubWindow.show();
    }
    onBodyHeightChanged: clampPreferredSizes()
    onUseMobileMainLayoutChanged: reportLayoutBranch("useMobileMainLayoutChanged")

    Loader {
        id: mainLayoutLoader

        anchors.fill: parent
        sourceComponent: applicationWindow.activeMainLayout === "mobile" ? mobileMainLayoutComponent : desktopMainLayoutComponent

        onSourceComponentChanged: applicationWindow.reportLayoutBranch("loaderSourceChanged")
    }
    Component {
        id: desktopMainLayoutComponent

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
                spacing: LV.Theme.gapNone

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
                    activeToolbarIndex: applicationWindow.hierarchyActiveToolbarIndex
                    bookmarksViewModel: (typeof bookmarksHierarchyViewModel !== "undefined") ? bookmarksHierarchyViewModel : null
                    compactCanvasColor: applicationWindow.canvasColor
                    compactMode: applicationWindow.adaptiveCompactMode
                    contentPanelColor: applicationWindow.contentPanelColor
                    contentsDisplayColor: applicationWindow.contentsDisplayColor
                    drawerColor: applicationWindow.drawerColor
                    drawerHeight: applicationWindow.drawerHeight
                    eventViewModel: (typeof eventHierarchyViewModel !== "undefined") ? eventHierarchyViewModel : null
                    libraryViewModel: (typeof libraryHierarchyViewModel !== "undefined") ? libraryHierarchyViewModel : null
                    listViewColor: applicationWindow.listViewColor
                    listViewWidth: applicationWindow.listViewWidth
                    minContentWidth: applicationWindow.minContentWidth
                    minDisplayHeight: applicationWindow.minDisplayHeight
                    minDrawerHeight: applicationWindow.minDrawerHeight
                    minListViewWidth: applicationWindow.minListViewWidth
                    minRightPanelWidth: applicationWindow.minRightPanelWidth
                    minSidebarWidth: applicationWindow.minSidebarWidth
                    presetViewModel: (typeof presetHierarchyViewModel !== "undefined") ? presetHierarchyViewModel : null
                    progressViewModel: (typeof progressHierarchyViewModel !== "undefined") ? progressHierarchyViewModel : null
                    projectsViewModel: (typeof projectsHierarchyViewModel !== "undefined") ? projectsHierarchyViewModel : null
                    resourcesViewModel: (typeof resourcesHierarchyViewModel !== "undefined") ? resourcesHierarchyViewModel : null
                    rightPanelColor: applicationWindow.rightPanelColor
                    rightPanelWidth: applicationWindow.rightPanelWidth
                    sidebarColor: applicationWindow.sidebarColor
                    sidebarWidth: applicationWindow.sidebarWidth
                    splitterColor: applicationWindow.bodySplitterColor
                    splitterThickness: applicationWindow.bodySplitterThickness
                    tagsViewModel: (typeof tagsHierarchyViewModel !== "undefined") ? tagsHierarchyViewModel : null

                    onActiveToolbarIndexChangeRequested: function (index) {
                        if (index >= 0 && index !== applicationWindow.hierarchyActiveToolbarIndex)
                            applicationWindow.hierarchyActiveToolbarIndex = index;
                    }
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
    }
    Component {
        id: mobileMainLayoutComponent

        BodyPanelView.MobileNormalLayout {
            anchors.fill: parent
            canvasColor: applicationWindow.canvasColor
            controlSurfaceColor: LV.Theme.panelBackground10
            statusPlaceholderText: "Placeholder"
        }
    }
    Window {
        id: onboardingSubWindow

        color: applicationWindow.windowColor
        flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
        height: applicationWindow.onboardingDefaultHeight
        minimumHeight: applicationWindow.onboardingMinHeight
        minimumWidth: applicationWindow.onboardingMinWidth
        modality: Qt.ApplicationModal
        title: "WhatSon Onboarding"
        transientParent: applicationWindow
        visible: false
        width: applicationWindow.onboardingDefaultWidth
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
