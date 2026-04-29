pragma ComponentBehavior: Bound

import QtQuick
import "../../../models/editor/display/ContentsMinimapSnapshotSupport.js" as MinimapSnapshotSupport

QtObject {
    id: controller

    property var contentsView: null
    property var eventPump: null
    property var minimapLayer: null
    property var refreshCoordinator: null
    property var structuredDocumentFlow: null
    property var viewportCoordinator: null

    function refreshMinimapSnapshot() {
        if (!controller.contentsView.lineGeometryRefreshEnabled)
            return;
        const currentNoteId = controller.contentsView.activeLineGeometryNoteId();
        const useStructuredMinimap = controller.contentsView.hasStructuredMinimapEntries();
        const nextLineGroups = useStructuredMinimap
                ? controller.contentsView.buildStructuredMinimapLineGroupsForRange(
                    1,
                    Math.max(1, controller.contentsView.effectiveStructuredMinimapEntries().length))
                : controller.contentsView.buildMinimapLineGroupsForRange(
                    1,
                    controller.contentsView.logicalLineCount);
        const nextRows = MinimapSnapshotSupport.flattenLineGroups(nextLineGroups, controller.contentsView.editorLineHeight);
        const nextSilhouetteHeight = controller.contentsView.minimapSilhouetteHeight(nextRows);
        const nextTrackHeight = Math.min(controller.contentsView.minimapAvailableTrackHeight, nextSilhouetteHeight);

        controller.contentsView.minimapLineGroups = nextLineGroups;
        controller.contentsView.minimapLineGroupsNoteId = currentNoteId;
        controller.contentsView.minimapVisualRows = nextRows;
        controller.contentsView.minimapResolvedSilhouetteHeight = nextSilhouetteHeight;
        controller.contentsView.minimapResolvedTrackHeight = nextTrackHeight;
        controller.refreshMinimapViewportTracking(nextTrackHeight);
        controller.refreshMinimapCursorTracking(nextRows);
    }

    function refreshMinimapCursorTracking(rowsOverride) {
        if (!controller.contentsView.minimapRefreshEnabled)
            return;
        const nextCurrentVisualRow = controller.contentsView.minimapCurrentVisualRow(rowsOverride);
        controller.contentsView.minimapResolvedCurrentLineHeight = controller.contentsView.minimapVisualRowPaintHeight(nextCurrentVisualRow);
        controller.contentsView.minimapResolvedCurrentLineWidth = controller.viewportCoordinator.minimapLineBarWidth(
                    Number(nextCurrentVisualRow.contentWidth) || 0,
                    Number(nextCurrentVisualRow.contentAvailableWidth) || 0,
                    Number(nextCurrentVisualRow.charCount) || 0,
                    controller.contentsView.minimapResolvedTrackWidth);
        controller.contentsView.minimapResolvedCurrentLineY = controller.contentsView.minimapVisualRowPaintY(nextCurrentVisualRow);
    }

    function refreshMinimapViewportTracking(trackHeightOverride) {
        if (!controller.contentsView.minimapRefreshEnabled)
            return;
        const nextScrollable = controller.contentsView.isMinimapScrollable();
        const nextViewportHeight = controller.viewportCoordinator.minimapViewportHeight(
                    !!controller.contentsView.editorFlickable,
                    controller.contentsView.minimapContentHeight(),
                    controller.contentsView.minimapViewportMinHeight);
        const nextViewportY = controller.viewportCoordinator.minimapViewportY(
                    !!controller.contentsView.editorFlickable,
                    controller.contentsView.editorFlickable ? Number(controller.contentsView.editorFlickable.contentY) || 0 : 0,
                    controller.contentsView.minimapContentHeight(),
                    nextViewportHeight);
        controller.contentsView.minimapScrollable = nextScrollable;
        controller.contentsView.minimapResolvedViewportHeight = nextViewportHeight;
        controller.contentsView.minimapResolvedViewportY = nextViewportY;
    }

    function resetNoteEntryLineGeometryState() {
        controller.resetGutterRefreshState();
        controller.contentsView.minimapLineGroups = [];
        controller.contentsView.minimapLineGroupsNoteId = "";
        controller.contentsView.minimapVisualRows = [];
        controller.contentsView.minimapScrollable = false;
        controller.contentsView.minimapResolvedCurrentLineHeight = 1;
        controller.contentsView.minimapResolvedCurrentLineWidth = 0;
        controller.contentsView.minimapResolvedCurrentLineY = 0;
        controller.contentsView.minimapResolvedSilhouetteHeight = 1;
        controller.contentsView.minimapResolvedTrackHeight = 1;
        controller.contentsView.minimapResolvedViewportHeight = 0;
        controller.contentsView.minimapResolvedViewportY = 0;
    }

    function resetGutterRefreshState() {
        controller.contentsView.visibleGutterLineEntries = [
                    {
                        "lineNumber": 1,
                        "y": 0
                    }
                ];
        controller.contentsView.logicalLineDocumentYCache = [];
        controller.contentsView.logicalLineDocumentYCacheRevision = -1;
        controller.contentsView.logicalLineDocumentYCacheLineCount = 0;
        controller.contentsView.logicalLineGutterDocumentYCache = [];
        controller.contentsView.logicalLineGutterDocumentYCacheRevision = -1;
        controller.contentsView.logicalLineGutterDocumentYCacheLineCount = 0;
        controller.contentsView.structuredGutterGeometrySignature = "";
    }

    function refreshLiveLogicalLineMetrics() {
        const metrics = controller.contentsView.hasStructuredLogicalLineGeometry()
                ? controller.viewportCoordinator.buildLogicalLineMetricsFromStructuredEntries(
                      controller.contentsView.effectiveStructuredLogicalLineEntries())
                : controller.viewportCoordinator.buildLogicalLineMetricsFromText(
                      controller.contentsView.activeLogicalTextSnapshot());
        return controller.contentsView.applyLiveLogicalLineMetrics(
                    Number(metrics.logicalTextLength) || 0,
                    metrics.lineStartOffsets || [0],
                    Number(metrics.lineCount) || 1);
    }

    function activeLogicalLineCountSnapshot() {
        const metrics = controller.contentsView.hasStructuredLogicalLineGeometry()
                ? controller.viewportCoordinator.buildLogicalLineMetricsFromStructuredEntries(
                      controller.contentsView.effectiveStructuredLogicalLineEntries())
                : controller.viewportCoordinator.buildLogicalLineMetricsFromText(
                      controller.contentsView.activeLogicalTextSnapshot());
        return Math.max(1, Number(metrics.lineCount) || 1);
    }

    function scheduleGutterRefresh(passCount, reason) {
        const plan = controller.refreshCoordinator.scheduleGutterRefresh(
                    Number(passCount) || 0,
                    reason === undefined || reason === null ? "" : String(reason),
                    controller.activeLogicalLineCountSnapshot());
        if (plan.startTimer)
            controller.eventPump.startGutterRefreshTimer();
    }

    function scheduleNoteEntryGutterRefresh(noteId) {
        controller.contentsView.executeRefreshPlan(controller.refreshCoordinator.scheduleNoteEntryGutterRefresh(
                                                      noteId === undefined || noteId === null ? "" : String(noteId)));
    }

    function scheduleCursorDrivenUiRefresh() {
        const plan = controller.refreshCoordinator.scheduleCursorDrivenUiRefresh();
        if (!plan.queueCallLater)
            return;
        Qt.callLater(function () {
            controller.refreshCoordinator.clearCursorDrivenUiRefreshQueued();
            controller.refreshMinimapCursorTracking();
            if (controller.minimapLayer && controller.contentsView.minimapRefreshEnabled)
                controller.minimapLayer.requestRepaint();
        });
    }

    function scheduleViewportGutterRefresh() {
        const plan = controller.refreshCoordinator.scheduleViewportGutterRefresh();
        if (!plan.queueCallLater)
            return;
        Qt.callLater(function () {
            controller.refreshCoordinator.clearViewportGutterRefreshQueued();
            controller.contentsView.refreshVisibleGutterEntries();
        });
    }

    function scheduleMinimapSnapshotRefresh(forceFull) {
        const plan = controller.refreshCoordinator.scheduleMinimapSnapshotRefresh(!!forceFull);
        if (!plan.queueCallLater)
            return;
        Qt.callLater(function () {
            controller.refreshCoordinator.clearMinimapSnapshotRefreshQueued();
            controller.refreshMinimapSnapshot();
        });
    }

    function scrollEditorViewportToMinimapPosition(localY) {
        const flickable = controller.contentsView.editorFlickable;
        if (!flickable)
            return;
        const plan = controller.viewportCoordinator.minimapScrollPlan(
                    Number(localY) || 0,
                    Math.max(1, controller.contentsView.editorOccupiedContentHeight()));
        if (plan.apply)
            flickable.contentY = Number(plan.contentY) || 0;
    }

    function correctTypingViewport(forceAnchor) {
        const flickable = controller.contentsView.editorFlickable;
        if (!flickable || flickable.contentY === undefined)
            return;
        const plan = controller.viewportCoordinator.typingViewportCorrectionPlan(
                    !!forceAnchor,
                    Number(flickable.height) || controller.contentsView.editorViewportHeight,
                    Math.max(Number(flickable.contentHeight) || 0,
                             controller.contentsView.editorOccupiedContentHeight()),
                    Number(flickable.contentY) || 0,
                    controller.contentsView.currentCursorVisualRowRect());
        if (plan.apply)
            flickable.contentY = Number(plan.contentY) || 0;
    }

    function scheduleTypingViewportCorrection(forceAnchor) {
        const plan = controller.refreshCoordinator.scheduleTypingViewportCorrection(!!forceAnchor);
        if (!plan.queueCallLater)
            return;
        Qt.callLater(function () {
            const forceCorrection = controller.refreshCoordinator.takeTypingViewportForceCorrectionRequested();
            controller.refreshCoordinator.clearTypingViewportCorrectionQueued();
            controller.correctTypingViewport(forceCorrection);
        });
    }
}
