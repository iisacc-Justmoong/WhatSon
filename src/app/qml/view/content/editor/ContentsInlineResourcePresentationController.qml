pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var contentEditor: null
    property var editorViewport: null
    property var bodyResourceRenderer: null
    property var resourceTagController: null
    property bool showPrintEditorLayout: false
    property real printPaperTextWidth: 0
    property int editorHorizontalInset: 0
    property int resourceEditorPlaceholderLineCount: 1
    property bool richTextInlineImageRenderingEnabled: false
    property var encodeXmlAttributeValueHandler: null

    function inlineResourcePreviewWidth() {
        if (controller.showPrintEditorLayout)
            return Math.max(120, Math.floor(controller.printPaperTextWidth));

        const editorWidth = controller.contentEditor && controller.contentEditor.width !== undefined
                ? Number(controller.contentEditor.width) || 0
                : 0;
        const viewportWidth = controller.editorViewport ? Number(controller.editorViewport.width) || 0 : 0;
        const availableWidth = Math.max(editorWidth, viewportWidth) - controller.editorHorizontalInset * 2;
        return Math.max(120, Math.floor(Math.max(120, availableWidth)));
    }

    function resourceEntryOpenTarget(resourceEntry) {
        const safeEntry = resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({});
        const sourceUrl = safeEntry.source !== undefined ? String(safeEntry.source).trim() : "";
        if (sourceUrl.length > 0)
            return sourceUrl;
        const resolvedPath = safeEntry.resolvedPath !== undefined ? String(safeEntry.resolvedPath).trim() : "";
        return resolvedPath;
    }

    function richTextParagraphHtml(innerHtml) {
        const paragraphBody = innerHtml === undefined || innerHtml === null || String(innerHtml).length === 0
                ? "&nbsp;"
                : String(innerHtml);
        return "<p style=\"margin-top:0px;margin-bottom:0px;\">" + paragraphBody + "</p>";
    }

    function inlineResourcePlaceholderHtml(lineCount) {
        const lines = [];
        const placeholderLineCount = Math.max(0, Math.floor(Number(lineCount) || 0));
        for (let index = 0; index < placeholderLineCount; ++index)
            lines.push(controller.richTextParagraphHtml("&nbsp;"));
        return lines.join("");
    }

    function resourcePlaceholderBlockHtml() {
        return controller.inlineResourcePlaceholderHtml(controller.resourceEditorPlaceholderLineCount);
    }

    function inlineResourceBlockHtml(resourceEntry) {
        const safeEntry = resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({});
        const renderMode = safeEntry.renderMode !== undefined ? String(safeEntry.renderMode).trim().toLowerCase() : "";
        const sourceUrl = controller.resourceEntryOpenTarget(safeEntry);
        const encodedSourceUrl = controller.encodeXmlAttributeValueHandler
                && typeof controller.encodeXmlAttributeValueHandler === "function"
                ? controller.encodeXmlAttributeValueHandler(sourceUrl)
                : sourceUrl;
        const previewWidth = controller.inlineResourcePreviewWidth();
        if (renderMode === "image" && encodedSourceUrl.length > 0) {
            return "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" style=\"margin-top:0px;margin-bottom:0px;\">"
                    + "<tr><td align=\"center\">"
                    + "<img src=\"" + encodedSourceUrl + "\" width=\"" + String(previewWidth) + "\" />"
                    + "</td></tr></table>";
        }
        return controller.resourcePlaceholderBlockHtml();
    }

    function resourceEntryCanRenderInlineInRichText(resourceEntry) {
        const safeEntry = resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({});
        const renderMode = safeEntry.renderMode !== undefined ? String(safeEntry.renderMode).trim().toLowerCase() : "";
        const sourceUrl = controller.resourceEntryOpenTarget(safeEntry);
        return !!(controller.richTextInlineImageRenderingEnabled
                  && renderMode === "image"
                  && sourceUrl.length > 0);
    }

    function renderEditorSurfaceHtmlWithInlineResources(editorHtml) {
        const baseEditorHtml = editorHtml === undefined || editorHtml === null ? "" : String(editorHtml);
        const renderedResources = controller.resourceTagController
                ? controller.resourceTagController.normalizedImportedResourceEntries(
                      controller.bodyResourceRenderer ? controller.bodyResourceRenderer.renderedResources : [])
                : [];
        return baseEditorHtml.replace(
                    /<!--whatson-resource-block:(\d+)-->[\s\S]*?<!--\/whatson-resource-block:\1-->/g,
                    function (_match, resourceIndexText) {
                        const resourceIndex = Math.max(0, Math.floor(Number(resourceIndexText) || 0));
                        const entry = resourceIndex < renderedResources.length ? renderedResources[resourceIndex] : null;
                        return controller.resourceEntryCanRenderInlineInRichText(entry)
                                ? controller.inlineResourceBlockHtml(entry)
                                : controller.resourcePlaceholderBlockHtml();
                    });
    }
}
