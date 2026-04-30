pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import ".." as MobileView
import "../../panels" as PanelView

Item {
    id: mobileHierarchyPage

    property var activeHierarchyBindingSnapshot: selectionCoordinator.activeHierarchyBindingSnapshot
    readonly property var activeContentController: mobileHierarchyPage.activeHierarchyBindingSnapshot ? mobileHierarchyPage.activeHierarchyBindingSnapshot.controller : null
    readonly property var activeNoteListModel: mobileHierarchyPage.sidebarHierarchyController ? mobileHierarchyPage.sidebarHierarchyController.activeNoteListModel : null
    readonly property int activeToolbarIndex: {
        const snapshot = mobileHierarchyPage.activeHierarchyBindingSnapshot;
        const numericIndex = Number(snapshot && snapshot.index !== undefined ? snapshot.index : 0);
        return isFinite(numericIndex) ? Math.floor(numericIndex) : 0;
    }
    readonly property bool backNavigationAvailable: mobileScaffold.activePageRouter ? mobileScaffold.activePageRouter.canGoBack : false
    readonly property int backSwipeEdgeWidth: LV.Theme.gap24
    property bool backSwipeDragCanceled: false
    property color canvasColor: LV.Theme.panelBackground01
    property color controlSurfaceColor: LV.Theme.panelBackground10
    readonly property string detailRoutePath: "/mobile/detail"
    readonly property string editorRoutePath: "/mobile/editor"
    property var editorViewModeController: null
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
        },
        {
            "path": mobileHierarchyPage.editorRoutePath,
            "component": editorBodyComponent
        },
        {
            "path": mobileHierarchyPage.detailRoutePath,
            "component": detailBodyComponent
        }
    ]
    property var navigationModeController: null
    readonly property string noteListRoutePath: "/mobile/note-list"
    readonly property string resolvedBodyRoutePath: mobileHierarchyPage.displayedBodyRoutePath()
    readonly property bool detailPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.detailRoutePath
    readonly property bool editorPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.editorRoutePath
    readonly property bool hierarchyPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.hierarchyRoutePath
    readonly property bool noteListPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.noteListRoutePath
    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("mobile.MobileHierarchyPage") : null
    property var resourcesImportController: null
    required property var sidebarHierarchyController
    property string statusPlaceholderText: ""
    property string statusSearchText: ""
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    property var windowInteractions: null
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarController: null
    property bool agendaOverlayVisible: false
    property var agendaController: null
    property bool monthCalendarOverlayVisible: false
    property var monthCalendarController: null
    property var noteDeletionController: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarController: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarController: null
    readonly property var resolvedNoteDeletionController: {
        const activeController = mobileHierarchyPage.activeContentController;
        if (activeController && (activeController.deleteNotesByIds !== undefined || activeController.deleteNoteById !== undefined || activeController.clearNoteFoldersByIds !== undefined || activeController.clearNoteFoldersById !== undefined)) {
            return activeController;
        }
        if (mobileHierarchyPage.noteDeletionController)
            return mobileHierarchyPage.noteDeletionController;
        if (mobileHierarchyPage.windowInteractions && mobileHierarchyPage.windowInteractions.resolveLibraryNoteCreationController !== undefined)
            return mobileHierarchyPage.windowInteractions.resolveLibraryNoteCreationController();
        return null;
    }
    signal agendaRequested
    signal agendaOverlayDismissRequested
    signal dayCalendarRequested
    signal dayCalendarOverlayDismissRequested
    signal monthCalendarRequested
    signal monthCalendarOverlayOpenRequested
    signal monthCalendarOverlayDismissRequested
    signal viewHookRequested
    signal weekCalendarRequested
    signal weekCalendarOverlayDismissRequested
    signal yearCalendarRequested
    signal yearCalendarOverlayDismissRequested

    function backSwipeViewportWidth() {
        return Math.max(1, Math.round(Number(mobileScaffold.bodyWidth) || 0));
    }
    function syncActiveHierarchyBindings() {
        selectionCoordinator.activeHierarchyBindingSnapshot = selectionCoordinator.activeHierarchyBindingSnapshotFromSidebar(mobileHierarchyPage.sidebarHierarchyController);
    }
    function normalizedInteger(value, fallbackValue) {
        return routeStateStore.normalizedInteger(value, fallbackValue);
    }
    function backSwipeGestureEventData(localX, localY, totalDeltaX, totalDeltaY, sessionId) {
        const localPoint = Qt.point(Number(localX) || 0, Number(localY) || 0);
        const globalPoint = backSwipeEdgeZone.mapToGlobal(localPoint);
        return backSwipeCoordinator.gestureEventData(Number(localX) || 0, Number(localY) || 0, Number(totalDeltaX) || 0, Number(totalDeltaY) || 0, routeStateStore.normalizedInteger(sessionId, -1), Number(globalPoint.x) || 0, Number(globalPoint.y) || 0);
    }
    function clearActiveHierarchySelection() {
        if (!mobileHierarchyPage.activeContentController || mobileHierarchyPage.activeContentController.setHierarchySelectedIndex === undefined)
            return false;
        mobileHierarchyPage.activeContentController.setHierarchySelectedIndex(-1);
        return true;
    }
    function currentHierarchySelectionIndex() {
        return selectionCoordinator.currentHierarchySelectionIndex(mobileHierarchyPage.activeContentController, routeStateStore.preservedNoteListSelectionIndex);
    }
    function displayedBodyRoutePath() {
        return navigationCoordinator.displayedBodyRoutePath(mobileScaffold.bodyItem, mobileScaffold.activePageRouter);
    }
    function requestOpenDetailPanelPage() {
        const plan = navigationCoordinator.openDetailPanelPlan(!!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, mobileScaffold.activePageRouter ? String(mobileScaffold.activePageRouter.currentPath) : "", mobileHierarchyPage.displayedBodyRoutePath(), mobileHierarchyPage.routeStackDepth());
        if (!plan.allowed)
            return false;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        mobileHierarchyPage.cancelPendingDetailPopRepair();
        mobileHierarchyPage.dismissCalendarOverlaysForEditorActivation();
        const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection();
        if (plan.alreadyOpen)
            return true;
        if (plan.directPush) {
            mobileHierarchyPage.resetBackSwipeState();
            mobileHierarchyPage.restoreNoteListSelection(preservedSelectionIndex);
            mobileScaffold.activePageRouter.push(mobileHierarchyPage.detailRoutePath);
            mobileHierarchyPage.requestViewHook();
            return true;
        }
        return mobileHierarchyPage.routeToCanonicalDetailPanel(preservedSelectionIndex);
    }
    function beginBackSwipeGesture(eventData) {
        const edgeOrigin = mobileHierarchyPage.mapToGlobal(Qt.point(0, 0));
        const plan = backSwipeCoordinator.beginGesturePlan(mobileHierarchyPage.backNavigationAvailable, !!pageTransitionController.active, eventData && typeof eventData === "object" ? eventData : ({}), Number(edgeOrigin.x) || 0);
        if (!plan.begin)
            return false;
        if (!pageTransitionController.beginBack({
            "sessionId": Number(plan.sessionId) || -1,
            "source": "edge-pan"
        }))
            return false;
        backSwipeCoordinator.backSwipeSessionId = Number(plan.sessionId) || -1;
        return true;
    }
    function cancelBackSwipeGesture(eventData) {
        const plan = backSwipeCoordinator.cancelGesturePlan(eventData && typeof eventData === "object" ? eventData : ({}));
        if (!plan.cancel)
            return false;
        backSwipeCoordinator.backSwipeSessionId = -1;
        if (Number(plan.sessionId) >= 0)
            backSwipeCoordinator.backSwipeConsumedSessionId = Number(plan.sessionId) || -1;
        return pageTransitionController.cancel();
    }
    function isWithinBackSwipeEdge(globalX) {
        const edgeOrigin = mobileHierarchyPage.mapToGlobal(Qt.point(0, 0));
        return Number(globalX) >= edgeOrigin.x && Number(globalX) <= edgeOrigin.x + mobileHierarchyPage.backSwipeEdgeWidth;
    }
    function resetBackSwipeState() {
        backSwipeCoordinator.resetState();
    }
    function cancelPendingEditorPopRepair() {
        popRepairPolicy.editorPopRepairRequestId += 1;
    }
    function cancelPendingDetailPopRepair() {
        popRepairPolicy.detailPopRepairRequestId += 1;
    }
    function rememberNoteListSelection(selectionIndex) {
        return routeStateStore.rememberSelectionIndex(selectionIndex, mobileHierarchyPage.currentHierarchySelectionIndex());
    }
    function restoreNoteListSelection(selectionIndex) {
        if (!mobileHierarchyPage.activeContentController || mobileHierarchyPage.activeContentController.setHierarchySelectedIndex === undefined)
            return false;
        const targetSelectionIndex = routeStateStore.resolvedSelectionRestoreTarget(selectionIndex);
        mobileHierarchyPage.activeContentController.setHierarchySelectedIndex(targetSelectionIndex);
        return targetSelectionIndex >= 0;
    }
    function routeStackDepth() {
        return canonicalRoutePlanner.routeStackDepth(mobileScaffold.activePageRouter && mobileScaffold.activePageRouter.depth !== undefined ? mobileScaffold.activePageRouter.depth : 0);
    }
    function routeToCanonicalNoteList(selectionIndex) {
        const plan = canonicalRoutePlanner.canonicalRoutePlan(mobileHierarchyPage.noteListRoutePath, !!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, !!pageTransitionController.active);
        if (!plan.allowed)
            return false;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        mobileHierarchyPage.cancelPendingDetailPopRepair();
        if (plan.cancelTransition)
            pageTransitionController.cancel();
        mobileHierarchyPage.resetBackSwipeState();
        const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection(selectionIndex);
        routeStateStore.routeSelectionSyncSuppressed = true;
        routeSelectionSyncPolicy.routeSelectionSyncSuppressed = true;
        if (plan.resetToRoot)
            mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);
        mobileHierarchyPage.restoreNoteListSelection(preservedSelectionIndex);
        for (const path of (plan.pushPaths || []))
            mobileScaffold.activePageRouter.push(String(path));
        Qt.callLater(function () {
            routeStateStore.routeSelectionSyncSuppressed = false;
            routeSelectionSyncPolicy.routeSelectionSyncSuppressed = false;
        });
        if (plan.requestViewHook)
            mobileHierarchyPage.requestViewHook();
        return true;
    }
    function routeToCanonicalEditor(selectionIndex) {
        const plan = canonicalRoutePlanner.canonicalRoutePlan(mobileHierarchyPage.editorRoutePath, !!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, !!pageTransitionController.active);
        if (!plan.allowed)
            return false;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        mobileHierarchyPage.cancelPendingDetailPopRepair();
        if (plan.cancelTransition)
            pageTransitionController.cancel();
        mobileHierarchyPage.resetBackSwipeState();
        const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection(selectionIndex);
        routeStateStore.routeSelectionSyncSuppressed = true;
        routeSelectionSyncPolicy.routeSelectionSyncSuppressed = true;
        if (plan.resetToRoot)
            mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);
        mobileHierarchyPage.restoreNoteListSelection(preservedSelectionIndex);
        for (const path of (plan.pushPaths || []))
            mobileScaffold.activePageRouter.push(String(path));
        Qt.callLater(function () {
            routeStateStore.routeSelectionSyncSuppressed = false;
            routeSelectionSyncPolicy.routeSelectionSyncSuppressed = false;
        });
        if (plan.requestViewHook)
            mobileHierarchyPage.requestViewHook();
        return true;
    }
    function routeToCanonicalDetailPanel(selectionIndex) {
        const plan = canonicalRoutePlanner.canonicalRoutePlan(mobileHierarchyPage.detailRoutePath, !!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, !!pageTransitionController.active);
        if (!plan.allowed)
            return false;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        mobileHierarchyPage.cancelPendingDetailPopRepair();
        if (plan.cancelTransition)
            pageTransitionController.cancel();
        mobileHierarchyPage.resetBackSwipeState();
        const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection(selectionIndex);
        routeStateStore.routeSelectionSyncSuppressed = true;
        routeSelectionSyncPolicy.routeSelectionSyncSuppressed = true;
        if (plan.resetToRoot)
            mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);
        mobileHierarchyPage.restoreNoteListSelection(preservedSelectionIndex);
        for (const path of (plan.pushPaths || []))
            mobileScaffold.activePageRouter.push(String(path));
        Qt.callLater(function () {
            routeStateStore.routeSelectionSyncSuppressed = false;
            routeSelectionSyncPolicy.routeSelectionSyncSuppressed = false;
        });
        if (plan.requestViewHook)
            mobileHierarchyPage.requestViewHook();
        return true;
    }
    function verifyCommittedEditorPopState(requestId, attemptsRemaining) {
        const plan = popRepairPolicy.repairVerificationPlan(requestId, true, !!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, mobileHierarchyPage.displayedBodyRoutePath(), mobileScaffold.activePageRouter ? String(mobileScaffold.activePageRouter.currentPath) : "", mobileHierarchyPage.routeStackDepth(), attemptsRemaining);
        if (!plan.valid)
            return false;
        if (plan.done)
            return true;
        if (plan.retry) {
            Qt.callLater(function () {
                mobileHierarchyPage.verifyCommittedEditorPopState(requestId, Number(plan.nextAttemptsRemaining) || 0);
            });
            return false;
        }
        if (plan.fallbackToCanonicalRoute)
            mobileHierarchyPage.routeToCanonicalNoteList();
        return false;
    }
    function verifyCommittedDetailPopState(requestId, attemptsRemaining) {
        const plan = popRepairPolicy.repairVerificationPlan(requestId, false, !!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, mobileHierarchyPage.displayedBodyRoutePath(), mobileScaffold.activePageRouter ? String(mobileScaffold.activePageRouter.currentPath) : "", mobileHierarchyPage.routeStackDepth(), attemptsRemaining);
        if (!plan.valid)
            return false;
        if (plan.done)
            return true;
        if (plan.retry) {
            Qt.callLater(function () {
                mobileHierarchyPage.verifyCommittedDetailPopState(requestId, Number(plan.nextAttemptsRemaining) || 0);
            });
            return false;
        }
        if (plan.fallbackToCanonicalRoute)
            mobileHierarchyPage.routeToCanonicalEditor();
        return false;
    }
    function handleCommittedRouteTransition(state) {
        const plan = popRepairPolicy.committedTransitionPlan(state && typeof state === "object" ? state : ({}), !!mobileHierarchyPage.activeNoteListModel, mobileHierarchyPage.displayedBodyRoutePath());
        if (plan.requestViewHook)
            mobileHierarchyPage.requestViewHook();
        if (!mobileHierarchyPage.activeNoteListModel)
            return;
        if (plan.rememberSelection)
            mobileHierarchyPage.rememberNoteListSelection();
        if (plan.cancelDetailRepair)
            mobileHierarchyPage.cancelPendingDetailPopRepair();
        if (plan.cancelEditorRepair)
            mobileHierarchyPage.cancelPendingEditorPopRepair();
        if (!plan.scheduleRepair)
            return;
        const repairRequestId = Number(plan.repairRequestId) || 0;
        const attemptsRemaining = Number(plan.attemptsRemaining) || 0;
        Qt.callLater(function () {
            if (plan.repairEditor)
                mobileHierarchyPage.verifyCommittedEditorPopState(repairRequestId, attemptsRemaining);
            else
                mobileHierarchyPage.verifyCommittedDetailPopState(repairRequestId, attemptsRemaining);
        });
    }
    function dismissCurrentPage() {
        const plan = navigationCoordinator.dismissPagePlan(!!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, mobileHierarchyPage.displayedBodyRoutePath());
        if (!plan.allowed)
            return false;
        if (plan.dismissToEditor)
            return mobileHierarchyPage.routeToCanonicalEditor();
        if (plan.dismissToNoteList)
            return mobileHierarchyPage.routeToCanonicalNoteList();
        if (plan.dismissToHierarchy)
            return mobileHierarchyPage.routeToHierarchyRoot();
        return false;
    }
    function requestBackToHierarchy() {
        return mobileHierarchyPage.dismissCurrentPage();
    }
    function requestCreateFolder() {
        if (mobileScaffold.bodyItem && mobileScaffold.bodyItem.requestCreateFolder !== undefined)
            mobileScaffold.bodyItem.requestCreateFolder();
    }
    function requestOpenNoteList(item, itemId, index) {
        const plan = navigationCoordinator.openNoteListPlan(!!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, mobileScaffold.activePageRouter ? String(mobileScaffold.activePageRouter.currentPath) : "", mobileHierarchyPage.displayedBodyRoutePath(), mobileHierarchyPage.routeStackDepth());
        if (!plan.allowed)
            return;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        mobileHierarchyPage.cancelPendingDetailPopRepair();
        const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection(itemId);
        if (plan.alreadyOpen)
            return;
        if (plan.directPush) {
            mobileHierarchyPage.resetBackSwipeState();
            mobileHierarchyPage.restoreNoteListSelection(preservedSelectionIndex);
            mobileScaffold.activePageRouter.push(mobileHierarchyPage.noteListRoutePath);
            mobileHierarchyPage.requestViewHook();
            return;
        }
        mobileHierarchyPage.routeToCanonicalNoteList(preservedSelectionIndex);
    }
    function dismissCalendarOverlaysForEditorActivation() {
        const plan = navigationCoordinator.overlayDismissPlan(mobileHierarchyPage.agendaOverlayVisible, mobileHierarchyPage.dayCalendarOverlayVisible, mobileHierarchyPage.weekCalendarOverlayVisible, mobileHierarchyPage.monthCalendarOverlayVisible, mobileHierarchyPage.yearCalendarOverlayVisible);
        if (plan.dismissAgenda)
            mobileHierarchyPage.agendaOverlayDismissRequested();
        if (plan.dismissDay)
            mobileHierarchyPage.dayCalendarOverlayDismissRequested();
        if (plan.dismissWeek)
            mobileHierarchyPage.weekCalendarOverlayDismissRequested();
        if (plan.dismissMonth)
            mobileHierarchyPage.monthCalendarOverlayDismissRequested();
        if (plan.dismissYear)
            mobileHierarchyPage.yearCalendarOverlayDismissRequested();
    }
    function requestOpenEditor(noteId, index) {
        const plan = navigationCoordinator.openEditorPlan(noteId, !!mobileHierarchyPage.activeContentController, !!mobileHierarchyPage.activeNoteListModel, !!mobileScaffold.activePageRouter, mobileScaffold.activePageRouter ? String(mobileScaffold.activePageRouter.currentPath) : "", mobileHierarchyPage.displayedBodyRoutePath(), mobileHierarchyPage.routeStackDepth());
        if (!plan.allowed)
            return;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        mobileHierarchyPage.cancelPendingDetailPopRepair();
        mobileHierarchyPage.dismissCalendarOverlaysForEditorActivation();
        const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection();
        if (plan.alreadyOpen)
            return;
        if (plan.directPush) {
            mobileHierarchyPage.resetBackSwipeState();
            mobileHierarchyPage.restoreNoteListSelection(preservedSelectionIndex);
            mobileScaffold.activePageRouter.push(mobileHierarchyPage.editorRoutePath);
            mobileHierarchyPage.requestViewHook();
            return;
        }
        mobileHierarchyPage.routeToCanonicalEditor(preservedSelectionIndex);
    }
    function ensureCalendarSurfaceVisible() {
        const plan = navigationCoordinator.calendarSurfacePlan(!!mobileScaffold.activePageRouter, !!mobileHierarchyPage.activeNoteListModel, mobileHierarchyPage.displayedBodyRoutePath());
        if (!plan.allowed)
            return false;
        if (plan.alreadyVisible)
            return true;
        if (!plan.routeToEditor)
            return false;
        mobileHierarchyPage.routeToCanonicalEditor();
        return true;
    }
    function requestOpenAgenda() {
        if (!mobileHierarchyPage.ensureCalendarSurfaceVisible())
            return false;
        mobileHierarchyPage.agendaRequested();
        return true;
    }
    function requestOpenDayCalendar() {
        if (!mobileHierarchyPage.ensureCalendarSurfaceVisible())
            return false;
        mobileHierarchyPage.dayCalendarRequested();
        return true;
    }
    function requestOpenWeekCalendar() {
        if (!mobileHierarchyPage.ensureCalendarSurfaceVisible())
            return false;
        mobileHierarchyPage.weekCalendarRequested();
        return true;
    }
    function requestOpenMonthCalendar() {
        if (!mobileHierarchyPage.ensureCalendarSurfaceVisible())
            return false;
        mobileHierarchyPage.monthCalendarRequested();
        return true;
    }
    function requestOpenYearCalendar() {
        if (!mobileHierarchyPage.ensureCalendarSurfaceVisible())
            return false;
        mobileHierarchyPage.yearCalendarRequested();
        return true;
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }
    function routeToHierarchyRoot() {
        const plan = canonicalRoutePlanner.hierarchyRootPlan(!!mobileScaffold.activePageRouter, !!pageTransitionController.active, mobileHierarchyPage.displayedBodyRoutePath(), !!(mobileScaffold.activePageRouter && mobileScaffold.activePageRouter.canGoBack));
        if (!plan.allowed)
            return false;
        mobileHierarchyPage.cancelPendingEditorPopRepair();
        mobileHierarchyPage.cancelPendingDetailPopRepair();
        if (plan.cancelTransition)
            pageTransitionController.cancel();
        mobileHierarchyPage.resetBackSwipeState();
        if (plan.alreadyRoot)
            return false;
        if (plan.setRoot)
            mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);
        if (plan.requestViewHook)
            mobileHierarchyPage.requestViewHook();
        return true;
    }
    function syncRouteSelectionState() {
        const displayedPath = mobileHierarchyPage.displayedBodyRoutePath();
        const plan = routeSelectionSyncPolicy.routeSelectionSyncPlan(!!mobileScaffold.activePageRouter, displayedPath, mobileScaffold.activePageRouter ? String(mobileScaffold.activePageRouter.currentPath) : "", mobileHierarchyPage.routeStackDepth());
        routeStateStore.lastObservedRoutePath = String(plan.nextPath || displayedPath);
        routeSelectionSyncPolicy.lastObservedRoutePath = String(plan.nextPath || displayedPath);
        if (!plan.valid || !plan.clearHierarchySelection)
            return false;
        return mobileHierarchyPage.clearActiveHierarchySelection();
    }
    function finishBackSwipeGesture(eventData, cancelled) {
        const plan = backSwipeCoordinator.finishGesturePlan(eventData && typeof eventData === "object" ? eventData : ({}), !!cancelled, !!pageTransitionController.active, Number(pageTransitionController.progress) || 0);
        if (!plan.valid)
            return false;
        backSwipeCoordinator.backSwipeSessionId = -1;
        if (Number(plan.sessionId) >= 0)
            backSwipeCoordinator.backSwipeConsumedSessionId = Number(plan.sessionId) || -1;
        if (!pageTransitionController.active)
            return false;
        if (plan.cancelled)
            return pageTransitionController.cancel();
        if (!plan.shouldCommit)
            return pageTransitionController.cancel();
        return mobileHierarchyPage.dismissCurrentPage();
    }
    function updateBackSwipeGesture(eventData) {
        const plan = backSwipeCoordinator.updateGesturePlan(eventData && typeof eventData === "object" ? eventData : ({}), !!pageTransitionController.active, mobileHierarchyPage.backSwipeViewportWidth(), LV.Theme.gap8);
        if (!plan.valid)
            return false;
        if (plan.cancel)
            return mobileHierarchyPage.cancelBackSwipeGesture(eventData);
        if (!plan.update)
            return false;
        return pageTransitionController.update(Number(plan.progress) || 0, {
            "velocityX": Number(plan.velocityX) || 0,
            "velocityY": Number(plan.velocityY) || 0
        });
    }

    Component.onCompleted: mobileHierarchyPage.syncActiveHierarchyBindings()

    MobileHierarchyRouteStateStore {
        id: routeStateStore

        lastObservedRoutePath: canonicalRoutePlanner.hierarchyRoutePath
    }
    MobileHierarchyCanonicalRoutePlanner {
        id: canonicalRoutePlanner

        detailRoutePath: mobileHierarchyPage.detailRoutePath
        editorRoutePath: mobileHierarchyPage.editorRoutePath
        hierarchyRoutePath: mobileHierarchyPage.hierarchyRoutePath
        noteListRoutePath: mobileHierarchyPage.noteListRoutePath
    }
    MobileHierarchyPopRepairPolicy {
        id: popRepairPolicy

        detailRoutePath: mobileHierarchyPage.detailRoutePath
        editorRoutePath: mobileHierarchyPage.editorRoutePath
        noteListRoutePath: mobileHierarchyPage.noteListRoutePath
    }
    MobileHierarchyRouteSelectionSyncPolicy {
        id: routeSelectionSyncPolicy

        hierarchyRoutePath: mobileHierarchyPage.hierarchyRoutePath
        lastObservedRoutePath: routeStateStore.lastObservedRoutePath
        routeSelectionSyncSuppressed: routeStateStore.routeSelectionSyncSuppressed
    }
    MobileHierarchySelectionCoordinator {
        id: selectionCoordinator
    }
    MobileHierarchyNavigationCoordinator {
        id: navigationCoordinator

        detailRoutePath: mobileHierarchyPage.detailRoutePath
        editorRoutePath: mobileHierarchyPage.editorRoutePath
        hierarchyRoutePath: mobileHierarchyPage.hierarchyRoutePath
        noteListRoutePath: mobileHierarchyPage.noteListRoutePath
    }
    MobileHierarchyBackSwipeCoordinator {
        id: backSwipeCoordinator

        backSwipeEdgeWidth: mobileHierarchyPage.backSwipeEdgeWidth
    }
    onSidebarHierarchyControllerChanged: mobileHierarchyPage.syncActiveHierarchyBindings()
    onActiveContentControllerChanged: noteCreationCoordinator.routePendingCreatedNoteToEditor()
    onActiveNoteListModelChanged: {
        if (mobileHierarchyPage.activeNoteListModel) {
            noteCreationCoordinator.routePendingCreatedNoteToEditor();
            return;
        }
        mobileHierarchyPage.routeToHierarchyRoot();
    }
    onResolvedBodyRoutePathChanged: mobileHierarchyPage.syncRouteSelectionState()

    QtObject {
        id: noteCreationCoordinator

        property var activeContentController: null
        property var activeNoteListModel: null
        property var activePageRouter: null
        property var noteCreationController: null
        property string pendingCreatedNoteId: ""
        property var windowInteractions: null

        signal openEditorRequested(string noteId, int index)

        function syncNoteCreationController() {
            if (!noteCreationCoordinator.windowInteractions || noteCreationCoordinator.windowInteractions.resolveLibraryNoteCreationController === undefined) {
                noteCreationCoordinator.noteCreationController = null;
                return;
            }

            const resolvedController = noteCreationCoordinator.windowInteractions.resolveLibraryNoteCreationController();
            noteCreationCoordinator.noteCreationController = resolvedController !== undefined ? resolvedController : null;
        }
        function requestCreateNote() {
            if (noteCreationCoordinator.windowInteractions && noteCreationCoordinator.windowInteractions.createNoteFromShortcut !== undefined)
                noteCreationCoordinator.windowInteractions.createNoteFromShortcut();
        }
        function routePendingCreatedNoteToEditor() {
            const pendingNoteId = noteCreationCoordinator.pendingCreatedNoteId === undefined || noteCreationCoordinator.pendingCreatedNoteId === null ? "" : String(noteCreationCoordinator.pendingCreatedNoteId).trim();
            if (pendingNoteId.length === 0 || !noteCreationCoordinator.activeContentController || !noteCreationCoordinator.activeNoteListModel || !noteCreationCoordinator.activePageRouter)
                return false;
            noteCreationCoordinator.pendingCreatedNoteId = "";
            noteCreationCoordinator.openEditorRequested(pendingNoteId, -1);
            return true;
        }
        function scheduleCreatedNoteEditorRoute(noteId) {
            const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
            if (normalizedNoteId.length === 0)
                return false;
            noteCreationCoordinator.pendingCreatedNoteId = normalizedNoteId;
            Qt.callLater(function () {
                noteCreationCoordinator.routePendingCreatedNoteToEditor();
            });
            return true;
        }

        Component.onCompleted: noteCreationCoordinator.syncNoteCreationController()
        onWindowInteractionsChanged: noteCreationCoordinator.syncNoteCreationController()

        property Connections noteCreationControllerConnections: Connections {
            target: noteCreationCoordinator.noteCreationController
            ignoreUnknownSignals: true

            function onEmptyNoteCreated(noteId) {
                noteCreationCoordinator.scheduleCreatedNoteEditorRoute(noteId);
            }
        }
    }
    Connections {
        target: mobileHierarchyPage.sidebarHierarchyController
        ignoreUnknownSignals: true

        function onActiveBindingsChanged() {
            mobileHierarchyPage.syncActiveHierarchyBindings();
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
        compactDetailPanelVisible: mobileHierarchyPage.editorPageActive
        compactEditorViewVisible: mobileHierarchyPage.editorPageActive
        compactLeadingActionVisible: false
        compactNoteListControlsVisible: mobileHierarchyPage.noteListPageActive
        compactSettingsVisible: mobileHierarchyPage.hierarchyPageActive
        controlSurfaceColor: mobileHierarchyPage.controlSurfaceColor
        detailPanelCollapsed: !mobileHierarchyPage.detailPageActive
        editorViewModeController: mobileHierarchyPage.editorViewModeController
        navigationModeController: mobileHierarchyPage.navigationModeController
        statusPlaceholderText: mobileHierarchyPage.statusPlaceholderText
        statusSearchText: mobileHierarchyPage.statusSearchText
        windowInteractions: mobileHierarchyPage.windowInteractions

        onCompactAddFolderRequested: mobileHierarchyPage.requestCreateFolder()
        onCompactLeadingActionRequested: mobileHierarchyPage.requestBackToHierarchy()
        onCreateNoteRequested: noteCreationCoordinator.requestCreateNote()
        onAgendaRequested: mobileHierarchyPage.requestOpenAgenda()
        onDayCalendarRequested: mobileHierarchyPage.requestOpenDayCalendar()
        onMonthCalendarRequested: mobileHierarchyPage.requestOpenMonthCalendar()
        onStatusSearchSubmitted: function (text) {
            mobileHierarchyPage.statusSearchText = text;
        }
        onStatusSearchTextEdited: function (text) {
            mobileHierarchyPage.statusSearchText = text;
        }
        onToggleDetailPanelRequested: mobileHierarchyPage.requestOpenDetailPanelPage()
        onViewHookRequested: mobileHierarchyPage.requestViewHook()
        onWeekCalendarRequested: mobileHierarchyPage.requestOpenWeekCalendar()
        onYearCalendarRequested: mobileHierarchyPage.requestOpenYearCalendar()
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
        width: visible ? backSwipeCoordinator.backSwipeEdgeWidth : 0
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
                    const beginEventData = mobileHierarchyPage.backSwipeGestureEventData(backSwipeDragHandler.centroid.pressPosition.x, backSwipeDragHandler.centroid.pressPosition.y, 0, 0, backSwipeCoordinator.nextGeneratedSessionId());
                    if (!mobileHierarchyPage.beginBackSwipeGesture(beginEventData))
                        return;
                    mobileHierarchyPage.updateBackSwipeGesture(mobileHierarchyPage.backSwipeGestureEventData(backSwipeDragHandler.centroid.position.x, backSwipeDragHandler.centroid.position.y, Number(backSwipeDragHandler.centroid.position.x) - Number(backSwipeDragHandler.centroid.pressPosition.x), Number(backSwipeDragHandler.centroid.position.y) - Number(backSwipeDragHandler.centroid.pressPosition.y), backSwipeCoordinator.backSwipeSessionId));
                    return;
                }
                if (backSwipeCoordinator.backSwipeSessionId < 0)
                    return;
                mobileHierarchyPage.finishBackSwipeGesture(mobileHierarchyPage.backSwipeGestureEventData(backSwipeDragHandler.centroid.position.x, backSwipeDragHandler.centroid.position.y, Number(backSwipeDragHandler.centroid.position.x) - Number(backSwipeDragHandler.centroid.pressPosition.x), Number(backSwipeDragHandler.centroid.position.y) - Number(backSwipeDragHandler.centroid.pressPosition.y), backSwipeCoordinator.backSwipeSessionId), mobileHierarchyPage.backSwipeDragCanceled);
                mobileHierarchyPage.backSwipeDragCanceled = false;
            }
            onCentroidChanged: {
                if (!active)
                    return;
                mobileHierarchyPage.updateBackSwipeGesture(mobileHierarchyPage.backSwipeGestureEventData(backSwipeDragHandler.centroid.position.x, backSwipeDragHandler.centroid.position.y, Number(backSwipeDragHandler.centroid.position.x) - Number(backSwipeDragHandler.centroid.pressPosition.x), Number(backSwipeDragHandler.centroid.position.y) - Number(backSwipeDragHandler.centroid.pressPosition.y), backSwipeCoordinator.backSwipeSessionId));
            }
            onCanceled: {
                mobileHierarchyPage.backSwipeDragCanceled = true;
                mobileHierarchyPage.finishBackSwipeGesture(mobileHierarchyPage.backSwipeGestureEventData(backSwipeDragHandler.centroid.position.x, backSwipeDragHandler.centroid.position.y, Number(backSwipeDragHandler.centroid.position.x) - Number(backSwipeDragHandler.centroid.pressPosition.x), Number(backSwipeDragHandler.centroid.position.y) - Number(backSwipeDragHandler.centroid.pressPosition.y), backSwipeCoordinator.backSwipeSessionId), true);
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
            searchHeaderMinHeight: LV.Theme.gap18
            searchHeaderTopGap: LV.Theme.gap2
            searchHeaderVerticalInset: LV.Theme.gapNone
            searchListGap: LV.Theme.gap2
            searchText: mobileHierarchyPage.hierarchySearchText
            sidebarHierarchyController: mobileHierarchyPage.sidebarHierarchyController
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
            hierarchyController: mobileHierarchyPage.activeContentController
            noteListModel: mobileHierarchyPage.activeNoteListModel
            noteDeletionController: mobileHierarchyPage.resolvedNoteDeletionController
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
            contentController: mobileHierarchyPage.activeContentController
            displayColor: mobileHierarchyPage.canvasColor
            editorViewModeController: mobileHierarchyPage.editorViewModeController
            frameHorizontalInsetOverride: LV.Theme.gapNone
            isMobilePlatform: Window.window && Window.window.isMobilePlatform !== undefined ? Boolean(Window.window.isMobilePlatform) : false
            libraryHierarchyController: noteCreationCoordinator.noteCreationController
            minimapVisible: false
            noteListModel: mobileHierarchyPage.activeNoteListModel
            resourcesImportController: mobileHierarchyPage.resourcesImportController
            sidebarHierarchyController: mobileHierarchyPage.sidebarHierarchyController
            agendaOverlayVisible: mobileHierarchyPage.agendaOverlayVisible
            agendaController: mobileHierarchyPage.agendaController
            dayCalendarOverlayVisible: mobileHierarchyPage.dayCalendarOverlayVisible
            dayCalendarController: mobileHierarchyPage.dayCalendarController
            monthCalendarOverlayVisible: mobileHierarchyPage.monthCalendarOverlayVisible
            monthCalendarController: mobileHierarchyPage.monthCalendarController
            weekCalendarOverlayVisible: mobileHierarchyPage.weekCalendarOverlayVisible
            weekCalendarController: mobileHierarchyPage.weekCalendarController
            yearCalendarOverlayVisible: mobileHierarchyPage.yearCalendarOverlayVisible
            yearCalendarController: mobileHierarchyPage.yearCalendarController

            onViewHookRequested: mobileHierarchyPage.requestViewHook()
            onAgendaOverlayCloseRequested: mobileHierarchyPage.agendaOverlayDismissRequested()
            onDayCalendarOverlayCloseRequested: mobileHierarchyPage.dayCalendarOverlayDismissRequested()
            onMonthCalendarOverlayOpenRequested: mobileHierarchyPage.monthCalendarOverlayOpenRequested()
            onMonthCalendarOverlayCloseRequested: mobileHierarchyPage.monthCalendarOverlayDismissRequested()
            onWeekCalendarOverlayCloseRequested: mobileHierarchyPage.weekCalendarOverlayDismissRequested()
            onYearCalendarOverlayCloseRequested: mobileHierarchyPage.yearCalendarOverlayDismissRequested()
        }
    }
    Component {
        id: detailBodyComponent

        Item {
            anchors.fill: parent
            property bool detailPanelPage: true

            PanelView.DetailPanelLayout {
                anchors.fill: parent
                panelColor: "transparent"

                onViewHookRequested: mobileHierarchyPage.requestViewHook()
            }
        }
    }
}
