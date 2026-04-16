pragma ComponentBehavior: Bound

import QtQuick
import "ContentsEditorDebugTrace.js" as EditorTrace

QtObject {
    id: controller
    objectName: "contentsResourceImportController"

    property var contentEditor: null
    property var editorViewport: null
    property var structuredDocumentFlow: null
    property var editorSession: null
    property var editorTypingController: null
    property var editorProjection: null
    property var bodyResourceRenderer: null
    property bool preferNativeInputHandling: false
    property bool resourceBlocksRenderedInlineByRichTextEditor: false
    property bool showPrintEditorLayout: false
    property real printPaperTextWidth: 0
    property int editorHorizontalInset: 0
    property int resourceEditorPlaceholderLineCount: 1
    property bool richTextInlineImageRenderingEnabled: false
    property string editorText: ""
    property string documentPresentationSourceText: ""
    property string selectedNoteId: ""
    property string selectedNoteBodyNoteId: ""
    property string selectedNoteBodyText: ""
    property bool showStructuredDocumentFlow: false
    property bool hasSelectedNote: false
    property bool showDedicatedResourceViewer: false
    property bool showFormattedTextRenderer: false
    property var resourcesImportViewModel: null
    property int resourceImportModeNone: 0
    property int resourceImportModeUrls: 1
    property int resourceImportModeClipboard: 2
    property int resourceImportConflictPolicyAbort: 0
    property var currentEditorCursorPositionHandler: null
    property var resourceTagTextForImportedEntryHandler: null
    property var encodeXmlAttributeValueHandler: null
    property var scheduleEditorRichTextSurfaceSyncHandler: null
    property var clearResourceDropActiveHandler: null
    property var clipboardImageAvailableHandler: null
    readonly property bool resourceDropEditorSurfaceGuardActive: editorSurfaceGuardController.resourceDropEditorSurfaceGuardActive
    readonly property bool resourceImportConflictAlertOpen: resourceImportConflictController.resourceImportConflictAlertOpen
    readonly property bool programmaticEditorSurfaceSyncActive: editorSurfaceGuardController.programmaticEditorSurfaceSyncActive

    property QtObject dropPayloadParser: ContentsResourceDropPayloadParser {
        id: dropPayloadParser
    }

    property QtObject resourceTagController: ContentsResourceTagController {
        id: resourceTagController

        bodyResourceRenderer: controller.bodyResourceRenderer
        documentPresentationSourceText: controller.documentPresentationSourceText
        editorSession: controller.editorSession
        editorText: controller.editorText
        editorTypingController: controller.editorTypingController
        currentEditorCursorPositionHandler: controller.currentEditorCursorPositionHandler
        resourceTagTextForImportedEntryHandler: controller.resourceTagTextForImportedEntryHandler
        selectedNoteBodyNoteId: controller.selectedNoteBodyNoteId
        selectedNoteBodyText: controller.selectedNoteBodyText
        selectedNoteId: controller.selectedNoteId
        showStructuredDocumentFlow: controller.showStructuredDocumentFlow
        structuredDocumentFlow: controller.structuredDocumentFlow
    }

    property QtObject inlineResourcePresentationController: ContentsInlineResourcePresentationController {
        id: inlineResourcePresentationController

        bodyResourceRenderer: controller.bodyResourceRenderer
        contentEditor: controller.contentEditor
        editorHorizontalInset: controller.editorHorizontalInset
        editorViewport: controller.editorViewport
        encodeXmlAttributeValueHandler: controller.encodeXmlAttributeValueHandler
        printPaperTextWidth: controller.printPaperTextWidth
        resourceEditorPlaceholderLineCount: controller.resourceEditorPlaceholderLineCount
        resourceTagController: resourceTagController
        richTextInlineImageRenderingEnabled: controller.richTextInlineImageRenderingEnabled
        showPrintEditorLayout: controller.showPrintEditorLayout
    }

    property QtObject editorSurfaceGuardController: ContentsEditorSurfaceGuardController {
        id: editorSurfaceGuardController

        contentEditor: controller.contentEditor
        editorProjection: controller.editorProjection
        preferNativeInputHandling: controller.preferNativeInputHandling
    }

    property QtObject resourceImportConflictController: ContentsResourceImportConflictController {
        id: resourceImportConflictController

        clearResourceDropActiveHandler: controller.clearResourceDropActiveHandler
        clipboardImageAvailableHandler: controller.clipboardImageAvailableHandler
        editorSurfaceGuardController: editorSurfaceGuardController
        hasSelectedNote: controller.hasSelectedNote
        resourceImportConflictPolicyAbort: controller.resourceImportConflictPolicyAbort
        resourceImportModeClipboard: controller.resourceImportModeClipboard
        resourceImportModeNone: controller.resourceImportModeNone
        resourceImportModeUrls: controller.resourceImportModeUrls
        resourceTagController: resourceTagController
        resourcesImportViewModel: controller.resourcesImportViewModel
        showDedicatedResourceViewer: controller.showDedicatedResourceViewer
        showFormattedTextRenderer: controller.showFormattedTextRenderer
    }

    function canAcceptResourceDropUrls(urls) {
        EditorTrace.trace("resourceImportController", "canAcceptResourceDropUrls", "urlCount=" + (Array.isArray(urls) ? urls.length : 0), controller)
        return resourceImportConflictController.canAcceptResourceDropUrls(urls);
    }

    function clearPendingResourceImportConflict() {
        EditorTrace.trace("resourceImportController", "clearPendingResourceImportConflict", "", controller)
        resourceImportConflictController.clearPendingResourceImportConflict();
    }

    function normalizedResourceImportConflict(conflict) {
        return resourceImportConflictController.normalizedResourceImportConflict(conflict);
    }

    function resourceImportConflictAlertMessage() {
        return resourceImportConflictController.resourceImportConflictAlertMessage();
    }

    function scheduleResourceImportConflictPrompt(importMode, urls, conflict) {
        EditorTrace.trace(
                    "resourceImportController",
                    "scheduleResourceImportConflictPrompt",
                    "importMode=" + importMode
                    + " urlCount=" + (Array.isArray(urls) ? urls.length : 0)
                    + " conflict={" + EditorTrace.describeObject(conflict) + "}",
                    controller)
        return resourceImportConflictController.scheduleResourceImportConflictPrompt(importMode, urls, conflict);
    }

    function finalizeInsertedImportedResources(importedEntries) {
        EditorTrace.trace("resourceImportController", "finalizeInsertedImportedResources", "entryCount=" + (Array.isArray(importedEntries) ? importedEntries.length : 0), controller)
        return resourceImportConflictController.finalizeInsertedImportedResources(importedEntries);
    }

    function cancelPendingResourceImportConflict() {
        EditorTrace.trace("resourceImportController", "cancelPendingResourceImportConflict", "", controller)
        resourceImportConflictController.cancelPendingResourceImportConflict();
    }

    function executePendingResourceImportWithPolicy(conflictPolicy) {
        EditorTrace.trace("resourceImportController", "executePendingResourceImportWithPolicy", "conflictPolicy=" + conflictPolicy, controller)
        return resourceImportConflictController.executePendingResourceImportWithPolicy(conflictPolicy);
    }

    function importUrlsAsResourcesWithPrompt(urls) {
        EditorTrace.trace("resourceImportController", "importUrlsAsResourcesWithPrompt", "urlCount=" + (Array.isArray(urls) ? urls.length : 0), controller)
        return resourceImportConflictController.importUrlsAsResourcesWithPrompt(urls);
    }

    function extractResourceDropUrls(drop) {
        EditorTrace.trace("resourceImportController", "extractResourceDropUrls", "hasDrop=" + !!drop, controller)
        return dropPayloadParser.extractResourceDropUrls(drop);
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

    function renderEditorSurfaceHtmlWithInlineResources(editorHtml) {
        EditorTrace.trace("resourceImportController", "renderEditorSurfaceHtmlWithInlineResources", EditorTrace.describeText(editorHtml), controller)
        return inlineResourcePresentationController.renderEditorSurfaceHtmlWithInlineResources(editorHtml);
    }

    function refreshInlineResourcePresentation() {
        EditorTrace.trace("resourceImportController", "refreshInlineResourcePresentation", "", controller)
        if (!controller.resourceBlocksRenderedInlineByRichTextEditor)
            return;
        const rendererRenderedText = controller.editorProjection
                && controller.editorProjection.editorSurfaceHtml !== undefined
                && controller.editorProjection.editorSurfaceHtml !== null
                ? String(controller.editorProjection.editorSurfaceHtml)
                : "";
        const nextRenderedText = inlineResourcePresentationController.renderEditorSurfaceHtmlWithInlineResources(rendererRenderedText);
        if (controller.editorProjection && controller.editorProjection.richTextSurfaceHtml !== nextRenderedText) {
            editorSurfaceGuardController.markProgrammaticEditorSurfaceSync();
            controller.editorProjection.richTextSurfaceHtml = nextRenderedText;
        }
        if (controller.scheduleEditorRichTextSurfaceSyncHandler
                && typeof controller.scheduleEditorRichTextSurfaceSyncHandler === "function") {
            controller.scheduleEditorRichTextSurfaceSyncHandler();
        }
    }

    function markProgrammaticEditorSurfaceSync() {
        EditorTrace.trace("resourceImportController", "markProgrammaticEditorSurfaceSync", "", controller)
        editorSurfaceGuardController.markProgrammaticEditorSurfaceSync();
    }

    function restoreEditorSurfaceFromPresentation() {
        EditorTrace.trace("resourceImportController", "restoreEditorSurfaceFromPresentation", "", controller)
        editorSurfaceGuardController.restoreEditorSurfaceFromPresentation();
    }

    function releaseResourceDropEditorSurfaceGuard(restoreSurface) {
        EditorTrace.trace("resourceImportController", "releaseResourceDropEditorSurfaceGuard", "restoreSurface=" + restoreSurface, controller)
        editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard(restoreSurface);
    }

    function insertImportedResourceTags(importedEntries) {
        EditorTrace.trace("resourceImportController", "insertImportedResourceTags", "entryCount=" + (Array.isArray(importedEntries) ? importedEntries.length : 0), controller)
        return resourceTagController.insertImportedResourceTags(importedEntries);
    }

    function pasteClipboardImageAsResource() {
        EditorTrace.trace("resourceImportController", "pasteClipboardImageAsResource", "", controller)
        return resourceImportConflictController.pasteClipboardImageAsResource();
    }

    Component.onCompleted: {
        EditorTrace.trace("resourceImportController", "mount", "", controller)
    }

    Component.onDestruction: {
        EditorTrace.trace("resourceImportController", "unmount", "", controller)
    }
}
