pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property var contentEditor: null
    property var editorSession: null
    property var editorViewport: null
    property var selectionBridge: null
    property var textMetricsBridge: null
    property var textFormatRenderer: null
    property var selectionContextMenu: null

    property int cachedSelectionStart: -1
    property int cachedSelectionEnd: -1
    property int contextMenuSelectionStart: -1
    property int contextMenuSelectionEnd: -1

    readonly property var contextMenuItems: [
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

    function persistEditorTextImmediately(nextText) {
        if (!controller.view)
            return false;
        const noteId = controller.view.selectedNoteId === undefined || controller.view.selectedNoteId === null
                ? ""
                : String(controller.view.selectedNoteId).trim();
        if (noteId.length === 0)
            return false;
        if (!controller.selectionBridge
                || controller.selectionBridge.persistEditorTextForNote === undefined
                || !controller.view.contentPersistenceContractAvailable)
            return false;
        return !!controller.selectionBridge.persistEditorTextForNote(noteId, nextText);
    }

    function resetEditorSelectionCache() {
        controller.cachedSelectionStart = -1;
        controller.cachedSelectionEnd = -1;
        controller.contextMenuSelectionStart = -1;
        controller.contextMenuSelectionEnd = -1;
    }

    function cacheEditorSelectionRange(selectionRange) {
        if (!selectionRange)
            return false;
        const start = Number(selectionRange.start);
        const end = Number(selectionRange.end);
        if (!isFinite(start) || !isFinite(end) || end <= start)
            return false;
        controller.cachedSelectionStart = Math.floor(start);
        controller.cachedSelectionEnd = Math.floor(end);
        return true;
    }

    function cachedEditorSelectionRange() {
        if (controller.cachedSelectionEnd <= controller.cachedSelectionStart)
            return ({
                    "start": -1,
                    "end": -1
                });
        return ({
                "start": controller.cachedSelectionStart,
                "end": controller.cachedSelectionEnd
            });
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

    function currentSelectedEditorText() {
        if (!controller.contentEditor)
            return "";
        if (controller.contentEditor.selectedText !== undefined)
            return String(controller.contentEditor.selectedText === undefined || controller.contentEditor.selectedText === null ? "" : controller.contentEditor.selectedText);
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.selectedText !== undefined)
            return String(controller.contentEditor.editorItem.selectedText === undefined || controller.contentEditor.editorItem.selectedText === null ? "" : controller.contentEditor.editorItem.selectedText);
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.inputItem && controller.contentEditor.editorItem.inputItem.selectedText !== undefined)
            return String(controller.contentEditor.editorItem.inputItem.selectedText === undefined || controller.contentEditor.editorItem.inputItem.selectedText === null ? "" : controller.contentEditor.editorItem.inputItem.selectedText);
        return "";
    }

    function currentEditorCursorPosition() {
        if (!controller.contentEditor)
            return NaN;
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
        if (controller.contentEditor.editorItem
                && controller.contentEditor.editorItem.inputItem
                && controller.contentEditor.editorItem.inputItem.cursorPosition !== undefined) {
            const cursor = Number(controller.contentEditor.editorItem.inputItem.cursorPosition);
            if (isFinite(cursor))
                return cursor;
        }
        return NaN;
    }

    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition) {
        if (!controller.view)
            return ({
                    "start": -1,
                    "end": -1
                });
        const sourceText = controller.view.editorText === undefined || controller.view.editorText === null ? "" : String(controller.view.editorText);
        const normalizedSelectedText = selectedText === undefined || selectedText === null ? "" : String(selectedText);
        if (sourceText.length === 0 || normalizedSelectedText.length === 0)
            return ({
                    "start": -1,
                    "end": -1
                });
        const normalizedCursor = isFinite(cursorPosition)
                ? Math.max(0, Math.min(sourceText.length, Math.floor(cursorPosition)))
                : sourceText.length;
        let start = sourceText.lastIndexOf(normalizedSelectedText, normalizedCursor);
        if (start < 0)
            start = sourceText.indexOf(normalizedSelectedText, normalizedCursor);
        if (start < 0)
            start = sourceText.indexOf(normalizedSelectedText);
        if (start < 0)
            return ({
                    "start": -1,
                    "end": -1
                });
        return ({
                "start": start,
                "end": start + normalizedSelectedText.length
            });
    }

    function syncCachedEditorSelectionRange() {
        controller.cacheEditorSelectionRange(controller.selectedEditorRange());
    }

    function sourceOffsetForLogicalOffset(logicalOffset) {
        if (!controller.view)
            return -1;
        const sourceText = controller.view.editorText === undefined || controller.view.editorText === null ? "" : String(controller.view.editorText);
        const numericOffset = Number(logicalOffset);
        if (!isFinite(numericOffset))
            return -1;
        const normalizedOffset = Math.max(0, Math.floor(numericOffset));
        if (controller.textMetricsBridge && controller.textMetricsBridge.sourceOffsetForLogicalOffset !== undefined) {
            const mappedOffset = Number(controller.textMetricsBridge.sourceOffsetForLogicalOffset(normalizedOffset));
            if (isFinite(mappedOffset))
                return Math.max(0, Math.min(sourceText.length, Math.floor(mappedOffset)));
        }
        return Math.max(0, Math.min(sourceText.length, normalizedOffset));
    }

    function selectedEditorRange() {
        if (!controller.contentEditor || !controller.view)
            return ({
                    "start": -1,
                    "end": -1
                });
        const text = controller.view.editorText === undefined || controller.view.editorText === null ? "" : String(controller.view.editorText);
        let start = controller.contentEditor.selectionStart !== undefined ? Number(controller.contentEditor.selectionStart) : NaN;
        let end = controller.contentEditor.selectionEnd !== undefined ? Number(controller.contentEditor.selectionEnd) : NaN;
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
        if (isFinite(start) && isFinite(end)) {
            const logicalRangeStart = Math.max(0, Math.floor(Math.min(start, end)));
            const logicalRangeEnd = Math.max(0, Math.floor(Math.max(start, end)));
            const sourceRangeStart = controller.sourceOffsetForLogicalOffset(logicalRangeStart);
            const sourceRangeEnd = controller.sourceOffsetForLogicalOffset(logicalRangeEnd);
            if (sourceRangeEnd > sourceRangeStart) {
                const explicitRange = {
                    "start": sourceRangeStart,
                    "end": sourceRangeEnd
                };
                controller.cacheEditorSelectionRange(explicitRange);
                return explicitRange;
            }
        }
        const selectedText = controller.currentSelectedEditorText();
        const cursorPosition = controller.currentEditorCursorPosition();
        const inferredRange = controller.inferSelectionRangeFromSelectedText(selectedText, cursorPosition);
        if (inferredRange.end > inferredRange.start) {
            controller.cacheEditorSelectionRange(inferredRange);
            return inferredRange;
        }
        if (!isFinite(start) || !isFinite(end))
            return ({
                    "start": -1,
                    "end": -1
                });
        const rangeStart = Math.max(0, Math.min(text.length, Math.floor(Math.min(start, end))));
        const rangeEnd = Math.max(0, Math.min(text.length, Math.floor(Math.max(start, end))));
        if (rangeEnd > rangeStart) {
            controller.cacheEditorSelectionRange({
                    "start": rangeStart,
                    "end": rangeEnd
                });
        }
        return ({
                "start": rangeStart,
                "end": rangeEnd
            });
    }

    function normalizeInlineStyleTag(tagName) {
        const normalizedTagName = tagName === undefined || tagName === null ? "" : String(tagName).trim().toLowerCase();
        if (normalizedTagName === "bold" || normalizedTagName === "b" || normalizedTagName === "strong")
            return "bold";
        if (normalizedTagName === "italic" || normalizedTagName === "i" || normalizedTagName === "em")
            return "italic";
        if (normalizedTagName === "underline" || normalizedTagName === "u")
            return "underline";
        if (normalizedTagName === "strikethrough" || normalizedTagName === "strike" || normalizedTagName === "s"
                || normalizedTagName === "del")
            return "strikethrough";
        if (normalizedTagName === "highlight" || normalizedTagName === "mark")
            return "highlight";
        return "";
    }

    function inlineStyleWrapTags(styleTag) {
        if (!controller.view)
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        switch (styleTag) {
        case "bold":
            return ({
                    "openTag": "<span style=\"font-weight:800;\">",
                    "closeTag": "</span>"
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

    function handleInlineFormatShortcutKeyPress(event) {
        if (!event || !controller.view || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer)
            return false;

        const modifiers = event.modifiers;
        const commandPressed = (modifiers & Qt.MetaModifier) || (modifiers & Qt.ControlModifier);
        if (!commandPressed)
            return false;

        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        let handled = false;
        switch (event.key) {
        case Qt.Key_B:
            handled = controller.wrapSelectedEditorTextWithTag("bold");
            break;
        case Qt.Key_I:
            handled = controller.wrapSelectedEditorTextWithTag("italic");
            break;
        case Qt.Key_U:
            handled = controller.wrapSelectedEditorTextWithTag("underline");
            break;
        case Qt.Key_X:
            if (shiftPressed)
                handled = controller.wrapSelectedEditorTextWithTag("strikethrough");
            break;
        case Qt.Key_H:
            if (shiftPressed)
                handled = controller.wrapSelectedEditorTextWithTag("highlight");
            break;
        default:
            break;
        }

        if (handled)
            event.accepted = true;
        return handled;
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
            const normalizedTagName = rawTagName === undefined || rawTagName === null
                    ? ""
                    : String(rawTagName).trim().toLowerCase();
            const isClosingTag = slashToken !== undefined && slashToken !== null && String(slashToken).length > 0;
            if (normalizedTagName === "br")
                return "<br/>";
            const styleTag = controller.normalizeInlineStyleTag(normalizedTagName);
            if (styleTag.length === 0)
                return token;
            const wrapTags = controller.inlineStyleWrapTags(styleTag);
            if (wrapTags.openTag.length === 0 || wrapTags.closeTag.length === 0)
                return token;
            return isClosingTag ? wrapTags.closeTag : wrapTags.openTag;
        });
    }

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

    function scheduleEditorRichTextSurfaceSync() {
        Qt.callLater(function () {
            controller.applyEditorRichTextSurface();
        });
    }

    function openEditorSelectionContextMenu(localX, localY) {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer)
            return false;
        let selectionRange = controller.selectedEditorRange();
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.cachedEditorSelectionRange();
        if (selectionRange.end <= selectionRange.start) {
            const inferredRange = controller.inferSelectionRangeFromSelectedText(
                        controller.currentSelectedEditorText(),
                        controller.currentEditorCursorPosition());
            if (inferredRange.end > inferredRange.start)
                selectionRange = inferredRange;
        }
        if (selectionRange.end <= selectionRange.start || !controller.selectionContextMenu)
            return false;
        if (controller.selectionContextMenu.opened)
            controller.selectionContextMenu.close();
        controller.contextMenuSelectionStart = selectionRange.start;
        controller.contextMenuSelectionEnd = selectionRange.end;
        controller.selectionContextMenu.openFor(controller.editorViewport, Number(localX) || 0, Number(localY) || 0);
        return true;
    }

    function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer)
            return false;
        const normalizedTagName = controller.normalizeInlineStyleTag(tagName);
        if (normalizedTagName.length === 0)
            return false;
        const wrapTags = controller.inlineStyleWrapTags(normalizedTagName);
        if (wrapTags.openTag.length === 0 || wrapTags.closeTag.length === 0)
            return false;
        let selectionRange = explicitSelectionRange && explicitSelectionRange.start !== undefined
                && explicitSelectionRange.end !== undefined
                ? ({
                       "start": Number(explicitSelectionRange.start),
                       "end": Number(explicitSelectionRange.end)
                   })
                : ({
                       "start": -1,
                       "end": -1
                   });
        if (!isFinite(selectionRange.start) || !isFinite(selectionRange.end))
            selectionRange = ({
                    "start": -1,
                    "end": -1
                });
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.selectedEditorRange();
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.contextMenuEditorSelectionRange();
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.cachedEditorSelectionRange();
        if (selectionRange.end <= selectionRange.start)
            return false;
        const currentText = controller.view.editorText === undefined || controller.view.editorText === null ? "" : String(controller.view.editorText);
        const boundedStart = Math.max(0, Math.min(currentText.length, Math.floor(selectionRange.start)));
        const boundedEnd = Math.max(0, Math.min(currentText.length, Math.floor(selectionRange.end)));
        if (boundedEnd <= boundedStart)
            return false;
        const nextText = currentText.slice(0, boundedStart)
                + wrapTags.openTag
                + currentText.slice(boundedStart, boundedEnd)
                + wrapTags.closeTag
                + currentText.slice(boundedEnd);
        if (controller.view.editorText !== nextText)
            controller.view.editorText = nextText;
        if (controller.editorSession && controller.editorSession.markLocalEditorAuthority !== undefined)
            controller.editorSession.markLocalEditorAuthority();
        const saved = controller.persistEditorTextImmediately(nextText);
        if (!saved && controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
            controller.editorSession.scheduleEditorPersistence();
        controller.contextMenuSelectionStart = -1;
        controller.contextMenuSelectionEnd = -1;
        controller.cacheEditorSelectionRange({
                "start": boundedStart + wrapTags.openTag.length,
                "end": boundedEnd + wrapTags.openTag.length
            });
        controller.view.editorTextEdited(nextText);
        return true;
    }

    function handleSelectionContextMenuEvent(eventName) {
        const contextSelectionRange = controller.contextMenuEditorSelectionRange();
        if (eventName === "editor.format.bold")
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
}
