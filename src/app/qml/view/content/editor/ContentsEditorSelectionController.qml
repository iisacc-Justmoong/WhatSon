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
    property real contextMenuSelectionCursorPosition: NaN
    property int contextMenuSelectionStart: -1
    property string contextMenuSelectionText: ""
    property var editorSession: null
    property var editorViewport: null
    property var queuedInlineFormatWrapKeys: ({})
    property var queuedMarkdownListMutationKeys: ({})
    property var queuedStructuredShortcutMutationKeys: ({})
    property var selectionBridge: null
    property var selectionContextMenu: null
    property var textFormatRenderer: null
    property var textMetricsBridge: null
    property var view: null

    function preferNativeInputHandling() {
        return !!(controller.view
                  && controller.view.preferNativeInputHandling !== undefined
                  && controller.view.preferNativeInputHandling);
    }

    function applyEditorRichTextSurface() {
        if (!controller.contentEditor)
            return;
        const preferredTextFormat = controller.preferNativeInputHandling() ? TextEdit.PlainText : TextEdit.RichText;
        const preferredRenderedOutput = !controller.preferNativeInputHandling();
        if (controller.contentEditor.textFormat !== undefined && controller.contentEditor.textFormat !== preferredTextFormat)
            controller.contentEditor.textFormat = preferredTextFormat;
        if (controller.contentEditor.showRenderedOutput !== undefined && controller.contentEditor.showRenderedOutput !== preferredRenderedOutput)
            controller.contentEditor.showRenderedOutput = preferredRenderedOutput;
        const editorItem = controller.contentEditor.editorItem;
        if (!editorItem)
            return;
        if (editorItem.textFormat !== undefined && editorItem.textFormat !== preferredTextFormat)
            editorItem.textFormat = preferredTextFormat;
        if (editorItem.showRenderedOutput !== undefined && editorItem.showRenderedOutput !== preferredRenderedOutput)
            editorItem.showRenderedOutput = preferredRenderedOutput;
        const inputItem = editorItem.inputItem;
        if (!inputItem)
            return;
        if (inputItem.textFormat !== undefined && inputItem.textFormat !== preferredTextFormat)
            inputItem.textFormat = preferredTextFormat;
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
            "cursorPosition": isFinite(controller.contextMenuSelectionCursorPosition)
                              ? Number(controller.contextMenuSelectionCursorPosition)
                              : controller.currentEditorCursorPosition()
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
    function currentSourceText() {
        if (!controller.view || controller.view.editorText === undefined || controller.view.editorText === null)
            return "";
        return String(controller.view.editorText);
    }
    function clampLogicalPosition(position, maximumLength) {
        const numericPosition = Number(position);
        if (!isFinite(numericPosition))
            return 0;
        return Math.max(0, Math.min(Math.floor(numericPosition), Math.max(0, Number(maximumLength) || 0)));
    }
    function logicalLineStartForOffset(text, offset) {
        const logicalText = text === undefined || text === null ? "" : String(text);
        const safeOffset = controller.clampLogicalPosition(offset, logicalText.length);
        if (safeOffset <= 0)
            return 0;
        return logicalText.lastIndexOf("\n", safeOffset - 1) + 1;
    }
    function logicalLineEndForStart(text, start) {
        const logicalText = text === undefined || text === null ? "" : String(text);
        const safeStart = controller.clampLogicalPosition(start, logicalText.length);
        const newlineOffset = logicalText.indexOf("\n", safeStart);
        return newlineOffset >= 0 ? newlineOffset : logicalText.length;
    }
    function logicalLengthForSourceText(sourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalLengthForSourceText !== undefined) {
            const logicalLength = Number(controller.textMetricsBridge.logicalLengthForSourceText(normalizedSourceText));
            if (isFinite(logicalLength))
                return Math.max(0, Math.floor(logicalLength));
        }
        return normalizedSourceText.length;
    }
    function normalizeMarkdownListKind(listKind) {
        const normalizedKind = listKind === undefined || listKind === null ? "" : String(listKind).trim().toLowerCase();
        if (normalizedKind === "unordered" || normalizedKind === "bullet" || normalizedKind === "bulleted")
            return "unordered";
        if (normalizedKind === "ordered" || normalizedKind === "numbered" || normalizedKind === "number")
            return "ordered";
        return "";
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
    function markdownListTargetLineEntries(selectionSnapshot) {
        const logicalText = controller.currentLogicalText();
        const sourceText = controller.currentSourceText();
        const resolvedSelection = controller.resolveSelectionSnapshot(selectionSnapshot || null);
        const hasSelection = resolvedSelection.end > resolvedSelection.start;
        const snapshotCursor = selectionSnapshot && selectionSnapshot.cursorPosition !== undefined
                ? Number(selectionSnapshot.cursorPosition)
                : controller.currentEditorCursorPosition();
        const fallbackCursor = isFinite(snapshotCursor) ? snapshotCursor : 0;
        const baseStart = hasSelection ? resolvedSelection.start : fallbackCursor;
        const baseEnd = hasSelection ? resolvedSelection.end : fallbackCursor;
        const boundedStart = controller.clampLogicalPosition(baseStart, logicalText.length);
        const boundedEnd = controller.clampLogicalPosition(baseEnd, logicalText.length);
        let inclusiveEndOffset = hasSelection ? Math.max(boundedStart, boundedEnd - 1) : boundedStart;
        if (hasSelection
                && inclusiveEndOffset > boundedStart
                && inclusiveEndOffset < logicalText.length
                && logicalText.charAt(inclusiveEndOffset) === "\n") {
            inclusiveEndOffset -= 1;
        }
        const firstLineStart = controller.logicalLineStartForOffset(logicalText, boundedStart);
        const lastLineStart = controller.logicalLineStartForOffset(logicalText, inclusiveEndOffset);
        const entries = [];
        let lineStart = firstLineStart;
        while (lineStart <= logicalText.length) {
            const lineEnd = controller.logicalLineEndForStart(logicalText, lineStart);
            const sourceLineStart = controller.sourceOffsetForLogicalOffset(lineStart);
            const sourceLineEnd = controller.sourceOffsetForLogicalOffset(lineEnd);
            entries.push({
                    "logicalEnd": lineEnd,
                    "logicalLine": logicalText.slice(lineStart, lineEnd),
                    "logicalStart": lineStart,
                    "sourceEnd": sourceLineEnd,
                    "sourceLine": sourceText.slice(sourceLineStart, sourceLineEnd),
                    "sourceStart": sourceLineStart
                });
            if (lineStart === lastLineStart || lineEnd >= logicalText.length)
                break;
            lineStart = lineEnd + 1;
        }
        return {
            "cursorPosition": controller.clampLogicalPosition(fallbackCursor, logicalText.length),
            "hasSelection": hasSelection,
            "lines": entries,
            "selectionEnd": boundedEnd,
            "selectionStart": boundedStart
        };
    }
    function markdownListLineState(lineEntry) {
        const sourceLine = lineEntry && lineEntry.sourceLine !== undefined && lineEntry.sourceLine !== null ? String(lineEntry.sourceLine) : "";
        const unorderedMatch = /^([ \t]*)([-+*\u2022])(\s+)(.*)$/.exec(sourceLine);
        if (unorderedMatch) {
            return {
                "body": unorderedMatch[4],
                "indent": unorderedMatch[1],
                "kind": "unordered",
                "marker": unorderedMatch[2],
                "prefixLength": unorderedMatch[1].length + unorderedMatch[2].length + unorderedMatch[3].length,
                "sourceLine": sourceLine,
                "spacing": unorderedMatch[3]
            };
        }
        const orderedMatch = /^([ \t]*)(\d+)([.)])(\s+)(.*)$/.exec(sourceLine);
        if (orderedMatch) {
            return {
                "body": orderedMatch[5],
                "delimiter": orderedMatch[3],
                "indent": orderedMatch[1],
                "kind": "ordered",
                "number": orderedMatch[2],
                "prefixLength": orderedMatch[1].length + orderedMatch[2].length + orderedMatch[3].length + orderedMatch[4].length,
                "sourceLine": sourceLine,
                "spacing": orderedMatch[4]
            };
        }
        const plainMatch = /^([ \t]*)(.*)$/.exec(sourceLine);
        return {
            "body": plainMatch ? plainMatch[2] : "",
            "indent": plainMatch ? plainMatch[1] : "",
            "kind": "plain",
            "prefixLength": (plainMatch ? plainMatch[1] : "").length,
            "sourceLine": sourceLine
        };
    }
    function markdownListTransformedSourceLine(lineState, listKind, removeListMarker, orderedIndex) {
        const normalizedKind = controller.normalizeMarkdownListKind(listKind);
        if (normalizedKind.length === 0)
            return lineState && lineState.sourceLine !== undefined ? String(lineState.sourceLine) : "";
        if (!lineState)
            return "";
        if (removeListMarker) {
            if (lineState.kind !== normalizedKind)
                return lineState.sourceLine;
            return lineState.indent + lineState.body;
        }
        if (normalizedKind === "unordered")
            return lineState.indent + "- " + lineState.body;
        return lineState.indent + String(Math.max(1, Number(orderedIndex) || 1)) + ". " + lineState.body;
    }
    function markdownListLineShouldTransform(lineEntry, hasSelection) {
        if (!hasSelection)
            return true;
        const logicalLine = lineEntry && lineEntry.logicalLine !== undefined && lineEntry.logicalLine !== null ? String(lineEntry.logicalLine) : "";
        return logicalLine.trim().length > 0;
    }
    function queueMarkdownListMutation(listKind) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        const normalizedKind = controller.normalizeMarkdownListKind(listKind);
        if (normalizedKind.length === 0)
            return false;
        const selectionRange = controller.selectedEditorRange();
        const selectionSnapshot = controller.currentEditorSelectionSnapshot();
        const capturedSnapshot = {
            "cursorPosition": selectionSnapshot && selectionSnapshot.cursorPosition !== undefined ? Number(selectionSnapshot.cursorPosition) : controller.currentEditorCursorPosition(),
            "end": selectionRange.end,
            "selectedText": selectionSnapshot && selectionSnapshot.selectedText !== undefined ? selectionSnapshot.selectedText : controller.currentSelectedEditorText(),
            "start": selectionRange.start,
            "selectionEnd": selectionSnapshot && selectionSnapshot.selectionEnd !== undefined ? Number(selectionSnapshot.selectionEnd) : NaN,
            "selectionStart": selectionSnapshot && selectionSnapshot.selectionStart !== undefined ? Number(selectionSnapshot.selectionStart) : NaN
        };
        const noteId = controller.currentSelectedNoteId();
        const queueKey = noteId
                + "::list::"
                + normalizedKind
                + "::"
                + String(capturedSnapshot.start)
                + "::"
                + String(capturedSnapshot.end)
                + "::"
                + String(capturedSnapshot.cursorPosition);
        if (controller.queuedMarkdownListMutationKeys[queueKey])
            return true;
        controller.queuedMarkdownListMutationKeys[queueKey] = true;
        Qt.callLater(function () {
            delete controller.queuedMarkdownListMutationKeys[queueKey];
            if (!controller.view || !controller.view.hasSelectedNote)
                return;
            if (controller.currentSelectedNoteId() !== noteId)
                return;
            controller.toggleMarkdownListForSelection(normalizedKind, capturedSnapshot);
        });
        return true;
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
    function queueStructuredShortcutMutation(shortcutKind) {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        const normalizedKind = shortcutKind === undefined || shortcutKind === null
                ? ""
                : String(shortcutKind).toLowerCase();
        if (normalizedKind !== "agenda"
                && normalizedKind !== "callout"
                && normalizedKind !== "break")
            return false;
        const noteId = controller.currentSelectedNoteId();
        const selectionSnapshot = controller.currentEditorSelectionSnapshot();
        const rawCursor = selectionSnapshot && selectionSnapshot.cursorPosition !== undefined
                ? Number(selectionSnapshot.cursorPosition)
                : controller.currentEditorCursorPosition();
        const normalizedCursor = isFinite(rawCursor) ? Math.max(0, Math.floor(rawCursor)) : -1;
        const queueKey = noteId + "::shortcut::" + normalizedKind + "::" + String(normalizedCursor);
        if (controller.queuedStructuredShortcutMutationKeys[queueKey])
            return true;
        controller.queuedStructuredShortcutMutationKeys[queueKey] = true;
        Qt.callLater(function () {
            delete controller.queuedStructuredShortcutMutationKeys[queueKey];
            if (!controller.view || !controller.view.hasSelectedNote)
                return;
            if (controller.currentSelectedNoteId() !== noteId)
                return;
            if (normalizedKind === "agenda"
                    && controller.view.queueAgendaShortcutInsertion !== undefined) {
                controller.view.queueAgendaShortcutInsertion();
                return;
            }
            if (normalizedKind === "callout"
                    && controller.view.queueCalloutShortcutInsertion !== undefined) {
                controller.view.queueCalloutShortcutInsertion();
                return;
            }
            if (normalizedKind === "break"
                    && controller.view.queueBreakShortcutInsertion !== undefined) {
                controller.view.queueBreakShortcutInsertion();
            }
        });
        return true;
    }
    function handleInlineFormatShortcutKeyPress(event) {
        if (!event || !controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;

        const modifiers = event.modifiers;
        const eventText = event.text === undefined || event.text === null ? "" : String(event.text);
        const normalizedShortcutText = eventText.length === 1 ? eventText.toUpperCase() : "";
        const metaPressed = !!(modifiers & Qt.MetaModifier);
        const controlPressed = !!(modifiers & Qt.ControlModifier);
        const altPressed = !!(modifiers & Qt.AltModifier);
        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        const commandPressed = !!((modifiers & Qt.MetaModifier) || (modifiers & Qt.ControlModifier));
        const listShortcutPressed = !!((modifiers & Qt.MetaModifier) || (modifiers & Qt.AltModifier)) && !(modifiers & Qt.ControlModifier);
        const metaAltChord = metaPressed && altPressed && !shiftPressed;
        const controlAltFallbackChord = !metaPressed && controlPressed && altPressed && !shiftPressed;
        const agendaShortcutPressed = metaAltChord || controlAltFallbackChord;
        const calloutShortcutPressed = metaAltChord || controlAltFallbackChord;
        const breakShortcutPressed = metaAltChord || controlAltFallbackChord;
        const agendaShortcutKeyMatched = metaAltChord
                ? (event.key === Qt.Key_T || normalizedShortcutText === "T")
                : normalizedShortcutText === "T";
        const calloutShortcutKeyMatched = metaAltChord
                ? (event.key === Qt.Key_C || normalizedShortcutText === "C")
                : normalizedShortcutText === "C";
        const breakShortcutKeyMatched = metaAltChord
                ? (event.key === Qt.Key_H || normalizedShortcutText === "H")
                : normalizedShortcutText === "H";
        const autoRepeat = event.isAutoRepeat !== undefined && !!event.isAutoRepeat;
        if (autoRepeat
                && ((agendaShortcutPressed && agendaShortcutKeyMatched)
                    || (calloutShortcutPressed && calloutShortcutKeyMatched)
                    || (breakShortcutPressed && breakShortcutKeyMatched))) {
            event.accepted = true;
            return true;
        }
        let handled = false;
        switch (event.key) {
        case Qt.Key_B:
            if (commandPressed)
                handled = controller.queueInlineFormatWrap("bold");
            break;
        case Qt.Key_I:
            if (commandPressed)
                handled = controller.queueInlineFormatWrap("italic");
            break;
        case Qt.Key_U:
            if (commandPressed)
                handled = controller.queueInlineFormatWrap("underline");
            break;
        case Qt.Key_X:
            if (shiftPressed && commandPressed)
                handled = controller.queueInlineFormatWrap("strikethrough");
            break;
        case Qt.Key_E:
            if (shiftPressed && commandPressed)
                handled = controller.queueInlineFormatWrap("highlight");
            break;
        case Qt.Key_7:
        case Qt.Key_Ampersand:
            if (shiftPressed && listShortcutPressed)
                handled = controller.queueMarkdownListMutation("ordered");
            break;
        case Qt.Key_8:
        case Qt.Key_Asterisk:
            if (shiftPressed && listShortcutPressed)
                handled = controller.queueMarkdownListMutation("unordered");
            break;
        case Qt.Key_T:
            if (agendaShortcutPressed && agendaShortcutKeyMatched)
                handled = controller.queueStructuredShortcutMutation("agenda");
            break;
        case Qt.Key_C:
            if (calloutShortcutPressed && calloutShortcutKeyMatched)
                handled = controller.queueStructuredShortcutMutation("callout");
            break;
        case Qt.Key_H:
            if (breakShortcutPressed && breakShortcutKeyMatched)
                handled = controller.queueStructuredShortcutMutation("break");
            break;
        default:
            break;
        }
        if (!handled
                && agendaShortcutPressed
                && agendaShortcutKeyMatched
                ) {
            handled = controller.queueStructuredShortcutMutation("agenda");
        }
        if (!handled
                && calloutShortcutPressed
                && calloutShortcutKeyMatched
                ) {
            handled = controller.queueStructuredShortcutMutation("callout");
        }
        if (!handled
                && breakShortcutPressed
                && breakShortcutKeyMatched
                ) {
            handled = controller.queueStructuredShortcutMutation("break");
        }

        if (handled)
            event.accepted = true;
        return handled;
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
        if (!controller.view || !controller.editorSession || controller.editorSession.scheduleEditorPersistence === undefined)
            return false;
        controller.editorSession.scheduleEditorPersistence();
        return true;
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
    function toggleMarkdownListForSelection(listKind, selectionSnapshot) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        const normalizedKind = controller.normalizeMarkdownListKind(listKind);
        if (normalizedKind.length === 0)
            return false;
        const lineContext = controller.markdownListTargetLineEntries(selectionSnapshot || null);
        if (!lineContext.lines || lineContext.lines.length === 0)
            return false;
        const currentSourceText = controller.currentSourceText();
        const lineStates = [];
        let transformableCount = 0;
        let matchingCount = 0;
        for (let index = 0; index < lineContext.lines.length; ++index) {
            const lineEntry = lineContext.lines[index];
            const state = controller.markdownListLineState(lineEntry);
            const transformable = controller.markdownListLineShouldTransform(lineEntry, lineContext.hasSelection);
            if (transformable) {
                transformableCount += 1;
                if (state.kind === normalizedKind)
                    matchingCount += 1;
            }
            lineStates.push({
                    "entry": lineEntry,
                    "state": state,
                    "transformable": transformable
                });
        }
        if (transformableCount === 0 && lineStates.length > 0) {
            lineStates[0].transformable = true;
            transformableCount = 1;
            matchingCount = lineStates[0].state.kind === normalizedKind ? 1 : 0;
        }
        const removeListMarker = transformableCount > 0 && matchingCount === transformableCount;
        const blockSourceStart = lineStates[0].entry.sourceStart;
        const blockSourceEnd = lineStates[lineStates.length - 1].entry.sourceEnd;
        let nextBlockText = "";
        let blockCursor = blockSourceStart;
        let orderedIndex = 1;
        let selectionLogicalLength = 0;
        let nextCursorPosition = lineContext.cursorPosition;
        for (let index = 0; index < lineStates.length; ++index) {
            const lineInfo = lineStates[index];
            const lineEntry = lineInfo.entry;
            nextBlockText += currentSourceText.slice(blockCursor, lineEntry.sourceStart);
            const transformedSourceLine = lineInfo.transformable
                    ? controller.markdownListTransformedSourceLine(lineInfo.state, normalizedKind, removeListMarker, orderedIndex)
                    : lineInfo.state.sourceLine;
            const transformedLogicalLength = controller.logicalLengthForSourceText(transformedSourceLine);
            if (lineInfo.transformable && !removeListMarker && normalizedKind === "ordered")
                orderedIndex += 1;
            nextBlockText += transformedSourceLine;
            blockCursor = lineEntry.sourceEnd;
            selectionLogicalLength += transformedLogicalLength;
            if (index + 1 < lineStates.length)
                selectionLogicalLength += 1;
            if (!lineContext.hasSelection && lineContext.cursorPosition >= lineEntry.logicalStart && lineContext.cursorPosition <= lineEntry.logicalEnd) {
                const oldCursorInLine = lineContext.cursorPosition - lineEntry.logicalStart;
                const boundedCursorInLine = Math.max(0, Math.min(lineEntry.logicalLine.length, oldCursorInLine));
                const oldPrefixLength = Math.max(0, Number(lineInfo.state.prefixLength) || 0);
                const transformedLineState = controller.markdownListLineState({ "sourceLine": transformedSourceLine });
                const newPrefixLength = Math.max(0, Number(transformedLineState.prefixLength) || 0);
                let newCursorInLine = 0;
                if (removeListMarker && lineInfo.transformable && lineInfo.state.kind === normalizedKind) {
                    if (boundedCursorInLine <= oldPrefixLength)
                        newCursorInLine = transformedLineState.indent.length;
                    else
                        newCursorInLine = newPrefixLength + (boundedCursorInLine - oldPrefixLength);
                } else {
                    const contentOffset = Math.max(0, boundedCursorInLine - oldPrefixLength);
                    newCursorInLine = newPrefixLength + contentOffset;
                }
                nextCursorPosition = lineEntry.logicalStart + Math.max(0, Math.min(transformedLogicalLength, newCursorInLine));
            }
        }
        nextBlockText += currentSourceText.slice(blockCursor, blockSourceEnd);
        const nextText = currentSourceText.slice(0, blockSourceStart)
                + nextBlockText
                + currentSourceText.slice(blockSourceEnd);
        if (controller.view.editorText !== nextText)
            controller.view.editorText = nextText;
        controller.refreshPresentationStateAfterProgrammaticChange();
        if (controller.editorSession && controller.editorSession.markLocalEditorAuthority !== undefined)
            controller.editorSession.markLocalEditorAuthority();
        const saved = controller.persistEditorTextImmediately(nextText);
        if (!saved && controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
            controller.editorSession.scheduleEditorPersistence();
        if (lineContext.hasSelection) {
            controller.scheduleEditorSelection(
                        lineStates[0].entry.logicalStart,
                        lineStates[0].entry.logicalStart + selectionLogicalLength,
                        lineContext.cursorPosition);
        } else {
            controller.scheduleEditorSelection(NaN, NaN, nextCursorPosition);
        }
        controller.resetEditorSelectionCache();
        controller.view.editorTextEdited(nextText);
        return true;
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
        controller.refreshPresentationStateAfterProgrammaticChange();
        if (controller.editorSession && controller.editorSession.markLocalEditorAuthority !== undefined)
            controller.editorSession.markLocalEditorAuthority();
        const saved = controller.persistEditorTextImmediately(nextText);
        if (!saved && controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
            controller.editorSession.scheduleEditorPersistence();
        controller.resetEditorSelectionCache();
        controller.view.editorTextEdited(nextText);
        return true;
    }
}
