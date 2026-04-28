pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: model

    property var contentsView: null
    property var presentationViewModel: null
    property var structuredDocumentFlow: null

    function logEditorCreationState(reason) {
        model.presentationViewModel.logEditorCreationState(reason);
    }

    function commitDocumentPresentationRefresh() {
        model.presentationViewModel.commitDocumentPresentationRefresh();
    }

    function documentPresentationRenderDirty() {
        return model.presentationViewModel.documentPresentationRenderDirty();
    }

    function refreshInlineResourcePresentation() {
        model.presentationViewModel.refreshInlineResourcePresentation();
    }

    function requestViewHook(reason) {
        model.presentationViewModel.requestViewHook(reason);
    }

    function applyPresentationRefreshPlan(plan) {
        model.presentationViewModel.applyPresentationRefreshPlan(plan);
    }

    function executeRefreshPlan(plan) {
        const refreshPlan = plan && typeof plan === "object" ? plan : ({});
        if (refreshPlan.resetNoteEntryLineGeometry)
            model.contentsView.resetNoteEntryLineGeometryState();
        if (refreshPlan.requestStructuredLayoutRefresh
                && model.structuredDocumentFlow
                && !model.contentsView.selectedNoteBodyLoading
                && model.contentsView.selectedNoteBodyNoteId === model.contentsView.selectedNoteId) {
            model.scheduleStructuredDocumentOpenLayoutRefresh(
                        String(refreshPlan.gutterReason || "note-entry"));
        }
        if (refreshPlan.scheduleViewportGutterRefresh)
            model.contentsView.scheduleViewportGutterRefresh();
        if (refreshPlan.gutterPassCount !== undefined) {
            model.contentsView.scheduleGutterRefresh(
                        Number(refreshPlan.gutterPassCount) || 0,
                        String(refreshPlan.gutterReason || ""));
        }
    }

    function scheduleStructuredDocumentOpenLayoutRefresh(reason) {
        if (!model.structuredDocumentFlow)
            return;
        if (model.structuredDocumentFlow.scheduleEditorOpenLayoutCacheRefresh !== undefined) {
            model.structuredDocumentFlow.scheduleEditorOpenLayoutCacheRefresh(
                        reason === undefined || reason === null ? "" : String(reason));
            return;
        }
        if (model.structuredDocumentFlow.scheduleLayoutCacheRefresh !== undefined)
            model.structuredDocumentFlow.scheduleLayoutCacheRefresh();
    }

    function scheduleDeferredDocumentPresentationRefresh() {
        model.presentationViewModel.scheduleDeferredDocumentPresentationRefresh();
    }

    function scheduleDocumentPresentationRefresh(forceImmediate) {
        model.presentationViewModel.scheduleDocumentPresentationRefresh(forceImmediate);
    }
}
