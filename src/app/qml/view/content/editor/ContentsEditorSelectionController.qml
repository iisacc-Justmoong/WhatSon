pragma ComponentBehavior: Bound

import QtQuick

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
    property int contextMenuSelectionStart: -1
    property string contextMenuSelectionText: ""
    property var editorSession: null
    property var editorViewport: null
    property var queuedInlineFormatWrapKeys: ({})
    property var selectionBridge: null
    property var selectionContextMenu: null
    property var textFormatRenderer: null
    property var textMetricsBridge: null
    property var view: null

    function applyEditorRichTextSurface() {
        if (!controller.contentEditor)
            return;
        if (controller.contentEditor.textFormat !== undefined && controller.contentEditor.textFormat !== TextEdit.RichText)
            controller.contentEditor.textFormat = TextEdit.RichText;
        if (controller.contentEditor.showRenderedOutput !== undefined && !controller.contentEditor.showRenderedOutput)
            controller.contentEditor.showRenderedOutput = true;
        const editorItem = controller.contentEditor.editorItem;
        if (!editorItem)
            return;
        if (editorItem.textFormat !== undefined && editorItem.textFormat !== TextEdit.RichText)
            editorItem.textFormat = TextEdit.RichText;
        if (editorItem.showRenderedOutput !== undefined && !editorItem.showRenderedOutput)
            editorItem.showRenderedOutput = true;
        const inputItem = editorItem.inputItem;
        if (!inputItem)
            return;
        if (inputItem.textFormat !== undefined && inputItem.textFormat !== TextEdit.RichText)
            inputItem.textFormat = TextEdit.RichText;
        if (inputItem.selectByMouse !== undefined && !inputItem.selectByMouse)
            inputItem.selectByMouse = true;
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
            "cursorPosition": controller.currentEditorCursorPosition()
        };
    }
    function currentEditorCursorPosition() {
        if (!controller.contentEditor)
            return NaN;
        if (controller.contentEditor.selectionSnapshot !== undefined) {
            const snapshot = controller.contentEditor.selectionSnapshot();
            if (snapshot && snapshot.cursorPosition !== undefined) {
                const cursor = Number(snapshot.cursorPosition);
                if (isFinite(cursor))
                    return cursor;
            }
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
        if (controller.contentEditor.selectionSnapshot !== undefined) {
            const snapshot = controller.contentEditor.selectionSnapshot();
            if (snapshot)
                return snapshot;
        }
        return {
            "cursorPosition": controller.currentEditorCursorPosition(),
            "selectedText": controller.currentSelectedEditorText(),
            "selectionEnd": controller.contentEditor.selectionEnd,
            "selectionStart": controller.contentEditor.selectionStart
        };
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
        if (controller.contentEditor.selectionSnapshot !== undefined) {
            const snapshot = controller.contentEditor.selectionSnapshot();
            if (snapshot && snapshot.selectedText !== undefined)
                return String(snapshot.selectedText === undefined || snapshot.selectedText === null ? "" : snapshot.selectedText);
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
    function handleInlineFormatShortcutKeyPress(event) {
        if (!event || !controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;

        const modifiers = event.modifiers;
        const commandPressed = (modifiers & Qt.MetaModifier) || (modifiers & Qt.ControlModifier);
        if (!commandPressed)
            return false;

        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        let handled = false;
        switch (event.key) {
        case Qt.Key_B:
            handled = controller.queueInlineFormatWrap("bold");
            break;
        case Qt.Key_I:
            handled = controller.queueInlineFormatWrap("italic");
            break;
        case Qt.Key_U:
            handled = controller.queueInlineFormatWrap("underline");
            break;
        case Qt.Key_X:
            if (shiftPressed)
                handled = controller.queueInlineFormatWrap("strikethrough");
            break;
        case Qt.Key_E:
            if (shiftPressed)
                handled = controller.queueInlineFormatWrap("highlight");
            break;
        default:
            break;
        }

        if (handled)
            event.accepted = true;

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
    function normalizeBodySourceForRichTextEditor(sourceText) {
        const source = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        if (source.length === 0)
            return "";
        if (controller.textFormatRenderer && controller.textFormatRenderer.normalizeInlineStyleAliasesForEditor !== undefined) {
            const rendererNormalizedText = controller.textFormatRenderer.normalizeInlineStyleAliasesForEditor(source);
            if (rendererNormalizedText !== undefined && rendererNormalizedText !== null)
                return String(rendererNormalizedText);
        }
        const inlineTagPattern = /<\s*(\/?)\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>/gi;
        return source.replace(inlineTagPattern, function (token, slashToken, rawTagName) {
            const normalizedTagName = rawTagName === undefined || rawTagName === null ? "" : String(rawTagName).trim().toLowerCase();
            const isClosingTag = slashToken !== undefined && slashToken !== null && String(slashToken).length > 0;
            if (normalizedTagName === "br")
                return "<br/>";
            const styleTag = controller.normalizeInlineStyleTag(normalizedTagName);
            if (styleTag.length === 0)
                return token;
            const wrapTags = controller.richTextWrapTags(styleTag);
            if (wrapTags.openTag.length === 0 || wrapTags.closeTag.length === 0)
                return token;
            return isClosingTag ? wrapTags.closeTag : wrapTags.openTag;
        });
    }
    function normalizeInlineStyleTag(tagName) {
        const normalizedTagName = tagName === undefined || tagName === null ? "" : String(tagName).trim().toLowerCase();
        if (normalizedTagName === "plain" || normalizedTagName === "clear" || normalizedTagName === "none")
            return "plain";
        if (normalizedTagName === "bold" || normalizedTagName === "b" || normalizedTagName === "strong")
            return "bold";
        if (normalizedTagName === "italic" || normalizedTagName === "i" || normalizedTagName === "em")
            return "italic";
        if (normalizedTagName === "underline" || normalizedTagName === "u")
            return "underline";
        if (normalizedTagName === "strikethrough" || normalizedTagName === "strike" || normalizedTagName === "s" || normalizedTagName === "del")
            return "strikethrough";
        if (normalizedTagName === "highlight" || normalizedTagName === "mark")
            return "highlight";
        return "";
    }
    function normalizeSelectionTextValue(text) {
        let normalizedText = text === undefined || text === null ? "" : String(text);
        normalizedText = normalizedText.replace(/\r\n/g, "\n");
        normalizedText = normalizedText.replace(/\r/g, "\n");
        normalizedText = normalizedText.replace(/\u2028/g, "\n");
        normalizedText = normalizedText.replace(/\u2029/g, "\n");

    }
    function openEditorSelectionContextMenu(localX, localY) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        let selectionRange = controller.selectedEditorRange();
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
        controller.contextMenuSelectionText = controller.normalizeSelectionTextValue(controller.currentSelectedEditorText());
        controller.selectionContextMenu.openFor(controller.editorViewport, Number(localX) || 0, Number(localY) || 0);
        return true;
    }
    function persistEditorTextImmediately(nextText) {
        if (!controller.view)
            return false;
        const noteId = controller.view.selectedNoteId === undefined || controller.view.selectedNoteId === null ? "" : String(controller.view.selectedNoteId).trim();
        if (noteId.length === 0)
            return false;
        if (!controller.selectionBridge || controller.selectionBridge.persistEditorTextForNote === undefined || !controller.view.contentPersistenceContractAvailable)
            return false;
        const saved = !!controller.selectionBridge.persistEditorTextForNote(noteId, nextText);
        if (saved && controller.editorSession) {
            if (controller.editorSession.pendingBodySave !== undefined)
                controller.editorSession.pendingBodySave = false;
            if (controller.editorSession.localEditorAuthority !== undefined)
                controller.editorSession.localEditorAuthority = false;
        }

    }
    function queueInlineFormatWrap(tagName) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        const normalizedTagName = controller.normalizeInlineStyleTag(tagName);
        if (normalizedTagName.length === 0)
            return false;
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
        const capturedSelectionRange = selectionSnapshot;
        const noteId = controller.currentSelectedNoteId();
        const queueKey = noteId + "::" + normalizedTagName + "::" + String(capturedSelectionRange.start) + "::" + String(capturedSelectionRange.end) + "::" + controller.normalizeSelectionTextValue(capturedSelectionRange.selectedText);
        if (controller.queuedInlineFormatWrapKeys[queueKey])
            return true;
        controller.queuedInlineFormatWrapKeys[queueKey] = true;
        Qt.callLater(function () {
            delete controller.queuedInlineFormatWrapKeys[queueKey];
            if (!controller.view || !controller.view.hasSelectedNote)
                return;
            if (controller.currentSelectedNoteId() !== noteId)
                return;
            controller.wrapSelectedEditorTextWithTag(normalizedTagName, capturedSelectionRange);
        });
        return true;
    }
    function resetEditorSelectionCache() {
        controller.contextMenuSelectionStart = -1;
        controller.contextMenuSelectionEnd = -1;
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
    function richTextWrapTags(styleTag) {
        if (!controller.view)
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        switch (styleTag) {
        case "bold":
            return ({
                    "openTag": "<strong style=\"font-weight:900;\">",
                    "closeTag": "</strong>"
                });
        case "italic":
            return ({
                    "openTag": "<span style=\"font-style:italic;\">",
                    "closeTag": "</span>"
                });
        case "underline":
            return ({
                    "openTag": "<span style=\"text-decoration: underline;\">",
                    "closeTag": "</span>"
                });
        case "strikethrough":
            return ({
                    "openTag": "<span style=\"text-decoration: line-through;\">",
                    "closeTag": "</span>"
                });
        case "highlight":
            return ({
                    "openTag": controller.view.richTextHighlightOpenTag,
                    "closeTag": "</span>"
                });
        default:
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        }
    }
    function scheduleEditorRichTextSurfaceSync() {
        Qt.callLater(function () {
            controller.applyEditorRichTextSurface();
        });
    }
    function selectedEditorRange() {
        if (!controller.contentEditor)
            return ({
                    "start": -1,
                    "end": -1
                });
        const selectionSnapshot = controller.currentEditorSelectionSnapshot();
        const selectedText = selectionSnapshot.selectedText !== undefined ? selectionSnapshot.selectedText : controller.currentSelectedEditorText();
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
        const numericRange = {
            "start": isFinite(start) ? Math.max(0, Math.min(surfaceLength, Math.floor(Math.min(start, end)))) : NaN,
            "end": isFinite(end) ? Math.max(0, Math.min(surfaceLength, Math.floor(Math.max(start, end)))) : NaN,
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
        if (!controller.textFormatRenderer || controller.textFormatRenderer.applyInlineStyleToLogicalSelectionSource === undefined)
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
        const nextText = controller.textFormatRenderer.applyInlineStyleToLogicalSelectionSource(currentText, boundedStart, boundedEnd, normalizedTagName);
        if (nextText.length === 0 && currentText.length > 0)
            return false;
        if (controller.view.editorText !== nextText)
            controller.view.editorText = nextText;
        if (controller.editorSession && controller.editorSession.markLocalEditorAuthority !== undefined)
            controller.editorSession.markLocalEditorAuthority();
        const saved = controller.persistEditorTextImmediately(nextText);
        if (!saved && controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
            controller.editorSession.scheduleEditorPersistence();
        controller.contextMenuSelectionStart = -1;
        controller.contextMenuSelectionEnd = -1;
        controller.contextMenuSelectionText = "";
        controller.view.editorTextEdited(nextText);
        return true;
    }
}
