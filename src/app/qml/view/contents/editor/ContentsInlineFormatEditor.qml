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
    property var coordinateMapper: null
    property var atomicResourceSelectionRects: []
    property string displayGeometryText: control.text
    readonly property real displayContentHeight: control.renderedOverlayVisible
            ? renderedOverlay.contentHeight
            : textInput.contentHeight
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
    property int visiblePointerCursorLogicalOffset: -1
    property bool visiblePointerCursorUpdateActive: false
    property bool surfaceSelectionSyncActive: false
    property bool surfaceSelectionTextRefreshActive: false
    property bool surfaceSelectionToRawSyncScheduled: false
    property bool cursorNormalizationActive: false
    readonly property bool inputMethodComposing: textInput.inputMethodComposing
    readonly property string preeditText: String(textInput.editorItem.preeditText)
    property string renderedText: ""
    property int logicalCursorPosition: textInput.cursorPosition
    readonly property int resolvedProjectedCursorPosition: control.visiblePointerCursorLogicalOffset >= 0
            ? control.visiblePointerCursorLogicalOffset
            : (control.logicalSurfaceActive ? textInput.cursorPosition : control.logicalCursorPosition)
    readonly property int cursorPixelWidth: Math.max(1, Math.ceil(LV.Theme.strokeThin))
    readonly property rect projectedCursorRectangle: control.cursorProjectionRectangle()
    readonly property bool projectedCursorVisible: control.renderedOverlayVisible
            && control.focused
            && !control.nativeSelectionActive
    readonly property var logicalGutterRows: lineNumberRailMetrics.rows
    readonly property int visualLineCount: visualLineMetrics.visualLineCount
    readonly property var visualLineWidthRatios: visualLineMetrics.visualLineWidthRatios
    readonly property bool renderedOverlayAvailable: control.showRenderedOutput
            && control.renderedText.length > 0
    readonly property bool renderedResourceOverlayPinned: control.renderedOverlayAvailable
            && control.hasAtomicRenderedResourceBlocks()
    readonly property bool renderedOverlayVisible: control.renderedOverlayAvailable
            && (!control.nativeCompositionActive() || control.renderedResourceOverlayPinned)
    readonly property bool logicalSurfaceActive: control.showRenderedOutput
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

    function cursorProjectionRectangle() {
        const geometryItem = control.displayGeometryItem();
        if (geometryItem === null || geometryItem === undefined)
            return Qt.rect(0, 0, control.cursorPixelWidth, LV.Theme.textBodyLineHeight);
        const resolvedPosition = control.renderedOverlayVisible
                ? control.boundedCursorPosition(control.resolvedProjectedCursorPosition, renderedGeometryProbe.length)
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
                return {
                    "start": rawSelection.selectionStart,
                    "end": rawSelection.selectionEnd
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

    function normalizeCursorPositionAwayFromHiddenTagTokens() {
        if (control.cursorNormalizationActive)
            return false;
        if (control.logicalSurfaceActive) {
            control.previousRawCursorPosition = control.sourceCursorPosition;
            return false;
        }

        const plan = wysiwygEditorPolicy.hiddenTagCursorNormalizationPlan(
                    control.text,
                    textInput.cursorPosition,
                    control.previousRawCursorPosition,
                    control.renderedOverlayVisible,
                    control.nativeCompositionActive(),
                    control.nativeSelectionActive,
                    control.visiblePointerCursorUpdateActive);
        if (plan.clearVisiblePointerCursor === true)
            control.visiblePointerCursorLogicalOffset = -1;
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
        return wysiwygEditorPolicy.renderedLogicalSelectionRange(
                    control.text,
                    control.normalizedHtmlBlocks || [],
                    control.coordinateMapper,
                    sourceRange.start,
                    sourceRange.end,
                    Number(renderedOverlay.length) || 0);
    }

    function visibleLogicalOffsetAtPoint(localX, localY) {
        const geometryItem = control.renderedOverlayVisible ? renderedOverlay : renderedGeometryProbe;
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
        const surfaceLength = Math.max(0, Number(surfaceSelectionEditor.length) || 0);
        const start = control.boundedCursorPosition(startLogicalOffset, surfaceLength);
        const end = Math.max(start, control.boundedCursorPosition(endLogicalOffset, surfaceLength));
        const cursor = Math.max(start, Math.min(control.boundedCursorPosition(cursorLogicalOffset, surfaceLength), end));
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
                    surfaceSelectionEditor.selectionStart,
                    surfaceSelectionEditor.selectionEnd,
                    surfaceSelectionEditor.cursorPosition,
                    surfaceSelectionEditor.length,
                    control.text.length);
        if (rawSelection.valid !== true)
            return false;

        control.visiblePointerCursorUpdateActive = true;
        const restored = control.restoreSelectionRange(
                    rawSelection.selectionStart,
                    rawSelection.selectionEnd,
                    rawSelection.cursorSourceOffset);
        control.visiblePointerCursorUpdateActive = false;
        if (!restored)
            return false;
        textInput.forceEditorFocus();
        control.visiblePointerCursorLogicalOffset = rawSelection.selectionStart === rawSelection.selectionEnd
                ? rawSelection.surfaceCursor
                : -1;
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
        const logicalOffset = control.visibleLogicalOffsetAtPoint(localX, localY);
        const clickCount = control.updateVisiblePointerClickSequence(localX, localY);
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
        const logicalOffset = control.visibleLogicalOffsetAtPoint(localX, localY);
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
        return control.renderedOverlayVisible ? renderedOverlay : textInput.editorItem;
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
        const sourceLength = control.text.length;
        const sourceStart = Math.max(0, Math.min(Number(selectionStart) || 0, sourceLength));
        const sourceEnd = Math.max(sourceStart, Math.min(Number(selectionEnd) || sourceStart, sourceLength));
        const sourceCursor = Math.max(sourceStart, Math.min(Number(cursorPosition) || sourceEnd, sourceEnd));
        const start = control.logicalSurfaceActive
                ? control.sourceOffsetToLogicalOffset(sourceStart, false)
                : sourceStart;
        const end = control.logicalSurfaceActive
                ? Math.max(start, control.sourceOffsetToLogicalOffset(sourceEnd, true))
                : sourceEnd;
        const cursor = control.logicalSurfaceActive
                ? Math.max(start, Math.min(control.sourceOffsetToLogicalOffset(sourceCursor, true), end))
                : sourceCursor;
        if (start === end) {
            textInput.deselect();
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
            "cursorPosition": control.sourceCursorPosition,
            "selectionStart": control.selectionStart,
            "selectionEnd": control.selectionEnd,
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

        control.visiblePointerCursorLogicalOffset = control.boundedCursorPosition(
                    Number(payload.surfaceCursor) || textInput.cursorPosition,
                    textInput.length);
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

    function applyRenderedBackspaceMutation(event) {
        if (event.key !== Qt.Key_Backspace
                || control.logicalSurfaceActive
                || !control.renderedOverlayVisible
                || control.nativeCompositionActive()
                || control.nativeSelectionActive) {
            return false;
        }

        const modifiers = Number(event.modifiers) || 0;
        const commandModifier = modifiers & (Qt.ControlModifier | Qt.MetaModifier | Qt.AltModifier);
        if (commandModifier)
            return false;

        const payload = wysiwygEditorPolicy.visibleBackspaceMutationPayload(
                    control.text,
                    control.coordinateMapper,
                    control.resolvedProjectedCursorPosition,
                    renderedGeometryProbe.length);
        if (!control.applyTagManagementMutationPayload(payload))
            return false;

        if (payload.surfaceCursor !== undefined)
            control.visiblePointerCursorLogicalOffset = control.boundedCursorPosition(
                        Number(payload.surfaceCursor) || 0,
                        renderedGeometryProbe.length);
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

        if (control.applyRenderedBackspaceMutation(event)) {
            event.accepted = true;
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
    onDisplayGeometryTextChanged: control.syncNativeSurfaceTextFromProjection(false)
    onLogicalCursorPositionChanged: {
        if (control.visiblePointerCursorLogicalOffset === control.logicalCursorPosition)
            control.visiblePointerCursorLogicalOffset = -1;
    }
    onLogicalSurfaceActiveChanged: control.syncNativeSurfaceTextFromProjection(true)
    onNativeSelectionActiveChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onNormalizedHtmlBlocksChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onRenderedOverlayVisibleChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onRenderedTextChanged: control.scheduleRenderedOverlaySelectionRefresh()
    onTextChanged: control.syncNativeSurfaceTextFromProjection(false)

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
        text: control.renderedOverlayVisible ? control.renderedText : control.displayGeometryText
        textFormat: control.renderedOverlayVisible ? TextEdit.RichText : TextEdit.PlainText
        textMargin: LV.Theme.gapNone
        visible: control.displayGeometryText.length > 0
        wrapMode: TextEdit.Wrap
        z: -2
    }

    TextEdit {
        id: surfaceSelectionEditor

        activeFocusOnPress: true
        anchors.fill: renderedOverlay
        color: "transparent"
        enabled: control.renderedOverlayVisible && !control.nativeCompositionActive()
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        objectName: "contentsInlineFormatSurfaceSelectionEditor"
        opacity: 0
        persistentSelection: true
        readOnly: true
        selectByKeyboard: false
        selectByMouse: true
        selectedTextColor: "transparent"
        selectionColor: "transparent"
        text: control.renderedOverlayVisible ? control.renderedText : control.displayGeometryText
        textFormat: control.renderedOverlayVisible ? TextEdit.RichText : TextEdit.PlainText
        textMargin: LV.Theme.gapNone
        visible: enabled
        wrapMode: TextEdit.Wrap
        z: 4

        Keys.priority: Keys.BeforeItem
        Keys.onPressed: function (event) {
            control.forceActiveFocus();
            control.handleTagManagementKeyPress(event);
            if (!event.accepted)
                event.accepted = false;
        }

        onCursorPositionChanged: control.scheduleSurfaceSelectionToRawSync()
        onSelectionEndChanged: control.scheduleSurfaceSelectionToRawSync()
        onSelectionStartChanged: control.scheduleSurfaceSelectionToRawSync()
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
        text: control.projectedNativeSurfaceText()
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
        onCursorPositionChanged: control.normalizeCursorPositionAwayFromHiddenTagTokens()
        onSelectionEndChanged: control.scheduleRenderedOverlaySelectionRefresh()
        onSelectionStartChanged: control.scheduleRenderedOverlaySelectionRefresh()
    }
}
