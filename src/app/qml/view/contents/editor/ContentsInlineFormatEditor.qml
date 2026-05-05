pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: control

    property bool activeFocusOnPress: true
    property bool autoFocusOnPress: true
    property alias contentHeight: textInput.contentHeight
    property color cursorColor: LV.Theme.accentBlue
    property alias cursorPosition: textInput.cursorPosition
    property var atomicResourceSelectionRects: []
    property string displayGeometryText: textInput.text
    readonly property real displayContentHeight: control.renderedOverlayVisible
            ? renderedOverlay.contentHeight
            : textInput.contentHeight
    readonly property var editorItem: textInput.editorItem
    property bool focused: textInput.focused
    readonly property var inputItem: textInput.editorItem
    property int inputMethodHints: Qt.ImhNone
    property var logicalToSourceOffsets: []
    property int mouseSelectionMode: TextEdit.SelectCharacters
    readonly property bool nativeSelectionActive: textInput.selectionStart !== textInput.selectionEnd
    property var normalizedHtmlBlocks: []
    property bool overwriteMode: false
    property bool persistentSelection: true
    property int previousRawCursorPosition: 0
    property int visiblePointerSelectionAnchorLogicalOffset: 0
    property bool visiblePointerSelectionActive: false
    property bool cursorNormalizationActive: false
    readonly property bool inputMethodComposing: textInput.inputMethodComposing
    readonly property string preeditText: String(textInput.editorItem.preeditText)
    property string renderedText: ""
    property int logicalCursorPosition: textInput.cursorPosition
    readonly property int cursorPixelWidth: Math.max(1, Math.ceil(LV.Theme.strokeThin))
    readonly property rect projectedCursorRectangle: control.cursorProjectionRectangle()
    readonly property bool projectedCursorVisible: control.renderedOverlayVisible && control.focused
    readonly property var logicalGutterRows: control.resolvedLogicalGutterRows()
    readonly property int visualLineCount: control.resolvedVisibleLineCount()
    readonly property var visualLineWidthRatios: control.resolvedVisibleLineWidthRatios()
    readonly property bool renderedOverlayAvailable: control.showRenderedOutput
            && control.renderedText.length > 0
    readonly property bool renderedResourceOverlayPinned: control.renderedOverlayAvailable
            && control.hasAtomicRenderedResourceBlocks()
    readonly property bool renderedOverlayVisible: control.renderedOverlayAvailable
            && (!control.nativeCompositionActive() || control.renderedResourceOverlayPinned)
    readonly property bool nativeSelectionContainsVisibleLogicalContent: control.nativeSelectionActive
            && (control.sourceRangeContainsVisibleLogicalContent(control.selectedSourceRange())
                || control.sourceRangeIntersectsAtomicResourceBlock(control.selectedSourceRange()))
    readonly property bool nativeSelectionPaintVisible: !control.renderedOverlayVisible
            || control.nativeSelectionContainsVisibleLogicalContent
    readonly property bool renderedSelectionActive: control.renderedOverlayVisible
            && control.nativeSelectionActive
            && control.nativeSelectionContainsVisibleLogicalContent
    readonly property bool nativeCursorVisible: !control.renderedOverlayVisible
    readonly property bool preferNativeInputHandling: true
    property bool selectByKeyboard: true
    property bool selectByMouse: true
    property alias selectedText: textInput.selectedText
    property alias selectionEnd: textInput.selectionEnd
    property alias selectionStart: textInput.selectionStart
    property bool showRenderedOutput: false
    property var tagManagementKeyPressHandler: null
    property alias text: textInput.text
    property color textColor: LV.Theme.bodyColor

    signal textEdited(string text)
    signal viewHookRequested(string reason)

    focus: true

    function clearCachedSelectionSnapshot() {
    }

    function clearSelection() {
        textInput.deselect();
        control.clearRenderedOverlaySelection();
    }

    function currentPlainText() {
        return textInput.text;
    }

    function boundedCursorPosition(position, length) {
        const safeLength = Math.max(0, Number(length) || 0);
        return Math.max(0, Math.min(Number(position) || 0, safeLength));
    }

    function cursorProjectionRectangle() {
        const geometryItem = control.displayGeometryItem();
        if (geometryItem === null || geometryItem === undefined)
            return Qt.rect(0, 0, control.cursorPixelWidth, LV.Theme.textBodyLineHeight);
        const resolvedPosition = control.renderedOverlayVisible
                ? control.boundedCursorPosition(control.logicalCursorPosition, renderedGeometryProbe.length)
                : control.boundedCursorPosition(textInput.cursorPosition, textInput.length);
        const cursorRectangle = geometryItem.positionToRectangle(resolvedPosition);
        const mappedPoint = geometryItem.mapToItem(
                    control,
                    Number(cursorRectangle.x) || LV.Theme.gapNone,
                    Number(cursorRectangle.y) || LV.Theme.gapNone);
        return Qt.rect(
                    mappedPoint.x,
                    mappedPoint.y,
                    control.cursorPixelWidth,
                    Math.max(1, Number(cursorRectangle.height) || LV.Theme.textBodyLineHeight));
    }

    function resolvedTextItemLineCount(textItem) {
        if (textItem === null || textItem === undefined)
            return 1;

        const itemLineCount = Number(textItem.lineCount !== undefined ? textItem.lineCount : 0);
        const lineCountRows = isFinite(itemLineCount) && itemLineCount > 0
                ? Math.max(1, Math.ceil(itemLineCount))
                : 1;

        const itemContentHeight = Number(textItem.contentHeight !== undefined ? textItem.contentHeight : 0);
        const lineHeight = Math.max(1, Number(LV.Theme.textBodyLineHeight) || Number(LV.Theme.textBody) || 1);
        if (isFinite(itemContentHeight) && itemContentHeight > 0)
            return Math.max(lineCountRows, Math.ceil(itemContentHeight / lineHeight));

        return lineCountRows;
    }

    function resolvedTextItemWidth(textItem) {
        const itemWidth = Number(textItem && textItem.width !== undefined ? textItem.width : 0);
        if (isFinite(itemWidth) && itemWidth > 0)
            return itemWidth;
        return Math.max(1, Number(control.width) || 1);
    }

    function resolvedTextItemLineHeight() {
        return Math.max(1, Number(LV.Theme.textBodyLineHeight) || Number(LV.Theme.textBody) || 1);
    }

    function resolvedTextItemLineWidthRatios(textItem) {
        const rowCount = control.resolvedTextItemLineCount(textItem);
        const ratios = [];
        if (textItem === null || textItem === undefined) {
            for (let emptyIndex = 0; emptyIndex < rowCount; ++emptyIndex)
                ratios.push(1);
            return ratios;
        }

        const itemWidth = control.resolvedTextItemWidth(textItem);
        const lineHeight = control.resolvedTextItemLineHeight();
        const itemLineCount = Number(textItem.lineCount !== undefined ? textItem.lineCount : 0);
        const measuredLineRows = isFinite(itemLineCount) && itemLineCount > 0
                ? Math.max(1, Math.ceil(itemLineCount))
                : rowCount;
        const contentHeight = Math.max(
                    lineHeight,
                    Number(textItem.contentHeight !== undefined ? textItem.contentHeight : 0) || lineHeight);
        const minimumRatio = Math.min(1, Math.max(0, (Number(LV.Theme.strokeThin) || 1) / itemWidth));
        const canMeasureTextLine = textItem.positionAt !== undefined
                && textItem.positionToRectangle !== undefined;

        for (let rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
            if (!canMeasureTextLine || rowIndex >= measuredLineRows) {
                ratios.push(1);
                continue;
            }

            const probeY = Math.max(
                        0,
                        Math.min(contentHeight - 1, rowIndex * lineHeight + lineHeight / 2));
            const startOffset = textItem.positionAt(0, probeY);
            const endOffset = textItem.positionAt(Math.max(0, itemWidth - 1), probeY);
            const startRectangle = textItem.positionToRectangle(startOffset);
            const endRectangle = textItem.positionToRectangle(endOffset);
            const rawWidth = Math.abs((Number(endRectangle.x) || 0) - (Number(startRectangle.x) || 0));
            const normalizedWidth = Math.max(minimumRatio, Math.min(1, rawWidth / itemWidth));
            ratios.push(normalizedWidth);
        }
        return ratios;
    }

    function resolvedVisibleLineCount() {
        return control.resolvedTextItemLineCount(
                    control.renderedOverlayVisible ? renderedOverlay : textInput.editorItem);
    }

    function resolvedVisibleLineWidthRatios() {
        return control.resolvedTextItemLineWidthRatios(
                    control.renderedOverlayVisible ? renderedOverlay : textInput.editorItem);
    }

    function htmlBlockStableKey(block, fallbackIndex) {
        const tokenIndex = Number(block && block.htmlTokenStartIndex !== undefined ? block.htmlTokenStartIndex : -1);
        if (isFinite(tokenIndex) && tokenIndex >= 0)
            return "token:" + Math.floor(tokenIndex);

        const blockStart = control.htmlBlockSourceStart(block);
        const blockEnd = control.htmlBlockSourceEnd(block);
        if (blockStart >= 0 && blockEnd >= blockStart)
            return "source:" + blockStart + ":" + blockEnd;

        return "index:" + fallbackIndex;
    }

    function normalizedLogicalGutterBlocks() {
        const blocks = control.normalizedHtmlBlocks || [];
        const normalizedBlocks = [];
        const seenKeys = ({});
        for (let index = 0; index < blocks.length; ++index) {
            const block = blocks[index];
            if (block === null || block === undefined || typeof block !== "object")
                continue;

            const blockStart = control.htmlBlockSourceStart(block);
            const blockEnd = control.htmlBlockSourceEnd(block);
            if (blockStart < 0 || blockEnd < blockStart)
                continue;

            const key = control.htmlBlockStableKey(block, index);
            if (seenKeys[key] === true)
                continue;
            seenKeys[key] = true;
            normalizedBlocks.push(block);
        }

        if (normalizedBlocks.length > 0) {
            normalizedBlocks.sort(function (left, right) {
                const leftStart = control.htmlBlockSourceStart(left);
                const rightStart = control.htmlBlockSourceStart(right);
                if (leftStart !== rightStart)
                    return leftStart - rightStart;
                return control.htmlBlockSourceEnd(left) - control.htmlBlockSourceEnd(right);
            });
            return normalizedBlocks;
        }

        return [{
            "logicalLineCountHint": Math.max(1, String(textInput.text || "").split("\n").length),
            "sourceEnd": textInput.length,
            "sourceStart": 0,
            "sourceText": textInput.text
        }];
    }

    function sourceTextForGutterBlock(block) {
        if (block === null || block === undefined || typeof block !== "object")
            return "";
        if (block.sourceText !== undefined && block.sourceText !== null)
            return String(block.sourceText);
        if (block.plainText !== undefined && block.plainText !== null)
            return String(block.plainText);

        const blockStart = control.htmlBlockSourceStart(block);
        const blockEnd = control.htmlBlockSourceEnd(block);
        if (blockStart >= 0 && blockEnd >= blockStart)
            return String(textInput.text || "").slice(blockStart, blockEnd);
        return "";
    }

    function logicalLineCountForGutterBlock(block, sourceText) {
        const hint = Number(block && block.logicalLineCountHint !== undefined ? block.logicalLineCountHint : 0);
        const splitCount = Math.max(1, String(sourceText || "").split("\n").length);
        return Math.max(splitCount, isFinite(hint) && hint > 0 ? Math.floor(hint) : 1);
    }

    function logicalLineSourceRangesForBlock(block) {
        const blockStart = control.htmlBlockSourceStart(block);
        const blockEnd = control.htmlBlockSourceEnd(block);
        const safeStart = blockStart >= 0 ? blockStart : 0;
        const safeEnd = blockEnd >= safeStart ? blockEnd : safeStart;
        const sourceText = control.sourceTextForGutterBlock(block);
        const segments = String(sourceText || "").split("\n");
        const lineCount = control.logicalLineCountForGutterBlock(block, sourceText);
        const ranges = [];
        let sourceCursor = safeStart;
        for (let index = 0; index < lineCount; ++index) {
            const segment = index < segments.length ? String(segments[index]) : "";
            const lineStart = Math.min(sourceCursor, safeEnd);
            const lineEnd = Math.min(lineStart + segment.length, safeEnd);
            ranges.push({
                "end": lineEnd,
                "start": lineStart
            });
            sourceCursor = Math.min(safeEnd, lineEnd + 1);
        }
        return ranges;
    }

    function displayRectangleForLogicalRange(logicalStart, logicalEnd) {
        const geometryItem = control.displayGeometryItem();
        if (geometryItem === null || geometryItem === undefined
                || geometryItem.positionToRectangle === undefined) {
            return Qt.rect(0, 0, control.width, control.resolvedTextItemLineHeight());
        }

        const maxLength = Math.max(0, Number(geometryItem.length) || 0);
        const safeStart = Math.max(0, Math.min(Number(logicalStart) || 0, maxLength));
        const safeEnd = Math.max(safeStart, Math.min(Number(logicalEnd) || safeStart, maxLength));
        const startRectangle = geometryItem.positionToRectangle(safeStart);
        const lastLogicalOffset = Math.max(safeStart, Math.min(Math.max(0, safeEnd - 1), maxLength));
        const endRectangle = geometryItem.positionToRectangle(lastLogicalOffset);
        const startPoint = geometryItem.mapToItem(
                    control,
                    Number(startRectangle.x) || LV.Theme.gapNone,
                    Number(startRectangle.y) || LV.Theme.gapNone);
        const endPoint = geometryItem.mapToItem(
                    control,
                    Number(endRectangle.x) || LV.Theme.gapNone,
                    Number(endRectangle.y) || LV.Theme.gapNone);
        const lineHeight = control.resolvedTextItemLineHeight();
        const resolvedHeight = Math.max(
                    lineHeight,
                    endPoint.y + Math.max(lineHeight, Number(endRectangle.height) || lineHeight) - startPoint.y);
        return Qt.rect(startPoint.x, startPoint.y, control.width, resolvedHeight);
    }

    function textDisplayRectangleForSourceRange(sourceStart, sourceEnd) {
        const logicalStart = control.sourceOffsetToLogicalOffset(sourceStart, false);
        const logicalEnd = Math.max(
                    logicalStart,
                    control.sourceOffsetToLogicalOffset(Math.max(sourceStart, sourceEnd), true));
        return control.displayRectangleForLogicalRange(logicalStart, logicalEnd);
    }

    function logicalGutterRectangleForBlockLine(block, sourceRange) {
        if (control.htmlBlockIsIiHtmlResourceBlock(block))
            return control.resourceSelectionRectangleForBlock(block);
        return control.textDisplayRectangleForSourceRange(sourceRange.start, sourceRange.end);
    }

    function resolvedLogicalGutterRows() {
        const blocks = control.normalizedLogicalGutterBlocks();
        const rows = [];
        let lineNumber = 1;
        for (let blockIndex = 0; blockIndex < blocks.length; ++blockIndex) {
            const block = blocks[blockIndex];
            const ranges = control.logicalLineSourceRangesForBlock(block);
            for (let rangeIndex = 0; rangeIndex < ranges.length; ++rangeIndex) {
                const rectangle = control.logicalGutterRectangleForBlockLine(block, ranges[rangeIndex]);
                rows.push({
                    "height": Math.max(
                                  control.resolvedTextItemLineHeight(),
                                  Number(rectangle.height) || control.resolvedTextItemLineHeight()),
                    "number": lineNumber,
                    "sourceEnd": ranges[rangeIndex].end,
                    "sourceStart": ranges[rangeIndex].start,
                    "y": Math.max(0, Number(rectangle.y) || 0)
                });
                ++lineNumber;
            }
        }

        if (rows.length <= 0) {
            rows.push({
                "height": control.resolvedTextItemLineHeight(),
                "number": 1,
                "sourceEnd": 0,
                "sourceStart": 0,
                "y": 0
            });
        }
        return rows;
    }

    function eventRequestsBodyTagShortcut(event) {
        const key = event.key;
        const pureModifierKey = key === Qt.Key_Alt
                || key === Qt.Key_Control
                || key === Qt.Key_Meta
                || key === Qt.Key_Shift;
        if (pureModifierKey)
            return false;
        const modifiers = Number(event.modifiers) || 0;
        const commandHeld = (modifiers & Qt.ControlModifier) || (modifiers & Qt.MetaModifier);
        const optionHeld = modifiers & Qt.AltModifier;
        return Boolean((modifiers !== 0 && (key === Qt.Key_Return || key === Qt.Key_Enter))
                       || (commandHeld && optionHeld));
    }

    function eventRequestsInlineFormatShortcut(event) {
        const modifiers = Number(event.modifiers) || 0;
        const commandHeld = (modifiers & Qt.ControlModifier) || (modifiers & Qt.MetaModifier);
        const optionHeld = modifiers & Qt.AltModifier;
        const shiftHeld = modifiers & Qt.ShiftModifier;
        return Boolean(commandHeld && !optionHeld
                       && (event.key === Qt.Key_B
                           || event.key === Qt.Key_I
                           || event.key === Qt.Key_U
                           || (shiftHeld && event.key === Qt.Key_E)));
    }

    function eventRequestsPasteShortcut(event) {
        return event.matches(StandardKey.Paste);
    }

    function forceActiveFocus() {
        control.focus = true;
        textInput.forceEditorFocus();
    }

    function focusTerminalBodyPosition() {
        control.forceActiveFocus();
        control.clearSelection();
        return control.setCursorPositionPreservingNativeInput(textInput.length);
    }

    function logicalOffsetToSourceOffset(logicalOffset) {
        const fallbackOffset = control.boundedCursorPosition(logicalOffset, textInput.length);
        const offsets = control.logicalToSourceOffsets;
        if (!offsets || offsets.length === undefined || offsets.length <= 0)
            return fallbackOffset;
        const safeIndex = Math.max(0, Math.min(Number(logicalOffset) || 0, offsets.length - 1));
        const mappedOffset = Number(offsets[Math.floor(safeIndex)]);
        if (!isFinite(mappedOffset))
            return fallbackOffset;
        return control.boundedCursorPosition(mappedOffset, textInput.length);
    }

    function sourceOffsetForVisibleLogicalOffset(logicalOffset) {
        return control.logicalOffsetToSourceOffset(
                    control.boundedCursorPosition(logicalOffset, renderedGeometryProbe.length));
    }

    function sourceOffsetToLogicalOffset(sourceOffset, preferAfter) {
        const offsets = control.logicalToSourceOffsets;
        if (!offsets || offsets.length === undefined || offsets.length <= 0)
            return control.boundedCursorPosition(sourceOffset, textInput.length);

        const boundedSourceOffset = control.boundedCursorPosition(sourceOffset, textInput.length);
        if (preferAfter === true) {
            for (let index = 0; index < offsets.length; ++index) {
                const candidateOffset = Number(offsets[index]);
                if (isFinite(candidateOffset) && candidateOffset >= boundedSourceOffset)
                    return index;
            }
            return Math.max(0, offsets.length - 1);
        }

        for (let index = offsets.length - 1; index >= 0; --index) {
            const candidateOffset = Number(offsets[index]);
            if (isFinite(candidateOffset) && candidateOffset <= boundedSourceOffset)
                return index;
        }
        return 0;
    }

    function selectedSourceRange() {
        const start = Math.max(0, Math.min(textInput.selectionStart, textInput.selectionEnd));
        const end = Math.max(start, Math.max(textInput.selectionStart, textInput.selectionEnd));
        return {
            "start": start,
            "end": end
        };
    }

    function normalizedSourceTagName(tagToken) {
        const token = String(tagToken || "");
        if (token.length < 3 || token.charAt(0) !== "<")
            return "";
        let cursor = 1;
        while (cursor < token.length && /\s/.test(token.charAt(cursor)))
            ++cursor;
        if (cursor < token.length && token.charAt(cursor) === "/")
            ++cursor;
        while (cursor < token.length && /\s/.test(token.charAt(cursor)))
            ++cursor;
        const nameStart = cursor;
        while (cursor < token.length && /[A-Za-z0-9_.:-]/.test(token.charAt(cursor)))
            ++cursor;
        if (cursor <= nameStart)
            return "";
        return token.slice(nameStart, cursor).toLowerCase();
    }

    function sourceTagProducesVisibleSelection(tagName, closingTag) {
        const normalizedTagName = String(tagName || "").toLowerCase();
        if (closingTag)
            return false;
        return normalizedTagName === "resource"
                || normalizedTagName === "break"
                || normalizedTagName === "hr"
                || normalizedTagName === "tag";
    }

    function sourceOffsetIsInsideTagToken(text, sourceOffset) {
        const boundedOffset = Math.max(0, Math.min(Number(sourceOffset) || 0, Math.max(0, text.length - 1)));
        const previousOpen = text.lastIndexOf("<", boundedOffset);
        if (previousOpen < 0)
            return false;
        const previousClose = text.lastIndexOf(">", boundedOffset - 1);
        return previousOpen > previousClose;
    }

    function sourceTagTokenBoundsForCursor(sourceOffset) {
        const text = textInput.text;
        const sourceLength = text.length;
        if (sourceLength <= 0)
            return { "inside": false, "start": 0, "end": 0 };

        const offset = Math.max(0, Math.min(Number(sourceOffset) || 0, sourceLength));
        const probeOffset = Math.max(0, Math.min(offset, sourceLength - 1));
        const tagStart = text.lastIndexOf("<", probeOffset);
        if (tagStart < 0)
            return { "inside": false, "start": 0, "end": 0 };

        const previousClose = text.lastIndexOf(">", probeOffset - 1);
        const tagEnd = text.indexOf(">", tagStart + 1);
        const inside = tagEnd > tagStart
                && tagStart > previousClose
                && offset > tagStart
                && offset < tagEnd + 1;
        if (!inside)
            return { "inside": false, "start": tagStart, "end": tagEnd };
        return { "inside": true, "start": tagStart, "end": tagEnd };
    }

    function normalizeCursorPositionAwayFromHiddenTagTokens() {
        if (control.cursorNormalizationActive)
            return false;

        const currentCursorPosition = textInput.cursorPosition;
        if (!control.renderedOverlayVisible
                || control.nativeCompositionActive()
                || control.nativeSelectionActive) {
            control.previousRawCursorPosition = currentCursorPosition;
            return false;
        }

        const tokenBounds = control.sourceTagTokenBoundsForCursor(currentCursorPosition);
        if (!tokenBounds.inside) {
            control.previousRawCursorPosition = currentCursorPosition;
            return false;
        }

        const movingForward = currentCursorPosition >= control.previousRawCursorPosition;
        const targetCursorPosition = movingForward ? tokenBounds.end + 1 : tokenBounds.start;
        const boundedTarget = control.boundedCursorPosition(targetCursorPosition, textInput.length);
        if (boundedTarget === currentCursorPosition) {
            control.previousRawCursorPosition = currentCursorPosition;
            return false;
        }

        control.cursorNormalizationActive = true;
        textInput.cursorPosition = boundedTarget;
        control.cursorNormalizationActive = false;
        control.previousRawCursorPosition = textInput.cursorPosition;
        control.scheduleRenderedOverlaySelectionRefresh();
        return true;
    }

    function sourceRangeContainsVisibleLogicalContent(range) {
        const text = textInput.text;
        const sourceLength = text.length;
        const start = Math.max(0, Math.min(Number(range && range.start !== undefined ? range.start : 0) || 0, sourceLength));
        const end = Math.max(start, Math.min(Number(range && range.end !== undefined ? range.end : start) || start, sourceLength));
        let cursor = start;
        while (cursor < end) {
            if (control.sourceOffsetIsInsideTagToken(text, cursor)) {
                const tagStart = text.lastIndexOf("<", cursor);
                const tagEnd = text.indexOf(">", cursor);
                if (tagStart >= 0 && tagEnd >= cursor) {
                    const tagToken = text.slice(tagStart, tagEnd + 1);
                    const tagName = control.normalizedSourceTagName(tagToken);
                    const closingTag = /^\s*<\s*\//.test(tagToken);
                    if (control.sourceTagProducesVisibleSelection(tagName, closingTag))
                        return true;
                    cursor = tagEnd + 1;
                    continue;
                }
            }

            const currentChar = text.charAt(cursor);
            if (currentChar === "<") {
                const tagEnd = text.indexOf(">", cursor + 1);
                if (tagEnd > cursor) {
                    const tagToken = text.slice(cursor, tagEnd + 1);
                    const tagName = control.normalizedSourceTagName(tagToken);
                    const closingTag = /^\s*<\s*\//.test(tagToken);
                    if (control.sourceTagProducesVisibleSelection(tagName, closingTag))
                        return true;
                    cursor = tagEnd + 1;
                    continue;
                }
            }
            return true;
        }
        return false;
    }

    function htmlBlockSourceStart(block) {
        const value = Number(block && block.sourceStart !== undefined ? block.sourceStart : -1);
        return isFinite(value) ? Math.max(0, Math.floor(value)) : -1;
    }

    function htmlBlockSourceEnd(block) {
        const value = Number(block && block.sourceEnd !== undefined ? block.sourceEnd : -1);
        return isFinite(value) ? Math.max(0, Math.floor(value)) : -1;
    }

    function htmlBlockIntersectsSourceRange(block, range) {
        const blockStart = control.htmlBlockSourceStart(block);
        const blockEnd = control.htmlBlockSourceEnd(block);
        return blockStart >= 0 && blockEnd > blockStart
                && range.start < blockEnd && range.end > blockStart;
    }

    function sourceRangeIntersectsAtomicResourceBlock(range) {
        const blocks = control.normalizedHtmlBlocks || [];
        for (let index = 0; index < blocks.length; ++index) {
            const block = blocks[index];
            if (control.htmlBlockIsIiHtmlResourceBlock(block)
                    && control.htmlBlockIntersectsSourceRange(block, range)) {
                return true;
            }
        }
        return false;
    }

    function htmlBlockIsIiHtmlResourceBlock(block) {
        if (block === null || block === undefined || typeof block !== "object")
            return false;
        const blockKind = String(block.renderDelegateType || block.blockType || block.type || "").toLowerCase();
        if (blockKind !== "resource")
            return false;
        return String(block.htmlBlockObjectSource || "") === "iiHtmlBlock"
                && block.htmlBlockIsDisplayBlock !== false;
    }

    function hasAtomicRenderedResourceBlocks() {
        const blocks = control.normalizedHtmlBlocks || [];
        for (let index = 0; index < blocks.length; ++index) {
            if (control.htmlBlockIsIiHtmlResourceBlock(blocks[index]))
                return true;
        }
        return false;
    }

    function resourceLogicalRangeForBlock(block) {
        const blockStart = control.htmlBlockSourceStart(block);
        const blockEnd = control.htmlBlockSourceEnd(block);
        const logicalStart = control.sourceOffsetToLogicalOffset(blockStart, false);
        const logicalEnd = Math.max(
                    logicalStart + 1,
                    control.sourceOffsetToLogicalOffset(blockEnd, true));
        return {
            "start": logicalStart,
            "end": logicalEnd
        };
    }

    function resourceDisplayRectangleForBlock(block) {
        const range = control.resourceLogicalRangeForBlock(block);
        const maxLength = Math.max(0, Number(renderedOverlay.length) || 0);
        const logicalStart = Math.max(0, Math.min(range.start, maxLength));
        const logicalEnd = Math.max(logicalStart, Math.min(range.end, maxLength));
        const startRectangle = renderedOverlay.positionToRectangle(logicalStart);
        const endRectangle = renderedOverlay.positionToRectangle(logicalEnd);
        const localY = Math.max(0, Number(startRectangle.y) || 0);
        let localBottom = Number(endRectangle.y) || 0;
        if (!(localBottom > localY)) {
            localBottom = Math.max(localY, Number(renderedOverlay.contentHeight) || localY);
        }

        return {
            "x": Math.max(0, Number(startRectangle.x) || 0),
            "y": localY,
            "width": Math.max(Number(startRectangle.width) || 0, Number(renderedOverlay.width) || 0),
            "height": Math.max(LV.Theme.textBodyLineHeight, localBottom - localY)
        };
    }

    function renderedLogicalSelectionRange() {
        const sourceRange = control.selectedSourceRange();
        const maxLength = Math.max(0, Number(renderedOverlay.length) || 0);
        const intersectsAtomicResource = control.sourceRangeIntersectsAtomicResourceBlock(sourceRange);
        if (!control.sourceRangeContainsVisibleLogicalContent(sourceRange)
                && !intersectsAtomicResource) {
            return {
                "start": 0,
                "end": 0
            };
        }
        let start = control.sourceOffsetToLogicalOffset(sourceRange.start, false);
        let end = control.sourceOffsetToLogicalOffset(sourceRange.end, true);
        const blocks = control.normalizedHtmlBlocks || [];
        for (let index = 0; index < blocks.length; ++index) {
            const block = blocks[index];
            if (!control.htmlBlockIsIiHtmlResourceBlock(block)
                    || !control.htmlBlockIntersectsSourceRange(block, sourceRange)) {
                continue;
            }

            const resourceRange = control.resourceLogicalRangeForBlock(block);
            start = Math.min(start, resourceRange.start);
            end = Math.max(end, resourceRange.end);
        }

        start = Math.max(0, Math.min(start, maxLength));
        end = Math.max(start, Math.min(end, maxLength));
        return {
            "start": start,
            "end": end
        };
    }

    function visibleLogicalOffsetAtPoint(localX, localY) {
        const mappedPoint = control.mapToItem(
                    renderedGeometryProbe,
                    Number(localX) || 0,
                    Number(localY) || 0);
        const logicalOffset = renderedGeometryProbe.positionAt(mappedPoint.x, mappedPoint.y);
        return control.boundedCursorPosition(logicalOffset, renderedGeometryProbe.length);
    }

    function restoreVisibleLogicalSelectionRange(anchorLogicalOffset, currentLogicalOffset) {
        const anchorSourceOffset = control.sourceOffsetForVisibleLogicalOffset(anchorLogicalOffset);
        const currentSourceOffset = control.sourceOffsetForVisibleLogicalOffset(currentLogicalOffset);
        const selectionStart = Math.min(anchorSourceOffset, currentSourceOffset);
        const selectionEnd = Math.max(anchorSourceOffset, currentSourceOffset);
        control.restoreSelectionRange(selectionStart, selectionEnd, currentSourceOffset);
        control.scheduleRenderedOverlaySelectionRefresh();
        return true;
    }

    function beginVisiblePointerSelection(localX, localY) {
        if (!control.renderedOverlayVisible || control.nativeCompositionActive())
            return false;
        control.forceActiveFocus();
        const logicalOffset = control.visibleLogicalOffsetAtPoint(localX, localY);
        control.visiblePointerSelectionAnchorLogicalOffset = logicalOffset;
        control.visiblePointerSelectionActive = true;
        return control.restoreVisibleLogicalSelectionRange(logicalOffset, logicalOffset);
    }

    function updateVisiblePointerSelection(localX, localY) {
        if (!control.visiblePointerSelectionActive)
            return false;
        const logicalOffset = control.visibleLogicalOffsetAtPoint(localX, localY);
        return control.restoreVisibleLogicalSelectionRange(
                    control.visiblePointerSelectionAnchorLogicalOffset,
                    logicalOffset);
    }

    function finishVisiblePointerSelection() {
        control.visiblePointerSelectionActive = false;
    }

    function clearRenderedOverlaySelection() {
        if (renderedOverlay.deselect !== undefined)
            renderedOverlay.deselect();
        else
            renderedOverlay.select(0, 0);
        control.atomicResourceSelectionRects = [];
    }

    function resourceSelectionRectangleForBlock(block) {
        const displayRectangle = control.resourceDisplayRectangleForBlock(block);

        const mappedPoint = renderedOverlay.mapToItem(
                    control,
                    Number(displayRectangle.x) || 0,
                    Number(displayRectangle.y) || 0);
        return {
            "x": mappedPoint.x,
            "y": mappedPoint.y,
            "width": displayRectangle.width,
            "height": displayRectangle.height
        };
    }

    function buildAtomicResourceSelectionRects(sourceRange) {
        const blocks = control.normalizedHtmlBlocks || [];
        const rects = [];
        for (let index = 0; index < blocks.length; ++index) {
            const block = blocks[index];
            if (!control.htmlBlockIsIiHtmlResourceBlock(block)
                    || !control.htmlBlockIntersectsSourceRange(block, sourceRange)) {
                continue;
            }
            rects.push(control.resourceSelectionRectangleForBlock(block));
        }
        return rects;
    }

    function refreshRenderedOverlaySelection() {
        if (!control.renderedSelectionActive) {
            control.clearRenderedOverlaySelection();
            return;
        }

        const selectionRange = control.renderedLogicalSelectionRange();
        renderedOverlay.select(selectionRange.start, selectionRange.end);
        control.atomicResourceSelectionRects =
                control.buildAtomicResourceSelectionRects(control.selectedSourceRange());
    }

    function scheduleRenderedOverlaySelectionRefresh() {
        Qt.callLater(control.refreshRenderedOverlaySelection);
    }

    function inlineFormatSelectionSnapshot() {
        return selectionSnapshot();
    }

    function nativeCompositionActive() {
        return control.inputMethodComposing || control.preeditText.length > 0;
    }

    function displayGeometryItem() {
        return control.renderedOverlayVisible ? renderedGeometryProbe : textInput.editorItem;
    }

    function requestViewHook(reason) {
        control.viewHookRequested(reason !== undefined ? String(reason) : "manual");
    }

    function programmaticTextSyncPolicy(nextText) {
        return inlineEditorController.programmaticTextSyncPolicy(nextText);
    }

    function canDeferProgrammaticTextSync(nextText) {
        return inlineEditorController.canDeferProgrammaticTextSync(nextText);
    }

    function shouldRejectFocusedProgrammaticTextSync(nextText) {
        return inlineEditorController.shouldRejectFocusedProgrammaticTextSync(nextText);
    }

    function flushDeferredProgrammaticText(force) {
        inlineEditorController.flushDeferredProgrammaticText(force === true);
    }

    function clearDeferredProgrammaticText() {
        inlineEditorController.clearDeferredProgrammaticText();
    }

    function restoreSelectionRange(selectionStart, selectionEnd, cursorPosition) {
        const start = Math.max(0, Math.min(Number(selectionStart) || 0, textInput.length));
        const end = Math.max(start, Math.min(Number(selectionEnd) || start, textInput.length));
        const cursor = Math.max(start, Math.min(Number(cursorPosition) || end, end));
        if (start === end) {
            textInput.cursorPosition = cursor;
            return true;
        }
        if (cursor === start) {
            textInput.cursorPosition = end;
            textInput.editorItem.moveCursorSelection(start, TextEdit.SelectCharacters);
        } else {
            textInput.cursorPosition = start;
            textInput.editorItem.moveCursorSelection(end, TextEdit.SelectCharacters);
        }
        return true;
    }

    function selectionSnapshot() {
        return {
            "cursorPosition": textInput.cursorPosition,
            "selectionStart": textInput.selectionStart,
            "selectionEnd": textInput.selectionEnd,
            "selectedText": textInput.selectedText
        };
    }

    function setCursorPositionPreservingNativeInput(position) {
        if (nativeCompositionActive())
            return textInput.cursorPosition;
        textInput.cursorPosition = Math.max(0, Math.min(Number(position) || 0, textInput.length));
        return textInput.cursorPosition;
    }

    function setProgrammaticText(nextText) {
        const resolvedText = nextText === undefined || nextText === null ? "" : String(nextText);
        inlineEditorController.setProgrammaticText(resolvedText);
    }

    function applyTagManagementMutationPayload(payload) {
        if (payload === null || payload === undefined || typeof payload !== "object")
            return false;
        if (payload.applied !== true || payload.nextSourceText === undefined || payload.nextSourceText === null)
            return false;

        const nextText = String(payload.nextSourceText);
        inlineEditorController.applyImmediateProgrammaticText(nextText);

        const restorePayloadSelection = function () {
            const selectionStart = Math.max(0, Math.min(Number(payload.selectionStart) || 0, textInput.length));
            const selectionEnd = Math.max(selectionStart, Math.min(Number(payload.selectionEnd) || selectionStart, textInput.length));
            const cursorPosition = Math.max(selectionStart, Math.min(Number(payload.cursorPosition) || selectionEnd, selectionEnd));
            control.restoreSelectionRange(selectionStart, selectionEnd, cursorPosition);
        };
        restorePayloadSelection();
        control.scheduleRenderedOverlaySelectionRefresh();
        control.textEdited(nextText);
        Qt.callLater(restorePayloadSelection);
        return true;
    }

    function handleTagManagementKeyPress(event) {
        const key = event.key;
        if (key !== Qt.Key_Backspace
                && !control.eventRequestsPasteShortcut(event)
                && !control.eventRequestsInlineFormatShortcut(event)
                && !control.eventRequestsBodyTagShortcut(event)) {
            event.accepted = false;
            return;
        }

        if (control.tagManagementKeyPressHandler === null
                || control.tagManagementKeyPressHandler === undefined) {
            event.accepted = false;
            return;
        }

        const handled = Boolean(control.tagManagementKeyPressHandler(event));
        event.accepted = handled;
        if (!handled)
            event.accepted = false;
    }

    function syntheticTagManagementKeyEvent(key, modifiers) {
        return {
            "accepted": false,
            "key": key,
            "matches": function (standardKey) {
                return false;
            },
            "modifiers": modifiers
        };
    }

    function triggerTagManagementShortcut(key, modifiers) {
        const event = syntheticTagManagementKeyEvent(key, modifiers);
        control.handleTagManagementKeyPress(event);
        return event.accepted === true;
    }

    clip: true

    onLogicalToSourceOffsetsChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onNativeSelectionActiveChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onNormalizedHtmlBlocksChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onRenderedOverlayVisibleChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onRenderedTextChanged: control.scheduleRenderedOverlaySelectionRefresh()

    Keys.priority: Keys.BeforeItem
    Keys.onPressed: function (event) {
        control.handleTagManagementKeyPress(event);
    }

    ContentsInlineFormatEditorController {
        id: inlineEditorController

        control: control
        textInput: textInput.editorItem
    }

    TextEdit {
        id: renderedOverlay

        activeFocusOnPress: false
        anchors.fill: textInput
        color: control.textColor
        enabled: false
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        objectName: "contentsInlineFormatRenderedOverlay"
        readOnly: true
        selectByKeyboard: false
        selectByMouse: false
        text: control.renderedText
        textFormat: TextEdit.RichText
        textMargin: LV.Theme.gapNone
        selectedTextColor: control.textColor
        selectionColor: LV.Theme.primaryOverlay
        visible: control.renderedOverlayVisible
        wrapMode: TextEdit.Wrap
        z: -1

        onLinkActivated: function (link) {
            Qt.openUrlExternally(link);
        }
    }

    TextEdit {
        id: renderedGeometryProbe

        activeFocusOnPress: false
        anchors.fill: renderedOverlay
        color: "transparent"
        enabled: false
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        opacity: 0
        readOnly: true
        selectByKeyboard: false
        selectByMouse: false
        text: control.displayGeometryText
        textFormat: TextEdit.PlainText
        textMargin: LV.Theme.gapNone
        visible: control.displayGeometryText.length > 0
        wrapMode: TextEdit.Wrap
        z: -2
    }

    MouseArea {
        id: visibleSelectionPointerArea

        acceptedButtons: Qt.LeftButton
        anchors.fill: parent
        enabled: control.renderedOverlayVisible && !control.nativeCompositionActive()
        objectName: "contentsInlineFormatVisibleSelectionPointerArea"
        preventStealing: true
        z: 4

        onCanceled: control.finishVisiblePointerSelection()
        onPositionChanged: function (mouse) {
            if (pressed)
                control.updateVisiblePointerSelection(mouse.x, mouse.y);
        }
        onPressed: function (mouse) {
            mouse.accepted = control.beginVisiblePointerSelection(mouse.x, mouse.y);
        }
        onReleased: function (mouse) {
            control.updateVisiblePointerSelection(mouse.x, mouse.y);
            control.finishVisiblePointerSelection();
            mouse.accepted = true;
        }
    }

    Item {
        id: atomicResourceSelectionLayer

        anchors.fill: parent
        objectName: "contentsInlineFormatAtomicResourceSelectionLayer"
        visible: control.renderedSelectionActive
                 && control.atomicResourceSelectionRects.length > 0
        z: 2

        Repeater {
            model: control.atomicResourceSelectionRects

            Rectangle {
                required property var modelData

                color: LV.Theme.primaryOverlay
                height: Math.max(LV.Theme.textBodyLineHeight, Number(modelData.height) || LV.Theme.textBodyLineHeight)
                width: Math.max(LV.Theme.strokeThin, Number(modelData.width) || LV.Theme.strokeThin)
                x: Number(modelData.x) || LV.Theme.gapNone
                y: Number(modelData.y) || LV.Theme.gapNone
            }
        }
    }

    Rectangle {
        id: projectedCursor

        objectName: "contentsInlineFormatProjectedCursor"
        color: control.cursorColor
        height: control.projectedCursorRectangle.height
        radius: width / 2
        visible: control.projectedCursorVisible
        width: control.cursorPixelWidth
        x: control.projectedCursorRectangle.x
        y: control.projectedCursorRectangle.y
        z: 3

        onVisibleChanged: {
            if (visible)
                opacity = 1.0;
        }
        onXChanged: {
            if (visible)
                opacity = 1.0;
        }
        onYChanged: {
            if (visible)
                opacity = 1.0;
        }

        SequentialAnimation {
            running: projectedCursor.visible
            loops: Animation.Infinite

            PropertyAction {
                target: projectedCursor
                property: "opacity"
                value: 1.0
            }
            PauseAnimation {
                duration: 520
            }
            PropertyAction {
                target: projectedCursor
                property: "opacity"
                value: 0.0
            }
            PauseAnimation {
                duration: 420
            }
        }
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: control.focused
                 && control.tagManagementKeyPressHandler !== null
                 && control.tagManagementKeyPressHandler !== undefined
        sequence: "Ctrl+Alt+C"

        onActivated: control.triggerTagManagementShortcut(Qt.Key_C, Qt.ControlModifier | Qt.AltModifier)
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: control.focused
                 && control.tagManagementKeyPressHandler !== null
                 && control.tagManagementKeyPressHandler !== undefined
        sequence: "Meta+Alt+C"

        onActivated: control.triggerTagManagementShortcut(Qt.Key_C, Qt.MetaModifier | Qt.AltModifier)
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: control.focused
                 && control.tagManagementKeyPressHandler !== null
                 && control.tagManagementKeyPressHandler !== undefined
        sequence: "Ctrl+Alt+A"

        onActivated: control.triggerTagManagementShortcut(Qt.Key_A, Qt.ControlModifier | Qt.AltModifier)
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: control.focused
                 && control.tagManagementKeyPressHandler !== null
                 && control.tagManagementKeyPressHandler !== undefined
        sequence: "Meta+Alt+A"

        onActivated: control.triggerTagManagementShortcut(Qt.Key_A, Qt.MetaModifier | Qt.AltModifier)
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: control.focused
                 && control.tagManagementKeyPressHandler !== null
                 && control.tagManagementKeyPressHandler !== undefined
        sequence: "Ctrl+Shift+E"

        onActivated: control.triggerTagManagementShortcut(Qt.Key_E, Qt.ControlModifier | Qt.ShiftModifier)
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: control.focused
                 && control.tagManagementKeyPressHandler !== null
                 && control.tagManagementKeyPressHandler !== undefined
        sequence: "Meta+Shift+E"

        onActivated: control.triggerTagManagementShortcut(Qt.Key_E, Qt.MetaModifier | Qt.ShiftModifier)
    }

    LV.TextEditor {
        id: textInput

        anchors.fill: parent
        anchors.leftMargin: LV.Theme.gap16
        anchors.rightMargin: LV.Theme.gap16
        activeFocusOnPress: control.autoFocusOnPress
        autoFocusOnPress: control.autoFocusOnPress
        backgroundColor: "transparent"
        backgroundColorDisabled: "transparent"
        backgroundColorFocused: "transparent"
        backgroundColorHover: "transparent"
        backgroundColorPressed: "transparent"
        centeredTextHeight: Math.max(1, textInput.resolvedEditorHeight - textInput.insetVertical * 2)
        cornerRadius: LV.Theme.gapNone
        cursorDelegate: Rectangle {
            objectName: "contentsInlineFormatNativeCursor"
            color: control.cursorColor
            radius: width / 2
            visible: control.nativeCursorVisible
            width: control.nativeCursorVisible ? control.cursorPixelWidth : 0

            SequentialAnimation on opacity {
                running: control.nativeCursorVisible
                loops: Animation.Infinite

                NumberAnimation {
                    duration: 1
                    to: 1.0
                }
                PauseAnimation {
                    duration: 520
                }
                NumberAnimation {
                    duration: 1
                    to: 0.0
                }
                PauseAnimation {
                    duration: 420
                }
            }
        }
        editorHeight: Math.max(1, control.height)
        enforceModeDefaults: true
        fieldMinHeight: LV.Theme.gap16
        fontFamily: LV.Theme.fontBody
        fontPixelSize: LV.Theme.textBody
        insetHorizontal: LV.Theme.gapNone
        insetVertical: LV.Theme.gapNone
        inputMethodHints: control.inputMethodHints
        mode: plainTextMode
        mouseSelectionMode: control.mouseSelectionMode
        overwriteMode: control.overwriteMode
        persistentSelection: control.persistentSelection
        preferNativeGestures: true
        selectByKeyboard: control.selectByKeyboard
        selectByMouse: control.selectByMouse
        selectedTextColor: control.renderedOverlayVisible ? "transparent" : control.textColor
        selectionColor: control.nativeSelectionPaintVisible ? LV.Theme.primaryOverlay : "transparent"
        showRenderedOutput: false
        showScrollBar: false
        textColor: control.renderedOverlayVisible ? "transparent" : control.textColor
        textColorDisabled: textColor
        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.Wrap

        Keys.priority: Keys.BeforeItem
        Keys.onPressed: function (event) {
            control.handleTagManagementKeyPress(event);
        }

        onTextEdited: function (text) {
            control.textEdited(text);
        }
        onCursorPositionChanged: control.normalizeCursorPositionAwayFromHiddenTagTokens()
        onSelectionEndChanged: control.scheduleRenderedOverlaySelectionRefresh()
        onSelectionStartChanged: control.scheduleRenderedOverlaySelectionRefresh()
    }
}
