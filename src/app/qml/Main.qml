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
    readonly property int baseDrawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property int baseListViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property int baseRightPanelWidth: 194
    readonly property int baseSidebarWidth: hierarchyToolbarWidth
    readonly property int bodyHeight: Math.max(0, height - adaptiveStatusBarHeight - navigationBarHeight)
    readonly property color bodySplitterColor: LV.Theme.panelBackground10
    readonly property int bodySplitterThickness: Math.max(1, Math.round(LV.Theme.strokeThin))
    readonly property var bookmarksHierarchyVm: bookmarksHierarchyViewModel
    readonly property color canvasColor: LV.Theme.panelBackground01
    readonly property color contentsDisplayColor: canvasColor
    property bool desktopOnboardingWindowVisible: false
    readonly property int desktopMinimumBodyWidth: (hideSidebar ? 0 : minSidebarWidth)
                                               + (hideListView ? 0 : minListViewWidth)
                                               + minContentWidth
                                               + (hideRightPanel ? 0 : minRightPanelWidth)
                                               + bodySplitterThickness * Math.max(0, ((hideSidebar ? 0 : 1)
                                                                                      + (hideListView ? 0 : 1)
                                                                                      + 1
                                                                                      + (hideRightPanel ? 0 : 1)) - 1)
    readonly property color drawerColor: LV.Theme.panelBackground04
    readonly property int drawerHeight: Math.max(minDrawerHeight, Math.min(preferredDrawerHeight, Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness)))
    readonly property var editorViewModeVm: editorViewModeViewModel
    readonly property var eventHierarchyVm: eventHierarchyViewModel
    readonly property bool hideListView: false
    property bool hideRightPanel: false
    property bool hideSidebar: false
    readonly property int hierarchyHorizontalInset: 2
    readonly property int hierarchyToolbarButtonSize: LV.Theme.gap20
    readonly property int hierarchyToolbarCount: hierarchyToolbarIconNames.length
    readonly property var hierarchyToolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property real hierarchyToolbarSpacing: hierarchyToolbarCount > 1 ? 40 / (hierarchyToolbarCount - 1) : 0
    readonly property int hierarchyToolbarTrackWidth: hierarchyToolbarCount > 0 ? Math.round(hierarchyToolbarCount * hierarchyToolbarButtonSize + (hierarchyToolbarCount - 1) * hierarchyToolbarSpacing) : hierarchyToolbarButtonSize
    readonly property int hierarchyToolbarWidth: hierarchyToolbarTrackWidth + hierarchyHorizontalInset * 2
    // Fail-fast binding contract: these context properties must exist from main.cpp.
    readonly property int libraryHierarchyIndex: 0
    readonly property var libraryHierarchyVm: libraryHierarchyViewModel
    readonly property color listViewColor: LV.Theme.panelBackground02
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
    readonly property color mobileSafeAreaBackdropColor: canvasColor
    readonly property color navigationBarColor: canvasColor
    readonly property int navigationBarHeight: LV.Theme.gap24
    readonly property var navigationModeVm: navigationModeViewModel
    readonly property int onboardingDefaultHeight: onboardingMinHeight
    readonly property int onboardingDefaultWidth: onboardingMinWidth
    readonly property var onboardingRoute: ({
            path: applicationWindow.onboardingRoutePath,
            component: onboardingPageComponent
        })
    readonly property string onboardingRoutePath: "/onboarding"
    property var onboardingHubController: null
    property var onboardingRouteBootstrapController: null
    readonly property int onboardingMinHeight: 420
    readonly property int onboardingMinWidth: 620
    property int preferredDrawerHeight: baseDrawerHeight
    property int preferredListViewWidth: baseListViewWidth
    property int preferredRightPanelWidth: baseRightPanelWidth
    property int preferredSidebarWidth: baseSidebarWidth
    readonly property var presetHierarchyVm: presetHierarchyViewModel
    readonly property var progressHierarchyVm: progressHierarchyViewModel
    readonly property var projectsHierarchyVm: projectsHierarchyViewModel
    readonly property var resourcesHierarchyVm: resourcesHierarchyViewModel
    readonly property color rightPanelColor: canvasColor
    readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)
    readonly property color sidebarColor: canvasColor
    readonly property var sidebarHierarchyVm: sidebarHierarchyViewModel
    readonly property int sidebarWidth: hideSidebar ? 0 : Math.max(minSidebarWidth, preferredSidebarWidth)
    readonly property color statusBarColor: canvasColor
    readonly property int statusBarHeight: LV.Theme.controlHeightMd
    readonly property bool onboardingRouteCommitPending: onboardingRouteBootstrapController ? onboardingRouteBootstrapController.routeCommitPending : false
    readonly property string startupRoutePath: onboardingRouteBootstrapController ? onboardingRouteBootstrapController.startupRoutePath : workspaceRoutePath
    readonly property var tagsHierarchyVm: tagsHierarchyViewModel
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

    signal viewHookRequested

    function clampPreferredSizes() {
        preferredSidebarWidth = Math.max(minSidebarWidth, preferredSidebarWidth);
        preferredListViewWidth = Math.max(minListViewWidth, preferredListViewWidth);
        preferredRightPanelWidth = Math.max(minRightPanelWidth, preferredRightPanelWidth);

        var maxDrawerHeight = Math.max(minDrawerHeight, bodyHeight - minDisplayHeight - bodySplitterThickness);
        preferredDrawerHeight = Math.max(minDrawerHeight, Math.min(maxDrawerHeight, preferredDrawerHeight));
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
    forceFullWindowAreaOnMobile: applicationWindow.isMobilePlatform
    forcedDeviceTierPreset: -1
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
        clampPreferredSizes();
        windowInteractions.applyRenderQualityPolicy("completed");
        windowInteractions.reportLayoutBranch("completed");
        if (applicationWindow.useEmbeddedOnboardingRoute) {
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
        windowInteractions.handleResizeForRenderQuality("heightChanged");
        resizeDebounceTimer.restart();
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
        windowInteractions.handleResizeForRenderQuality("widthChanged");
        resizeDebounceTimer.restart();
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
        libraryHierarchyViewModel: applicationWindow.libraryHierarchyVm
        navigationModeViewModel: applicationWindow.navigationModeVm
        panelViewModelRegistry: panelViewModelRegistry
        sidebarHierarchyViewModel: applicationWindow.sidebarHierarchyVm
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
        }
    }
    Connections {
        target: applicationWindow.onboardingHubController

        function onHubLoaded(hubPath) {
            if (!applicationWindow.useEmbeddedOnboardingRoute)
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
    Loader {
        active: applicationWindow.platform === "osx"
        source: applicationWindow.platform === "osx"
            ? Qt.resolvedUrl("window/MacNativeMenuBar.qml")
            : ""

        onLoaded: {
            if (item)
                item.hostWindow = applicationWindow
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
                    panelColor: applicationWindow.statusBarColor
                    panelHeight: applicationWindow.statusBarHeight

                    onWindowMoveRequested: {
                        applicationWindow.requestWindowMove();
                    }
                }
                BodyPanelView.NavigationBarLayout {
                    id: navigationBar

                    compactMode: false
                    detailPanelCollapsed: applicationWindow.hideRightPanel
                    editorViewModeViewModel: applicationWindow.editorViewModeVm
                    navigationModeViewModel: applicationWindow.navigationModeVm
                    panelColor: applicationWindow.navigationBarColor
                    panelHeight: applicationWindow.navigationBarHeight
                    sidebarCollapsed: applicationWindow.hideSidebar

                    onToggleDetailPanelRequested: applicationWindow.toggleDetailPanelVisibility()
                    onToggleSidebarRequested: applicationWindow.toggleSidebarVisibility()
                }
                BodyPanelView.BodyLayout {
                    id: hStack

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    compactCanvasColor: applicationWindow.canvasColor
                    compactMode: false
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
                    noteDeletionViewModel: applicationWindow.libraryHierarchyVm
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
            }
        }
    }
    Component {
        id: mobileMainLayoutComponent

        MobilePageView.MobileHierarchyPage {
            anchors.fill: parent
            canvasColor: applicationWindow.canvasColor
            controlSurfaceColor: "transparent"
            editorViewModeViewModel: applicationWindow.editorViewModeVm
            navigationModeViewModel: applicationWindow.navigationModeVm
            sidebarHierarchyViewModel: applicationWindow.sidebarHierarchyVm
            statusPlaceholderText: ""
            toolbarIconNames: applicationWindow.hierarchyToolbarIconNames
            windowInteractions: windowInteractions
        }
    }
    WindowView.Onboarding {
        id: onboardingSubWindow

        defaultHeight: applicationWindow.onboardingDefaultHeight
        defaultWidth: applicationWindow.onboardingDefaultWidth
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
