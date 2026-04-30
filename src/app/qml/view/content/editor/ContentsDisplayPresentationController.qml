pragma ComponentBehavior: Bound

import QtQuick
import "../../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace

QtObject {
    id: controller

    property var contentsView: null
    property var documentSourceResolver: null
    property var editorProjection: null
    property var editorSession: null
    property var editorTypingController: null
    property var eventPump: null
    property var minimapLayer: null
    property var noteBodyMountCoordinator: null
    property var panelViewModel: null
    property var presentationRefreshController: null
    property var resourceImportController: null
    property var selectionBridge: null
    property var structuredDocumentFlow: null
    property var traceFormatter: null

    function logEditorCreationState(reason) {
        const normalizedReason = reason === undefined || reason === null ? "" : String(reason);
        EditorTrace.trace(
                    "displayView",
                    "editorCreationState",
                    "reason=" + normalizedReason
                    + " selectedNoteId=" + controller.contentsView.selectedNoteId
                    + " bodyNoteId=" + controller.contentsView.selectedNoteBodyNoteId
                    + " bodyResolved=" + controller.contentsView.selectedNoteBodyResolved
                    + " bodyLoading=" + controller.contentsView.selectedNoteBodyLoading
                    + " editorBoundNoteId=" + controller.contentsView.editorBoundNoteId
                    + " sessionBound=" + controller.contentsView.editorSessionBoundToSelectedNote
                    + " structuredRequested=" + controller.contentsView.structuredDocumentFlowRequested
                    + " structuredVisible=" + controller.contentsView.showStructuredDocumentFlow
                    + " activeSurface=" + controller.contentsView.activeSurfaceKind
                    + " mountPending=" + controller.contentsView.noteDocumentMountPending
                    + " mounted=" + controller.contentsView.noteDocumentMounted
                    + " mountFailureReason=" + controller.contentsView.noteDocumentMountFailureReason,
                    controller.contentsView)
        EditorTrace.trace(
                    "displayView",
                    "editorCreationSurfaces",
                    "reason=" + normalizedReason
                    + " activeSurface=" + controller.contentsView.activeSurfaceKind
                    + " structuredFlow={" + controller.traceFormatter.describeEditorSurfaceObject(controller.structuredDocumentFlow) + "}",
                    controller.contentsView)
        EditorTrace.trace(
                    "displayView",
                    "editorCreationCoordinators",
                    "reason=" + normalizedReason
                    + " selectionBridge={"
                    + EditorTrace.describeObject(controller.selectionBridge, [
                                                    "objectName",
                                                    "selectedNoteId",
                                                    "selectedNoteBodyNoteId",
                                                    "selectedNoteBodyResolved",
                                                    "selectedNoteBodyLoading"
                                                ]) + "}"
                    + " mountCoordinator={"
                    + EditorTrace.describeObject(controller.noteBodyMountCoordinator, [
                                                    "objectName",
                                                    "mountPending",
                                                    "noteMounted",
                                                    "mountFailed",
                                                    "mountFailureReason"
                                                ]) + "}"
                    + " documentSourceResolver={"
                    + EditorTrace.describeObject(controller.documentSourceResolver, [
                                                    "selectedNoteId",
                                                    "selectedNoteBodyNoteId",
                                                    "selectedNoteBodyResolved",
                                                    "editorBoundNoteId"
                                                ]) + "}"
                    + " editorSession={"
                    + EditorTrace.describeObject(controller.editorSession, [
                                                    "objectName",
                                                    "editorBoundNoteId",
                                                    "pendingBodySave",
                                                    "syncingEditorTextFromModel"
                                                ]) + "}",
                    controller.contentsView)
    }

    function commitDocumentPresentationRefresh() {
        EditorTrace.trace(
                    "displayView",
                    "commitDocumentPresentationRefresh",
                    "projectionEnabled=" + controller.contentsView.documentPresentationProjectionEnabled
                    + " structured=" + controller.contentsView.showStructuredDocumentFlow
                    + " " + EditorTrace.describeText(controller.contentsView.documentPresentationSourceText),
                    controller.contentsView)
        const needsHtmlProjection = controller.contentsView.documentPresentationProjectionEnabled;
        if (!needsHtmlProjection) {
            if (controller.contentsView.renderedEditorHtml !== "")
                controller.contentsView.renderedEditorHtml = "";
            controller.contentsView.scheduleMinimapSnapshotRefresh(false);
            if (controller.minimapLayer && controller.contentsView.minimapRefreshEnabled)
                controller.minimapLayer.requestRepaint();
            controller.editorTypingController.synchronizeLiveEditingStateFromPresentation();
            return;
        }
        const nextRenderedText = controller.editorProjection
                && controller.editorProjection.editorSurfaceHtml !== undefined
                && controller.editorProjection.editorSurfaceHtml !== null
                ? String(controller.editorProjection.editorSurfaceHtml)
                : "";
        const editorRenderedText = controller.contentsView.resourceBlocksRenderedInlineByHtmlProjection
                ? controller.resourceImportController.renderEditorSurfaceHtmlWithInlineResources(nextRenderedText)
                : nextRenderedText;
        if (controller.contentsView.renderedEditorHtml !== editorRenderedText)
            controller.contentsView.renderedEditorHtml = editorRenderedText;
        controller.contentsView.scheduleMinimapSnapshotRefresh(false);
        if (controller.minimapLayer && controller.contentsView.minimapRefreshEnabled)
            controller.minimapLayer.requestRepaint();
        controller.editorTypingController.synchronizeLiveEditingStateFromPresentation();
    }

    function documentPresentationRenderDirty() {
        const needsHtmlProjection = controller.contentsView.documentPresentationProjectionEnabled;
        if (!needsHtmlProjection)
            return controller.contentsView.renderedEditorHtml !== "";
        const rendererRenderedText = controller.editorProjection
                && controller.editorProjection.editorSurfaceHtml !== undefined
                && controller.editorProjection.editorSurfaceHtml !== null
                ? String(controller.editorProjection.editorSurfaceHtml)
                : "";
        const expectedRenderedText = controller.contentsView.resourceBlocksRenderedInlineByHtmlProjection
                ? controller.resourceImportController.renderEditorSurfaceHtmlWithInlineResources(rendererRenderedText)
                : rendererRenderedText;
        return controller.contentsView.renderedEditorHtml !== expectedRenderedText;
    }

    function refreshInlineResourcePresentation() {
        controller.commitDocumentPresentationRefresh();
    }

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        EditorTrace.trace("displayView", "requestViewHook", "reason=" + hookReason, controller.contentsView)
        if (controller.panelViewModel && controller.panelViewModel.requestViewModelHook)
            controller.panelViewModel.requestViewModelHook(hookReason);
        controller.contentsView.viewHookRequested();
    }

    function applyPresentationRefreshPlan(plan) {
        const refreshPlan = plan && typeof plan === "object" ? plan : ({});
        EditorTrace.trace(
                    "displayView",
                    "applyPresentationRefreshPlan",
                    "reason=" + String(refreshPlan.reason || "")
                    + " clear=" + !!refreshPlan.clearPresentation
                    + " commit=" + !!refreshPlan.commitRefresh
                    + " startTimer=" + !!refreshPlan.startTimer
                    + " stopTimer=" + !!refreshPlan.stopTimer,
                    controller.contentsView)
        controller.eventPump.applyPresentationRefreshTimerPlan(refreshPlan);
        if (refreshPlan.clearPresentation && controller.contentsView.renderedEditorHtml !== "")
            controller.contentsView.renderedEditorHtml = "";
        if (refreshPlan.commitRefresh)
            controller.commitDocumentPresentationRefresh();
        if (refreshPlan.requestMinimapRefresh)
            controller.contentsView.scheduleMinimapSnapshotRefresh(false);
        if (refreshPlan.requestMinimapRepaint && controller.minimapLayer && controller.contentsView.minimapRefreshEnabled)
            controller.minimapLayer.requestRepaint();
    }

    function scheduleDeferredDocumentPresentationRefresh() {
        controller.applyPresentationRefreshPlan(controller.presentationRefreshController.planDeferredRequest());
    }

    function scheduleDocumentPresentationRefresh(forceImmediate) {
        const immediate = !!forceImmediate;
        EditorTrace.trace(
                    "displayView",
                    "scheduleDocumentPresentationRefresh",
                    "immediate=" + immediate
                    + " focused=" + controller.contentsView.editorInputFocused
                    + " typingProtected=" + controller.contentsView.typingSessionSyncProtected
                    + " structured=" + controller.contentsView.showStructuredDocumentFlow,
                    controller.contentsView)
        controller.applyPresentationRefreshPlan(
                    controller.presentationRefreshController.planRefreshRequest(immediate));
    }
}
