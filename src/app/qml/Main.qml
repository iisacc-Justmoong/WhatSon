pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQml
import QtQuick.Window
import Qt.labs.platform as Platform
import LVRS 1.0 as LV
import "view/panels" as BodyPanelView

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
    readonly property var focusRetainedUiTokens: ["button", "combobox", "checkbox", "radiobutton", "switch", "slider", "spinbox", "dial", "textinput", "textedit", "inputfield", "editor", "menu", "popup", "tooltip", "mousearea", "taphandler", "flickable", "listview", "scrollview", "tableview", "scrollbar", "hierarchy", "contextmenu", "itemdelegate", "notelistitem"]
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
    readonly property string workspaceRoutePath: "/"
    readonly property var workspaceShellRoute: ({
            path: applicationWindow.workspaceRoutePath,
            component: workspacePageComponent
        })

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
    function clearActiveFocus(reason) {
        let current = applicationWindow.activeFocusItem;
        let depthGuard = 0;

        while (current && depthGuard < 32) {
            if (current.focus !== undefined)
                current.focus = false;
            current = current.parent;
            depthGuard += 1;
        }
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
    function nativeMenuPlaceholderText() {
        return " ";
    }
    function reportLayoutBranch(source) {
        const currentPath = activePageRouter && activePageRouter.currentPath !== undefined ? String(activePageRouter.currentPath) : "<none>";
        console.log("[whatson:debug][main.layout][" + source + "] platform=" + platform + " adaptiveLayoutProfile=" + adaptiveLayoutProfile + " adaptiveNavigationMode=" + adaptiveNavigationMode + " adaptiveMobileLayout=" + adaptiveMobileLayout + " adaptiveDesktopLayout=" + adaptiveDesktopLayout + " currentPath=" + currentPath);
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

        for (let i = 0; i < applicationWindow.focusRetainedUiTokens.length; ++i) {
            const token = String(applicationWindow.focusRetainedUiTokens[i]).toLowerCase();
            if (token.length > 0 && searchable.indexOf(token) >= 0)
                return true;
        }

        if (text.length > 0 || label.length > 0 || title.length > 0)
            return true;

        return !(className.indexOf("rectangle") >= 0 || (className.indexOf("item") >= 0 && objectName === "unnamed"));
    }

    autoAttachRuntimeEvents: true
    globalEventListenersEnabled: true
    height: windowDefaultHeight
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
        applyRenderQualityPolicy("completed");
        reportLayoutBranch("completed");
        if (applicationWindow.onboardingVisible)
            onboardingSubWindow.show();
    }
    onAdaptiveLayoutStateChanged: reportLayoutBranch("adaptiveLayoutStateChanged")
    onBodyHeightChanged: clampPreferredSizes()
    onHeightChanged: handleResizeForRenderQuality("heightChanged")
    onPageStackNavigated: function (path, params) {
        applicationWindow.reportLayoutBranch("pageStackNavigated");
    }
    onPageStackNavigationFailed: function (path) {
        console.warn("[whatson:debug][main.route][navigationFailed] path=" + path);
    }
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
    LV.EventListener {
        action: function (eventData) {
            if (!applicationWindow.activeFocusItem)
                return;
            if (!eventData || (eventData.buttons & Qt.LeftButton) !== Qt.LeftButton)
                return;
            if (applicationWindow.shouldRetainFocusForUiHit(eventData.ui))
                return;
            applicationWindow.clearActiveFocus("blankGlobalPress");
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

                onSourceComponentChanged: applicationWindow.reportLayoutBranch("workspaceSourceChanged")
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
