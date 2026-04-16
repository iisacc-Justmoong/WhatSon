pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property var contentEditor: null
    property var editorViewport: null
    property var structuredDocumentFlow: null
    property var editorSession: null
    property var editorTypingController: null
    property var textMetricsBridge: null
    property var textFormatRenderer: null
    property var bodyResourceRenderer: null
    readonly property bool resourceDropEditorSurfaceGuardActive: editorSurfaceGuardController.resourceDropEditorSurfaceGuardActive
    readonly property bool resourceImportConflictAlertOpen: resourceImportConflictController.resourceImportConflictAlertOpen
    readonly property bool programmaticEditorSurfaceSyncActive: editorSurfaceGuardController.programmaticEditorSurfaceSyncActive

    property QtObject dropPayloadParser: ContentsResourceDropPayloadParser {
        id: dropPayloadParser
    }

    property QtObject resourceTagController: ContentsResourceTagController {
        id: resourceTagController

        bodyResourceRenderer: controller.bodyResourceRenderer
        editorSession: controller.editorSession
        editorTypingController: controller.editorTypingController
        structuredDocumentFlow: controller.structuredDocumentFlow
        view: controller.view
    }

    property QtObject inlineResourcePresentationController: ContentsInlineResourcePresentationController {
        id: inlineResourcePresentationController

        bodyResourceRenderer: controller.bodyResourceRenderer
        contentEditor: controller.contentEditor
        editorViewport: controller.editorViewport
        resourceTagController: resourceTagController
        view: controller.view
    }

    property QtObject editorSurfaceGuardController: ContentsEditorSurfaceGuardController {
        id: editorSurfaceGuardController

        contentEditor: controller.contentEditor
        preferNativeInputHandling: controller.view ? controller.view.preferNativeInputHandling : false
        renderedEditorText: controller.view && controller.view.renderedEditorText !== undefined
                            ? controller.view.renderedEditorText
                            : ""
        textMetricsBridge: controller.textMetricsBridge
    }

    property QtObject resourceImportConflictController: ContentsResourceImportConflictController {
        id: resourceImportConflictController

        editorSurfaceGuardController: editorSurfaceGuardController
        resourceTagController: resourceTagController
        view: controller.view
    }

    function canAcceptResourceDropUrls(urls) {
        return resourceImportConflictController.canAcceptResourceDropUrls(urls);
    }

    function clearPendingResourceImportConflict() {
        resourceImportConflictController.clearPendingResourceImportConflict();
    }

    function normalizedResourceImportConflict(conflict) {
        return resourceImportConflictController.normalizedResourceImportConflict(conflict);
    }

    function resourceImportConflictAlertMessage() {
        return resourceImportConflictController.resourceImportConflictAlertMessage();
    }

    function scheduleResourceImportConflictPrompt(importMode, urls, conflict) {
        return resourceImportConflictController.scheduleResourceImportConflictPrompt(importMode, urls, conflict);
    }

    function finalizeInsertedImportedResources(importedEntries) {
        return resourceImportConflictController.finalizeInsertedImportedResources(importedEntries);
    }

    function cancelPendingResourceImportConflict() {
        resourceImportConflictController.cancelPendingResourceImportConflict();
    }

    function executePendingResourceImportWithPolicy(conflictPolicy) {
        return resourceImportConflictController.executePendingResourceImportWithPolicy(conflictPolicy);
    }

    function importUrlsAsResourcesWithPrompt(urls) {
        return resourceImportConflictController.importUrlsAsResourcesWithPrompt(urls);
    }

    function appendResourceDropPayloadLines(rawText, urls) {
        dropPayloadParser.appendResourceDropPayloadLines(rawText, urls);
    }

    function appendResourceDropMimePayload(drop, mimeType, urls) {
        dropPayloadParser.appendResourceDropMimePayload(drop, mimeType, urls);
    }

    function extractResourceDropUrls(drop) {
        return dropPayloadParser.extractResourceDropUrls(drop);
    }

    function normalizedImportedResourceEntries(importedEntries) {
        return resourceTagController.normalizedImportedResourceEntries(importedEntries);
    }

    function resourceBlockSourceText(tagTexts) {
        return resourceTagController.resourceBlockSourceText(tagTexts);
    }

    function sourceContainsCanonicalResourceTag(sourceText) {
        return resourceTagController.sourceContainsCanonicalResourceTag(sourceText);
    }

    function canonicalResourceTagCount(sourceText) {
        return resourceTagController.canonicalResourceTagCount(sourceText);
    }

    function resourceTagLossDetected(previousSourceText, nextSourceText) {
        return resourceTagController.resourceTagLossDetected(previousSourceText, nextSourceText);
    }

    function inlineResourcePreviewWidth() {
        return inlineResourcePresentationController.inlineResourcePreviewWidth();
    }

    function resourceEntryOpenTarget(resourceEntry) {
        return inlineResourcePresentationController.resourceEntryOpenTarget(resourceEntry);
    }

    function richTextParagraphHtml(innerHtml) {
        return inlineResourcePresentationController.richTextParagraphHtml(innerHtml);
    }

    function inlineResourcePlaceholderHtml(lineCount) {
        return inlineResourcePresentationController.inlineResourcePlaceholderHtml(lineCount);
    }

    function resourcePlaceholderBlockHtml() {
        return inlineResourcePresentationController.resourcePlaceholderBlockHtml();
    }

    function inlineResourceBlockHtml(resourceEntry) {
        return inlineResourcePresentationController.inlineResourceBlockHtml(resourceEntry);
    }

    function resourceEntryCanRenderInlineInRichText(resourceEntry) {
        return inlineResourcePresentationController.resourceEntryCanRenderInlineInRichText(resourceEntry);
    }

    function renderEditorSurfaceHtmlWithInlineResources(editorHtml) {
        return inlineResourcePresentationController.renderEditorSurfaceHtmlWithInlineResources(editorHtml);
    }

    function refreshInlineResourcePresentation() {
        if (!controller.view || !controller.view.resourceBlocksRenderedInlineByRichTextEditor)
            return;
        const rendererRenderedText = controller.textFormatRenderer
                && controller.textFormatRenderer.editorSurfaceHtml !== undefined
                && controller.textFormatRenderer.editorSurfaceHtml !== null
                ? String(controller.textFormatRenderer.editorSurfaceHtml)
                : "";
        const nextRenderedText = inlineResourcePresentationController.renderEditorSurfaceHtmlWithInlineResources(rendererRenderedText);
        if (controller.view.renderedEditorText !== nextRenderedText) {
            editorSurfaceGuardController.markProgrammaticEditorSurfaceSync();
            controller.view.renderedEditorText = nextRenderedText;
        }
        controller.view.scheduleEditorRichTextSurfaceSync();
    }

    function activateResourceDropEditorSurfaceGuard() {
        editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard();
    }

    function markProgrammaticEditorSurfaceSync() {
        editorSurfaceGuardController.markProgrammaticEditorSurfaceSync();
    }

    function restoreEditorSurfaceFromPresentation() {
        editorSurfaceGuardController.restoreEditorSurfaceFromPresentation();
    }

    function releaseResourceDropEditorSurfaceGuard(restoreSurface) {
        editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard(restoreSurface);
    }

    function insertImportedResourceTags(importedEntries) {
        return resourceTagController.insertImportedResourceTags(importedEntries);
    }

    function pasteClipboardImageAsResource() {
        return resourceImportConflictController.pasteClipboardImageAsResource();
    }
}
