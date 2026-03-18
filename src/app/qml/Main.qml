pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQml
import QtQuick.Window
import Qt.labs.platform as Platform
import LVRS 1.0 as LV
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
    readonly property real hierarchyToolbarSpacing: hierarchyToolbarCount > 1 ? 40 / (hierarchyToolbarCount - 1) : 0
    readonly property int hierarchyToolbarTrackWidth: hierarchyToolbarCount > 0 ? Math.round(hierarchyToolbarCount * hierarchyToolbarButtonSize + (hierarchyToolbarCount - 1) * hierarchyToolbarSpacing) : hierarchyToolbarButtonSize
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
    readonly property int onboardingDefaultHeight: onboardingMinHeight
    readonly property int onboardingDefaultWidth: onboardingMinWidth
    property var onboardingHubController: null
    readonly property int onboardingMinHeight: 420
    readonly property int onboardingMinWidth: 620
    property bool onboardingVisible: false
    property int preferredDrawerHeight: baseDrawerHeight
    property int preferredListViewWidth: baseListViewWidth
    property int preferredRightPanelWidth: baseRightPanelWidth
    property int preferredSidebarWidth: baseSidebarWidth
    readonly property var presetHierarchyVm: presetHierarchyViewModel
    readonly property var progressHierarchyVm: progressHierarchyViewModel
    readonly property var projectsHierarchyVm: projectsHierarchyViewModel
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

    autoAttachRuntimeEvents: true
    globalEventListenersEnabled: true
    height: windowDefaultHeight
    internalRouterRegisterAsGlobalNavigator: true
    minimumHeight: windowMinHeight
    minimumWidth: adaptiveMobileLayout ? windowMobileMinWidth : desktopMinimumBodyWidth
    navItems: []
    navigationEnabled: false
    pageInitialPath: workspaceRoutePath
    pageRoutes: [workspaceShellRoute]
    useInternalPageStack: true
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
        if (applicationWindow.onboardingVisible)
            onboardingSubWindow.show();
    }
    onAdaptiveLayoutStateChanged: windowInteractions.reportLayoutBranch("adaptiveLayoutStateChanged")
    onBodyHeightChanged: clampPreferredSizes()
    onHeightChanged: {
        windowInteractions.handleResizeForRenderQuality("heightChanged");
        resizeDebounceTimer.restart();
    }
    onOnboardingVisibleChanged: {
        if (applicationWindow.onboardingVisible) {
            onboardingSubWindow.show();
        } else {
            onboardingSubWindow.close();
        }
    }
    onPageStackNavigated: function (path, params) {
        windowInteractions.reportLayoutBranch("pageStackNavigated");
    }
    onPageStackNavigationFailed: function (path) {
        console.warn("[whatson:debug][main.route][navigationFailed] path=" + path);
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
        navigationModeViewModel: applicationWindow.navigationModeVm
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
    Loader {
        active: applicationWindow.platform === "osx"
        sourceComponent: nativeMenuBarComponent
    }
    Component {
        id: nativeMenuBarComponent

        Platform.MenuBar {
            window: applicationWindow

            Platform.Menu {
                title: qsTr("File")

                Platform.MenuItem {
                    enabled: false
                    text: applicationWindow.nativeMenuPlaceholderText()
                }
            }
            Platform.Menu {
                title: qsTr("Edit")

                Platform.MenuItem {
                    enabled: false
                    text: applicationWindow.nativeMenuPlaceholderText()
                }
            }
            Platform.Menu {
                title: qsTr("View")

                Platform.MenuItem {
                    enabled: false
                    text: applicationWindow.nativeMenuPlaceholderText()
                }
            }
            Platform.Menu {
                title: qsTr("Window")

                Platform.MenuItem {
                    enabled: false
                    text: applicationWindow.nativeMenuPlaceholderText()
                }
            }
            Platform.Menu {
                title: qsTr("Help")

                Platform.MenuItem {
                    enabled: false
                    text: applicationWindow.nativeMenuPlaceholderText()
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
                    compactMode: false
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

        BodyPanelView.MobileNormalLayout {
            anchors.fill: parent
            canvasColor: applicationWindow.canvasColor
            controlSurfaceColor: LV.Theme.panelBackground10
            statusPlaceholderText: ""
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
            applicationWindow.onboardingVisible = false;
            onboardingSubWindow.close();
        }
        onDismissed: {
            applicationWindow.onboardingVisible = false;
        }
        onSelectFileRequested: {
            applicationWindow.onboardingVisible = false;
            onboardingSubWindow.close();
        }
    }
}
