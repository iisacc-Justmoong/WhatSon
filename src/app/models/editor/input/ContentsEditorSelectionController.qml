pragma ComponentBehavior: Bound

import QtQuick
import "../format/ContentsRawInlineStyleMutationSupport.js" as RawInlineStyleMutationSupport

QtObject {
    id: controller

    property var contentEditor: null
    readonly property var contextMenuItems: [
        {
            "label": "Plain",
            "keyVisible": false,
            "eventName": "editor.format.plain"
        },
        {
            "label": "Bold",
            "keyVisible": false,
            "eventName": "editor.format.bold"
        },
        {
            "label": "Italic",
            "keyVisible": false,
            "eventName": "editor.format.italic"
        },
        {
            "label": "Underline",
            "keyVisible": false,
            "eventName": "editor.format.underline"
        },
        {
            "label": "Strikethrough",
            "keyVisible": false,
            "eventName": "editor.format.strikethrough"
        },
        {
            "label": "Highlight",
            "keyVisible": false,
            "eventName": "editor.format.highlight"
        }
    ]
    property int contextMenuSelectionEnd: -1
    property real contextMenuSelectionCursorPosition: NaN
    property int contextMenuSelectionStart: -1
    property string contextMenuSelectionText: ""
    property var editorSession: null
    property var editorViewport: null
    property var selectionBridge: null
    property var selectionContextMenu: null
    property var textMetricsBridge: null
    property var view: null

    function preferNativeInputHandling() {
        return !!(controller.view
                  && controller.view.preferNativeInputHandling !== undefined
                  && controller.view.preferNativeInputHandling);
    }
    function contextMenuEditorSelectionRange() {
        if (controller.contextMenuSelectionEnd <= controller.contextMenuSelectionStart)
            return ({
                    "start": -1,
                    "end": -1
                });
        return ({
                "start": controller.contextMenuSelectionStart,
                "end": controller.contextMenuSelectionEnd
            });
    }
    function contextMenuEditorSelectionSnapshot() {
        const selectionRange = controller.contextMenuEditorSelectionRange();
        return {
            "start": selectionRange.start,
            "end": selectionRange.end,
            "selectedText": controller.contextMenuSelectionText,
            "cursorPosition": isFinite(controller.contextMenuSelectionCursorPosition)
                              ? Number(controller.contextMenuSelectionCursorPosition)
                              : controller.currentEditorCursorPosition()
        };
    }
    function currentInlineFormatSelectionSnapshot() {
        if (!controller.contentEditor)
            return {
                "cursorPosition": NaN,
                "selectedText": "",
                "selectionEnd": NaN,
                "selectionStart": NaN
            };
        if (controller.contentEditor.inlineFormatSelectionSnapshot !== undefined) {
            const inlineSnapshot = controller.contentEditor.inlineFormatSelectionSnapshot();
            if (inlineSnapshot)
                return inlineSnapshot;
        }
        if (controller.contentEditor.selectionSnapshot !== undefined) {
            const snapshot = controller.contentEditor.selectionSnapshot();
            if (snapshot)
                return snapshot;
        }
        return {
            "cursorPosition": controller.contentEditor.cursorPosition,
            "selectedText": controller.contentEditor.selectedText,
            "selectionEnd": controller.contentEditor.selectionEnd,
            "selectionStart": controller.contentEditor.selectionStart
        };
    }
    function currentEditorCursorPosition() {
        if (!controller.contentEditor)
            return NaN;
        const selectionSnapshot = controller.currentInlineFormatSelectionSnapshot();
        if (selectionSnapshot && selectionSnapshot.cursorPosition !== undefined) {
            const selectionCursor = Number(selectionSnapshot.cursorPosition);
            if (isFinite(selectionCursor))
                return selectionCursor;
        }
        if (controller.contentEditor.cursorPosition !== undefined) {
            const cursor = Number(controller.contentEditor.cursorPosition);
            if (isFinite(cursor))
                return cursor;
        }
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.cursorPosition !== undefined) {
            const cursor = Number(controller.contentEditor.editorItem.cursorPosition);
            if (isFinite(cursor))
                return cursor;
        }
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.inputItem && controller.contentEditor.editorItem.inputItem.cursorPosition !== undefined) {
            const cursor = Number(controller.contentEditor.editorItem.inputItem.cursorPosition);
            if (isFinite(cursor))
                return cursor;
        }

    }
    function currentEditorInputItem() {
        if (!controller.contentEditor)
            return null;
        if (controller.contentEditor.getFormattedText !== undefined && controller.contentEditor.getText !== undefined)
            return controller.contentEditor;
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.inputItem)
            return controller.contentEditor.editorItem.inputItem;
        if (controller.contentEditor.editorItem)
            return controller.contentEditor.editorItem;

    }
    function currentEditorPlainText() {
        const surfaceLength = controller.currentEditorSurfaceLength();
        if (controller.contentEditor && controller.contentEditor.currentPlainText !== undefined)
            return controller.normalizeSelectionTextValue(controller.contentEditor.currentPlainText());
        if (controller.contentEditor && controller.contentEditor.getText !== undefined)
            return controller.normalizeSelectionTextValue(controller.contentEditor.getText(0, surfaceLength));
        const inputItem = controller.currentEditorInputItem();
        if (inputItem && inputItem.getText !== undefined)
            return controller.normalizeSelectionTextValue(inputItem.getText(0, surfaceLength));
        return controller.normalizeSelectionTextValue(controller.currentLogicalText());
    }
    function currentEditorSelectionSnapshot() {
        if (!controller.contentEditor)
            return {
                "cursorPosition": NaN,
                "selectedText": "",
                "selectionEnd": NaN,
                "selectionStart": NaN
            };
        return controller.currentInlineFormatSelectionSnapshot();
    }
    function currentRawEditorSelectionRange() {
        if (!controller.contentEditor)
            return ({
                    "start": -1,
                    "end": -1
                });
        const selectionSnapshot = controller.currentEditorSelectionSnapshot();
        let start = selectionSnapshot.selectionStart !== undefined ? Number(selectionSnapshot.selectionStart) : NaN;
        let end = selectionSnapshot.selectionEnd !== undefined ? Number(selectionSnapshot.selectionEnd) : NaN;
        if (!isFinite(start) || !isFinite(end)) {
            if (controller.contentEditor.editorItem) {
                if (!isFinite(start) && controller.contentEditor.editorItem.selectionStart !== undefined)
                    start = Number(controller.contentEditor.editorItem.selectionStart);
                if (!isFinite(end) && controller.contentEditor.editorItem.selectionEnd !== undefined)
                    end = Number(controller.contentEditor.editorItem.selectionEnd);
                if (controller.contentEditor.editorItem.inputItem) {
                    if (!isFinite(start) && controller.contentEditor.editorItem.inputItem.selectionStart !== undefined)
                        start = Number(controller.contentEditor.editorItem.inputItem.selectionStart);
                    if (!isFinite(end) && controller.contentEditor.editorItem.inputItem.selectionEnd !== undefined)
                        end = Number(controller.contentEditor.editorItem.inputItem.selectionEnd);
                }
            }
        }
        const surfaceLength = controller.currentEditorSurfaceLength();
        const boundedStart = isFinite(start) ? Math.max(0, Math.min(surfaceLength, Math.floor(Math.min(start, end)))) : NaN;
        const boundedEnd = isFinite(end) ? Math.max(0, Math.min(surfaceLength, Math.floor(Math.max(start, end)))) : NaN;
        if (!isFinite(boundedStart) || !isFinite(boundedEnd) || boundedEnd <= boundedStart) {
            return ({
                    "start": -1,
                    "end": -1
                });
        }
        return ({
                "start": boundedStart,
                "end": boundedEnd
            });
    }
    function currentEditorSurfaceLength() {
        if (controller.contentEditor && controller.contentEditor.length !== undefined) {
            const lengthValue = Number(controller.contentEditor.length);
            if (isFinite(lengthValue))
                return Math.max(0, Math.floor(lengthValue));
        }
        const inputItem = controller.currentEditorInputItem();
        if (inputItem && inputItem.length !== undefined) {
            const lengthValue = Number(inputItem.length);
            if (isFinite(lengthValue))
                return Math.max(0, Math.floor(lengthValue));
        }

    }
    function currentLogicalText() {
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalText !== undefined)
            return String(controller.textMetricsBridge.logicalText === undefined || controller.textMetricsBridge.logicalText === null ? "" : controller.textMetricsBridge.logicalText);
        if (!controller.view || controller.view.editorText === undefined || controller.view.editorText === null)
            return "";
        return String(controller.view.editorText);
    }
    function currentSelectedEditorText() {
        if (!controller.contentEditor)
            return "";
        const selectionSnapshot = controller.currentInlineFormatSelectionSnapshot();
        if (selectionSnapshot && selectionSnapshot.selectedText !== undefined) {
            return String(
                        selectionSnapshot.selectedText === undefined || selectionSnapshot.selectedText === null
                        ? ""
                        : selectionSnapshot.selectedText);
        }
        if (controller.contentEditor.selectedText !== undefined)
            return String(controller.contentEditor.selectedText === undefined || controller.contentEditor.selectedText === null ? "" : controller.contentEditor.selectedText);
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.selectedText !== undefined)
            return String(controller.contentEditor.editorItem.selectedText === undefined || controller.contentEditor.editorItem.selectedText === null ? "" : controller.contentEditor.editorItem.selectedText);
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.inputItem && controller.contentEditor.editorItem.inputItem.selectedText !== undefined)
            return String(controller.contentEditor.editorItem.inputItem.selectedText === undefined || controller.contentEditor.editorItem.inputItem.selectedText === null ? "" : controller.contentEditor.editorItem.inputItem.selectedText);
        return "";
    }
    function currentSelectedNoteId() {
        if (!controller.view || controller.view.selectedNoteId === undefined || controller.view.selectedNoteId === null)
            return "";
        return String(controller.view.selectedNoteId).trim();
    }
    function currentSourceText() {
        if (!controller.view || controller.view.editorText === undefined || controller.view.editorText === null)
            return "";
        return String(controller.view.editorText);
    }
    function resourceTagLossDetectedForMutation(currentSourceText, nextSourceText) {
        if (!controller.view || controller.view.resourceTagLossDetected === undefined)
            return false;
        return !!controller.view.resourceTagLossDetected(currentSourceText, nextSourceText);
    }
    function restoreEditorSurfaceFromSourcePresentation() {
        if (controller.view && controller.view.restoreEditorSurfaceFromPresentation !== undefined) {
            controller.view.restoreEditorSurfaceFromPresentation();
            return;
        }
        if (controller.view && controller.view.commitDocumentPresentationRefresh !== undefined)
            controller.view.commitDocumentPresentationRefresh();
    }
    function clampLogicalPosition(position, maximumLength) {
        const numericPosition = Number(position);
        if (!isFinite(numericPosition))
            return 0;
        return Math.max(0, Math.min(Math.floor(numericPosition), Math.max(0, Number(maximumLength) || 0)));
    }
    function sourceOffsetForLogicalOffset(logicalOffset) {
        const safeOffset = Math.max(0, Math.floor(Number(logicalOffset) || 0));
        if (controller.textMetricsBridge && controller.textMetricsBridge.sourceOffsetForLogicalOffset !== undefined) {
            const mappedOffset = Number(controller.textMetricsBridge.sourceOffsetForLogicalOffset(safeOffset));
            if (isFinite(mappedOffset))
                return Math.max(0, Math.floor(mappedOffset));
        }
        const currentSourceText = controller.currentSourceText();
        return Math.max(0, Math.min(currentSourceText.length, safeOffset));
    }
    function selectionCursorUsesStartEdge(cursorPosition, selectionStart, selectionEnd) {
        const boundedStart = Math.max(0, Math.floor(Number(selectionStart) || 0));
        const boundedEnd = Math.max(boundedStart, Math.floor(Number(selectionEnd) || 0));
        const numericCursor = Number(cursorPosition);
        if (!isFinite(numericCursor))
            return false;
        const boundedCursor = controller.clampLogicalPosition(numericCursor, boundedEnd);
        if (boundedCursor <= boundedStart)
            return true;
        if (boundedCursor >= boundedEnd)
            return false;
        return Math.abs(boundedCursor - boundedStart) <= Math.abs(boundedCursor - boundedEnd);
    }
    function restoreEditorSelection(inputItem, selectionStart, selectionEnd, cursorPosition) {
        if (!inputItem)
            return false;
        const boundedStart = Math.max(0, Math.floor(Number(selectionStart) || 0));
        const boundedEnd = Math.max(boundedStart, Math.floor(Number(selectionEnd) || 0));
        if (boundedEnd <= boundedStart)
            return false;
        const activeCursor = isFinite(Number(cursorPosition)) ? Number(cursorPosition) : controller.currentEditorCursorPosition();
        const activeEdgeIsStart = controller.selectionCursorUsesStartEdge(activeCursor, boundedStart, boundedEnd);
        if (inputItem.moveCursorSelection !== undefined) {
            const anchorPosition = activeEdgeIsStart ? boundedEnd : boundedStart;
            const activePosition = activeEdgeIsStart ? boundedStart : boundedEnd;
            inputItem.cursorPosition = anchorPosition;
            inputItem.moveCursorSelection(activePosition, TextEdit.SelectCharacters);
            return true;
        }
        if (inputItem.select !== undefined) {
            inputItem.select(boundedStart, boundedEnd);
            return true;
        }
        return false;
    }
    function scheduleEditorSelection(selectionStart, selectionEnd, cursorPosition) {
        const requestedStart = Number(selectionStart);
        const requestedEnd = Number(selectionEnd);
        const requestedCursor = Number(cursorPosition);
        Qt.callLater(function () {
            if (!controller.contentEditor)
                return;
            const logicalLength = controller.currentLogicalText().length;
            const boundedStart = controller.clampLogicalPosition(requestedStart, logicalLength);
            const boundedEnd = controller.clampLogicalPosition(requestedEnd, logicalLength);
            const fallbackCursor = isFinite(requestedCursor) ? requestedCursor : controller.currentEditorCursorPosition();
            const boundedCursor = controller.clampLogicalPosition(fallbackCursor, logicalLength);
            const editorItem = controller.contentEditor.editorItem ? controller.contentEditor.editorItem : null;
            const inputItem = editorItem && editorItem.inputItem ? editorItem.inputItem : null;
            if (isFinite(requestedStart) && isFinite(requestedEnd) && boundedEnd > boundedStart) {
                controller.restoreEditorSelection(inputItem, boundedStart, boundedEnd, boundedCursor);
                return;
            }
            if (inputItem && inputItem.deselect !== undefined)
                inputItem.deselect();
            if (controller.contentEditor.cursorPosition !== undefined)
                controller.contentEditor.cursorPosition = boundedCursor;
            if (editorItem && editorItem.cursorPosition !== undefined)
                editorItem.cursorPosition = boundedCursor;
            if (inputItem && inputItem.cursorPosition !== undefined)
                inputItem.cursorPosition = boundedCursor;
        });
    }
    function editorPlainTextSlice(start, end) {
        const surfaceLength = controller.currentEditorSurfaceLength();
        const boundedStart = Math.max(0, Math.min(surfaceLength, Math.floor(Number(start) || 0)));
        const boundedEnd = Math.max(0, Math.min(surfaceLength, Math.floor(Number(end) || 0)));
        if (boundedEnd <= boundedStart)
            return "";
        if (controller.contentEditor && controller.contentEditor.getText !== undefined)
            return controller.normalizeSelectionTextValue(controller.contentEditor.getText(boundedStart, boundedEnd));
        const inputItem = controller.currentEditorInputItem();
        if (inputItem && inputItem.getText !== undefined)
            return controller.normalizeSelectionTextValue(inputItem.getText(boundedStart, boundedEnd));
        return controller.currentEditorPlainText().slice(boundedStart, boundedEnd);
    }
    function handleSelectionContextMenuEvent(eventName) {
        const contextSelectionRange = controller.contextMenuEditorSelectionSnapshot();
        if (eventName === "editor.format.plain")
            controller.wrapSelectedEditorTextWithTag("plain", contextSelectionRange);
        else if (eventName === "editor.format.bold")
            controller.wrapSelectedEditorTextWithTag("bold", contextSelectionRange);
        else if (eventName === "editor.format.italic")
            controller.wrapSelectedEditorTextWithTag("italic", contextSelectionRange);
        else if (eventName === "editor.format.underline")
            controller.wrapSelectedEditorTextWithTag("underline", contextSelectionRange);
        else if (eventName === "editor.format.strikethrough")
            controller.wrapSelectedEditorTextWithTag("strikethrough", contextSelectionRange);
        else if (eventName === "editor.format.highlight")
            controller.wrapSelectedEditorTextWithTag("highlight", contextSelectionRange);
    }
    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition, preferredRangeStart, preferredRangeEnd) {
        if (!controller.view)
            return ({
                    "start": -1,
                    "end": -1
                });
        const plainText = controller.currentEditorPlainText();
        const normalizedSelectedText = controller.normalizeSelectionTextValue(selectedText);
        if (plainText.length === 0 || normalizedSelectedText.length === 0)
            return ({
                    "start": -1,
                    "end": -1
                });
        const occurrenceRanges = [];
        let searchOffset = 0;
        while (searchOffset <= plainText.length) {
            const occurrenceStart = plainText.indexOf(normalizedSelectedText, searchOffset);
            if (occurrenceStart < 0)
                break;
            occurrenceRanges.push({
                "start": occurrenceStart,
                "end": occurrenceStart + normalizedSelectedText.length
            });
            searchOffset = occurrenceStart + 1;
        }
        if (occurrenceRanges.length === 0)
            return ({
                    "start": -1,
                    "end": -1
                });
        let anchor = NaN;
        const numericPreferredStart = Number(preferredRangeStart);
        const numericPreferredEnd = Number(preferredRangeEnd);
        if (isFinite(numericPreferredStart) && isFinite(numericPreferredEnd) && numericPreferredEnd > numericPreferredStart)
            anchor = Math.max(0, Math.min(plainText.length, Math.floor((numericPreferredStart + numericPreferredEnd) / 2)));
        else if (isFinite(cursorPosition))
            anchor = Math.max(0, Math.min(plainText.length, Math.floor(cursorPosition)));
        let bestOccurrence = occurrenceRanges[0];
        let bestScore = Number.POSITIVE_INFINITY;
        for (let index = 0; index < occurrenceRanges.length; ++index) {
            const occurrence = occurrenceRanges[index];
            const occurrenceMidpoint = (occurrence.start + occurrence.end) / 2;
            const occurrenceScore = isFinite(anchor) ? Math.abs(occurrenceMidpoint - anchor) : Math.abs(occurrence.end - plainText.length);
            if (occurrenceScore < bestScore) {
                bestScore = occurrenceScore;
                bestOccurrence = occurrence;
            }
        }
        return ({
                "start": bestOccurrence.start,
                "end": bestOccurrence.end
            });
    }
    function inlineStyleWrapTags(styleTag) {
        if (!controller.view)
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        switch (styleTag) {
        case "plain":
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        case "bold":
            return ({
                    "openTag": "<bold>",
                    "closeTag": "</bold>"
                });
        case "italic":
            return ({
                    "openTag": "<italic>",
                    "closeTag": "</italic>"
                });
        case "underline":
            return ({
                    "openTag": "<underline>",
                    "closeTag": "</underline>"
                });
        case "strikethrough":
            return ({
                    "openTag": "<strikethrough>",
                    "closeTag": "</strikethrough>"
                });
        case "highlight":
            return ({
                    "openTag": "<highlight>",
                    "closeTag": "</highlight>"
                });
        default:
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        }
    }
    function normalizeInlineStyleTag(tagName) {
        return RawInlineStyleMutationSupport.normalizedInlineStyleTag(tagName);
    }
    function normalizeSelectionTextValue(text) {
        let normalizedText = text === undefined || text === null ? "" : String(text);
        normalizedText = normalizedText.replace(/\r\n/g, "\n");
        normalizedText = normalizedText.replace(/\r/g, "\n");
        normalizedText = normalizedText.replace(/\u2028/g, "\n");
        normalizedText = normalizedText.replace(/\u2029/g, "\n");
        normalizedText = normalizedText.replace(/\uFFFC/g, "");
        normalizedText = normalizedText.replace(/\u00a0/g, " ");
        return normalizedText;
    }
    function openEditorSelectionContextMenu(localX, localY) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        let selectionRange = controller.resolveSelectionSnapshot(controller.contextMenuEditorSelectionSnapshot());
        if (selectionRange.end <= selectionRange.start) {
            selectionRange = controller.selectedEditorRange();
        }
        if (selectionRange.end <= selectionRange.start) {
            const inferredRange = controller.inferSelectionRangeFromSelectedText(controller.currentSelectedEditorText(), controller.currentEditorCursorPosition());
            if (inferredRange.end > inferredRange.start)
                selectionRange = inferredRange;
        }
        if (selectionRange.end <= selectionRange.start || !controller.selectionContextMenu)
            return false;
        if (controller.selectionContextMenu.opened)
            controller.selectionContextMenu.close();
        controller.contextMenuSelectionStart = selectionRange.start;
        controller.contextMenuSelectionEnd = selectionRange.end;
        controller.contextMenuSelectionCursorPosition = controller.currentEditorCursorPosition();
        controller.contextMenuSelectionText = controller.editorPlainTextSlice(selectionRange.start, selectionRange.end);
        controller.selectionContextMenu.openFor(controller.editorViewport, Number(localX) || 0, Number(localY) || 0);
        return true;
    }
    function primeContextMenuSelectionSnapshot() {
        const rawSelectionRange = controller.currentRawEditorSelectionRange();
        let selectionRange = rawSelectionRange;
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.selectedEditorRange();
        if (selectionRange.end <= selectionRange.start) {
            controller.resetEditorSelectionCache();
            return false;
        }
        controller.contextMenuSelectionStart = selectionRange.start;
        controller.contextMenuSelectionEnd = selectionRange.end;
        controller.contextMenuSelectionCursorPosition = controller.currentEditorCursorPosition();
        controller.contextMenuSelectionText = controller.editorPlainTextSlice(selectionRange.start, selectionRange.end);
        return true;
    }
    function persistEditorTextImmediately(nextText) {
        if (!controller.view
                || !controller.editorSession
                || controller.editorSession.persistEditorTextImmediately === undefined)
            return false;
        return !!controller.editorSession.persistEditorTextImmediately(nextText);
    }
    function queueInlineFormatWrap(tagName) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        const normalizedTagName = controller.normalizeInlineStyleTag(tagName);
        if (normalizedTagName.length === 0)
            return false;
        if (controller.view.queueStructuredInlineFormatWrap !== undefined
                && controller.view.queueStructuredInlineFormatWrap(normalizedTagName)) {
            return true;
        }
        let selectionRange = controller.selectedEditorRange();
        const selectedText = controller.currentSelectedEditorText();
        const selectionSnapshot = {
            "start": selectionRange.start,
            "end": selectionRange.end,
            "selectedText": selectedText,
            "cursorPosition": controller.currentEditorCursorPosition()
        };
        if (selectionRange.end <= selectionRange.start)
            return false;
        return controller.wrapSelectedEditorTextWithTag(normalizedTagName, selectionSnapshot);
    }
    function resetEditorSelectionCache() {
        controller.contextMenuSelectionStart = -1;
        controller.contextMenuSelectionEnd = -1;
        controller.contextMenuSelectionCursorPosition = NaN;
        controller.contextMenuSelectionText = "";
    }
    function resolveSelectionSnapshot(selectionSnapshot) {
        const normalizedSelectedText = controller.normalizeSelectionTextValue(selectionSnapshot && selectionSnapshot.selectedText !== undefined ? selectionSnapshot.selectedText : controller.currentSelectedEditorText());
        const cursorPosition = selectionSnapshot && selectionSnapshot.cursorPosition !== undefined ? Number(selectionSnapshot.cursorPosition) : controller.currentEditorCursorPosition();
        const start = selectionSnapshot && selectionSnapshot.start !== undefined ? Number(selectionSnapshot.start) : NaN;
        const end = selectionSnapshot && selectionSnapshot.end !== undefined ? Number(selectionSnapshot.end) : NaN;
        if (isFinite(start) && isFinite(end) && end > start) {
            const candidateRange = {
                "start": Math.floor(start),
                "end": Math.floor(end)
            };
            if (normalizedSelectedText.length === 0 || controller.selectionRangeMatchesSelectedText(candidateRange, normalizedSelectedText)) {
                return candidateRange;
            }
        }
        if (normalizedSelectedText.length > 0)
            return controller.inferSelectionRangeFromSelectedText(normalizedSelectedText, cursorPosition);
        return {
            "start": -1,
            "end": -1
        };
    }
    function refreshPresentationStateAfterProgrammaticChange() {
        if (!controller.view || controller.view.commitDocumentPresentationRefresh === undefined)
            return;
        controller.view.commitDocumentPresentationRefresh();
    }
    function selectedEditorRange() {
        if (!controller.contentEditor)
            return ({
                    "start": -1,
                    "end": -1
                });
        const selectionSnapshot = controller.currentEditorSelectionSnapshot();
        const selectedText = selectionSnapshot.selectedText !== undefined ? selectionSnapshot.selectedText : controller.currentSelectedEditorText();
        const numericSelectionRange = controller.currentRawEditorSelectionRange();
        const numericRange = {
            "start": numericSelectionRange.start,
            "end": numericSelectionRange.end,
            "selectedText": selectedText,
            "cursorPosition": selectionSnapshot.cursorPosition !== undefined ? Number(selectionSnapshot.cursorPosition) : controller.currentEditorCursorPosition()
        };
        const resolvedRange = controller.resolveSelectionSnapshot(numericRange);
        if (resolvedRange.end > resolvedRange.start)
            return resolvedRange;
        return ({
                "start": -1,
                "end": -1
            });
    }
    function selectionRangeMatchesSelectedText(selectionRange, selectedText) {
        if (!selectionRange)
            return false;
        const start = Number(selectionRange.start);
        const end = Number(selectionRange.end);
        if (!isFinite(start) || !isFinite(end) || end <= start)
            return false;
        const normalizedSelectedText = controller.normalizeSelectionTextValue(selectedText);
        if (normalizedSelectedText.length === 0)
            return false;
        return controller.editorPlainTextSlice(start, end) === normalizedSelectedText;
    }
    function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        const normalizedTagName = controller.normalizeInlineStyleTag(tagName);
        if (normalizedTagName.length === 0)
            return false;
        let selectionRange = controller.resolveSelectionSnapshot(explicitSelectionRange || null);
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.selectedEditorRange();
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.resolveSelectionSnapshot(controller.contextMenuEditorSelectionSnapshot());
        if (selectionRange.end <= selectionRange.start)
            return false;
        const currentText = controller.view.editorText === undefined || controller.view.editorText === null ? "" : String(controller.view.editorText);
        const logicalLength = controller.currentLogicalText().length;
        const boundedStart = Math.max(0, Math.min(logicalLength, Math.floor(selectionRange.start)));
        const boundedEnd = Math.max(0, Math.min(logicalLength, Math.floor(selectionRange.end)));
        if (boundedEnd <= boundedStart)
            return false;
        const mutationPayload = RawInlineStyleMutationSupport.buildInlineStyleSelectionPayload(
                    currentText,
                    boundedStart,
                    boundedEnd,
                    normalizedTagName);
        if (!mutationPayload || !mutationPayload.applied)
            return false;
        const nextText = String(mutationPayload.nextSourceText);
        if (nextText.length === 0 && currentText.length > 0)
            return false;
        if (controller.resourceTagLossDetectedForMutation(currentText, nextText)) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
        }
        if (!controller.commitRawEditorTextMutation(nextText))
            return false;
        const committedText = controller.committedEditorText(nextText);
        controller.refreshPresentationStateAfterProgrammaticChange();
        controller.resetEditorSelectionCache();
        controller.view.editorTextEdited(committedText);
        return true;
    }

    function commitRawEditorTextMutation(nextSourceText) {
        if (!controller.editorSession
                || controller.editorSession.commitRawEditorTextMutation === undefined) {
            return false;
        }
        return !!controller.editorSession.commitRawEditorTextMutation(nextSourceText);
    }

    function committedEditorText(fallbackText) {
        if (controller.editorSession && controller.editorSession.editorText !== undefined
                && controller.editorSession.editorText !== null) {
            return String(controller.editorSession.editorText);
        }
        return fallbackText === undefined || fallbackText === null ? "" : String(fallbackText);
    }
}
