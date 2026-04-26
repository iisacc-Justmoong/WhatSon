pragma ComponentBehavior: Bound

import QtQuick
import "../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace

Item {
    id: eventPump

    property var bodyResourceRenderer: null
    property var contentsView: null
    property var editorProjection: null
    property var editorSession: null
    property var editorTypingController: null
    property var minimapLayer: null
    property var noteBodyMountCoordinator: null
    property var presentationRefreshController: null
    property var resourceImportController: null
    property var selectionBridge: null
    property var selectionSyncCoordinator: null
    property var structuredBlockRenderer: null
    property var structuredDocumentFlow: null
    property var traceFormatter: null

    height: 0
    visible: false
    width: 0

    function applyPresentationRefreshTimerPlan(refreshPlan) {
        const plan = refreshPlan && typeof refreshPlan === "object" ? refreshPlan : ({});
        if (plan.stopTimer && documentPresentationRefreshTimer.running)
            documentPresentationRefreshTimer.stop();
        if (plan.startTimer) {
            if (documentPresentationRefreshTimer.running)
                documentPresentationRefreshTimer.stop();
            documentPresentationRefreshTimer.start();
        }
    }

    function startGutterRefreshTimer() {
        if (!gutterRefreshTimer.running)
            gutterRefreshTimer.start();
    }

    function stopDocumentPresentationRefreshTimer() {
        if (documentPresentationRefreshTimer.running)
            documentPresentationRefreshTimer.stop();
    }

    Timer {
        id: documentPresentationRefreshTimer

        interval: eventPump.contentsView ? eventPump.contentsView.documentPresentationRefreshIntervalMs : 0
        repeat: false

        onTriggered: {
            if (!eventPump.contentsView || !eventPump.presentationRefreshController)
                return;
            eventPump.contentsView.applyPresentationRefreshPlan(
                        eventPump.presentationRefreshController.planDeferredTrigger());
        }
    }

    Timer {
        id: gutterRefreshTimer

        interval: 16
        repeat: true

        onTriggered: {
            if (!eventPump.contentsView)
                return;
            eventPump.contentsView.commitGutterRefresh();
            if (eventPump.contentsView.gutterRefreshPassesRemaining <= 1) {
                eventPump.contentsView.gutterRefreshPassesRemaining = 0;
                stop();
                return;
            }
            eventPump.contentsView.gutterRefreshPassesRemaining -= 1;
        }
    }

    Timer {
        id: noteSnapshotRefreshTimer

        interval: eventPump.contentsView ? eventPump.contentsView.noteSnapshotRefreshIntervalMs : 0
        repeat: true
        running: eventPump.contentsView ? eventPump.contentsView.noteSnapshotRefreshEnabled : false

        onTriggered: {
            if (eventPump.contentsView)
                eventPump.contentsView.pollSelectedNoteSnapshot();
        }
    }

    Connections {
        target: eventPump.bodyResourceRenderer

        function onRenderedResourcesChanged() {
            if (!eventPump.contentsView
                    || !eventPump.contentsView.resourceBlocksRenderedInlineByHtmlProjection)
                return;
            eventPump.contentsView.refreshInlineResourcePresentation();
            eventPump.contentsView.scheduleGutterRefresh(2, "resource-line-geometry");
        }

        ignoreUnknownSignals: true
    }

    Connections {
        function onLogicalTextChanged() {
            if (eventPump.contentsView.structuredHostGeometryActive)
                return;
            const logicalText = eventPump.editorProjection.logicalText;
            eventPump.contentsView.liveLogicalTextLength = logicalText !== undefined && logicalText !== null
                    ? String(logicalText).length
                    : 0;
        }

        function onLogicalLineCountChanged() {
            if (eventPump.contentsView.structuredHostGeometryActive)
                return;
            if (eventPump.contentsView.refreshLiveLogicalLineMetrics())
                eventPump.contentsView.scheduleGutterRefresh(1, "projection-line-metrics");
        }

        function onLogicalLineStartOffsetsChanged() {
            if (eventPump.contentsView.structuredHostGeometryActive)
                return;
            if (eventPump.contentsView.refreshLiveLogicalLineMetrics())
                eventPump.contentsView.scheduleGutterRefresh(1, "projection-line-metrics");
        }

        target: eventPump.editorProjection
    }

    Connections {
        function onEmptyNoteCreated(noteId) {
            eventPump.contentsView.scheduleEditorFocusForNote(noteId);
        }

        ignoreUnknownSignals: true
        target: eventPump.contentsView ? eventPump.contentsView.libraryHierarchyViewModel : null
    }

    Connections {
        function onViewSessionSnapshotReconciled(noteId, refreshed, success, _errorMessage) {
            eventPump.selectionSyncCoordinator.handleSnapshotReconcileFinished(noteId, success);
        }

        target: eventPump.selectionBridge
    }

    Connections {
        function onMountFlushRequested(plan) {
            const mountPlan = plan && typeof plan === "object" ? plan : ({});
            EditorTrace.trace(
                        "displayView",
                        "selectionFlow.mountPlan",
                        "plan={" + eventPump.traceFormatter.describeSelectionPlan(mountPlan) + "}",
                        eventPump.contentsView)
            let snapshotRefreshAccepted = false;
            let scheduledFollowUpMount = false;
            if (mountPlan.attemptSnapshotRefresh
                    && eventPump.selectionBridge
                    && eventPump.selectionBridge.refreshSelectedNoteSnapshot !== undefined) {
                snapshotRefreshAccepted = !!eventPump.selectionBridge.refreshSelectedNoteSnapshot();
                eventPump.noteBodyMountCoordinator.handleSnapshotRefreshFinished(
                            String(mountPlan.selectedNoteId || ""),
                            snapshotRefreshAccepted);
                if (snapshotRefreshAccepted) {
                    const refreshedNoteId = String(mountPlan.selectedNoteId || "").trim();
                    const currentBodyNoteId = eventPump.contentsView.selectedNoteBodyNoteId === undefined
                            || eventPump.contentsView.selectedNoteBodyNoteId === null
                            ? ""
                            : String(eventPump.contentsView.selectedNoteBodyNoteId).trim();
                    const currentBodyText = eventPump.contentsView.selectedNoteBodyText === undefined
                            || eventPump.contentsView.selectedNoteBodyText === null
                            ? ""
                            : String(eventPump.contentsView.selectedNoteBodyText);
                    if (!eventPump.contentsView.selectedNoteBodyLoading
                            && refreshedNoteId.length > 0
                            && (currentBodyText.length === 0
                                || (currentBodyNoteId.length > 0 && currentBodyNoteId !== refreshedNoteId))) {
                        scheduledFollowUpMount = true;
                        eventPump.contentsView.scheduleSelectionModelSync({
                                                                               "scheduleReconcile": true,
                                                                               "fallbackRefresh": true
                                                                           });
                    }
                }
            }
            const selectionSynced = eventPump.contentsView.executeSelectionDeliveryPlan(
                        mountPlan,
                        "fallbackRefreshIfMountSkipped");
            EditorTrace.trace(
                        "displayView",
                        "selectionFlow.mountResult",
                        "reason=" + String(mountPlan.reason || "")
                        + " selectedNoteId=" + String(mountPlan.selectedNoteId || "")
                        + " selectionSynced=" + selectionSynced
                        + " snapshotRefreshAccepted=" + snapshotRefreshAccepted
                        + " scheduledFollowUpMount=" + scheduledFollowUpMount
                        + " focusEditor=" + !!mountPlan.focusEditorForSelectedNote,
                        eventPump.contentsView)
        }

        target: eventPump.noteBodyMountCoordinator
    }

    Connections {
        function onSelectionSyncFlushRequested(plan) {
            const selectionPlan = plan && typeof plan === "object" ? plan : ({});
            EditorTrace.trace(
                        "displayView",
                        "selectionFlow.selectionSyncPlan",
                        "plan={" + eventPump.traceFormatter.describeSelectionPlan(selectionPlan) + "}",
                        eventPump.contentsView)
            const selectionSynced = eventPump.contentsView.executeSelectionDeliveryPlan(
                        selectionPlan,
                        "fallbackRefreshIfSyncSkipped");
            EditorTrace.trace(
                        "displayView",
                        "selectionFlow.selectionSyncResult",
                        "reason=" + String(selectionPlan.reason || "")
                        + " selectedNoteId=" + String(selectionPlan.selectedNoteId || "")
                        + " selectionSynced=" + selectionSynced
                        + " focusEditor=" + !!selectionPlan.focusEditorForSelectedNote,
                        eventPump.contentsView)
        }

        function onSnapshotReconcileRequested() {
            eventPump.contentsView.reconcileEditorEntrySnapshotOnce();
        }

        function onEditorFocusRequested() {
            eventPump.contentsView.focusEditorForPendingNote();
        }

        target: eventPump.selectionSyncCoordinator
    }

    Connections {
        function onRenderedBlocksChanged() {
            if (eventPump.contentsView.structuredHostGeometryActive) {
                if (eventPump.contentsView.hasPendingNoteEntryGutterRefresh(eventPump.contentsView.selectedNoteId)
                        && eventPump.structuredDocumentFlow
                        && eventPump.structuredDocumentFlow.scheduleLayoutCacheRefresh !== undefined) {
                    eventPump.structuredDocumentFlow.scheduleLayoutCacheRefresh();
                }
                eventPump.contentsView.scheduleMinimapSnapshotRefresh(true);
                eventPump.contentsView.scheduleGutterRefresh(1, "structured-line-geometry");
            }
        }

        target: eventPump.structuredBlockRenderer
    }

    Connections {
        function onCachedLogicalLineEntriesChanged() {
            if (!eventPump.contentsView.structuredHostGeometryActive)
                return;
            if (eventPump.contentsView.finalizePendingNoteEntryGutterRefresh(
                        eventPump.contentsView.selectedNoteId,
                        "structured-layout-cache")) {
                return;
            }
            const metricsChanged = eventPump.contentsView.refreshLiveLogicalLineMetrics();
            const geometryChanged = eventPump.contentsView.consumeStructuredGutterGeometryChange();
            if (!metricsChanged && !geometryChanged)
                return;
            eventPump.contentsView.scheduleMinimapSnapshotRefresh(true);
            eventPump.contentsView.scheduleGutterRefresh(
                        geometryChanged ? 2 : 1,
                        geometryChanged ? "structured-line-geometry" : "structured-line-metrics");
        }

        function onCurrentLogicalLineNumberChanged() {
            if (!eventPump.contentsView.structuredHostGeometryActive)
                return;
            eventPump.contentsView.scheduleCursorDrivenUiRefresh();
        }

        function onImplicitHeightChanged() {
            if (!eventPump.contentsView.structuredHostGeometryActive)
                return;
            eventPump.contentsView.scheduleMinimapSnapshotRefresh(true);
            eventPump.contentsView.scheduleGutterRefresh(2, "structured-line-geometry");
        }

        ignoreUnknownSignals: true
        target: eventPump.structuredDocumentFlow
    }

    Connections {
        function onEditorTextSynchronized() {
            if (eventPump.contentsView.showStructuredDocumentFlow) {
                if (!eventPump.contentsView.structuredHostGeometryActive) {
                    eventPump.contentsView.finalizePendingNoteEntryGutterRefresh(
                                eventPump.editorSession.editorBoundNoteId,
                                "editor-text-synchronized");
                }
                if (eventPump.contentsView.renderedEditorHtml !== "")
                    eventPump.contentsView.renderedEditorHtml = "";
                return;
            }
            if (eventPump.contentsView.finalizePendingNoteEntryGutterRefresh(
                        eventPump.editorSession.editorBoundNoteId,
                        "editor-text-synchronized")) {
                return;
            }
            eventPump.contentsView.scheduleMinimapSnapshotRefresh(true);
            eventPump.contentsView.scheduleDocumentPresentationRefresh(true);
            eventPump.contentsView.scheduleGutterRefresh(4);
        }

        target: eventPump.editorSession
    }

    Connections {
        function onActiveBlockCursorRevisionChanged() {
            eventPump.contentsView.scheduleCursorDrivenUiRefresh();
            eventPump.contentsView.scheduleTypingViewportCorrection(false);
        }

        function onImplicitHeightChanged() {
            eventPump.contentsView.scheduleTypingViewportCorrection(true);
        }

        function onFocusedChanged() {
            if (eventPump.structuredDocumentFlow && eventPump.structuredDocumentFlow.focused)
                eventPump.contentsView.scheduleTypingViewportCorrection(true);
        }

        function onVisibleChanged() {
            EditorTrace.trace(
                        "displayView",
                        "structuredDocumentFlowVisibleChanged",
                        "visible=" + eventPump.structuredDocumentFlow.visible
                        + " " + eventPump.contentsView.describeEditorSurfaceObject(eventPump.structuredDocumentFlow),
                        eventPump.contentsView)
            eventPump.contentsView.logEditorCreationState("structuredDocumentFlowVisibleChanged");
            eventPump.contentsView.scheduleSelectionModelSync({
                                                                  "scheduleReconcile": true
                                                              });
        }

        ignoreUnknownSignals: true
        target: eventPump.contentsView && eventPump.contentsView.showStructuredDocumentFlow
                ? eventPump.structuredDocumentFlow
                : null
    }

    Connections {
        function onContentYChanged() {
            eventPump.contentsView.scheduleViewportGutterRefresh();
        }

        function onHeightChanged() {
            eventPump.contentsView.scheduleViewportGutterRefresh();
            eventPump.contentsView.scheduleTypingViewportCorrection(true);
        }

        function onWidthChanged() {
            eventPump.contentsView.scheduleGutterRefresh(2);
        }

        target: eventPump.contentsView ? eventPump.contentsView.editorFlickable : null
    }
}
