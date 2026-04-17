pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0

QtObject {
    id: controller

    property var editorSession: null
    property var editorTypingController: null
    property var structuredDocumentFlow: null
    property var bodyResourceRenderer: null
    property string editorText: ""
    property string documentPresentationSourceText: ""
    property string selectedNoteId: ""
    property string selectedNoteBodyNoteId: ""
    property string selectedNoteBodyText: ""
    property bool showStructuredDocumentFlow: false
    property var currentEditorCursorPositionHandler: null

    ContentsResourceTagTextGenerator {
        id: resourceTagTextGenerator
    }

    function currentEditorCursorPosition() {
        if (controller.currentEditorCursorPositionHandler
                && typeof controller.currentEditorCursorPositionHandler === "function") {
            return Number(controller.currentEditorCursorPositionHandler()) || 0;
        }
        return 0;
    }

    function normalizedImportedResourceEntries(importedEntries) {
        if (Array.isArray(importedEntries))
            return importedEntries;
        if (!importedEntries)
            return [];

        const explicitLength = Number(importedEntries.length);
        if (isFinite(explicitLength) && explicitLength >= 0) {
            const normalizedEntries = [];
            for (let index = 0; index < Math.floor(explicitLength); ++index)
                normalizedEntries.push(importedEntries[index]);
            return normalizedEntries;
        }

        const explicitCount = Number(importedEntries.count);
        if (isFinite(explicitCount) && explicitCount >= 0) {
            const normalizedEntries = [];
            for (let index = 0; index < Math.floor(explicitCount); ++index)
                normalizedEntries.push(importedEntries[index]);
            return normalizedEntries;
        }

        const indexedKeys = Object.keys(importedEntries).filter(function (key) {
            return /^\d+$/.test(key);
        }).sort(function (lhs, rhs) {
            return Number(lhs) - Number(rhs);
        });
        if (indexedKeys.length === 0)
            return [];

        const normalizedEntries = [];
        for (let index = 0; index < indexedKeys.length; ++index)
            normalizedEntries.push(importedEntries[indexedKeys[index]]);
        return normalizedEntries;
    }

    function resourceBlockSourceText(tagTexts) {
        if (!Array.isArray(tagTexts) || tagTexts.length === 0)
            return "";

        const blockSourceText = tagTexts.join("\n");
        const currentSourceText = controller.editorText === undefined || controller.editorText === null
                ? ""
                : String(controller.editorText);
        const cursorSourceOffset = controller.editorTypingController
                && controller.editorTypingController.sourceOffsetForLogicalOffset !== undefined
                ? controller.editorTypingController.sourceOffsetForLogicalOffset(
                      controller.currentEditorCursorPosition())
                : controller.currentEditorCursorPosition();
        const boundedCursorOffset = Math.max(0, Math.min(currentSourceText.length, Number(cursorSourceOffset) || 0));
        const previousCharacter = boundedCursorOffset > 0 ? currentSourceText.charAt(boundedCursorOffset - 1) : "";
        const nextCharacter = boundedCursorOffset < currentSourceText.length ? currentSourceText.charAt(boundedCursorOffset) : "";
        const leadingBreak = currentSourceText.length > 0 && previousCharacter !== "\n" ? "\n" : "";
        const trailingBreak = boundedCursorOffset < currentSourceText.length && nextCharacter !== "\n" ? "\n" : "";
        return leadingBreak + blockSourceText + trailingBreak;
    }

    function sourceContainsCanonicalResourceTag(sourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        if (normalizedSourceText.length === 0)
            return false;
        return /<resource\b[^>]*\/?>/i.test(normalizedSourceText);
    }

    function canonicalResourceTagCount(sourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        if (normalizedSourceText.length === 0)
            return 0;
        const matches = normalizedSourceText.match(/<resource\b[^>]*\/?>/gi);
        return matches ? matches.length : 0;
    }

    function resourceTagLossDetected(previousSourceText, nextSourceText) {
        if (controller.showStructuredDocumentFlow)
            return false;

        const selectedBodyCount = ((!(controller.editorSession && controller.editorSession.localEditorAuthority))
                                   && controller.selectedNoteBodyNoteId === controller.selectedNoteId)
                ? controller.canonicalResourceTagCount(controller.selectedNoteBodyText)
                : 0;
        const baselineCount = Math.max(
                    controller.canonicalResourceTagCount(previousSourceText),
                    controller.canonicalResourceTagCount(controller.documentPresentationSourceText),
                    selectedBodyCount);
        return baselineCount > controller.canonicalResourceTagCount(nextSourceText);
    }

    function insertImportedResourceTags(importedEntries) {
        const normalizedImportedEntries = controller.normalizedImportedResourceEntries(importedEntries);
        if (normalizedImportedEntries.length === 0)
            return false;

        const tagTexts = [];
        for (let index = 0; index < normalizedImportedEntries.length; ++index) {
            const tagText = resourceTagTextGenerator
                    && resourceTagTextGenerator.buildCanonicalResourceTag !== undefined
                    ? String(resourceTagTextGenerator.buildCanonicalResourceTag(
                                 normalizedImportedEntries[index]) || "")
                    : "";
            if (tagText.length > 0)
                tagTexts.push(tagText);
        }
        if (tagTexts.length === 0)
            return false;

        let inserted = false;
        if (controller.showStructuredDocumentFlow
                && controller.structuredDocumentFlow
                && controller.structuredDocumentFlow.insertResourceBlocksAtActivePosition !== undefined) {
            inserted = controller.structuredDocumentFlow.insertResourceBlocksAtActivePosition(tagTexts);
        } else if (controller.editorTypingController
                   && controller.editorTypingController.insertRawSourceTextAtCursor !== undefined) {
            const insertedSourceText = controller.resourceBlockSourceText(tagTexts);
            inserted = controller.editorTypingController.insertRawSourceTextAtCursor(
                        insertedSourceText,
                        insertedSourceText.length);
        }

        if (!inserted)
            return false;
        if (controller.bodyResourceRenderer
                && controller.bodyResourceRenderer.requestRenderRefresh !== undefined) {
            controller.bodyResourceRenderer.requestRenderRefresh();
        }
        return inserted;
    }
}
