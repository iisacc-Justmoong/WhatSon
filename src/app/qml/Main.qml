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

    property int topPadding: LV.Theme.gapNone
    property int rightPadding: LV.Theme.gapNone
    property int bottomPadding: LV.Theme.gapNone
    property int leftPadding: LV.Theme.gapNone
    readonly property int adaptiveStatusBarHeight: adaptiveMobileLayout ? 0 : statusBarHeight
    readonly property int baseListViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property int baseRightPanelWidth: LV.Theme.inputMinWidth + LV.Theme.gap14
    readonly property int baseSidebarWidth: hierarchyToolbarWidth
    readonly property int bodyHeight: Math.max(0, height - adaptiveStatusBarHeight - navigationBarHeight)
    readonly property color bodySplitterColor: LV.Theme.panelBackground10
    readonly property int bodySplitterThickness: Math.max(1, Math.round(LV.Theme.strokeThin))
    readonly property color canvasColor: LV.Theme.panelBackground01
    readonly property color desktopPanelSurfaceColor: LV.Theme.accentTransparent
    readonly property color mobileControlSurfaceColor: LV.Theme.panelBackground10
    property bool desktopOnboardingWindowVisible: false
    readonly property int desktopMinimumBodyWidth: (hideSidebar ? 0 : minSidebarWidth) + (hideListView ? 0 : minListViewWidth) + minContentWidth + (hideRightPanel ? 0 : minRightPanelWidth) + bodySplitterThickness * Math.max(0, ((hideSidebar ? 0 : 1) + (hideListView ? 0 : 1) + 1 + (hideRightPanel ? 0 : 1)) - 1)
    readonly property bool hideListView: false
    property bool hideRightPanel: false
    property bool hideSidebar: false
    readonly property int hierarchyHorizontalInset: LV.Theme.gap2
    readonly property int hierarchyToolbarButtonSize: LV.Theme.gap20
    readonly property int hierarchyToolbarCount: hierarchyToolbarIconNames.length
    readonly property var hierarchyToolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property real hierarchyToolbarSpacing: hierarchyToolbarCount > 1 ? (LV.Theme.gap20 + LV.Theme.gap20) / (hierarchyToolbarCount - 1) : LV.Theme.gapNone
    readonly property int hierarchyToolbarTrackWidth: hierarchyToolbarCount > 0 ? Math.round(hierarchyToolbarCount * hierarchyToolbarButtonSize + (hierarchyToolbarCount - 1) * hierarchyToolbarSpacing) : hierarchyToolbarButtonSize
    readonly property int hierarchyToolbarWidth: hierarchyToolbarTrackWidth + hierarchyHorizontalInset * 2
    readonly property var rootEditorViewModeController: typeof editorViewModeController !== "undefined" ? editorViewModeController : null
    readonly property int libraryHierarchyIndex: 0
    readonly property var rootLibraryHierarchyController: typeof libraryHierarchyController !== "undefined" ? libraryHierarchyController : null
    readonly property var rootLibraryNoteMutationController: typeof libraryNoteMutationController !== "undefined" ? libraryNoteMutationController : null
    readonly property int listViewWidth: hideListView ? 0 : Math.max(minListViewWidth, preferredListViewWidth)
    readonly property int minContentWidth: LV.Theme.dialogMaxWidth - LV.Theme.gap20 * 2
    readonly property int minListViewWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    readonly property int minRightPanelWidth: LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + Math.round(LV.Theme.strokeThin)
    readonly property var rootNavigationModeController: typeof navigationModeController !== "undefined" ? navigationModeController : null
    readonly property var rootPanelControllerRegistry: panelControllerRegistry
    readonly property var rootSidebarHierarchyController: typeof sidebarHierarchyController !== "undefined" ? sidebarHierarchyController : null
    readonly property var rootNoteActiveState: typeof noteActiveState !== "undefined" ? noteActiveState : null
    readonly property var rootResourcesImportController: typeof resourcesImportController !== "undefined" ? resourcesImportController : null
    readonly property var rootAgendaController: typeof agendaController !== "undefined" ? agendaController : null
    readonly property var rootDayCalendarController: typeof dayCalendarController !== "undefined" ? dayCalendarController : null
    readonly property var rootMonthCalendarController: typeof monthCalendarController !== "undefined" ? monthCalendarController : null
    readonly property var rootWeekCalendarController: typeof weekCalendarController !== "undefined" ? weekCalendarController : null
    readonly property var rootYearCalendarController: typeof yearCalendarController !== "undefined" ? yearCalendarController : null
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
    readonly property int onboardingMinHeight: LV.Theme.scaffoldBlobSecondaryHeight + LV.Theme.gap20 + LV.Theme.gap20
    readonly property int onboardingMinWidth: LV.Theme.scaffoldBlobSecondaryWidth - LV.Theme.gap20
    property int preferredListViewWidth: baseListViewWidth
    property int preferredRightPanelWidth: baseRightPanelWidth
    property int preferredSidebarWidth: baseSidebarWidth
    readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)
    readonly property int sidebarWidth: hideSidebar ? 0 : Math.max(minSidebarWidth, preferredSidebarWidth)
    readonly property int statusBarHeight: LV.Theme.controlHeightMd
    readonly property bool onboardingRouteCommitPending: onboardingRouteBootstrapController ? onboardingRouteBootstrapController.routeCommitPending : false
    readonly property bool useIosInlineOnboardingSequence: applicationWindow.platform === "ios"
    readonly property bool useRoutedEmbeddedOnboardingRoute: !applicationWindow.useIosInlineOnboardingSequence && (adaptiveMobileLayout || isMobilePlatform)
    readonly property bool useEmbeddedOnboardingPresentation: applicationWindow.useIosInlineOnboardingSequence || applicationWindow.useRoutedEmbeddedOnboardingRoute
    readonly property string startupRoutePath: applicationWindow.useIosInlineOnboardingSequence ? workspaceRoutePath : (onboardingRouteBootstrapController ? onboardingRouteBootstrapController.startupRoutePath : workspaceRoutePath)
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

    signal viewHookRequested

    function currentIsoDate() {
        return Qt.formatDateTime(new Date(), "yyyy-MM-dd");
    }
    function resetAgendaOverlayCursorToToday() {
        if (applicationWindow.rootAgendaController && applicationWindow.rootAgendaController.setDisplayedDateIso !== undefined) {
            applicationWindow.rootAgendaController.setDisplayedDateIso(applicationWindow.currentIsoDate());
        }
    }
    function resetDayCalendarCursorToToday() {
        if (applicationWindow.rootDayCalendarController && applicationWindow.rootDayCalendarController.setDisplayedDateIso !== undefined) {
            applicationWindow.rootDayCalendarController.setDisplayedDateIso(applicationWindow.currentIsoDate());
        }
    }
    function resetWeekCalendarCursorToToday() {
        if (applicationWindow.rootWeekCalendarController && applicationWindow.rootWeekCalendarController.setDisplayedWeekStartIso !== undefined) {
            applicationWindow.rootWeekCalendarController.setDisplayedWeekStartIso(applicationWindow.currentIsoDate());
        }
    }
    function resetMonthCalendarCursorToToday() {
        if (applicationWindow.rootMonthCalendarController && applicationWindow.rootMonthCalendarController.focusToday !== undefined) {
            applicationWindow.rootMonthCalendarController.focusToday();
        }
    }
    function resetYearCalendarCursorToToday() {
        if (applicationWindow.rootYearCalendarController && applicationWindow.rootYearCalendarController.focusToday !== undefined) {
            applicationWindow.rootYearCalendarController.focusToday();
        }
    }
    function openAgendaOverlay(resetToToday) {
        if (resetToToday !== false)
            applicationWindow.resetAgendaOverlayCursorToToday();
        applicationWindow.agendaOverlayVisible = true;
        applicationWindow.dayCalendarOverlayVisible = false;
        applicationWindow.weekCalendarOverlayVisible = false;
        applicationWindow.monthCalendarOverlayVisible = false;
        applicationWindow.yearCalendarOverlayVisible = false;
    }
    function openDayCalendarOverlay(resetToToday) {
        if (resetToToday !== false)
            applicationWindow.resetDayCalendarCursorToToday();
        applicationWindow.agendaOverlayVisible = false;
        applicationWindow.dayCalendarOverlayVisible = true;
        applicationWindow.weekCalendarOverlayVisible = false;
        applicationWindow.monthCalendarOverlayVisible = false;
        applicationWindow.yearCalendarOverlayVisible = false;
    }
    function openMonthCalendarOverlay(resetToToday) {
        if (resetToToday !== false)
            applicationWindow.resetMonthCalendarCursorToToday();
        applicationWindow.agendaOverlayVisible = false;
        applicationWindow.dayCalendarOverlayVisible = false;
        applicationWindow.weekCalendarOverlayVisible = false;
        applicationWindow.monthCalendarOverlayVisible = true;
        applicationWindow.yearCalendarOverlayVisible = false;
    }
    function openWeekCalendarOverlay(resetToToday) {
        if (resetToToday !== false)
            applicationWindow.resetWeekCalendarCursorToToday();
        applicationWindow.agendaOverlayVisible = false;
        applicationWindow.dayCalendarOverlayVisible = false;
        applicationWindow.weekCalendarOverlayVisible = true;
        applicationWindow.monthCalendarOverlayVisible = false;
        applicationWindow.yearCalendarOverlayVisible = false;
    }
    function openYearCalendarOverlay(resetToToday) {
        if (resetToToday !== false)
            applicationWindow.resetYearCalendarCursorToToday();
        applicationWindow.agendaOverlayVisible = false;
        applicationWindow.dayCalendarOverlayVisible = false;
        applicationWindow.weekCalendarOverlayVisible = false;
        applicationWindow.yearCalendarOverlayVisible = true;
        applicationWindow.monthCalendarOverlayVisible = false;
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
        if (applicationWindow.useEmbeddedOnboardingPresentation && applicationWindow.onboardingRouteBootstrapController) {
            applicationWindow.onboardingRouteBootstrapController.reopenEmbeddedOnboarding();
            return;
        }

        applicationWindow.desktopOnboardingWindowVisible = true;
        onboardingSubWindow.show();
        onboardingSubWindow.raise();
        onboardingSubWindow.requestActivate();
    }
    function applyRequestedRoute(targetPath, routeSource) {
        if (!applicationWindow.activePageRouter)
            return;

        const currentPath = applicationWindow.activePageRouter.currentPath !== undefined ? String(applicationWindow.activePageRouter.currentPath) : "";
        if (currentPath === targetPath)
            return;

        applicationWindow.activePageRouter.setRoot(targetPath);
        console.log("[whatson:debug][main.route][" + routeSource + "] target=" + targetPath);
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
    pageRoutes: applicationWindow.useIosInlineOnboardingSequence ? [workspaceShellRoute] : [onboardingRoute, workspaceShellRoute]
    useInternalPageStack: true
    usePlatformSafeMargin: false
    visible: true
    width: windowDefaultWidth
    windowColor: canvasColor
    windowDragHandleEnabled: false
    windowDragHandleHeight: navigationBarHeight
    windowDragHandleTopMargin: statusBarHeight

    Component.onCompleted: {
        clampPreferredSizes();
        windowInteractions.applyRenderQualityPolicy("completed");
        windowInteractions.reportLayoutBranch("completed");
        if (applicationWindow.useRoutedEmbeddedOnboardingRoute) {
            Qt.callLater(function () {
                applicationWindow.applyRequestedRoute(applicationWindow.startupRoutePath, "completed");
            });
        } else if (applicationWindow.desktopOnboardingWindowVisible) {
            onboardingSubWindow.show();
        }
    }
    onAdaptiveLayoutStateChanged: windowInteractions.reportLayoutBranch("adaptiveLayoutStateChanged")
    onBodyHeightChanged: clampPreferredSizes()
    onHeightChanged: {
        if (applicationWindow.isDesktopPlatform) {
            windowInteractions.handleResizeForRenderQuality("heightChanged");
            resizeDebounceTimer.restart();
        }
    }
    onPageStackNavigated: function (path, params) {
        windowInteractions.reportLayoutBranch("pageStackNavigated");
        if (applicationWindow.onboardingRouteBootstrapController)
            applicationWindow.onboardingRouteBootstrapController.handlePageStackNavigated(String(path));
    }
    onPageStackNavigationFailed: function (path) {
        console.warn("[whatson:debug][main.route][navigationFailed] path=" + path);
        if (applicationWindow.onboardingRouteBootstrapController)
            applicationWindow.onboardingRouteBootstrapController.handlePageStackNavigationFailed(String(path));
    }
    onWidthChanged: {
        if (applicationWindow.isDesktopPlatform) {
            windowInteractions.handleResizeForRenderQuality("widthChanged");
            resizeDebounceTimer.restart();
        }
    }

    QtObject {
        id: windowInteractions

        property var activePageRouter: null
        readonly property string addNewPanelKey: "navigation.NavigationAddNewBar"
        property bool adaptiveDesktopLayout: false
        property string adaptiveLayoutProfile: ""
        property bool adaptiveMobileLayout: false
        property string adaptiveNavigationMode: ""
        property var focusRetainedUiTokens: ["button", "combobox", "checkbox", "radiobutton", "switch", "slider", "spinbox", "dial", "textinput", "textedit", "inputfield", "editor", "menu", "popup", "tooltip", "mousearea", "taphandler", "flickable", "listview", "scrollview", "tableview", "scrollbar", "hierarchy", "contextmenu", "itemdelegate", "notelistitem"]
        property var hostWindow: null
        property int libraryHierarchyIndex: 0
        property var libraryNoteMutationController: applicationWindow.rootLibraryNoteMutationController
        property var navigationModeController: applicationWindow.rootNavigationModeController
        property var panelControllerRegistry: null
        readonly property bool resizeRenderGuardSupported: windowInteractions.hostWindow ? windowInteractions.hostWindow.isDesktopPlatform : false
        property bool resizeDrWasSuspended: false
        property bool resizeInProgress: false
        property int resizeRenderGuardDebounceMs: 220
        property bool resizeRenderGuardEnabled: true
        property var sidebarHierarchyController: applicationWindow.rootSidebarHierarchyController

        function applyRenderQualityPolicy(source) {
            if (!windowInteractions.hostWindow || (!windowInteractions.hostWindow.isDesktopPlatform && !windowInteractions.hostWindow.isMobilePlatform))
                return;

            const guardPolicy = windowInteractions.resizeRenderGuardSupported ? "desktopResizeSuspendResumeGuard" : "mobileResizeGuardDisabled";
            const forcedTierPreset = windowInteractions.hostWindow.forcedDeviceTierPreset !== undefined ? windowInteractions.hostWindow.forcedDeviceTierPreset : -1;
            const fullWindowAreaOnMobileEnabled = windowInteractions.hostWindow.fullWindowAreaOnMobileEnabled === true;
            const delegateMobileWindowingToSystem = windowInteractions.hostWindow.delegateMobileWindowingToSystem === true;
            console.log("[whatson:debug][render.policy][" + source + "] platform=" + windowInteractions.hostWindow.platform + " action=" + guardPolicy + " dynamicResolutionEnabled=" + LV.RenderQuality.dynamicResolutionEnabled + " forcedDeviceTierPreset=" + forcedTierPreset + " fullWindowAreaOnMobileEnabled=" + fullWindowAreaOnMobileEnabled + " delegateMobileWindowingToSystem=" + delegateMobileWindowingToSystem);
        }
        function clearActiveFocus(reason) {
            let current = windowInteractions.hostWindow ? windowInteractions.hostWindow.activeFocusItem : null;
            let depthGuard = 0;

            while (current && depthGuard < 32) {
                if (current.focus !== undefined)
                    current.focus = false;
                current = current.parent;
                depthGuard += 1;
            }
        }
        function resolvePanelController(panelKey) {
            const normalizedPanelKey = panelKey === undefined || panelKey === null ? "" : String(panelKey).trim();
            if (normalizedPanelKey.length === 0 || !windowInteractions.panelControllerRegistry || windowInteractions.panelControllerRegistry.panelController === undefined)
                return null;
            return windowInteractions.panelControllerRegistry.panelController(normalizedPanelKey);
        }
        function resolveLibraryNoteCreationController() {
            return windowInteractions.libraryNoteMutationController;
        }
        function createNoteFromShortcut() {
            const addNewPanelController = windowInteractions.resolvePanelController(windowInteractions.addNewPanelKey);
            if (addNewPanelController && addNewPanelController.requestControllerHook !== undefined) {
                addNewPanelController.requestControllerHook("create-note");
                return true;
            }
            const noteMutationController = windowInteractions.resolveLibraryNoteCreationController();
            if (!noteMutationController || noteMutationController.createEmptyNote === undefined)
                return false;
            const sidebarHierarchyController = windowInteractions.sidebarHierarchyController;
            if (sidebarHierarchyController && sidebarHierarchyController.setActiveHierarchyIndex !== undefined)
                sidebarHierarchyController.setActiveHierarchyIndex(windowInteractions.libraryHierarchyIndex);
            return Boolean(noteMutationController.createEmptyNote());
        }
        function cycleNavigationModeFromShortcut() {
            if (windowInteractions.hasFocusedTextInput())
                return;
            const navigationModeController = windowInteractions.navigationModeController;
            if (navigationModeController && navigationModeController.requestNextMode !== undefined)
                navigationModeController.requestNextMode();
        }
        function finalizeResizeRenderQualityPolicy() {
            if (!windowInteractions.resizeRenderGuardEnabled || !windowInteractions.resizeRenderGuardSupported || !windowInteractions.hostWindow)
                return;

            windowInteractions.resizeInProgress = false;
            if (!windowInteractions.resizeDrWasSuspended) {
                console.log("[whatson:debug][render.policy][resizeEnd] platform=" + windowInteractions.hostWindow.platform + " action=resumeSkipped");
                return;
            }

            LV.RenderQuality.dynamicResolutionEnabled = false;
            LV.RenderQuality.dynamicResolutionEnabled = true;
            windowInteractions.resizeDrWasSuspended = false;
            console.log("[whatson:debug][render.policy][resizeEnd] platform=" + windowInteractions.hostWindow.platform + " action=resumeWithMaxScaleReset");
        }
        function handleResizeForRenderQuality(source) {
            if (!windowInteractions.resizeRenderGuardEnabled || !windowInteractions.resizeRenderGuardSupported || !windowInteractions.hostWindow)
                return;

            if (!windowInteractions.resizeInProgress) {
                windowInteractions.resizeInProgress = true;
                windowInteractions.resizeDrWasSuspended = LV.RenderQuality.dynamicResolutionEnabled;

                if (windowInteractions.resizeDrWasSuspended) {
                    LV.RenderQuality.dynamicResolutionEnabled = false;
                    console.log("[whatson:debug][render.policy][" + source + "] platform=" + windowInteractions.hostWindow.platform + " action=resizeBegin suspendDynamicResolution=true");
                } else {
                    console.log("[whatson:debug][render.policy][" + source + "] platform=" + windowInteractions.hostWindow.platform + " action=resizeBegin suspendDynamicResolution=false");
                }
            }
        }
        function hasFocusedTextInput() {
            let current = windowInteractions.hostWindow ? windowInteractions.hostWindow.activeFocusItem : null;
            while (current) {
                const isTextEditingItem = current.text !== undefined && current.cursorPosition !== undefined && current.selectedText !== undefined;
                if (isTextEditingItem)
                    return true;
                current = current.parent;
            }
            return false;
        }
        function reportLayoutBranch(source) {
            const currentPath = windowInteractions.activePageRouter && windowInteractions.activePageRouter.currentPath !== undefined ? String(windowInteractions.activePageRouter.currentPath) : "<none>";
            console.log("[whatson:debug][main.layout][" + source + "] platform=" + (windowInteractions.hostWindow ? windowInteractions.hostWindow.platform : "") + " adaptiveLayoutProfile=" + windowInteractions.adaptiveLayoutProfile + " adaptiveNavigationMode=" + windowInteractions.adaptiveNavigationMode + " adaptiveMobileLayout=" + windowInteractions.adaptiveMobileLayout + " adaptiveDesktopLayout=" + windowInteractions.adaptiveDesktopLayout + " currentPath=" + currentPath);
        }
        function shouldRetainFocusForUiHit(uiData) {
            if (!uiData || uiData.insideWindow === false)
                return true;

            const className = uiData.className === undefined || uiData.className === null ? "" : String(uiData.className).toLowerCase();
            const objectName = uiData.objectName === undefined || uiData.objectName === null ? "" : String(uiData.objectName).toLowerCase();
            const path = uiData.path === undefined || uiData.path === null ? "" : String(uiData.path).toLowerCase();
            const text = uiData.text === undefined || uiData.text === null ? "" : String(uiData.text).trim();
            const label = uiData.label === undefined || uiData.label === null ? "" : String(uiData.label).trim();
            const title = uiData.title === undefined || uiData.title === null ? "" : String(uiData.title).trim();
            const searchable = className + " " + objectName + " " + path;

            for (let i = 0; i < windowInteractions.focusRetainedUiTokens.length; ++i) {
                const token = String(windowInteractions.focusRetainedUiTokens[i]).toLowerCase();
                if (token.length > 0 && searchable.indexOf(token) >= 0)
                    return true;
            }

            if (text.length > 0 || label.length > 0 || title.length > 0)
                return true;

            return !(className.indexOf("rectangle") >= 0 || (className.indexOf("item") >= 0 && objectName === "unnamed"));
        }
    }
    Timer {
        id: resizeDebounceTimer

        interval: windowInteractions.resizeRenderGuardDebounceMs
        repeat: false

        onTriggered: windowInteractions.finalizeResizeRenderQualityPolicy()
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
            const normalizedTargetPath = String(targetPath);
            const normalizedReason = String(reason);
            const sync = function () {
                if (applicationWindow.useIosInlineOnboardingSequence) {
                    if (normalizedTargetPath === applicationWindow.workspaceRoutePath && applicationWindow.onboardingRouteBootstrapController) {
                        applicationWindow.onboardingRouteBootstrapController.handlePageStackNavigated(applicationWindow.workspaceRoutePath);
                    }
                    return;
                }

                if (!applicationWindow.useRoutedEmbeddedOnboardingRoute)
                    return;

                applicationWindow.applyRequestedRoute(normalizedTargetPath, normalizedReason);
            };

            if (deferred)
                Qt.callLater(sync);
            else
                sync();
        }
    }
    Connections {
        target: applicationWindow.onboardingHubController

        function onHubLoaded(hubPath) {
            if (!applicationWindow.useEmbeddedOnboardingPresentation)
                return;
            if (applicationWindow.onboardingRouteBootstrapController)
                applicationWindow.onboardingRouteBootstrapController.handleHubLoaded();
        }

        function onOperationFailed(message) {
            if (applicationWindow.onboardingRouteBootstrapController)
                applicationWindow.onboardingRouteBootstrapController.handleOperationFailed(message);
        }
    }
    Item {
        id: mobileSafeAreaColorOverride

        parent: applicationWindow.nativeWindowContentRoot
        visible: applicationWindow.isMobilePlatform && parent !== null && applicationWindow.contentItem !== null && !applicationWindow.fullWindowAreaOnMobileEnabled
        z: 9000
        anchors.fill: parent

        readonly property real resolvedContentX: applicationWindow.contentItem ? applicationWindow.contentItem.x : 0
        readonly property real resolvedContentY: applicationWindow.contentItem ? applicationWindow.contentItem.y : 0
        readonly property real resolvedContentRight: applicationWindow.contentItem ? applicationWindow.contentItem.x + applicationWindow.contentItem.width : width
        readonly property real resolvedContentBottom: applicationWindow.contentItem ? applicationWindow.contentItem.y + applicationWindow.contentItem.height : height
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
    Loader {
        active: applicationWindow.platform === "osx"
        source: applicationWindow.platform === "osx" ? Qt.resolvedUrl("window/MacNativeMenuBar.qml") : ""

        onLoaded: {
            if (item) {
                item.hostWindow = applicationWindow;
                item.resourcesImportController = applicationWindow.rootResourcesImportController;
            }
        }
    }
    Component {
        id: onboardingPageComponent

        Item {
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
            clip: true

            Loader {
                id: workspaceLayoutLoader

                anchors.fill: parent
                sourceComponent: applicationWindow.useIosInlineOnboardingSequence && applicationWindow.onboardingRouteBootstrapController && applicationWindow.onboardingRouteBootstrapController.embeddedOnboardingVisible ? iosInlineOnboardingSequenceComponent : (applicationWindow.adaptiveMobileLayout ? mobileMainLayoutComponent : desktopMainLayoutComponent)

                onSourceComponentChanged: windowInteractions.reportLayoutBranch("workspaceSourceChanged")
            }
        }
    }
    Component {
        id: iosInlineOnboardingSequenceComponent

        WindowView.IosInlineOnboardingSequence {
            anchors.fill: parent
            canvasColor: applicationWindow.canvasColor
            hostWindow: applicationWindow
            hubSessionController: applicationWindow.onboardingHubController

            onDismissRequested: {
                if (applicationWindow.onboardingRouteBootstrapController)
                    applicationWindow.onboardingRouteBootstrapController.dismissEmbeddedOnboarding();
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
                    editorViewModeController: applicationWindow.rootEditorViewModeController
                    navigationModeController: applicationWindow.rootNavigationModeController
                    panelColor: applicationWindow.desktopPanelSurfaceColor
                    panelHeight: applicationWindow.navigationBarHeight
                    sidebarCollapsed: applicationWindow.hideSidebar

                    onToggleDetailPanelRequested: applicationWindow.toggleDetailPanelVisibility()
                    onToggleSidebarRequested: applicationWindow.toggleSidebarVisibility()
                    onAgendaRequested: applicationWindow.openAgendaOverlay(true)
                    onDayCalendarRequested: applicationWindow.openDayCalendarOverlay(true)
                    onMonthCalendarRequested: applicationWindow.openMonthCalendarOverlay(true)
                    onWeekCalendarRequested: applicationWindow.openWeekCalendarOverlay(true)
                    onYearCalendarRequested: applicationWindow.openYearCalendarOverlay(true)
                }
                BodyPanelView.BodyLayout {
                    id: hStack

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    compactCanvasColor: applicationWindow.canvasColor
                    compactMode: false
                    contentsDisplayColor: applicationWindow.desktopPanelSurfaceColor
                    editorViewModeController: applicationWindow.rootEditorViewModeController
                    isMobilePlatform: applicationWindow.isMobilePlatform
                    listViewColor: applicationWindow.desktopPanelSurfaceColor
                    listViewWidth: applicationWindow.listViewWidth
                    libraryHierarchyController: applicationWindow.rootLibraryHierarchyController
                    minContentWidth: applicationWindow.minContentWidth
                    minListViewWidth: applicationWindow.minListViewWidth
                    minRightPanelWidth: applicationWindow.minRightPanelWidth
                    minSidebarWidth: applicationWindow.minSidebarWidth
                    noteDeletionController: applicationWindow.rootLibraryNoteMutationController
                    noteActiveState: applicationWindow.rootNoteActiveState
                    rightPanelColor: applicationWindow.desktopPanelSurfaceColor
                    rightPanelWidth: applicationWindow.rightPanelWidth
                    resourcesImportController: applicationWindow.rootResourcesImportController
                    sidebarColor: applicationWindow.desktopPanelSurfaceColor
                    sidebarHierarchyController: applicationWindow.rootSidebarHierarchyController
                    sidebarHorizontalInset: applicationWindow.hierarchyHorizontalInset
                    sidebarWidth: applicationWindow.sidebarWidth
                    splitterColor: applicationWindow.bodySplitterColor
                    splitterThickness: applicationWindow.bodySplitterThickness
                    agendaOverlayVisible: applicationWindow.agendaOverlayVisible
                    agendaController: applicationWindow.rootAgendaController
                    dayCalendarOverlayVisible: applicationWindow.dayCalendarOverlayVisible
                    dayCalendarController: applicationWindow.rootDayCalendarController
                    monthCalendarOverlayVisible: applicationWindow.monthCalendarOverlayVisible
                    monthCalendarController: applicationWindow.rootMonthCalendarController
                    weekCalendarOverlayVisible: applicationWindow.weekCalendarOverlayVisible
                    weekCalendarController: applicationWindow.rootWeekCalendarController
                    yearCalendarOverlayVisible: applicationWindow.yearCalendarOverlayVisible
                    yearCalendarController: applicationWindow.rootYearCalendarController

                    onNoteActivated: function (index, noteId) {
                        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
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
                    onMonthCalendarOverlayOpenRequested: applicationWindow.openMonthCalendarOverlay(false)
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
            controlSurfaceColor: applicationWindow.mobileControlSurfaceColor
            editorViewModeController: applicationWindow.rootEditorViewModeController
            navigationModeController: applicationWindow.rootNavigationModeController
            noteActiveState: applicationWindow.rootNoteActiveState
            sidebarHierarchyController: applicationWindow.rootSidebarHierarchyController
            statusPlaceholderText: ""
            toolbarIconNames: applicationWindow.hierarchyToolbarIconNames
            resourcesImportController: applicationWindow.rootResourcesImportController
            windowInteractions: windowInteractions
            agendaOverlayVisible: applicationWindow.agendaOverlayVisible
            agendaController: applicationWindow.rootAgendaController
            dayCalendarOverlayVisible: applicationWindow.dayCalendarOverlayVisible
            dayCalendarController: applicationWindow.rootDayCalendarController
            monthCalendarOverlayVisible: applicationWindow.monthCalendarOverlayVisible
            monthCalendarController: applicationWindow.rootMonthCalendarController
            weekCalendarOverlayVisible: applicationWindow.weekCalendarOverlayVisible
            weekCalendarController: applicationWindow.rootWeekCalendarController
            yearCalendarOverlayVisible: applicationWindow.yearCalendarOverlayVisible
            yearCalendarController: applicationWindow.rootYearCalendarController

            onAgendaRequested: applicationWindow.openAgendaOverlay(true)
            onDayCalendarRequested: applicationWindow.openDayCalendarOverlay(true)
            onMonthCalendarRequested: applicationWindow.openMonthCalendarOverlay(true)
            onWeekCalendarRequested: applicationWindow.openWeekCalendarOverlay(true)
            onYearCalendarRequested: applicationWindow.openYearCalendarOverlay(true)
            onAgendaOverlayDismissRequested: applicationWindow.agendaOverlayVisible = false
            onDayCalendarOverlayDismissRequested: applicationWindow.dayCalendarOverlayVisible = false
            onMonthCalendarOverlayOpenRequested: applicationWindow.openMonthCalendarOverlay(false)
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
