pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import ".." as MobileView
import "../../panels" as PanelView

Item {
    id: mobileHierarchyPage

    readonly property var activeNoteListModel: mobileHierarchyPage.sidebarHierarchyViewModel
        ? mobileHierarchyPage.sidebarHierarchyViewModel.resolvedNoteListModel
        : null
    readonly property int activeToolbarIndex: mobileHierarchyPage.sidebarHierarchyViewModel
        ? mobileHierarchyPage.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex
        : 0
    readonly property bool backNavigationAvailable: mobileScaffold.activePageRouter
        ? mobileScaffold.activePageRouter.canGoBack
        : false
    readonly property int backSwipeEdgeWidth: LV.Theme.gap24
    property int backSwipeSessionId: -1
    property color canvasColor: LV.Theme.panelBackground01
    property color controlSurfaceColor: LV.Theme.panelBackground10
    property var editorViewModeViewModel: null
    readonly property string hierarchyRoutePath: "/mobile/hierarchy"
    property string hierarchySearchText: ""
    readonly property var mobileBodyRoutes: [
        {
            "path": mobileHierarchyPage.hierarchyRoutePath,
            "component": hierarchyBodyComponent
        },
        {
            "path": mobileHierarchyPage.noteListRoutePath,
            "component": noteListBodyComponent
        }
    ]
    property var navigationModeViewModel: null
    readonly property string noteListRoutePath: "/mobile/note-list"
    readonly property bool noteListPageActive: mobileScaffold.activePageRouter
        ? String(mobileScaffold.activePageRouter.currentPath) === mobileHierarchyPage.noteListRoutePath
        : false
    required property var sidebarHierarchyViewModel
    property string statusPlaceholderText: ""
    property string statusSearchText: ""
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    property var windowInteractions: null

    signal viewHookRequested

    function backSwipeViewportWidth() {
        return Math.max(1, Math.round(Number(mobileScaffold.bodyWidth) || 0));
    }
    function beginBackSwipeGesture(eventData) {
        if (!mobileHierarchyPage.backNavigationAvailable || !eventData || pageTransitionController.active)
            return false;
        const sessionId = Math.floor(Number(eventData.sessionId) || -1);
        const startGlobalX = Number(eventData.startGlobalX !== undefined ? eventData.startGlobalX : eventData.globalX);
        if (sessionId < 0 || !isFinite(startGlobalX) || !mobileHierarchyPage.isWithinBackSwipeEdge(startGlobalX))
            return false;
        if (!pageTransitionController.beginBack({
                    "sessionId": sessionId,
                    "source": "edge-pan"
                }))
            return false;
        mobileHierarchyPage.backSwipeSessionId = sessionId;
        return true;
    }
    function cancelBackSwipeGesture(eventData) {
        const sessionId = Math.floor(Number(eventData && eventData.sessionId !== undefined ? eventData.sessionId : -1) || -1);
        if (mobileHierarchyPage.backSwipeSessionId < 0)
            return false;
        if (sessionId >= 0 && sessionId !== mobileHierarchyPage.backSwipeSessionId)
            return false;
        mobileHierarchyPage.backSwipeSessionId = -1;
        return pageTransitionController.cancel();
    }
    function isWithinBackSwipeEdge(globalX) {
        const edgeOrigin = mobileHierarchyPage.mapToGlobal(Qt.point(0, 0));
        return globalX >= edgeOrigin.x && globalX <= edgeOrigin.x + mobileHierarchyPage.backSwipeEdgeWidth;
    }
    function requestBackToHierarchy() {
        if (!mobileScaffold.activePageRouter)
            return;
        if (pageTransitionController.active)
            pageTransitionController.cancel();
        mobileHierarchyPage.backSwipeSessionId = -1;
        if (mobileScaffold.activePageRouter.canGoBack) {
            mobileScaffold.activePageRouter.back();
            mobileHierarchyPage.requestViewHook();
            return;
        }
        mobileHierarchyPage.routeToHierarchyRoot();
    }
    function requestCreateFolder() {
        if (mobileScaffold.bodyItem && mobileScaffold.bodyItem.requestCreateFolder !== undefined)
            mobileScaffold.bodyItem.requestCreateFolder();
    }
    function requestCreateNote() {
        mobileHierarchyPage.requestViewHook();
        if (mobileHierarchyPage.windowInteractions && mobileHierarchyPage.windowInteractions.createNoteFromShortcut !== undefined)
            mobileHierarchyPage.windowInteractions.createNoteFromShortcut();
    }
    function requestOpenNoteList(item, itemId, index) {
        if (!mobileHierarchyPage.activeNoteListModel || !mobileScaffold.activePageRouter)
            return;
        if (String(mobileScaffold.activePageRouter.currentPath) === mobileHierarchyPage.noteListRoutePath)
            return;
        mobileScaffold.activePageRouter.push(mobileHierarchyPage.noteListRoutePath);
        mobileHierarchyPage.requestViewHook();
    }
    function requestViewHook() {
        viewHookRequested();
    }
    function routeToHierarchyRoot() {
        if (!mobileScaffold.activePageRouter)
            return false;
        if (pageTransitionController.active)
            pageTransitionController.cancel();
        mobileHierarchyPage.backSwipeSessionId = -1;
        if (String(mobileScaffold.activePageRouter.currentPath) === mobileHierarchyPage.hierarchyRoutePath
                && !mobileScaffold.activePageRouter.canGoBack)
            return false;
        mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);
        mobileHierarchyPage.requestViewHook();
        return true;
    }
    function finishBackSwipeGesture(eventData, cancelled) {
        const sessionId = Math.floor(Number(eventData && eventData.sessionId !== undefined ? eventData.sessionId : -1) || -1);
        if (mobileHierarchyPage.backSwipeSessionId < 0)
            return false;
        if (sessionId >= 0 && sessionId !== mobileHierarchyPage.backSwipeSessionId)
            return false;
        mobileHierarchyPage.backSwipeSessionId = -1;
        if (!pageTransitionController.active)
            return false;
        if (cancelled)
            return pageTransitionController.cancel();
        const velocityX = Number(eventData && eventData.velocityX !== undefined ? eventData.velocityX : 0) || 0;
        const velocityY = Number(eventData && eventData.velocityY !== undefined ? eventData.velocityY : 0) || 0;
        const shouldCommit = pageTransitionController.shouldCommit(
            pageTransitionController.progress,
            velocityX,
            velocityY);
        return pageTransitionController.finish(shouldCommit);
    }
    function updateBackSwipeGesture(eventData) {
        if (!eventData)
            return false;
        if (mobileHierarchyPage.backSwipeSessionId < 0 && !mobileHierarchyPage.beginBackSwipeGesture(eventData))
            return false;
        const sessionId = Math.floor(Number(eventData.sessionId) || -1);
        if (sessionId !== mobileHierarchyPage.backSwipeSessionId || !pageTransitionController.active)
            return false;
        const absoluteDeltaX = Math.abs(Number(eventData.totalDeltaX) || 0);
        const absoluteDeltaY = Math.abs(Number(eventData.totalDeltaY) || 0);
        if (absoluteDeltaY > absoluteDeltaX && absoluteDeltaY >= LV.Theme.gap8)
            return mobileHierarchyPage.cancelBackSwipeGesture(eventData);
        const progress = Math.max(
            0,
            Math.min(1, (Number(eventData.totalDeltaX) || 0) / mobileHierarchyPage.backSwipeViewportWidth()));
        return pageTransitionController.update(progress, {
                    "velocityX": Number(eventData.velocityX) || 0,
                    "velocityY": Number(eventData.velocityY) || 0
                });
    }

    onActiveNoteListModelChanged: {
        if (mobileHierarchyPage.activeNoteListModel)
            return;
        mobileHierarchyPage.routeToHierarchyRoot();
    }

    MobileView.MobilePageScaffold {
        id: mobileScaffold

        anchors.fill: parent
        bodyInitialPath: mobileHierarchyPage.hierarchyRoutePath
        bodyRoutes: mobileHierarchyPage.mobileBodyRoutes
        canvasColor: mobileHierarchyPage.canvasColor
        compactAddFolderVisible: !mobileHierarchyPage.noteListPageActive
        compactLeadingActionVisible: false
        controlSurfaceColor: mobileHierarchyPage.controlSurfaceColor
        editorViewModeViewModel: mobileHierarchyPage.editorViewModeViewModel
        navigationModeViewModel: mobileHierarchyPage.navigationModeViewModel
        statusPlaceholderText: mobileHierarchyPage.statusPlaceholderText
        statusSearchText: mobileHierarchyPage.statusSearchText
        windowInteractions: mobileHierarchyPage.windowInteractions

        onCompactAddFolderRequested: mobileHierarchyPage.requestCreateFolder()
        onCompactLeadingActionRequested: mobileHierarchyPage.requestBackToHierarchy()
        onCreateNoteRequested: mobileHierarchyPage.requestCreateNote()
        onStatusSearchSubmitted: function (text) {
            mobileHierarchyPage.statusSearchText = text;
        }
        onStatusSearchTextEdited: function (text) {
            mobileHierarchyPage.statusSearchText = text;
        }
        onViewHookRequested: mobileHierarchyPage.requestViewHook()
    }
    LV.PageTransitionController {
        id: pageTransitionController

        router: mobileScaffold.activePageRouter

        onCommitted: mobileHierarchyPage.requestViewHook()
    }
    Item {
        id: backSwipeEdgeZone

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        visible: mobileHierarchyPage.backNavigationAvailable
        width: visible ? mobileHierarchyPage.backSwipeEdgeWidth : 0
        z: 3

        LV.EventListener {
            enabled: backSwipeEdgeZone.visible
            trigger: "touchStarted"
            action: function (eventData) {
                mobileHierarchyPage.beginBackSwipeGesture(eventData);
            }
        }
        LV.EventListener {
            enabled: backSwipeEdgeZone.visible
            trigger: "touchUpdated"
            action: function (eventData) {
                mobileHierarchyPage.updateBackSwipeGesture(eventData);
            }
        }
        LV.EventListener {
            enabled: backSwipeEdgeZone.visible
            trigger: "touchEnded"
            action: function (eventData) {
                mobileHierarchyPage.finishBackSwipeGesture(eventData, false);
            }
        }
        LV.EventListener {
            enabled: backSwipeEdgeZone.visible
            trigger: "touchCancelled"
            action: function (eventData) {
                mobileHierarchyPage.finishBackSwipeGesture(eventData, true);
            }
        }
    }
    Component {
        id: hierarchyBodyComponent

        PanelView.HierarchySidebarLayout {
            footerVisible: false
            horizontalInset: LV.Theme.gapNone
            panelColor: mobileHierarchyPage.canvasColor
            searchFieldVisible: true
            searchHeaderHorizontalInset: LV.Theme.gapNone
            searchHeaderMinHeight: LV.Theme.gap18
            searchHeaderTopGap: LV.Theme.gap2
            searchHeaderVerticalInset: LV.Theme.gapNone
            searchListGap: LV.Theme.gap2
            searchText: mobileHierarchyPage.hierarchySearchText
            sidebarHierarchyViewModel: mobileHierarchyPage.sidebarHierarchyViewModel
            toolbarFrameWidth: width
            toolbarIconNames: mobileHierarchyPage.toolbarIconNames
            verticalInset: LV.Theme.gapNone

            onHierarchyItemActivated: function (item, itemId, index) {
                mobileHierarchyPage.requestOpenNoteList(item, itemId, index);
            }
            onSearchSubmitted: function (text) {
                mobileHierarchyPage.hierarchySearchText = text;
            }
            onSearchTextEdited: function (text) {
                mobileHierarchyPage.hierarchySearchText = text;
            }
        }
    }
    Component {
        id: noteListBodyComponent

        PanelView.ListBarLayout {
            activeToolbarIndex: mobileHierarchyPage.activeToolbarIndex
            headerVisible: false
            noteListModel: mobileHierarchyPage.activeNoteListModel
            panelColor: mobileHierarchyPage.canvasColor
            searchText: mobileHierarchyPage.statusSearchText
        }
    }
}
