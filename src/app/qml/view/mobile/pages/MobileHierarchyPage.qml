pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import ".." as MobileView
import "../../panels" as PanelView

Item {
    id: mobileHierarchyPage

    readonly property var activeContentViewModel: mobileHierarchyPage.sidebarHierarchyViewModel
        ? mobileHierarchyPage.sidebarHierarchyViewModel.resolvedHierarchyViewModel
        : null
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
    property int backSwipeConsumedSessionId: -1
    property bool backSwipeDragCanceled: false
    property int backSwipeGeneratedSessionId: 0
    property int editorPopRepairRequestId: 0
    property int backSwipeSessionId: -1
    property color canvasColor: LV.Theme.panelBackground01
    property color controlSurfaceColor: LV.Theme.panelBackground10
    readonly property string editorRoutePath: "/mobile/editor"
    property var editorViewModeViewModel: null
    readonly property string hierarchyRoutePath: "/mobile/hierarchy"
    property string hierarchySearchText: ""
    property string lastObservedRoutePath: hierarchyRoutePath
    readonly property var libraryNoteCreationViewModel: mobileHierarchyPage.windowInteractions
        && mobileHierarchyPage.windowInteractions.libraryHierarchyViewModel !== undefined
        ? mobileHierarchyPage.windowInteractions.libraryHierarchyViewModel
        : null
    readonly property var mobileBodyRoutes: [
        {
            "path": mobileHierarchyPage.hierarchyRoutePath,
            "component": hierarchyBodyComponent
        },
        {
            "path": mobileHierarchyPage.noteListRoutePath,
            "component": noteListBodyComponent
        },
        {
            "path": mobileHierarchyPage.editorRoutePath,
            "component": editorBodyComponent
        }
    ]
    property var navigationModeViewModel: null
    readonly property string noteListRoutePath: "/mobile/note-list"
    readonly property string resolvedBodyRoutePath: mobileHierarchyPage.displayedBodyRoutePath()
    readonly property bool editorPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.editorRoutePath
    readonly property bool noteListPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.noteListRoutePath
    property string pendingCreatedNoteId: ""
    required property var sidebarHierarchyViewModel
    property string statusPlaceholderText: ""
    property string statusSearchText: ""
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    property var windowInteractions: null

    signal viewHookRequested

    function backSwipeViewportWidth() {
        return Math.max(1, Math.round(Number(mobileScaffold.bodyWidth) || 0));
    }
    function backSwipeGestureEventData(localX, localY, totalDeltaX, totalDeltaY, sessionId) {
        const localPoint = Qt.point(Number(localX) || 0, Number(localY) || 0);
        const globalPoint = backSwipeEdgeZone.mapToGlobal(localPoint);
        return {
            "globalX": globalPoint.x,
            "globalY": globalPoint.y,
            "sessionId": Math.floor(Number(sessionId) || -1),
            "startGlobalX": globalPoint.x - (Number(totalDeltaX) || 0),
            "startGlobalY": globalPoint.y - (Number(totalDeltaY) || 0),
            "totalDeltaX": Number(totalDeltaX) || 0,
            "totalDeltaY": Number(totalDeltaY) || 0,
            "velocityX": 0,
            "velocityY": 0
        };
    }
    function clearActiveHierarchySelection() {
        if (!mobileHierarchyPage.activeContentViewModel
                || mobileHierarchyPage.activeContentViewModel.setSelectedIndex === undefined)
            return false;
        mobileHierarchyPage.activeContentViewModel.setSelectedIndex(-1);
        return true;
    }
    function displayedBodyRoutePath() {
        const bodyItem = mobileScaffold.bodyItem;
        if (bodyItem) {
            if (bodyItem.contentViewModel !== undefined)
                return mobileHierarchyPage.editorRoutePath;
            if (bodyItem.noteListModel !== undefined)
                return mobileHierarchyPage.noteListRoutePath;
            if (bodyItem.sidebarHierarchyViewModel !== undefined)
                return mobileHierarchyPage.hierarchyRoutePath;
        }
        if (!mobileScaffold.activePageRouter)
            return "";
        return String(mobileScaffold.activePageRouter.currentPath);
    }
    function beginBackSwipeGesture(eventData) {
        if (!mobileHierarchyPage.backNavigationAvailable || !eventData || pageTransitionController.active)
            return false;
        const sessionId = Math.floor(Number(eventData.sessionId) || -1);
        const startGlobalX = Number(eventData.startGlobalX !== undefined ? eventData.startGlobalX : eventData.globalX);
        if (sessionId < 0
                || sessionId === mobileHierarchyPage.backSwipeConsumedSessionId
                || !isFinite(startGlobalX)
                || !mobileHierarchyPage.isWithinBackSwipeEdge(startGlobalX))
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
        if (sessionId >= 0)
            mobileHierarchyPage.backSwipeConsumedSessionId = sessionId;
        return pageTransitionController.cancel();
    }
    function isWithinBackSwipeEdge(globalX) {
        const edgeOrigin = mobileHierarchyPage.mapToGlobal(Qt.point(0, 0));
        return globalX >= edgeOrigin.x && globalX <= edgeOrigin.x + mobileHierarchyPage.backSwipeEdgeWidth;
    }
    function resetBackSwipeState() {
        mobileHierarchyPage.backSwipeConsumedSessionId = -1;
        mobileHierarchyPage.backSwipeSessionId = -1;
    }
    function cancelPendingEditorPopRepair() {
        mobileHierarchyPage.editorPopRepairRequestId += 1;
    }
    function routeStackDepth() {
        if (!mobileScaffold.activePageRouter || mobileScaffold.activePageRouter.depth === undefined)
            return 0;
        return Math.max(0, Math.floor(Number(mobileScaffold.activePageRouter.depth) || 0));
    }
    function routeToCanonicalNoteList() {
        if (!mobileScaffold.activePageRouter || !mobileHierarchyPage.activeNoteListModel)
            return false;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        if (pageTransitionController.active)
            pageTransitionController.cancel();
        mobileHierarchyPage.resetBackSwipeState();
        mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);
        mobileScaffold.activePageRouter.push(mobileHierarchyPage.noteListRoutePath);
        mobileHierarchyPage.requestViewHook();
        return true;
    }
    function routeToCanonicalEditor() {
        if (!mobileScaffold.activePageRouter || !mobileHierarchyPage.activeNoteListModel)
            return false;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        if (pageTransitionController.active)
            pageTransitionController.cancel();
        mobileHierarchyPage.resetBackSwipeState();
        mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);
        mobileScaffold.activePageRouter.push(mobileHierarchyPage.noteListRoutePath);
        mobileScaffold.activePageRouter.push(mobileHierarchyPage.editorRoutePath);
        mobileHierarchyPage.requestViewHook();
        return true;
    }
    function verifyCommittedEditorPopState(requestId, attemptsRemaining) {
        if (requestId !== mobileHierarchyPage.editorPopRepairRequestId
                || !mobileScaffold.activePageRouter
                || !mobileHierarchyPage.activeNoteListModel)
            return false;
        const currentPath = String(mobileScaffold.activePageRouter.currentPath);
        const displayedPath = mobileHierarchyPage.displayedBodyRoutePath();
        if (currentPath === mobileHierarchyPage.noteListRoutePath
                && displayedPath === mobileHierarchyPage.noteListRoutePath)
            return true;
        if (attemptsRemaining > 0 && currentPath === mobileHierarchyPage.noteListRoutePath) {
            Qt.callLater(function () {
                mobileHierarchyPage.verifyCommittedEditorPopState(requestId, attemptsRemaining - 1);
            });
            return false;
        }
        mobileHierarchyPage.routeToCanonicalNoteList();
        return false;
    }
    function handleCommittedRouteTransition(state) {
        mobileHierarchyPage.requestViewHook();
        const transitionState = state || ({});
        const operation = transitionState.operation !== undefined ? String(transitionState.operation) : "";
        const fromPath = transitionState.fromPath !== undefined ? String(transitionState.fromPath) : "";
        const toPath = transitionState.toPath !== undefined ? String(transitionState.toPath) : "";
        if (operation !== "pop" || fromPath !== mobileHierarchyPage.editorRoutePath || !mobileHierarchyPage.activeNoteListModel)
            return;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        const repairRequestId = mobileHierarchyPage.editorPopRepairRequestId;
        Qt.callLater(function () {
            if (toPath.length > 0 && toPath !== mobileHierarchyPage.noteListRoutePath) {
                mobileHierarchyPage.routeToCanonicalNoteList();
                return;
            }
            mobileHierarchyPage.verifyCommittedEditorPopState(repairRequestId, 2);
        });
    }
    function requestBackToHierarchy() {
        if (!mobileScaffold.activePageRouter)
            return;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        if (pageTransitionController.active)
            pageTransitionController.cancel();
        mobileHierarchyPage.resetBackSwipeState();
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
    function routePendingCreatedNoteToEditor() {
        const pendingNoteId = mobileHierarchyPage.pendingCreatedNoteId === undefined || mobileHierarchyPage.pendingCreatedNoteId === null
                ? ""
                : String(mobileHierarchyPage.pendingCreatedNoteId).trim();
        if (pendingNoteId.length === 0
                || !mobileHierarchyPage.activeContentViewModel
                || !mobileHierarchyPage.activeNoteListModel
                || !mobileScaffold.activePageRouter)
            return false;
        mobileHierarchyPage.pendingCreatedNoteId = "";
        mobileHierarchyPage.requestOpenEditor(pendingNoteId, -1);
        return true;
    }
    function scheduleCreatedNoteEditorRoute(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length === 0)
            return false;
        mobileHierarchyPage.pendingCreatedNoteId = normalizedNoteId;
        Qt.callLater(function () {
            mobileHierarchyPage.routePendingCreatedNoteToEditor();
        });
        return true;
    }
    function requestOpenNoteList(item, itemId, index) {
        if (!mobileHierarchyPage.activeNoteListModel || !mobileScaffold.activePageRouter)
            return;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        const currentPath = String(mobileScaffold.activePageRouter.currentPath);
        const displayedPath = mobileHierarchyPage.displayedBodyRoutePath();
        const depth = mobileHierarchyPage.routeStackDepth();
        if (currentPath === mobileHierarchyPage.noteListRoutePath
                && displayedPath === mobileHierarchyPage.noteListRoutePath
                && depth >= 2)
            return;
        if (currentPath === mobileHierarchyPage.hierarchyRoutePath
                && displayedPath === mobileHierarchyPage.hierarchyRoutePath
                && depth <= 1) {
            mobileHierarchyPage.resetBackSwipeState();
            mobileScaffold.activePageRouter.push(mobileHierarchyPage.noteListRoutePath);
            mobileHierarchyPage.requestViewHook();
            return;
        }
        mobileHierarchyPage.routeToCanonicalNoteList();
    }
    function requestOpenEditor(noteId, index) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length === 0 || !mobileHierarchyPage.activeContentViewModel || !mobileHierarchyPage.activeNoteListModel || !mobileScaffold.activePageRouter)
            return;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        const currentPath = String(mobileScaffold.activePageRouter.currentPath);
        const displayedPath = mobileHierarchyPage.displayedBodyRoutePath();
        const depth = mobileHierarchyPage.routeStackDepth();
        if (currentPath === mobileHierarchyPage.editorRoutePath
                && displayedPath === mobileHierarchyPage.editorRoutePath
                && depth >= 3)
            return;
        if (currentPath === mobileHierarchyPage.noteListRoutePath
                && displayedPath === mobileHierarchyPage.noteListRoutePath
                && depth >= 2) {
            mobileHierarchyPage.resetBackSwipeState();
            mobileScaffold.activePageRouter.push(mobileHierarchyPage.editorRoutePath);
            mobileHierarchyPage.requestViewHook();
            return;
        }
        mobileHierarchyPage.routeToCanonicalEditor();
    }
    function requestViewHook() {
        viewHookRequested();
    }
    function routeToHierarchyRoot() {
        if (!mobileScaffold.activePageRouter)
            return false;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        if (pageTransitionController.active)
            pageTransitionController.cancel();
        mobileHierarchyPage.resetBackSwipeState();
        if (mobileHierarchyPage.displayedBodyRoutePath() === mobileHierarchyPage.hierarchyRoutePath
                && !mobileScaffold.activePageRouter.canGoBack)
            return false;
        mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);
        mobileHierarchyPage.requestViewHook();
        return true;
    }
    function syncRouteSelectionState() {
        if (!mobileScaffold.activePageRouter)
            return false;
        const currentPath = mobileHierarchyPage.displayedBodyRoutePath();
        const previousPath = mobileHierarchyPage.lastObservedRoutePath;
        mobileHierarchyPage.lastObservedRoutePath = currentPath;
        if (currentPath !== mobileHierarchyPage.hierarchyRoutePath
                || previousPath === mobileHierarchyPage.hierarchyRoutePath)
            return false;
        return mobileHierarchyPage.clearActiveHierarchySelection();
    }
    function finishBackSwipeGesture(eventData, cancelled) {
        const sessionId = Math.floor(Number(eventData && eventData.sessionId !== undefined ? eventData.sessionId : -1) || -1);
        if (mobileHierarchyPage.backSwipeSessionId < 0)
            return false;
        if (sessionId >= 0 && sessionId !== mobileHierarchyPage.backSwipeSessionId)
            return false;
        mobileHierarchyPage.backSwipeSessionId = -1;
        if (sessionId >= 0)
            mobileHierarchyPage.backSwipeConsumedSessionId = sessionId;
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
        const sessionId = Math.floor(Number(eventData.sessionId) || -1);
        if (sessionId < 0 || sessionId === mobileHierarchyPage.backSwipeConsumedSessionId)
            return false;
        if (mobileHierarchyPage.backSwipeSessionId < 0)
            return false;
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

    onActiveContentViewModelChanged: mobileHierarchyPage.routePendingCreatedNoteToEditor()
    onActiveNoteListModelChanged: {
        if (mobileHierarchyPage.activeNoteListModel) {
            mobileHierarchyPage.routePendingCreatedNoteToEditor();
            return;
        }
        mobileHierarchyPage.routeToHierarchyRoot();
    }

    Connections {
        target: mobileHierarchyPage.libraryNoteCreationViewModel
        ignoreUnknownSignals: true

        function onEmptyNoteCreated(noteId) {
            mobileHierarchyPage.scheduleCreatedNoteEditorRoute(noteId);
        }
    }
    Connections {
        target: mobileScaffold.activePageRouter
        ignoreUnknownSignals: true

        function onCurrentPathChanged() {
            mobileHierarchyPage.syncRouteSelectionState();
        }
    }

    MobileView.MobilePageScaffold {
        id: mobileScaffold

        anchors.fill: parent
        bodyInitialPath: mobileHierarchyPage.hierarchyRoutePath
        bodyRoutes: mobileHierarchyPage.mobileBodyRoutes
        canvasColor: mobileHierarchyPage.canvasColor
        compactAddFolderVisible: !mobileHierarchyPage.noteListPageActive && !mobileHierarchyPage.editorPageActive
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

        onCommitted: function (state) {
            mobileHierarchyPage.handleCommittedRouteTransition(state);
        }
    }
    Item {
        id: backSwipeEdgeZone

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        visible: mobileHierarchyPage.backNavigationAvailable
        width: visible ? mobileHierarchyPage.backSwipeEdgeWidth : 0
        z: 3

        DragHandler {
            id: backSwipeDragHandler

            acceptedDevices: PointerDevice.TouchScreen
            dragThreshold: 4
            enabled: backSwipeEdgeZone.visible
            grabPermissions: PointerHandler.CanTakeOverFromAnything
            target: null

            onActiveChanged: {
                if (active) {
                    mobileHierarchyPage.backSwipeDragCanceled = false;
                    mobileHierarchyPage.backSwipeGeneratedSessionId += 1;
                    const beginEventData = mobileHierarchyPage.backSwipeGestureEventData(
                                backSwipeDragHandler.centroid.pressPosition.x,
                                backSwipeDragHandler.centroid.pressPosition.y,
                                0,
                                0,
                                mobileHierarchyPage.backSwipeGeneratedSessionId);
                    if (!mobileHierarchyPage.beginBackSwipeGesture(beginEventData))
                        return;
                    mobileHierarchyPage.updateBackSwipeGesture(
                                mobileHierarchyPage.backSwipeGestureEventData(
                                    backSwipeDragHandler.centroid.position.x,
                                    backSwipeDragHandler.centroid.position.y,
                                    Number(backSwipeDragHandler.centroid.position.x) - Number(backSwipeDragHandler.centroid.pressPosition.x),
                                    Number(backSwipeDragHandler.centroid.position.y) - Number(backSwipeDragHandler.centroid.pressPosition.y),
                                    mobileHierarchyPage.backSwipeSessionId));
                    return;
                }
                if (mobileHierarchyPage.backSwipeSessionId < 0)
                    return;
                mobileHierarchyPage.finishBackSwipeGesture(
                            mobileHierarchyPage.backSwipeGestureEventData(
                                backSwipeDragHandler.centroid.position.x,
                                backSwipeDragHandler.centroid.position.y,
                                Number(backSwipeDragHandler.centroid.position.x) - Number(backSwipeDragHandler.centroid.pressPosition.x),
                                Number(backSwipeDragHandler.centroid.position.y) - Number(backSwipeDragHandler.centroid.pressPosition.y),
                                mobileHierarchyPage.backSwipeSessionId),
                            mobileHierarchyPage.backSwipeDragCanceled);
                mobileHierarchyPage.backSwipeDragCanceled = false;
            }
            onCentroidChanged: {
                if (!active)
                    return;
                mobileHierarchyPage.updateBackSwipeGesture(
                            mobileHierarchyPage.backSwipeGestureEventData(
                                backSwipeDragHandler.centroid.position.x,
                                backSwipeDragHandler.centroid.position.y,
                                Number(backSwipeDragHandler.centroid.position.x) - Number(backSwipeDragHandler.centroid.pressPosition.x),
                                Number(backSwipeDragHandler.centroid.position.y) - Number(backSwipeDragHandler.centroid.pressPosition.y),
                                mobileHierarchyPage.backSwipeSessionId));
            }
            onCanceled: {
                mobileHierarchyPage.backSwipeDragCanceled = true;
                mobileHierarchyPage.finishBackSwipeGesture(
                            mobileHierarchyPage.backSwipeGestureEventData(
                                backSwipeDragHandler.centroid.position.x,
                                backSwipeDragHandler.centroid.position.y,
                                Number(backSwipeDragHandler.centroid.position.x) - Number(backSwipeDragHandler.centroid.pressPosition.x),
                                Number(backSwipeDragHandler.centroid.position.y) - Number(backSwipeDragHandler.centroid.pressPosition.y),
                                mobileHierarchyPage.backSwipeSessionId),
                            true);
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

            onNoteActivated: function (index, noteId) {
                mobileHierarchyPage.requestOpenEditor(noteId, index);
            }
            onViewHookRequested: mobileHierarchyPage.requestViewHook()
        }
    }
    Component {
        id: editorBodyComponent

        PanelView.ContentViewLayout {
            contentViewModel: mobileHierarchyPage.activeContentViewModel
            displayColor: mobileHierarchyPage.canvasColor
            drawerVisible: false
            frameHorizontalInsetOverride: LV.Theme.gapNone
            gutterColor: "transparent"
            gutterWidthOverride: LV.Theme.gap20 * 2
            libraryHierarchyViewModel: mobileHierarchyPage.libraryNoteCreationViewModel
            lineNumberColumnLeftOverride: 14
            lineNumberColumnTextWidthOverride: LV.Theme.gap20 + LV.Theme.gap2
            minimapVisible: false
            noteListModel: mobileHierarchyPage.activeNoteListModel

            onViewHookRequested: mobileHierarchyPage.requestViewHook()
        }
    }
}
