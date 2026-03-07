pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQml
import QtQuick.Window
import LVRS 1.0 as LV
import "view/panels" as BodyPanelView

LV.ApplicationWindow {
    id: applicationWindow

    // Platform branch is OS-driven (LVRS runtime platform info), not size/adaptive mode.
    readonly property string activeMainLayout: isMobilePlatform ? "mobile" : "desktop"
    readonly property int adaptiveStatusBarHeight: activeMainLayout === "mobile" ? 0 : statusBarHeight
    readonly property int baseDrawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property int baseListViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property int baseRightPanelWidth: 194
    readonly property int baseSidebarWidth: hierarchyToolbarWidth
    readonly property int bodyHeight: Math.max(0, height - adaptiveStatusBarHeight - navigationBarHeight)
    readonly property color bodySplitterColor: LV.Theme.panelBackground10
    readonly property int bodySplitterThickness: Math.max(1, Math.round(LV.Theme.strokeThin))
    readonly property var bookmarksHierarchyVm: bookmarksHierarchyViewModel
    readonly property color canvasColor: LV.Theme.panelBackground01
    readonly property color contentPanelColor: LV.Theme.panelBackground07
    readonly property color contentsDisplayColor: LV.Theme.panelBackground09
    readonly property int desktopMinimumBodyWidth: minSidebarWidth + minListViewWidth + minContentWidth + minRightPanelWidth + bodySplitterThickness * 3
    readonly property color drawerColor: LV.Theme.panelBackground11
    readonly property int drawerHeight: Math.max(minDrawerHeight, Math.min(preferredDrawerHeight, Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness)))
    readonly property var editorViewModeVm: editorViewModeViewModel
    readonly property var eventHierarchyVm: eventHierarchyViewModel
    readonly property bool hideListView: false
    readonly property bool hideRightPanel: false
    readonly property int hierarchyHorizontalInset: 2
    readonly property int hierarchyToolbarButtonSize: LV.Theme.gap20
    readonly property int hierarchyToolbarCount: hierarchyToolbarIconNames.length
    readonly property var hierarchyToolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property int hierarchyToolbarSpacing: LV.Theme.gap2
    readonly property int hierarchyToolbarTrackWidth: hierarchyToolbarCount > 0 ? hierarchyToolbarCount * hierarchyToolbarButtonSize + (hierarchyToolbarCount - 1) * hierarchyToolbarSpacing : hierarchyToolbarButtonSize
    readonly property int hierarchyToolbarWidth: hierarchyToolbarTrackWidth + hierarchyHorizontalInset * 2
    // Fail-fast binding contract: these context properties must exist from main.cpp.
    readonly property var libraryHierarchyVm: libraryHierarchyViewModel
    readonly property color listViewColor: LV.Theme.panelBackground08
    readonly property int listViewWidth: hideListView ? 0 : Math.max(minListViewWidth, preferredListViewWidth)
    readonly property int minContentWidth: LV.Theme.dialogMaxWidth - LV.Theme.gap20 * 2
    readonly property int minDisplayHeight: LV.Theme.gap20 * 8
    readonly property int minDrawerHeight: LV.Theme.gap20 * 6
    readonly property int minListViewWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    readonly property int minRightPanelWidth: 145
    readonly property int minSidebarWidth: {
        var toolbarWidth = (typeof hierarchyToolbarWidth === "number" && isFinite(hierarchyToolbarWidth)) ? hierarchyToolbarWidth : (LV.Theme.gap20 * 7 + LV.Theme.gap12);
        return toolbarWidth;
    }
    readonly property color navigationBarColor: LV.Theme.panelBackground06
    readonly property int navigationBarHeight: LV.Theme.gap24
    readonly property var navigationModeVm: navigationModeViewModel
    readonly property int onboardingDefaultHeight: LV.Theme.gap20 * 21
    readonly property int onboardingDefaultWidth: LV.Theme.gap20 * 28
    readonly property int onboardingMinHeight: LV.Theme.controlHeightMd * 10
    readonly property int onboardingMinWidth: LV.Theme.gap20 * 26
    property bool onboardingVisible: false
    property int preferredDrawerHeight: baseDrawerHeight
    property int preferredListViewWidth: baseListViewWidth
    property int preferredRightPanelWidth: baseRightPanelWidth
    property int preferredSidebarWidth: baseSidebarWidth
    readonly property var presetHierarchyVm: presetHierarchyViewModel
    readonly property var progressHierarchyVm: progressHierarchyViewModel
    readonly property var projectsHierarchyVm: projectsHierarchyViewModel
    property bool resizeDrWasSuspended: false
    property bool resizeInProgress: false
    property int resizeRenderGuardDebounceMs: 220
    property bool resizeRenderGuardEnabled: true
    readonly property var resourcesHierarchyVm: resourcesHierarchyViewModel
    readonly property color rightPanelColor: LV.Theme.panelBackground06
    readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)
    readonly property color sidebarColor: LV.Theme.panelBackground04
    readonly property var sidebarHierarchyVm: sidebarHierarchyViewModel
    readonly property int sidebarWidth: Math.max(minSidebarWidth, preferredSidebarWidth)
    readonly property color statusBarColor: LV.Theme.panelBackground06
    readonly property int statusBarHeight: LV.Theme.controlHeightMd
    readonly property var tagsHierarchyVm: tagsHierarchyViewModel
    readonly property int windowDefaultHeight: LV.Theme.gap24 * 31 + LV.Theme.gap4
    readonly property int windowDefaultWidth: LV.Theme.controlHeightMd * 35 + LV.Theme.gap5
    readonly property int windowMinHeight: LV.Theme.gap20 * 21
    readonly property int windowMobileMinWidth: LV.Theme.controlHeightMd * 10

    signal viewHookRequested

    function applyRenderQualityPolicy(source) {
        if (!isDesktopPlatform && !isMobilePlatform)
            return;

        console.log("[whatson:debug][render.policy][" + source + "] platform=" + platform + " action=resizeSuspendResumeGuard dynamicResolutionEnabled=" + LV.RenderQuality.dynamicResolutionEnabled);
    }
    function clampPreferredSizes() {
        preferredSidebarWidth = Math.max(minSidebarWidth, preferredSidebarWidth);
        preferredListViewWidth = Math.max(minListViewWidth, preferredListViewWidth);
        preferredRightPanelWidth = Math.max(minRightPanelWidth, preferredRightPanelWidth);

        var maxDrawerHeight = Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness);
        preferredDrawerHeight = Math.max(minDrawerHeight, Math.min(maxDrawerHeight, preferredDrawerHeight));
    }
    function cycleNavigationModeFromShortcut() {
        if (applicationWindow.hasFocusedTextInput())
            return;
        if (applicationWindow.navigationModeVm && applicationWindow.navigationModeVm.requestNextMode !== undefined)
            applicationWindow.navigationModeVm.requestNextMode();
    }
    function finalizeResizeRenderQualityPolicy() {
        if (!resizeRenderGuardEnabled || (!isMobilePlatform && !isDesktopPlatform))
            return;

        resizeInProgress = false;
        if (!resizeDrWasSuspended) {
            console.log("[whatson:debug][render.policy][resizeEnd] platform=" + platform + " action=resumeSkipped");
            return;
        }

        // Ensure max supersample scale is restored before re-enabling dynamic resolution.
        LV.RenderQuality.dynamicResolutionEnabled = false;
        LV.RenderQuality.dynamicResolutionEnabled = true;
        resizeDrWasSuspended = false;
        console.log("[whatson:debug][render.policy][resizeEnd] platform=" + platform + " action=resumeWithMaxScaleReset");
    }
    function handleResizeForRenderQuality(source) {
        if (!resizeRenderGuardEnabled || (!isMobilePlatform && !isDesktopPlatform))
            return;

        if (!resizeInProgress) {
            resizeInProgress = true;
            resizeDrWasSuspended = LV.RenderQuality.dynamicResolutionEnabled;

            if (resizeDrWasSuspended) {
                LV.RenderQuality.dynamicResolutionEnabled = false;
                console.log("[whatson:debug][render.policy][" + source + "] platform=" + platform + " action=resizeBegin suspendDynamicResolution=true");
            } else {
                console.log("[whatson:debug][render.policy][" + source + "] platform=" + platform + " action=resizeBegin suspendDynamicResolution=false");
            }
        }

        resizeDebounceTimer.restart();
    }
    function hasFocusedTextInput() {
        let current = applicationWindow.activeFocusItem;
        while (current) {
            const isTextEditingItem = current.text !== undefined && current.cursorPosition !== undefined && current.selectedText !== undefined;
            if (isTextEditingItem)
                return true;
            current = current.parent;
        }
        return false;
    }
    function reportLayoutBranch(source) {
        console.log("[whatson:debug][main.layout][" + source + "] platform=" + platform + " isDesktopPlatform=" + isDesktopPlatform + " isMobilePlatform=" + isMobilePlatform + " selectedLayout=" + activeMainLayout);
    }

    autoAttachRuntimeEvents: true
    globalEventListenersEnabled: true
    height: windowDefaultHeight
    minimumHeight: windowMinHeight
    minimumWidth: activeMainLayout === "mobile" ? windowMobileMinWidth : desktopMinimumBodyWidth
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
        applyRenderQualityPolicy("completed");
        reportLayoutBranch("completed");
        if (applicationWindow.onboardingVisible)
            onboardingSubWindow.show();
    }
    onBodyHeightChanged: clampPreferredSizes()
    onHeightChanged: handleResizeForRenderQuality("heightChanged")
    onWidthChanged: handleResizeForRenderQuality("widthChanged")

    Timer {
        id: resizeDebounceTimer

        interval: applicationWindow.resizeRenderGuardDebounceMs
        repeat: false

        onTriggered: applicationWindow.finalizeResizeRenderQualityPolicy()
    }
    Shortcut {
        autoRepeat: false
        context: Qt.ApplicationShortcut
        enabled: !applicationWindow.hasFocusedTextInput()
        sequence: "Tab"

        onActivated: applicationWindow.cycleNavigationModeFromShortcut()
    }
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
                    visible: applicationWindow.activeMainLayout !== "mobile"

                    onWindowMoveRequested: {
                        applicationWindow.requestWindowMove();
                    }
                }
                BodyPanelView.NavigationBarLayout {
                    id: navigationBar

                    compactMode: applicationWindow.activeMainLayout === "mobile"
                    editorViewModeViewModel: applicationWindow.editorViewModeVm
                    navigationModeViewModel: applicationWindow.navigationModeVm
                    panelColor: applicationWindow.navigationBarColor
                    panelHeight: applicationWindow.navigationBarHeight
                }
                BodyPanelView.BodyLayout {
                    id: hStack

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    compactCanvasColor: applicationWindow.canvasColor
                    compactMode: applicationWindow.activeMainLayout === "mobile"
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
                    sidebarHierarchyViewModel: applicationWindow.sidebarHierarchyVm
                    sidebarHorizontalInset: applicationWindow.hierarchyHorizontalInset
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
                    visible: applicationWindow.activeMainLayout === "mobile"
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
            statusPlaceholderText: ""
        }
    }
    Onboarding {
        id: onboardingSubWindow

        defaultHeight: applicationWindow.onboardingDefaultHeight
        defaultWidth: applicationWindow.onboardingDefaultWidth
        hostWindow: applicationWindow
        minHeight: applicationWindow.onboardingMinHeight
        minWidth: applicationWindow.onboardingMinWidth
        panelColor: applicationWindow.windowColor

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
