pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQml
import QtQuick.Window
import LVRS 1.0 as LV
import "view/mobile/pages" as MobilePageView
import "view/panels" as BodyPanelView
import "window" as WindowView

LV.ApplicationWindow {
    id: applicationWindow

    readonly property int adaptiveStatusBarHeight: adaptiveMobileLayout ? 0 : statusBarHeight
    readonly property int baseListViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property int baseRightPanelWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(194)))
    readonly property int baseSidebarWidth: hierarchyToolbarWidth
    readonly property int bodyHeight: Math.max(0, height - adaptiveStatusBarHeight - navigationBarHeight)
    readonly property color bodySplitterColor: LV.Theme.panelBackground10
    readonly property int bodySplitterThickness: Math.max(1, Math.round(LV.Theme.strokeThin))
    readonly property color canvasColor: LV.Theme.panelBackground01
    readonly property color desktopPanelSurfaceColor: "transparent"
    property bool desktopOnboardingWindowVisible: false
    readonly property int desktopMinimumBodyWidth: (hideSidebar ? 0 : minSidebarWidth)
                                               + (hideListView ? 0 : minListViewWidth)
                                               + minContentWidth
                                               + (hideRightPanel ? 0 : minRightPanelWidth)
                                               + bodySplitterThickness * Math.max(0, ((hideSidebar ? 0 : 1)
                                                                                      + (hideListView ? 0 : 1)
                                                                                      + 1
                                                                                      + (hideRightPanel ? 0 : 1)) - 1)
    readonly property bool hideListView: false
    property bool hideRightPanel: false
    property bool hideSidebar: false
    readonly property int hierarchyHorizontalInset: LV.Theme.gap2
    readonly property int hierarchyToolbarButtonSize: LV.Theme.gap20
    readonly property int hierarchyToolbarCount: hierarchyToolbarIconNames.length
    readonly property var hierarchyToolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property real hierarchyToolbarSpacing: hierarchyToolbarCount > 1 ? LV.Theme.scaleMetric(40) / (hierarchyToolbarCount - 1) : 0
    readonly property int hierarchyToolbarTrackWidth: hierarchyToolbarCount > 0 ? Math.round(hierarchyToolbarCount * hierarchyToolbarButtonSize + (hierarchyToolbarCount - 1) * hierarchyToolbarSpacing) : hierarchyToolbarButtonSize
    readonly property int hierarchyToolbarWidth: hierarchyToolbarTrackWidth + hierarchyHorizontalInset * 2
    readonly property var registeredViewModelKeys: LV.ViewModels.keys
    readonly property string libraryNoteMutationViewId: "windowInteractions.libraryNoteMutation"
    readonly property string navigationModeViewId: "windowInteractions.navigationMode"
    readonly property string sidebarHierarchyViewId: "windowInteractions.sidebarHierarchy"
    readonly property var rootEditorViewModeViewModel: {
        const _ = applicationWindow.registeredViewModelKeys;
        return LV.ViewModels.get("editorViewModeViewModel");
    }
    readonly property int libraryHierarchyIndex: 0
    readonly property var rootLibraryHierarchyViewModel: {
        const _ = applicationWindow.registeredViewModelKeys;
        return LV.ViewModels.get("libraryHierarchyViewModel");
    }
    readonly property var rootLibraryNoteMutationViewModel: {
        const _ = applicationWindow.registeredViewModelKeys;
        return LV.ViewModels.get("libraryNoteMutationViewModel");
    }
    readonly property int listViewWidth: hideListView ? 0 : Math.max(minListViewWidth, preferredListViewWidth)
    readonly property int minContentWidth: LV.Theme.dialogMaxWidth - LV.Theme.gap20 * 2
    readonly property int minListViewWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    readonly property int minRightPanelWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(145)))
    readonly property var rootNavigationModeViewModel: {
        const _ = applicationWindow.registeredViewModelKeys;
        return LV.ViewModels.get("navigationModeViewModel");
    }
    readonly property var rootPanelViewModelRegistry: panelViewModelRegistry
    readonly property var rootSidebarHierarchyViewModel: {
        const _ = applicationWindow.registeredViewModelKeys;
        return LV.ViewModels.get("sidebarHierarchyViewModel");
    }
    readonly property var rootAgendaViewModel: typeof agendaViewModel !== "undefined" ? agendaViewModel : null
    readonly property var rootDayCalendarViewModel: typeof dayCalendarViewModel !== "undefined" ? dayCalendarViewModel : null
    readonly property var rootMonthCalendarViewModel: typeof monthCalendarViewModel !== "undefined" ? monthCalendarViewModel : null
    readonly property var rootWeekCalendarViewModel: typeof weekCalendarViewModel !== "undefined" ? weekCalendarViewModel : null
    readonly property var rootYearCalendarViewModel: typeof yearCalendarViewModel !== "undefined" ? yearCalendarViewModel : null
    readonly property int minSidebarWidth: {
        var toolbarWidth = (typeof hierarchyToolbarWidth === "number" && isFinite(hierarchyToolbarWidth)) ? hierarchyToolbarWidth : (LV.Theme.gap20 * 7 + LV.Theme.gap12);
        return toolbarWidth;
    }
    readonly property color mobileSafeAreaBackdropColor: canvasColor
    readonly property int navigationBarHeight: LV.Theme.gap24
    readonly property var onboardingRoute: ({
            path: applicationWindow.onboardingRoutePath,
            component: onboardingPageComponent
        })
    readonly property string onboardingRoutePath: "/onboarding"
    property var onboardingHubController: null
    property var onboardingRouteBootstrapController: null
    readonly property int onboardingMinHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(420)))
    readonly property int onboardingMinWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(620)))
    property int preferredListViewWidth: baseListViewWidth
    property int preferredRightPanelWidth: baseRightPanelWidth
    property int preferredSidebarWidth: baseSidebarWidth
    readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)
    readonly property int sidebarWidth: hideSidebar ? 0 : Math.max(minSidebarWidth, preferredSidebarWidth)
    readonly property int statusBarHeight: LV.Theme.controlHeightMd
    readonly property bool onboardingRouteCommitPending: onboardingRouteBootstrapController ? onboardingRouteBootstrapController.routeCommitPending : false
    readonly property string startupRoutePath: onboardingRouteBootstrapController ? onboardingRouteBootstrapController.startupRoutePath : workspaceRoutePath
    readonly property string expectedEmbeddedRoutePath: {
        if (!applicationWindow.useEmbeddedOnboardingRoute)
            return applicationWindow.workspaceRoutePath;
        if (applicationWindow.onboardingRouteBootstrapController
                && applicationWindow.onboardingRouteBootstrapController.embeddedOnboardingVisible)
            return applicationWindow.onboardingRoutePath;
        return applicationWindow.workspaceRoutePath;
    }
    readonly property bool embeddedRouteHostReady: {
        if (!applicationWindow.useEmbeddedOnboardingRoute)
            return true;
        if (!applicationWindow.activePageRouter)
            return false;
        const currentPath = applicationWindow.activePageRouter.currentPath !== undefined
            ? String(applicationWindow.activePageRouter.currentPath).trim()
            : "";
        if (currentPath !== applicationWindow.expectedEmbeddedRoutePath)
            return false;
        return applicationWindow.activePageRouter.currentPageItem !== null
            && applicationWindow.activePageRouter.currentPageItem !== undefined;
    }
    readonly property bool embeddedRouteRecoveryNeeded: {
        if (!applicationWindow.useEmbeddedOnboardingRoute)
            return false;
        if (!applicationWindow.activePageRouter)
            return true;
        const currentPath = applicationWindow.activePageRouter.currentPath !== undefined
            ? String(applicationWindow.activePageRouter.currentPath).trim()
            : "";
        if (currentPath.length === 0)
            return true;
        if (currentPath === applicationWindow.expectedEmbeddedRoutePath)
            return applicationWindow.activePageRouter.currentPageItem === null
                || applicationWindow.activePageRouter.currentPageItem === undefined;
        return !applicationWindow.onboardingRouteCommitPending;
    }
    readonly property string embeddedRouteStartupStatusText: {
        if (!applicationWindow.embeddedRouteRecoveryNeeded)
            return "";
        if (applicationWindow.onboardingHubController
                && applicationWindow.onboardingHubController.busy)
            return "Preparing WhatSon workspace...";
        if (applicationWindow.onboardingRouteCommitPending)
            return "Opening WhatSon workspace...";
        return "Restoring WhatSon startup route...";
    }
    readonly property int startupRouteRecoveryLimit: 3
    readonly property bool useEmbeddedOnboardingRoute: adaptiveMobileLayout || isMobilePlatform
    readonly property int windowDefaultHeight: LV.Theme.gap24 * 31 + LV.Theme.gap4
    readonly property int windowDefaultWidth: LV.Theme.controlHeightMd * 35 + LV.Theme.gap5
    readonly property int windowMinHeight: LV.Theme.gap20 * 21
    readonly property int windowMobileMinWidth: LV.Theme.controlHeightMd * 10
    readonly property string workspaceRoutePath: "/"
    readonly property var workspaceShellRoute: ({
            path: applicationWindow.workspaceRoutePath,
            component: workspacePageComponent
        })
    property bool agendaOverlayVisible: false
    property bool dayCalendarOverlayVisible: false
    property bool monthCalendarOverlayVisible: false
    property bool weekCalendarOverlayVisible: false
    property bool yearCalendarOverlayVisible: false
    property int startupRouteRecoveryAttempts: 0
    property string startupRouteRecoveryReason: ""

    signal viewHookRequested

    function bindOwnedViewModel(viewId, key) {
        if (!LV.ViewModels.bindView(viewId, key, true))
            console.warn("[whatson:mvvm][bind] viewId=" + viewId + " key=" + key + " error=" + LV.ViewModels.lastError);
    }
    function registerRootViewModels() {
        LV.ViewModels.set("libraryHierarchyViewModel", libraryHierarchyViewModel);
        LV.ViewModels.set("libraryNoteMutationViewModel", libraryNoteMutationViewModel);
        LV.ViewModels.set("projectsHierarchyViewModel", projectsHierarchyViewModel);
        LV.ViewModels.set("bookmarksHierarchyViewModel", bookmarksHierarchyViewModel);
        LV.ViewModels.set("tagsHierarchyViewModel", tagsHierarchyViewModel);
        LV.ViewModels.set("resourcesHierarchyViewModel", resourcesHierarchyViewModel);
        LV.ViewModels.set("progressHierarchyViewModel", progressHierarchyViewModel);
        LV.ViewModels.set("eventHierarchyViewModel", eventHierarchyViewModel);
        LV.ViewModels.set("presetHierarchyViewModel", presetHierarchyViewModel);
        LV.ViewModels.set("detailPanelViewModel", detailPanelViewModel);
        LV.ViewModels.set("editorViewModeViewModel", editorViewModeViewModel);
        LV.ViewModels.set("navigationModeViewModel", navigationModeViewModel);
        LV.ViewModels.set("sidebarHierarchyViewModel", sidebarHierarchyViewModel);
    }
    function unbindOwnedViewModels() {
        LV.ViewModels.unbindView(applicationWindow.libraryNoteMutationViewId);
        LV.ViewModels.unbindView(applicationWindow.navigationModeViewId);
        LV.ViewModels.unbindView(applicationWindow.sidebarHierarchyViewId);
    }

    function clampPreferredSizes() {
        preferredSidebarWidth = Math.max(minSidebarWidth, preferredSidebarWidth);
        preferredListViewWidth = Math.max(minListViewWidth, preferredListViewWidth);
        preferredRightPanelWidth = Math.max(minRightPanelWidth, preferredRightPanelWidth);
    }
    function nativeMenuPlaceholderText() {
        return " ";
    }
    function showOnboardingWindow() {
        if (applicationWindow.useEmbeddedOnboardingRoute && applicationWindow.onboardingRouteBootstrapController) {
            applicationWindow.onboardingRouteBootstrapController.reopenEmbeddedOnboarding();
            return;
        }

        applicationWindow.desktopOnboardingWindowVisible = true;
        onboardingSubWindow.show();
        onboardingSubWindow.raise();
        onboardingSubWindow.requestActivate();
    }
    function applyRequestedRoute(targetPath, routeSource, forceReload) {
        if (!applicationWindow.activePageRouter)
            return;

        const shouldForceReload = forceReload === true;
        const currentPath = applicationWindow.activePageRouter.currentPath !== undefined ? String(applicationWindow.activePageRouter.currentPath) : "";
        if (!shouldForceReload && currentPath === targetPath)
            return;

        applicationWindow.activePageRouter.setRoot(targetPath);
        console.log("[whatson:debug][main.route][" + routeSource + "] target=" + targetPath + " forceReload=" + shouldForceReload);
    }
    function recoverEmbeddedRouteHost(source) {
        if (!applicationWindow.useEmbeddedOnboardingRoute
                || !applicationWindow.embeddedRouteRecoveryNeeded
                || !applicationWindow.activePageRouter)
            return;

        const currentPath = applicationWindow.activePageRouter.currentPath !== undefined
            ? String(applicationWindow.activePageRouter.currentPath).trim()
            : "";
        const targetPath = applicationWindow.expectedEmbeddedRoutePath;
        const forceReload = currentPath === targetPath && currentPath.length > 0;

        applicationWindow.startupRouteRecoveryAttempts += 1;
        console.warn("[whatson:debug][main.route][startupWatchdog] reason=" + source
                     + " attempt=" + applicationWindow.startupRouteRecoveryAttempts
                     + " current=" + currentPath
                     + " target=" + targetPath
                     + " forceReload=" + forceReload);
        applicationWindow.applyRequestedRoute(
                    targetPath,
                    "startupWatchdog:" + source,
                    forceReload);
        if (applicationWindow.embeddedRouteRecoveryNeeded
                && applicationWindow.startupRouteRecoveryAttempts < applicationWindow.startupRouteRecoveryLimit)
            startupRouteWatchdogTimer.restart();
    }
    function syncEmbeddedRouteWatchdog(source) {
        if (!applicationWindow.useEmbeddedOnboardingRoute)
            return;

        if (!applicationWindow.embeddedRouteRecoveryNeeded) {
            applicationWindow.startupRouteRecoveryReason = "";
            if (applicationWindow.startupRouteRecoveryAttempts !== 0)
                applicationWindow.startupRouteRecoveryAttempts = 0;
            if (startupRouteWatchdogTimer.running)
                startupRouteWatchdogTimer.stop();
            return;
        }

        applicationWindow.startupRouteRecoveryReason = source;
        if (!startupRouteWatchdogTimer.running)
            startupRouteWatchdogTimer.start();
    }
    function toggleDetailPanelVisibility() {
        applicationWindow.hideRightPanel = !applicationWindow.hideRightPanel;
    }
    function toggleSidebarVisibility() {
        applicationWindow.hideSidebar = !applicationWindow.hideSidebar;
    }

    autoAttachRuntimeEvents: true
    delegateMobileInsetsToSystem: false
    delegateMobileWindowingToSystem: false
    forceFullWindowAreaOnMobile: applicationWindow.isMobilePlatform
    forcedDeviceTierPreset: applicationWindow.platform === "ios" ? LV.RenderQuality.LowTier : -1
    globalEventListenersEnabled: true
    height: windowDefaultHeight
    initialRoutePath: startupRoutePath
    internalRouterRegisterAsGlobalNavigator: true
    minimumHeight: windowMinHeight
    minimumWidth: adaptiveMobileLayout ? windowMobileMinWidth : desktopMinimumBodyWidth
    mobileOversizedHeightEnabled: false
    navItems: []
    navigationEnabled: false
    pageInitialPath: startupRoutePath
    pageRoutes: [onboardingRoute, workspaceShellRoute]
    useInternalPageStack: true
    usePlatformSafeMargin: false
    visible: true
    width: windowDefaultWidth
    windowColor: canvasColor
    windowDragHandleEnabled: false
    windowDragHandleHeight: navigationBarHeight
    windowDragHandleTopMargin: statusBarHeight

    Component.onCompleted: {
        registerRootViewModels();
        bindOwnedViewModel(applicationWindow.libraryNoteMutationViewId, "libraryNoteMutationViewModel");
        bindOwnedViewModel(applicationWindow.navigationModeViewId, "navigationModeViewModel");
        bindOwnedViewModel(applicationWindow.sidebarHierarchyViewId, "sidebarHierarchyViewModel");
        clampPreferredSizes();
        windowInteractions.applyRenderQualityPolicy("completed");
        windowInteractions.reportLayoutBranch("completed");
        if (applicationWindow.useEmbeddedOnboardingRoute) {
            Qt.callLater(function () {
                applicationWindow.applyRequestedRoute(applicationWindow.startupRoutePath, "completed");
                applicationWindow.syncEmbeddedRouteWatchdog("completed");
            });
        } else if (applicationWindow.desktopOnboardingWindowVisible) {
            onboardingSubWindow.show();
        }
    }
    Component.onDestruction: {
        unbindOwnedViewModels();
    }
    onActivePageRouterChanged: applicationWindow.syncEmbeddedRouteWatchdog("activePageRouterChanged")
    onAdaptiveLayoutStateChanged: windowInteractions.reportLayoutBranch("adaptiveLayoutStateChanged")
    onBodyHeightChanged: clampPreferredSizes()
    onExpectedEmbeddedRoutePathChanged: applicationWindow.syncEmbeddedRouteWatchdog("expectedEmbeddedRoutePathChanged")
    onHeightChanged: {
        if (applicationWindow.isDesktopPlatform) {
            windowInteractions.handleResizeForRenderQuality("heightChanged");
            resizeDebounceTimer.restart();
        }
        applicationWindow.syncEmbeddedRouteWatchdog("heightChanged");
    }
    onPageStackNavigated: function (path, params) {
        windowInteractions.reportLayoutBranch("pageStackNavigated");
        applicationWindow.syncEmbeddedRouteWatchdog("pageStackNavigated");
        if (applicationWindow.onboardingRouteBootstrapController)
            applicationWindow.onboardingRouteBootstrapController.handlePageStackNavigated(String(path));
    }
    onPageStackNavigationFailed: function (path) {
        console.warn("[whatson:debug][main.route][navigationFailed] path=" + path);
        applicationWindow.syncEmbeddedRouteWatchdog("pageStackNavigationFailed");
        if (applicationWindow.onboardingRouteBootstrapController)
            applicationWindow.onboardingRouteBootstrapController.handlePageStackNavigationFailed(String(path));
    }
    onWidthChanged: {
        if (applicationWindow.isDesktopPlatform) {
            windowInteractions.handleResizeForRenderQuality("widthChanged");
            resizeDebounceTimer.restart();
        }
        applicationWindow.syncEmbeddedRouteWatchdog("widthChanged");
    }

    MainWindowInteractionController {
        id: windowInteractions

        activePageRouter: applicationWindow.activePageRouter
        adaptiveDesktopLayout: applicationWindow.adaptiveDesktopLayout
        adaptiveLayoutProfile: applicationWindow.adaptiveLayoutProfile
        adaptiveMobileLayout: applicationWindow.adaptiveMobileLayout
        adaptiveNavigationMode: applicationWindow.adaptiveNavigationMode
        hostWindow: applicationWindow
        libraryHierarchyIndex: applicationWindow.libraryHierarchyIndex
        libraryNoteMutationViewId: applicationWindow.libraryNoteMutationViewId
        navigationModeViewId: applicationWindow.navigationModeViewId
        panelViewModelRegistry: applicationWindow.rootPanelViewModelRegistry
        sidebarHierarchyViewId: applicationWindow.sidebarHierarchyViewId
    }
    Timer {
        id: resizeDebounceTimer

        interval: windowInteractions.resizeRenderGuardDebounceMs
        repeat: false

        onTriggered: windowInteractions.finalizeResizeRenderQualityPolicy()
    }
    Timer {
        id: startupRouteWatchdogTimer

        interval: 240
        repeat: false

        onTriggered: applicationWindow.recoverEmbeddedRouteHost(applicationWindow.startupRouteRecoveryReason)
    }
    Shortcut {
        autoRepeat: false
        context: Qt.ApplicationShortcut
        enabled: !windowInteractions.hasFocusedTextInput()
        sequence: "Tab"

        onActivated: windowInteractions.cycleNavigationModeFromShortcut()
    }
    Shortcut {
        autoRepeat: false
        context: Qt.ApplicationShortcut
        enabled: applicationWindow.isDesktopPlatform
        sequence: StandardKey.New

        onActivated: windowInteractions.createNoteFromShortcut()
    }
    LV.EventListener {
        action: function (eventData) {
            if (!applicationWindow.activeFocusItem)
                return;
            if (!eventData || (eventData.buttons & Qt.LeftButton) !== Qt.LeftButton)
                return;
            if (windowInteractions.shouldRetainFocusForUiHit(eventData.ui))
                return;
            windowInteractions.clearActiveFocus("blankGlobalPress");
        }
        enabled: applicationWindow.globalEventListenersEnabled
        includeInputState: false
        includeUiHit: true
        trigger: "globalPressed"
    }
    Connections {
        target: applicationWindow.onboardingRouteBootstrapController

        function onRouteSyncRequested(targetPath, deferred, reason) {
            if (!applicationWindow.useEmbeddedOnboardingRoute)
                return;

            const sync = function () {
                applicationWindow.applyRequestedRoute(String(targetPath), String(reason));
            };

            if (deferred)
                Qt.callLater(sync);
            else
                sync();

            applicationWindow.syncEmbeddedRouteWatchdog("routeSyncRequested");
        }
    }
    Connections {
        target: applicationWindow.onboardingHubController

        function onHubLoaded(hubPath) {
            if (!applicationWindow.useEmbeddedOnboardingRoute)
                return;
            if (applicationWindow.onboardingRouteBootstrapController)
                applicationWindow.onboardingRouteBootstrapController.handleHubLoaded();
            applicationWindow.syncEmbeddedRouteWatchdog("hubLoaded");
        }

        function onOperationFailed(message) {
            if (applicationWindow.onboardingRouteBootstrapController)
                applicationWindow.onboardingRouteBootstrapController.handleOperationFailed(message);
            applicationWindow.syncEmbeddedRouteWatchdog("operationFailed");
        }
    }
    Connections {
        target: applicationWindow.activePageRouter

        function onCurrentPageItemChanged() {
            applicationWindow.syncEmbeddedRouteWatchdog("currentPageItemChanged");
        }

        function onCurrentPathChanged() {
            applicationWindow.syncEmbeddedRouteWatchdog("currentPathChanged");
        }
    }
    Item {
        id: mobileSafeAreaColorOverride

        parent: applicationWindow.nativeWindowContentRoot
        visible: applicationWindow.isMobilePlatform
                 && parent !== null
                 && applicationWindow.contentItem !== null
                 && !applicationWindow.fullWindowAreaOnMobileEnabled
        z: 9000
        anchors.fill: parent

        readonly property real resolvedContentX: applicationWindow.contentItem ? applicationWindow.contentItem.x : 0
        readonly property real resolvedContentY: applicationWindow.contentItem ? applicationWindow.contentItem.y : 0
        readonly property real resolvedContentRight: applicationWindow.contentItem
            ? applicationWindow.contentItem.x + applicationWindow.contentItem.width
            : width
        readonly property real resolvedContentBottom: applicationWindow.contentItem
            ? applicationWindow.contentItem.y + applicationWindow.contentItem.height
            : height
        readonly property real leftInset: Math.max(0, Math.ceil(resolvedContentX))
        readonly property real topInset: Math.max(0, Math.ceil(resolvedContentY))
        readonly property real rightInset: Math.max(0, Math.ceil(width - resolvedContentRight))
        readonly property real bottomInset: Math.max(0, Math.ceil(height - resolvedContentBottom))

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: mobileSafeAreaColorOverride.topInset
            visible: height > 0
            color: applicationWindow.mobileSafeAreaBackdropColor
        }
        Rectangle {
            x: 0
            y: mobileSafeAreaColorOverride.topInset
            width: mobileSafeAreaColorOverride.leftInset
            height: Math.max(0, parent.height - mobileSafeAreaColorOverride.topInset - mobileSafeAreaColorOverride.bottomInset)
            visible: width > 0 && height > 0
            color: applicationWindow.mobileSafeAreaBackdropColor
        }
        Rectangle {
            x: parent.width - mobileSafeAreaColorOverride.rightInset
            y: mobileSafeAreaColorOverride.topInset
            width: mobileSafeAreaColorOverride.rightInset
            height: Math.max(0, parent.height - mobileSafeAreaColorOverride.topInset - mobileSafeAreaColorOverride.bottomInset)
            visible: width > 0 && height > 0
            color: applicationWindow.mobileSafeAreaBackdropColor
        }
        Rectangle {
            x: 0
            y: parent.height - mobileSafeAreaColorOverride.bottomInset
            width: parent.width
            height: mobileSafeAreaColorOverride.bottomInset
            visible: height > 0
            color: applicationWindow.mobileSafeAreaBackdropColor
        }
    }
    Rectangle {
        id: embeddedRouteStartupFallback

        anchors.fill: parent
        color: applicationWindow.canvasColor
        visible: applicationWindow.useEmbeddedOnboardingRoute
                 && applicationWindow.embeddedRouteRecoveryNeeded
        z: 9050

        LV.VStack {
            anchors.centerIn: parent
            spacing: LV.Theme.gap4
            width: Math.max(0, Math.min(parent.width - LV.Theme.gap20 * 2, LV.Theme.gap20 * 13))

            LV.Label {
                color: LV.Theme.textPrimary
                horizontalAlignment: Text.AlignHCenter
                style: header2
                text: "WhatSon"
                width: parent.width
                wrapMode: Text.NoWrap
            }
            LV.Label {
                color: LV.Theme.descriptionColor
                horizontalAlignment: Text.AlignHCenter
                style: description
                text: applicationWindow.embeddedRouteStartupStatusText
                width: parent.width
                wrapMode: Text.WordWrap
            }
        }
    }
    Loader {
        active: applicationWindow.platform === "osx"
        source: applicationWindow.platform === "osx"
            ? Qt.resolvedUrl("window/MacNativeMenuBar.qml")
            : ""

        onLoaded: {
            if (item) {
                item.hostWindow = applicationWindow
                item.resourcesImportViewModel = resourcesImportViewModel
            }
        }
    }
    Component {
        id: onboardingPageComponent

        Item {
            anchors.fill: parent
            clip: true

            Rectangle {
                anchors.fill: parent
                color: applicationWindow.canvasColor
            }
            WindowView.OnboardingContent {
                anchors.fill: parent
                autoCompleteOnHubLoaded: false
                hostWindow: applicationWindow
                hubSessionController: applicationWindow.onboardingHubController

                onDismissRequested: {
                    if (applicationWindow.onboardingRouteBootstrapController)
                        applicationWindow.onboardingRouteBootstrapController.dismissEmbeddedOnboarding();
                }
            }
        }
    }
    Component {
        id: workspacePageComponent

        Item {
            anchors.fill: parent
            clip: true

            Loader {
                id: workspaceLayoutLoader

                anchors.fill: parent
                sourceComponent: applicationWindow.adaptiveMobileLayout ? mobileMainLayoutComponent : desktopMainLayoutComponent

                onSourceComponentChanged: windowInteractions.reportLayoutBranch("workspaceSourceChanged")
            }
        }
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

                    Layout.preferredHeight: panelHeight
                    compactMode: false
                    panelColor: applicationWindow.desktopPanelSurfaceColor
                    panelHeight: applicationWindow.statusBarHeight

                    onWindowMoveRequested: {
                        applicationWindow.requestWindowMove();
                    }
                }
                BodyPanelView.NavigationBarLayout {
                    id: navigationBar

                    compactMode: false
                    detailPanelCollapsed: applicationWindow.hideRightPanel
                    editorViewModeViewModel: applicationWindow.rootEditorViewModeViewModel
                    navigationModeViewModel: applicationWindow.rootNavigationModeViewModel
                    panelColor: applicationWindow.desktopPanelSurfaceColor
                    panelHeight: applicationWindow.navigationBarHeight
                    sidebarCollapsed: applicationWindow.hideSidebar

                    onToggleDetailPanelRequested: applicationWindow.toggleDetailPanelVisibility()
                    onToggleSidebarRequested: applicationWindow.toggleSidebarVisibility()
                    onAgendaRequested: {
                        applicationWindow.agendaOverlayVisible = true;
                        applicationWindow.dayCalendarOverlayVisible = false;
                        applicationWindow.weekCalendarOverlayVisible = false;
                        applicationWindow.monthCalendarOverlayVisible = false;
                        applicationWindow.yearCalendarOverlayVisible = false;
                    }
                    onDayCalendarRequested: {
                        applicationWindow.agendaOverlayVisible = false;
                        applicationWindow.dayCalendarOverlayVisible = true;
                        applicationWindow.weekCalendarOverlayVisible = false;
                        applicationWindow.monthCalendarOverlayVisible = false;
                        applicationWindow.yearCalendarOverlayVisible = false;
                    }
                    onMonthCalendarRequested: {
                        applicationWindow.agendaOverlayVisible = false;
                        applicationWindow.dayCalendarOverlayVisible = false;
                        applicationWindow.weekCalendarOverlayVisible = false;
                        applicationWindow.monthCalendarOverlayVisible = true;
                        applicationWindow.yearCalendarOverlayVisible = false;
                    }
                    onWeekCalendarRequested: {
                        applicationWindow.agendaOverlayVisible = false;
                        applicationWindow.dayCalendarOverlayVisible = false;
                        applicationWindow.weekCalendarOverlayVisible = true;
                        applicationWindow.monthCalendarOverlayVisible = false;
                        applicationWindow.yearCalendarOverlayVisible = false;
                    }
                    onYearCalendarRequested: {
                        applicationWindow.agendaOverlayVisible = false;
                        applicationWindow.dayCalendarOverlayVisible = false;
                        applicationWindow.weekCalendarOverlayVisible = false;
                        applicationWindow.yearCalendarOverlayVisible = true;
                        applicationWindow.monthCalendarOverlayVisible = false;
                    }
                }
                BodyPanelView.BodyLayout {
                    id: hStack

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    compactCanvasColor: applicationWindow.canvasColor
                    compactMode: false
                    contentsDisplayColor: applicationWindow.desktopPanelSurfaceColor
                    editorViewModeViewModel: applicationWindow.rootEditorViewModeViewModel
                    isMobilePlatform: applicationWindow.isMobilePlatform
                    listViewColor: applicationWindow.desktopPanelSurfaceColor
                    listViewWidth: applicationWindow.listViewWidth
                    libraryHierarchyViewModel: applicationWindow.rootLibraryHierarchyViewModel
                    minContentWidth: applicationWindow.minContentWidth
                    minListViewWidth: applicationWindow.minListViewWidth
                    minRightPanelWidth: applicationWindow.minRightPanelWidth
                    minSidebarWidth: applicationWindow.minSidebarWidth
                    noteDeletionViewModel: applicationWindow.rootLibraryNoteMutationViewModel
                    rightPanelColor: applicationWindow.desktopPanelSurfaceColor
                    rightPanelWidth: applicationWindow.rightPanelWidth
                    resourcesImportViewModel: resourcesImportViewModel
                    sidebarColor: applicationWindow.desktopPanelSurfaceColor
                    sidebarHierarchyViewModel: applicationWindow.rootSidebarHierarchyViewModel
                    sidebarHorizontalInset: applicationWindow.hierarchyHorizontalInset
                    sidebarWidth: applicationWindow.sidebarWidth
                    splitterColor: applicationWindow.bodySplitterColor
                    splitterThickness: applicationWindow.bodySplitterThickness
                    agendaOverlayVisible: applicationWindow.agendaOverlayVisible
                    agendaViewModel: applicationWindow.rootAgendaViewModel
                    dayCalendarOverlayVisible: applicationWindow.dayCalendarOverlayVisible
                    dayCalendarViewModel: applicationWindow.rootDayCalendarViewModel
                    monthCalendarOverlayVisible: applicationWindow.monthCalendarOverlayVisible
                    monthCalendarViewModel: applicationWindow.rootMonthCalendarViewModel
                    weekCalendarOverlayVisible: applicationWindow.weekCalendarOverlayVisible
                    weekCalendarViewModel: applicationWindow.rootWeekCalendarViewModel
                    yearCalendarOverlayVisible: applicationWindow.yearCalendarOverlayVisible
                    yearCalendarViewModel: applicationWindow.rootYearCalendarViewModel

                    onNoteActivated: function (index, noteId) {
                        const normalizedNoteId = noteId === undefined || noteId === null
                            ? ""
                            : String(noteId).trim();
                        if (normalizedNoteId.length === 0)
                            return;
                        applicationWindow.agendaOverlayVisible = false;
                        applicationWindow.dayCalendarOverlayVisible = false;
                        applicationWindow.weekCalendarOverlayVisible = false;
                        applicationWindow.monthCalendarOverlayVisible = false;
                        applicationWindow.yearCalendarOverlayVisible = false;
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
                    onAgendaOverlayDismissRequested: applicationWindow.agendaOverlayVisible = false
                    onDayCalendarOverlayDismissRequested: applicationWindow.dayCalendarOverlayVisible = false
                    onMonthCalendarOverlayOpenRequested: {
                        applicationWindow.agendaOverlayVisible = false;
                        applicationWindow.dayCalendarOverlayVisible = false;
                        applicationWindow.weekCalendarOverlayVisible = false;
                        applicationWindow.monthCalendarOverlayVisible = true;
                        applicationWindow.yearCalendarOverlayVisible = false;
                    }
                    onMonthCalendarOverlayDismissRequested: applicationWindow.monthCalendarOverlayVisible = false
                    onWeekCalendarOverlayDismissRequested: applicationWindow.weekCalendarOverlayVisible = false
                    onYearCalendarOverlayDismissRequested: applicationWindow.yearCalendarOverlayVisible = false
                }
            }
        }
    }
    Component {
        id: mobileMainLayoutComponent

        MobilePageView.MobileHierarchyPage {
            anchors.fill: parent
            canvasColor: applicationWindow.canvasColor
            controlSurfaceColor: LV.Theme.panelBackground10
            editorViewModeViewModel: applicationWindow.rootEditorViewModeViewModel
            navigationModeViewModel: applicationWindow.rootNavigationModeViewModel
            sidebarHierarchyViewModel: applicationWindow.rootSidebarHierarchyViewModel
            statusPlaceholderText: ""
            toolbarIconNames: applicationWindow.hierarchyToolbarIconNames
            resourcesImportViewModel: resourcesImportViewModel
            windowInteractions: windowInteractions
            agendaOverlayVisible: applicationWindow.agendaOverlayVisible
            agendaViewModel: applicationWindow.rootAgendaViewModel
            dayCalendarOverlayVisible: applicationWindow.dayCalendarOverlayVisible
            dayCalendarViewModel: applicationWindow.rootDayCalendarViewModel
            monthCalendarOverlayVisible: applicationWindow.monthCalendarOverlayVisible
            monthCalendarViewModel: applicationWindow.rootMonthCalendarViewModel
            weekCalendarOverlayVisible: applicationWindow.weekCalendarOverlayVisible
            weekCalendarViewModel: applicationWindow.rootWeekCalendarViewModel
            yearCalendarOverlayVisible: applicationWindow.yearCalendarOverlayVisible
            yearCalendarViewModel: applicationWindow.rootYearCalendarViewModel

            onAgendaRequested: {
                applicationWindow.agendaOverlayVisible = true;
                applicationWindow.dayCalendarOverlayVisible = false;
                applicationWindow.weekCalendarOverlayVisible = false;
                applicationWindow.monthCalendarOverlayVisible = false;
                applicationWindow.yearCalendarOverlayVisible = false;
            }
            onDayCalendarRequested: {
                applicationWindow.agendaOverlayVisible = false;
                applicationWindow.dayCalendarOverlayVisible = true;
                applicationWindow.weekCalendarOverlayVisible = false;
                applicationWindow.monthCalendarOverlayVisible = false;
                applicationWindow.yearCalendarOverlayVisible = false;
            }
            onMonthCalendarRequested: {
                applicationWindow.agendaOverlayVisible = false;
                applicationWindow.dayCalendarOverlayVisible = false;
                applicationWindow.weekCalendarOverlayVisible = false;
                applicationWindow.monthCalendarOverlayVisible = true;
                applicationWindow.yearCalendarOverlayVisible = false;
            }
            onWeekCalendarRequested: {
                applicationWindow.agendaOverlayVisible = false;
                applicationWindow.dayCalendarOverlayVisible = false;
                applicationWindow.weekCalendarOverlayVisible = true;
                applicationWindow.monthCalendarOverlayVisible = false;
                applicationWindow.yearCalendarOverlayVisible = false;
            }
            onYearCalendarRequested: {
                applicationWindow.agendaOverlayVisible = false;
                applicationWindow.dayCalendarOverlayVisible = false;
                applicationWindow.weekCalendarOverlayVisible = false;
                applicationWindow.yearCalendarOverlayVisible = true;
                applicationWindow.monthCalendarOverlayVisible = false;
            }
            onAgendaOverlayDismissRequested: applicationWindow.agendaOverlayVisible = false
            onDayCalendarOverlayDismissRequested: applicationWindow.dayCalendarOverlayVisible = false
            onMonthCalendarOverlayOpenRequested: {
                applicationWindow.agendaOverlayVisible = false;
                applicationWindow.dayCalendarOverlayVisible = false;
                applicationWindow.weekCalendarOverlayVisible = false;
                applicationWindow.monthCalendarOverlayVisible = true;
                applicationWindow.yearCalendarOverlayVisible = false;
            }
            onMonthCalendarOverlayDismissRequested: applicationWindow.monthCalendarOverlayVisible = false
            onWeekCalendarOverlayDismissRequested: applicationWindow.weekCalendarOverlayVisible = false
            onYearCalendarOverlayDismissRequested: applicationWindow.yearCalendarOverlayVisible = false
        }
    }
    WindowView.Onboarding {
        id: onboardingSubWindow

        defaultHeight: applicationWindow.onboardingMinHeight
        defaultWidth: applicationWindow.onboardingMinWidth
        hostWindow: applicationWindow
        hubSessionController: applicationWindow.onboardingHubController
        minHeight: applicationWindow.onboardingMinHeight
        minWidth: applicationWindow.onboardingMinWidth

        onCreateFileRequested: {
            applicationWindow.desktopOnboardingWindowVisible = false;
            onboardingSubWindow.close();
        }
        onDismissed: {
            applicationWindow.desktopOnboardingWindowVisible = false;
        }
        onSelectFileRequested: {
            applicationWindow.desktopOnboardingWindowVisible = false;
            onboardingSubWindow.close();
        }
    }
}
