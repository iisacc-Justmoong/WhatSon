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
    property alias logicalCursorPosition: textInput.cursorPosition
    property var coordinateMapper: null
    property real editorBottomInset: LV.Theme.gap16
    property string displayGeometryText: control.text
    readonly property real displayTextContentHeight: control.renderedOverlayVisible
            ? renderedOverlay.contentHeight
            : textInput.contentHeight
    readonly property real displayContentHeight: control.displayTextContentHeight
            + control.editorBottomInset
    readonly property real displayBodyHeight: Math.max(0, control.displayTextContentHeight)
    readonly property var editorItem: textInput.editorItem
    property bool focused: textInput.focused
    readonly property var inputItem: textInput.editorItem
    property int inputMethodHints: Qt.ImhNone
    property int mouseSelectionMode: TextEdit.SelectCharacters
    readonly property bool nativeSelectionActive: textInput.selectionStart !== textInput.selectionEnd
    property var normalizedHtmlBlocks: []
    property bool overwriteMode: false
    property bool persistentSelection: true
    property int previousRawCursorPosition: 0
    property int visiblePointerClickCount: 0
    property real visiblePointerLastClickTimestamp: 0
    property real visiblePointerLastClickX: 0
    property real visiblePointerLastClickY: 0
    property int visiblePointerSelectionAnchorLogicalOffset: 0
    property bool visiblePointerSelectionActive: false
    property bool surfaceSelectionSyncActive: false
    property bool surfaceSelectionTextRefreshActive: false
    property bool surfaceSelectionToRawSyncScheduled: false
    property int surfaceSelectionCursorLogicalOffset: 0
    property int surfaceSelectionEndLogicalOffset: 0
    property int surfaceSelectionStartLogicalOffset: 0
    property bool cursorNormalizationActive: false
    readonly property bool inputMethodComposing: textInput.inputMethodComposing
    readonly property string preeditText: String(textInput.editorItem.preeditText)
    property string renderedText: ""
    readonly property int cursorPixelWidth: Math.max(1, Math.ceil(LV.Theme.strokeThin))
    readonly property var logicalGutterRows: lineNumberRailMetrics.rows
    readonly property int visualLineCount: visualLineMetrics.visualLineCount
    readonly property var visualLineWidthRatios: visualLineMetrics.visualLineWidthRatios
    readonly property bool renderedOverlayAvailable: control.showRenderedOutput
            && control.renderedText.length > 0
    readonly property bool renderedResourceOverlayPinned: control.renderedOverlayAvailable
            && control.hasAtomicRenderedResourceBlocks()
    readonly property bool renderedOverlayVisible: control.renderedOverlayAvailable
    readonly property bool logicalSurfaceActive: control.showRenderedOutput
    readonly property bool nativeSelectionContainsVisibleLogicalContent: control.nativeSelectionActive
            && (control.sourceRangeContainsVisibleLogicalContent(control.selectedSourceRange())
                || control.sourceRangeIntersectsAtomicResourceBlock(control.selectedSourceRange()))
    readonly property bool nativeSelectionPaintVisible: !control.renderedOverlayVisible
            || control.nativeSelectionContainsVisibleLogicalContent
    readonly property bool renderedSelectionActive: false
    readonly property bool atomicResourceCursorActive: control.logicalSurfaceActive
            && control.atomicResourceCursorActiveAtPosition(textInput.cursorPosition)
    readonly property bool nativeCursorVisible: control.focused
            && !control.nativeSelectionActive
            && !control.atomicResourceCursorActive
    readonly property bool preferNativeInputHandling: true
    property bool selectByKeyboard: true
    property bool selectByMouse: true
    readonly property string selectedText: textInput.selectedText
    readonly property var sourceSelectionRange: control.selectedSourceRange()
    readonly property int selectionEnd: Number(control.sourceSelectionRange.end) || 0
    readonly property int selectionStart: Number(control.sourceSelectionRange.start) || 0
    readonly property int sourceCursorPosition: control.sourceCursorFromSurfaceCursor(textInput.cursorPosition)
    readonly property int sourceSelectionEnd: control.selectionEnd
    readonly property int sourceSelectionStart: control.selectionStart
    property bool showRenderedOutput: false
    property var tagManagementKeyPressHandler: null
    property string text: ""
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
        return control.text;
    }

    function boundedCursorPosition(position, length) {
        return wysiwygEditorPolicy.boundedOffset(Number(position) || 0, Number(length) || 0);
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

    function projectedNativeSurfaceText() {
        return control.logicalSurfaceActive ? control.displayGeometryText : control.text;
    }

    function syncNativeSurfaceTextFromProjection(force) {
        const nextText = control.projectedNativeSurfaceText();
        if (force === true)
            inlineEditorController.applyImmediateProgrammaticText(nextText);
        else
            inlineEditorController.setProgrammaticText(nextText);
    }

    function logicalOffsetToSourceOffset(logicalOffset) {
        const mapper = control.coordinateMapper;
        if (mapper === null || mapper === undefined
                || mapper.sourceOffsetForVisibleLogicalOffset === undefined) {
            console.error("ContentsInlineFormatEditor requires coordinateMapper.sourceOffsetForVisibleLogicalOffset");
            return control.boundedCursorPosition(logicalOffset, control.text.length);
        }
        const mappedOffset = Number(mapper.sourceOffsetForVisibleLogicalOffset(
                                        Number(logicalOffset) || 0,
                                        renderedGeometryProbe.length));
        return control.boundedCursorPosition(mappedOffset, control.text.length);
    }

    function sourceOffsetForVisibleLogicalOffset(logicalOffset) {
        return control.logicalOffsetToSourceOffset(
                    control.boundedCursorPosition(logicalOffset, renderedGeometryProbe.length));
    }

    function sourceOffsetToLogicalOffset(sourceOffset, preferAfter) {
        const mapper = control.coordinateMapper;
        if (mapper === null || mapper === undefined
                || mapper.logicalOffsetForSourceOffsetWithAffinity === undefined) {
            console.error("ContentsInlineFormatEditor requires coordinateMapper.logicalOffsetForSourceOffsetWithAffinity");
            return control.boundedCursorPosition(sourceOffset, textInput.length);
        }
        const mappedOffset = Number(mapper.logicalOffsetForSourceOffsetWithAffinity(
                                        Number(sourceOffset) || 0,
                                        preferAfter === true));
        return control.boundedCursorPosition(mappedOffset, renderedGeometryProbe.length);
    }

    function sourceCursorFromSurfaceCursor(surfaceCursor) {
        if (!control.logicalSurfaceActive)
            return control.boundedCursorPosition(surfaceCursor, control.text.length);
        return control.sourceOffsetForVisibleLogicalOffset(surfaceCursor);
    }

    function selectedSourceRange() {
        const start = Math.max(0, Math.min(textInput.selectionStart, textInput.selectionEnd));
        const end = Math.max(start, Math.max(textInput.selectionStart, textInput.selectionEnd));
        if (control.logicalSurfaceActive) {
            const rawSelection = wysiwygEditorPolicy.rawSelectionForVisibleSurfaceSelection(
                        control.coordinateMapper,
                        start,
                        end,
                        textInput.cursorPosition,
                        textInput.length,
                        control.text.length);
            if (rawSelection.valid === true) {
                const visibleRange = wysiwygEditorPolicy.visibleContentSourceSelectionRange(
                            control.text,
                            rawSelection.selectionStart,
                            rawSelection.selectionEnd);
                return {
                    "start": visibleRange.start,
                    "end": visibleRange.end
                };
            }
        }
        return {
            "start": start,
            "end": end
        };
    }

    function normalizedSourceTagName(tagToken) {
        return wysiwygEditorPolicy.normalizedSourceTagName(String(tagToken || ""));
    }

    function sourceTagProducesVisibleSelection(tagName, closingTag) {
        return wysiwygEditorPolicy.sourceTagProducesVisibleSelection(
                    String(tagName || ""),
                    closingTag === true);
    }

    function sourceOffsetIsInsideTagToken(text, sourceOffset) {
        return wysiwygEditorPolicy.sourceOffsetIsInsideTagToken(
                    String(text || ""),
                    Number(sourceOffset) || 0);
    }

    function sourceTagTokenBoundsForCursor(sourceOffset) {
        return wysiwygEditorPolicy.sourceTagTokenBoundsForCursor(
                    control.text,
                    Number(sourceOffset) || 0);
    }

    function atomicResourceCursorNormalizationPlan(logicalCursor) {
        return wysiwygEditorPolicy.atomicResourceCursorNormalizationPlan(
                    control.text,
                    control.normalizedHtmlBlocks || [],
                    control.coordinateMapper,
                    Number(logicalCursor) || 0,
                    control.previousRawCursorPosition,
                    renderedGeometryProbe.length,
                    control.renderedOverlayVisible,
                    control.nativeCompositionActive(),
                    control.nativeSelectionActive);
    }

    function atomicResourceCursorActiveAtPosition(logicalCursor) {
        const plan = control.atomicResourceCursorNormalizationPlan(logicalCursor);
        return plan.resourceCursorActive === true;
    }

    function normalizeCursorPositionAwayFromAtomicResourceBlock() {
        if (!control.logicalSurfaceActive
                || control.cursorNormalizationActive
                || control.nativeCompositionActive()
                || control.nativeSelectionActive) {
            control.previousRawCursorPosition = control.sourceCursorPosition;
            return false;
        }

        const plan = control.atomicResourceCursorNormalizationPlan(textInput.cursorPosition);
        if (plan.resourceCursorActive !== true) {
            control.previousRawCursorPosition = plan.previousRawCursorPosition !== undefined
                    ? Number(plan.previousRawCursorPosition)
                    : control.sourceCursorPosition;
            return false;
        }

        if (plan.changed !== true) {
            control.previousRawCursorPosition = plan.previousRawCursorPosition !== undefined
                    ? Number(plan.previousRawCursorPosition)
                    : control.sourceCursorPosition;
            return false;
        }

        control.cursorNormalizationActive = true;
        textInput.cursorPosition = plan.targetLogicalCursor !== undefined
                ? Number(plan.targetLogicalCursor)
                : textInput.cursorPosition;
        control.cursorNormalizationActive = false;
        control.previousRawCursorPosition = plan.previousRawCursorPosition !== undefined
                ? Number(plan.previousRawCursorPosition)
                : control.sourceCursorPosition;
        control.scheduleRenderedOverlaySelectionRefresh();
        return true;
    }

    function normalizeCursorPositionAwayFromHiddenTagTokens() {
        if (control.cursorNormalizationActive)
            return false;
        if (control.logicalSurfaceActive) {
            return control.normalizeCursorPositionAwayFromAtomicResourceBlock();
        }

        const plan = wysiwygEditorPolicy.hiddenTagCursorNormalizationPlan(
                    control.text,
                    textInput.cursorPosition,
                    control.previousRawCursorPosition,
                    control.renderedOverlayVisible,
                    control.nativeCompositionActive(),
                    control.nativeSelectionActive,
                    false);
        if (plan.changed !== true) {
            control.previousRawCursorPosition = plan.previousRawCursorPosition !== undefined
                    ? Number(plan.previousRawCursorPosition)
                    : textInput.cursorPosition;
            return false;
        }

        control.cursorNormalizationActive = true;
        textInput.cursorPosition = plan.targetCursorPosition !== undefined
                ? Number(plan.targetCursorPosition)
                : textInput.cursorPosition;
        control.cursorNormalizationActive = false;
        control.previousRawCursorPosition = plan.previousRawCursorPosition !== undefined
                ? Number(plan.previousRawCursorPosition)
                : textInput.cursorPosition;
        control.scheduleRenderedOverlaySelectionRefresh();
        return true;
    }

    function sourceRangeContainsVisibleLogicalContent(range) {
        return wysiwygEditorPolicy.sourceRangeContainsVisibleLogicalContent(
                    control.text,
                    Number(range && range.start !== undefined ? range.start : 0) || 0,
                    Number(range && range.end !== undefined ? range.end : 0) || 0);
    }

    function htmlBlockSourceStart(block) {
        return wysiwygEditorPolicy.htmlBlockSourceStart(block);
    }

    function htmlBlockSourceEnd(block) {
        return wysiwygEditorPolicy.htmlBlockSourceEnd(block);
    }

    function htmlBlockIntersectsSourceRange(block, range) {
        return wysiwygEditorPolicy.htmlBlockIntersectsSourceRange(
                    block,
                    Number(range && range.start !== undefined ? range.start : 0) || 0,
                    Number(range && range.end !== undefined ? range.end : 0) || 0);
    }

    function sourceRangeIntersectsAtomicResourceBlock(range) {
        return wysiwygEditorPolicy.sourceRangeIntersectsAtomicResourceBlock(
                    control.normalizedHtmlBlocks || [],
                    Number(range && range.start !== undefined ? range.start : 0) || 0,
                    Number(range && range.end !== undefined ? range.end : 0) || 0);
    }

    function htmlBlockIsIiHtmlResourceBlock(block) {
        return wysiwygEditorPolicy.htmlBlockIsAtomicResourceBlock(block);
    }

    function hasAtomicRenderedResourceBlocks() {
        return wysiwygEditorPolicy.hasAtomicRenderedResourceBlocks(control.normalizedHtmlBlocks || []);
    }

    function resourceLogicalRangeForBlock(block) {
        return wysiwygEditorPolicy.resourceLogicalRangeForBlock(block, control.coordinateMapper);
    }

    function finiteNumber(value, fallback) {
        const number = Number(value);
        return isFinite(number) ? number : fallback;
    }

    function atomicResourceHitAtPoint(localX, localY) {
        const x = control.finiteNumber(localX, 0);
        const y = control.finiteNumber(localY, 0);
        const ranges = lineNumberRailMetrics.logicalLineRanges || [];
        const rows = editorGeometryProvider.lineNumberGeometryRows || [];
        const count = Math.min(ranges.length || 0, rows.length || 0);
        for (let index = 0; index < count; ++index) {
            const range = ranges[index] || {};
            if (range.resourceRange !== true)
                continue;
            const row = rows[index] || {};
            const rowX = control.finiteNumber(row.x, 0);
            const rowY = control.finiteNumber(row.y, 0);
            const rowWidth = Math.max(1, control.finiteNumber(row.width, control.width));
            const rowHeight = Math.max(
                        1,
                        control.finiteNumber(row.height, LV.Theme.textBodyLineHeight));
            if (x < rowX || x > rowX + rowWidth || y < rowY || y > rowY + rowHeight)
                continue;
            const start = control.boundedCursorPosition(
                        range.logicalStart !== undefined ? range.logicalStart : 0,
                        renderedGeometryProbe.length);
            const end = Math.max(
                        start,
                        control.boundedCursorPosition(
                            range.logicalEnd !== undefined ? range.logicalEnd : start,
                            renderedGeometryProbe.length));
            return {
                "hit": end > start,
                "start": start,
                "end": end
            };
        }
        return {
            "hit": false,
            "start": 0,
            "end": 0
        };
    }

    function visibleLogicalSelectionEndpointAtPoint(localX, localY, anchorLogicalOffset) {
        const resourceHit = control.atomicResourceHitAtPoint(localX, localY);
        if (resourceHit.hit === true) {
            const anchor = control.boundedCursorPosition(
                        anchorLogicalOffset,
                        renderedGeometryProbe.length);
            return anchor <= resourceHit.start ? resourceHit.end : resourceHit.start;
        }
        return control.visibleLogicalOffsetAtPoint(localX, localY);
    }

    function visibleLogicalOffsetAtPoint(localX, localY) {
        const geometryItem = control.displayGeometryItem();
        const mappedPoint = control.mapToItem(
                    geometryItem,
                    Number(localX) || 0,
                    Number(localY) || 0);
        const geometryWidth = Math.max(1, Number(geometryItem.width) || 1);
        const geometryHeight = Math.max(
                    1,
                    Number(geometryItem.contentHeight) || Number(geometryItem.height) || LV.Theme.textBodyLineHeight);
        const mappedY = Number(mappedPoint.y) || 0;
        const terminalBlankThreshold = geometryHeight + Math.max(1, Number(LV.Theme.textBodyLineHeight) || 1);
        if (mappedY > terminalBlankThreshold)
            return control.boundedCursorPosition(geometryItem.length, geometryItem.length);
        const clampedX = Math.max(0, Math.min(Number(mappedPoint.x) || 0, geometryWidth));
        const clampedY = Math.max(0, Math.min(mappedY, Math.max(0, geometryHeight - 1)));
        const logicalOffset = geometryItem.positionAt(clampedX, clampedY);
        return control.boundedCursorPosition(logicalOffset, geometryItem.length);
    }

    function restoreVisibleLogicalSelectionRange(anchorLogicalOffset, currentLogicalOffset) {
        const boundedCurrentLogicalOffset =
                control.boundedCursorPosition(currentLogicalOffset, renderedGeometryProbe.length);
        return control.restoreVisibleLogicalSelectionSpan(
                    anchorLogicalOffset,
                    boundedCurrentLogicalOffset,
                    boundedCurrentLogicalOffset);
    }

    function restoreVisibleLogicalSelectionSpan(startLogicalOffset, endLogicalOffset, cursorLogicalOffset) {
        const boundedStartLogicalOffset =
                control.boundedCursorPosition(startLogicalOffset, renderedGeometryProbe.length);
        const boundedEndLogicalOffset =
                control.boundedCursorPosition(endLogicalOffset, renderedGeometryProbe.length);
        const boundedCursorLogicalOffset =
                control.boundedCursorPosition(cursorLogicalOffset, renderedGeometryProbe.length);
        control.restoreSurfaceLogicalSelectionSpan(
                    boundedStartLogicalOffset,
                    boundedEndLogicalOffset,
                    boundedCursorLogicalOffset);
        return control.syncRawSelectionFromSurfaceSelection();
    }

    function restoreSurfaceLogicalSelectionSpan(startLogicalOffset, endLogicalOffset, cursorLogicalOffset) {
        const surfaceLength = Math.max(0, Number(renderedGeometryProbe.length) || 0);
        const start = control.boundedCursorPosition(startLogicalOffset, surfaceLength);
        const end = Math.max(start, control.boundedCursorPosition(endLogicalOffset, surfaceLength));
        const cursor = Math.max(start, Math.min(control.boundedCursorPosition(cursorLogicalOffset, surfaceLength), end));
        control.surfaceSelectionStartLogicalOffset = start;
        control.surfaceSelectionEndLogicalOffset = end;
        control.surfaceSelectionCursorLogicalOffset = cursor;
        control.surfaceSelectionSyncActive = true;
        if (start === end) {
            surfaceSelectionEditor.deselect();
            surfaceSelectionEditor.cursorPosition = cursor;
        } else if (cursor === start) {
            surfaceSelectionEditor.cursorPosition = end;
            surfaceSelectionEditor.moveCursorSelection(start, TextEdit.SelectCharacters);
        } else {
            surfaceSelectionEditor.cursorPosition = start;
            surfaceSelectionEditor.moveCursorSelection(end, TextEdit.SelectCharacters);
        }
        control.surfaceSelectionSyncActive = false;
        return true;
    }

    function syncRawSelectionFromSurfaceSelection() {
        if (!control.renderedOverlayVisible || control.nativeCompositionActive())
            return false;

        const rawSelection = wysiwygEditorPolicy.rawSelectionForVisibleSurfaceSelection(
                    control.coordinateMapper,
                    control.surfaceSelectionStartLogicalOffset,
                    control.surfaceSelectionEndLogicalOffset,
                    control.surfaceSelectionCursorLogicalOffset,
                    renderedGeometryProbe.length,
                    control.text.length);
        if (rawSelection.valid !== true)
            return false;

        const restored = control.restoreSelectionRange(
                    rawSelection.selectionStart,
                    rawSelection.selectionEnd,
                    rawSelection.cursorSourceOffset);
        if (!restored)
            return false;
        textInput.forceEditorFocus();
        control.scheduleRenderedOverlaySelectionRefresh();
        return true;
    }

    function scheduleSurfaceSelectionToRawSync() {
        if (control.surfaceSelectionSyncActive
                || control.surfaceSelectionTextRefreshActive
                || !control.renderedOverlayVisible
                || control.nativeCompositionActive()
                || control.surfaceSelectionToRawSyncScheduled) {
            return;
        }

        control.surfaceSelectionToRawSyncScheduled = true;
        Qt.callLater(function () {
            control.surfaceSelectionToRawSyncScheduled = false;
            if (control.syncRawSelectionFromSurfaceSelection())
                control.forceActiveFocus();
        });
    }

    function visibleLogicalLineRangeAtOffset(logicalOffset) {
        return wysiwygEditorPolicy.visibleLogicalLineRange(
                    String(control.displayGeometryText || ""),
                    Number(logicalOffset) || 0);
    }

    function visibleLogicalParagraphRangeAtOffset(logicalOffset) {
        return wysiwygEditorPolicy.visibleLogicalParagraphRange(
                    String(control.displayGeometryText || ""),
                    Number(logicalOffset) || 0);
    }

    function restoreVisibleLogicalLineSelectionAtLogicalOffset(logicalOffset) {
        const range = control.visibleLogicalLineRangeAtOffset(logicalOffset);
        return control.restoreVisibleLogicalSelectionSpan(range.start, range.end, range.end);
    }

    function restoreVisibleLogicalParagraphSelectionAtLogicalOffset(logicalOffset) {
        const range = control.visibleLogicalParagraphRangeAtOffset(logicalOffset);
        return control.restoreVisibleLogicalSelectionSpan(range.start, range.end, range.end);
    }

    function resetVisiblePointerClickSequence() {
        control.visiblePointerClickCount = 0;
        control.visiblePointerLastClickTimestamp = 0;
    }

    function updateVisiblePointerClickSequence(localX, localY) {
        const now = Date.now();
        const dx = Number(localX) - control.visiblePointerLastClickX;
        const dy = Number(localY) - control.visiblePointerLastClickY;
        const distance = Math.sqrt(dx * dx + dy * dy);
        const styleHints = Qt.styleHints || {};
        const interval = Number(styleHints.mouseDoubleClickInterval) || 500;
        const maximumDistance = Number(styleHints.startDragDistance) || LV.Theme.gap24;
        const continuingSequence =
                control.visiblePointerLastClickTimestamp > 0
                && now - control.visiblePointerLastClickTimestamp <= interval
                && distance <= maximumDistance;
        control.visiblePointerClickCount = continuingSequence
                ? control.visiblePointerClickCount + 1
                : 1;
        control.visiblePointerLastClickTimestamp = now;
        control.visiblePointerLastClickX = Number(localX) || 0;
        control.visiblePointerLastClickY = Number(localY) || 0;
        return control.visiblePointerClickCount;
    }

    function beginVisiblePointerSelection(localX, localY) {
        if (!control.renderedOverlayVisible || control.nativeCompositionActive())
            return false;
        control.forceActiveFocus();
        const clickCount = control.updateVisiblePointerClickSequence(localX, localY);
        const resourceHit = control.atomicResourceHitAtPoint(localX, localY);
        if (resourceHit.hit === true) {
            control.visiblePointerSelectionAnchorLogicalOffset = resourceHit.start;
            control.visiblePointerSelectionActive = true;
            if (clickCount >= 2)
                control.resetVisiblePointerClickSequence();
            return control.restoreVisibleLogicalSelectionSpan(
                        resourceHit.start,
                        resourceHit.end,
                        resourceHit.end);
        }
        const logicalOffset = control.visibleLogicalOffsetAtPoint(localX, localY);
        if (clickCount >= 3) {
            control.visiblePointerSelectionActive = false;
            control.resetVisiblePointerClickSequence();
            control.restoreVisibleLogicalParagraphSelectionAtLogicalOffset(logicalOffset);
            return true;
        }
        if (clickCount === 2) {
            control.visiblePointerSelectionActive = false;
            control.restoreVisibleLogicalLineSelectionAtLogicalOffset(logicalOffset);
            return true;
        }
        control.visiblePointerSelectionAnchorLogicalOffset = logicalOffset;
        control.visiblePointerSelectionActive = true;
        control.restoreVisibleLogicalSelectionRange(logicalOffset, logicalOffset);
        return true;
    }

    function updateVisiblePointerSelection(localX, localY) {
        if (!control.visiblePointerSelectionActive)
            return false;
        const logicalOffset = control.visibleLogicalSelectionEndpointAtPoint(
                    localX,
                    localY,
                    control.visiblePointerSelectionAnchorLogicalOffset);
        const restored = control.restoreVisibleLogicalSelectionRange(
                    control.visiblePointerSelectionAnchorLogicalOffset,
                    logicalOffset);
        if (restored && control.nativeSelectionActive)
            control.resetVisiblePointerClickSequence();
        return restored;
    }

    function finishVisiblePointerSelection() {
        control.visiblePointerSelectionActive = false;
    }

    function clearRenderedOverlaySelection() {
        if (renderedOverlay.deselect !== undefined)
            renderedOverlay.deselect();
        else
            renderedOverlay.select(0, 0);
    }

    function refreshRenderedOverlaySelection() {
        control.clearRenderedOverlaySelection();
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
        return control.logicalSurfaceActive ? renderedGeometryProbe : textInput.editorItem;
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
        control.forceActiveFocus();
        const sourceLength = control.text.length;
        const sourceStart = Math.max(0, Math.min(Number(selectionStart) || 0, sourceLength));
        let sourceEnd = Math.max(sourceStart, Math.min(Number(selectionEnd) || sourceStart, sourceLength));
        let sourceCursor = Math.max(sourceStart, Math.min(Number(cursorPosition) || sourceEnd, sourceEnd));
        const sourceRange = {
            "start": sourceStart,
            "end": sourceEnd
        };
        if (control.logicalSurfaceActive
                && sourceEnd > sourceStart
                && !control.sourceRangeContainsVisibleLogicalContent(sourceRange)
                && !control.sourceRangeIntersectsAtomicResourceBlock(sourceRange)) {
            sourceEnd = sourceStart;
            sourceCursor = sourceStart;
        }
        const collapsedSourceSelection = sourceStart === sourceEnd;
        const start = control.logicalSurfaceActive
                ? control.sourceOffsetToLogicalOffset(sourceStart, false)
                : sourceStart;
        let end = sourceEnd;
        let cursor = sourceCursor;
        if (control.logicalSurfaceActive) {
            if (collapsedSourceSelection) {
                end = start;
                cursor = start;
            } else {
                end = Math.max(start, control.sourceOffsetToLogicalOffset(sourceEnd, true));
                cursor = Math.max(
                            start,
                            Math.min(control.sourceOffsetToLogicalOffset(sourceCursor, true), end));
            }
        }
        if (start === end) {
            control.cursorNormalizationActive = true;
            textInput.deselect();
            textInput.cursorPosition = cursor;
            control.cursorNormalizationActive = false;
            return true;
        }
        control.cursorNormalizationActive = true;
        if (cursor === start) {
            textInput.cursorPosition = end;
            textInput.editorItem.moveCursorSelection(start, TextEdit.SelectCharacters);
        } else {
            textInput.cursorPosition = start;
            textInput.editorItem.moveCursorSelection(end, TextEdit.SelectCharacters);
        }
        control.cursorNormalizationActive = false;
        return true;
    }

    function selectionSnapshot() {
        return {
            "logicalCursorPosition": textInput.cursorPosition,
            "logicalSelectionEnd": textInput.selectionEnd,
            "logicalSelectionStart": textInput.selectionStart,
            "selectionStart": control.selectionStart,
            "selectionEnd": control.selectionEnd,
            "sourceCursorPosition": control.sourceCursorPosition,
            "sourceSelectionEnd": control.selectionEnd,
            "sourceSelectionStart": control.selectionStart,
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

    function handleNativeSurfaceTextEdited(nextSurfaceText) {
        const editedText = nextSurfaceText === undefined || nextSurfaceText === null
                ? ""
                : String(nextSurfaceText);
        if (!control.logicalSurfaceActive) {
            control.textEdited(editedText);
            return;
        }

        const payload = wysiwygEditorPolicy.visibleTextMutationPayload(
                    control.text,
                    control.coordinateMapper,
                    control.displayGeometryText,
                    editedText,
                    textInput.cursorPosition);
        if (payload.applied !== true || payload.nextSourceText === undefined || payload.nextSourceText === null)
            return;

        control.textEdited(String(payload.nextSourceText));
    }

    function applyTagManagementMutationPayload(payload) {
        if (payload === null || payload === undefined || typeof payload !== "object")
            return false;
        if (payload.applied !== true || payload.nextSourceText === undefined || payload.nextSourceText === null)
            return false;

        const nextText = String(payload.nextSourceText);
        if (!control.logicalSurfaceActive)
            inlineEditorController.applyImmediateProgrammaticText(nextText);

        const restorePayloadSelection = function () {
            const selectionStart = Math.max(0, Math.min(Number(payload.selectionStart) || 0, control.text.length));
            const selectionEnd = Math.max(selectionStart, Math.min(Number(payload.selectionEnd) || selectionStart, control.text.length));
            const cursorPosition = Math.max(selectionStart, Math.min(Number(payload.cursorPosition) || selectionEnd, selectionEnd));
            control.restoreSelectionRange(selectionStart, selectionEnd, cursorPosition);
        };
        control.textEdited(nextText);
        restorePayloadSelection();
        control.scheduleRenderedOverlaySelectionRefresh();
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

    onCoordinateMapperChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onDisplayGeometryTextChanged: {
        control.syncNativeSurfaceTextFromProjection(false);
    }
    onLogicalSurfaceActiveChanged: control.syncNativeSurfaceTextFromProjection(true)
    onNativeSelectionActiveChanged: {
        control.scheduleRenderedOverlaySelectionRefresh();
    }
    onNormalizedHtmlBlocksChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onRenderedOverlayVisibleChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onRenderedTextChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onTextChanged: {
        control.syncNativeSurfaceTextFromProjection(false);
    }

    Component.onCompleted: control.syncNativeSurfaceTextFromProjection(true)

    Keys.priority: Keys.BeforeItem
    Keys.onPressed: function (event) {
        control.handleTagManagementKeyPress(event);
    }

    ContentsInlineFormatEditorController {
        id: inlineEditorController

        control: control
        textInput: textInput.editorItem
    }

    ContentsWysiwygEditorPolicy {
        id: wysiwygEditorPolicy
    }

    ContentsEditorVisualLineMetrics {
        id: visualLineMetrics
    }

    ContentsEditorGeometryProvider {
        id: editorGeometryProvider

        fallbackLineHeight: LV.Theme.textBodyLineHeight
        fallbackWidth: control.width
        lineNumberRanges: lineNumberRailMetrics.logicalLineRanges
        logicalLength: control.displayGeometryText.length
        resourceItem: renderedOverlay
        targetItem: control
        textItem: control.displayGeometryItem()
        visualItem: control.renderedOverlayVisible ? renderedOverlay : textInput.editorItem
        visualLineHeight: LV.Theme.textBodyLineHeight
        visualStrokeThin: LV.Theme.strokeThin
        visualTextContentHeight: editorGeometryProvider.visualItem !== null
                                 && editorGeometryProvider.visualItem !== undefined
                                 && editorGeometryProvider.visualItem.contentHeight !== undefined
                ? editorGeometryProvider.visualItem.contentHeight
                : LV.Theme.gapNone
        visualTextLineCount: editorGeometryProvider.visualItem !== null
                             && editorGeometryProvider.visualItem !== undefined
                             && editorGeometryProvider.visualItem.lineCount !== undefined
                ? editorGeometryProvider.visualItem.lineCount
                : LV.Theme.gapNone
        visualTextWidth: editorGeometryProvider.visualItem !== null
                         && editorGeometryProvider.visualItem !== undefined
                         && editorGeometryProvider.visualItem.width !== undefined
                ? editorGeometryProvider.visualItem.width
                : LV.Theme.gapNone
    }

    ContentsLineNumberRailMetrics {
        id: lineNumberRailMetrics

        displayContentHeight: control.displayContentHeight
        geometryWidth: control.width
        logicalText: control.displayGeometryText
        normalizedHtmlBlocks: control.normalizedHtmlBlocks
        sourceText: control.text
        textLineHeight: LV.Theme.textBodyLineHeight
    }

    Binding {
        property: "measuredLineWidthRatios"
        target: visualLineMetrics
        value: editorGeometryProvider.visualLineWidthRatios
    }

    Binding {
        property: "measuredVisualLineCount"
        target: visualLineMetrics
        value: editorGeometryProvider.visualLineCount
    }

    Binding {
        property: "geometryRows"
        target: lineNumberRailMetrics
        value: editorGeometryProvider.lineNumberGeometryRows
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
        selectedTextColor: "transparent"
        selectionColor: "transparent"
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
        objectName: "contentsInlineFormatRenderedGeometryProbe"
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

    TextEdit {
        id: surfaceSelectionEditor

        activeFocusOnPress: false
        anchors.fill: renderedOverlay
        color: "transparent"
        enabled: false
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        objectName: "contentsInlineFormatSurfaceSelectionEditor"
        opacity: 0
        persistentSelection: true
        readOnly: true
        selectByKeyboard: false
        selectByMouse: false
        selectedTextColor: "transparent"
        selectionColor: "transparent"
        text: control.displayGeometryText
        textFormat: TextEdit.PlainText
        textMargin: LV.Theme.gapNone
        visible: control.renderedOverlayVisible && !control.nativeCompositionActive()
        wrapMode: TextEdit.Wrap
        z: 4

        onTextChanged: {
            control.surfaceSelectionTextRefreshActive = true;
            Qt.callLater(function () {
                control.surfaceSelectionTextRefreshActive = false;
            });
        }
    }

    MouseArea {
        id: visibleSelectionPointerArea

        acceptedButtons: Qt.LeftButton
        anchors.fill: parent
        enabled: control.renderedOverlayVisible && !control.nativeCompositionActive()
        hoverEnabled: true
        objectName: "contentsInlineFormatVisibleSelectionPointerArea"
        preventStealing: true
        z: 5

        onCanceled: control.finishVisiblePointerSelection()
        onPositionChanged: function (mouse) {
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
            control.handleNativeSurfaceTextEdited(text);
        }
        onCursorPositionChanged: {
            control.normalizeCursorPositionAwayFromHiddenTagTokens();
        }
        onSelectionEndChanged: {
            control.scheduleRenderedOverlaySelectionRefresh();
        }
        onSelectionStartChanged: {
            control.scheduleRenderedOverlaySelectionRefresh();
        }
    }

    Binding {
        property: "topPadding"
        target: textInput.editorItem
        value: LV.Theme.gapNone
    }
}
