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
    property var documentSourceMutationHandler: null
    property QtObject resourceInsertionPolicy: ContentsStructuredDocumentMutationPolicy {
    }
    property QtObject resourceTagTextGenerator: ContentsResourceTagTextGenerator {
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

    function currentResourceInsertionSourceOffset() {
        const currentSourceText = controller.editorText === undefined || controller.editorText === null
                ? ""
                : String(controller.editorText);
        if (controller.showStructuredDocumentFlow
                && controller.structuredDocumentFlow
                && controller.structuredDocumentFlow.shortcutInsertionSourceOffset !== undefined) {
            const structuredOffset = Number(
                        controller.structuredDocumentFlow.shortcutInsertionSourceOffset());
            if (isFinite(structuredOffset))
                return Math.max(0, Math.min(currentSourceText.length, Math.floor(structuredOffset)));
        }

        const logicalCursorOffset = Math.max(
                    0,
                    Math.floor(Number(controller.currentEditorCursorPosition()) || 0));
        if (controller.editorTypingController
                && controller.editorTypingController.sourceOffsetForCollapsedLogicalInsertion !== undefined) {
            const sourceOffset = Number(
                        controller.editorTypingController.sourceOffsetForCollapsedLogicalInsertion(
                            currentSourceText,
                            logicalCursorOffset));
            if (isFinite(sourceOffset))
                return Math.max(0, Math.min(currentSourceText.length, Math.floor(sourceOffset)));
        }
        if (controller.editorTypingController
                && controller.editorTypingController.sourceOffsetForLogicalOffset !== undefined) {
            const sourceOffset = Number(
                        controller.editorTypingController.sourceOffsetForLogicalOffset(
                            logicalCursorOffset));
            if (isFinite(sourceOffset))
                return Math.max(0, Math.min(currentSourceText.length, Math.floor(sourceOffset)));
        }
        return Math.max(0, Math.min(currentSourceText.length, logicalCursorOffset));
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
            const tagText = controller.resourceTagTextGenerator
                    && controller.resourceTagTextGenerator.buildCanonicalResourceTag !== undefined
                    ? String(controller.resourceTagTextGenerator.buildCanonicalResourceTag(
                                 normalizedImportedEntries[index]) || "")
                    : "";
            if (tagText.length > 0)
                tagTexts.push(tagText);
        }
        if (tagTexts.length === 0)
            return false;

        const currentSourceText = controller.editorText === undefined || controller.editorText === null
                ? ""
                : String(controller.editorText);
        const insertionOffset = controller.currentResourceInsertionSourceOffset();
        const insertionPayload = controller.resourceInsertionPolicy
                && controller.resourceInsertionPolicy.buildResourceInsertionPayload !== undefined
                ? controller.resourceInsertionPolicy.buildResourceInsertionPayload(
                      currentSourceText,
                      insertionOffset,
                      tagTexts)
                : ({});

        let inserted = false;
        const nextSourceText = insertionPayload.nextSourceText !== undefined
                && insertionPayload.nextSourceText !== null
                ? String(insertionPayload.nextSourceText)
                : currentSourceText;
        if (nextSourceText !== currentSourceText
                && controller.documentSourceMutationHandler
                && typeof controller.documentSourceMutationHandler === "function") {
            inserted = !!controller.documentSourceMutationHandler(
                        nextSourceText,
                        insertionPayload.focusRequest && typeof insertionPayload.focusRequest === "object"
                        ? insertionPayload.focusRequest
                        : ({ }));
        } else if (controller.editorTypingController
                   && controller.editorTypingController.insertRawSourceTextAtCursor !== undefined) {
            const blockSourceText = tagTexts.join("\n");
            inserted = controller.editorTypingController.insertRawSourceTextAtCursor(
                        blockSourceText,
                        blockSourceText.length);
        }

        if (!inserted)
            return false;
        return true;
    }
}
