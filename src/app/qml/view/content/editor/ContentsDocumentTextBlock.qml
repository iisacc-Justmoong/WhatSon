pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import WhatSon.App.Internal 1.0
import "ContentsLogicalLineLayoutSupport.js" as LogicalLineLayoutSupport
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: textBlock

    required property var blockData
    property bool nativeTextInputPriority: false
    property bool paperPaletteEnabled: false

    signal activated()
    signal adjacentAtomicBlockDeleteRequested(string side)
    signal boundaryNavigationRequested(string axis, string side)
    signal blockDeletionRequested(string direction)
    signal paragraphSplitRequested(int sourceOffset)
    signal sourceMutationRequested(string nextBlockSourceText, var focusRequest, string expectedPreviousSourceText)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property int currentLogicalLineNumber: textBlock.currentEditorLogicalLineNumber()
    readonly property bool focused: blockEditor.focused
    readonly property bool inputMethodComposing: blockEditor.inputMethodComposing !== undefined
                                                       && blockEditor.inputMethodComposing
    readonly property bool textEditable: true
    readonly property bool atomicBlock: false
    readonly property bool gutterCollapsed: false
    readonly property string minimapVisualKind: "text"
    readonly property int minimapRepresentativeCharCount: 0
    property bool hasAdjacentAtomicBlockAfter: false
    property bool hasAdjacentAtomicBlockBefore: false
    property bool hasAdjacentBlockAfter: false
    property bool hasAdjacentBlockBefore: false
    property bool paragraphBoundaryOperationsEnabled: false
    property bool paragraphMergeableBefore: false
    property bool paragraphMergeableAfter: false
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || 0)
    readonly property string sourceText: normalizedBlock.sourceText !== undefined ? String(normalizedBlock.sourceText) : ""
    readonly property color editorTextColor: paperPaletteEnabled ? "#111111" : LV.Theme.bodyColor
    readonly property bool inlineStyleOverlayVisible: inlineStyleRenderer
                                                      && inlineStyleRenderer.htmlOverlayVisible !== undefined
                                                      ? !!inlineStyleRenderer.htmlOverlayVisible
                                                      : /<\s*\/?\s*(?:bold|italic|underline|strikethrough|highlight)\b/i.test(
                                                            textBlock.sourceText)
    readonly property string authoritativePlainText: StructuredCursorSupport.plainTextFromInlineTaggedSource(
                                                        textBlock.authoritativeSourceText())
    readonly property string preeditText: blockEditor.preeditText === undefined || blockEditor.preeditText === null
                                          ? ""
                                          : String(blockEditor.preeditText)
    property bool _hasLiveEditSnapshot: false
    property string _liveEditPlainText: ""
    property string _liveEditSourceText: ""

    implicitHeight: blockEditor.implicitHeight
    width: parent ? parent.width : implicitWidth

    function authoritativeSourceText() {
        return StructuredCursorSupport.normalizedPlainText(textBlock.sourceText)
    }

    function visiblePlainText() {
        return textBlock.currentEditorPlainText()
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
        return Math.max(0, normalizedLineText.length)
    }

    function currentEditorPlainText() {
        if (!blockEditor)
            return textBlock.authoritativePlainText
        if (blockEditor.currentPlainText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(blockEditor.currentPlainText())
        if (blockEditor.getText === undefined)
            return textBlock.authoritativePlainText
        const editorLength = blockEditor.length !== undefined
                ? Math.max(0, Number(blockEditor.length) || 0)
                : 0
        return StructuredCursorSupport.normalizedPlainText(blockEditor.getText(0, editorLength))
    }

    function nativeCompositionActive() {
        return textBlock.inputMethodComposing || textBlock.preeditText.length > 0
    }

    function syncLiveEditSnapshotFromHost() {
        const hostSourceText = textBlock.authoritativeSourceText()
        const hostPlainText = textBlock.authoritativePlainText
        if (textBlock.focused
                && textBlock._hasLiveEditSnapshot
                && hostSourceText !== textBlock._liveEditSourceText
                && textBlock.currentEditorPlainText() !== hostPlainText) {
            return
        }
        textBlock._liveEditSourceText = hostSourceText
        textBlock._liveEditPlainText = hostPlainText
        textBlock._hasLiveEditSnapshot = true
    }

    function logicalLineLayoutEntries() {
        const plainTextValue = textBlock.currentEditorPlainText()
        const editorItem = blockEditor && blockEditor.editorItem ? blockEditor.editorItem : null
        const blockHeight = Math.max(
                    1,
                    Number(textBlock.implicitHeight) || 0,
                    Number(blockEditor ? blockEditor.implicitHeight : 0) || 0,
                    Number(blockEditor ? blockEditor.height : 0) || 0)
        return LogicalLineLayoutSupport.buildEntries(
                    plainTextValue,
                    blockHeight,
                    editorItem,
                    textBlock,
                    Math.round(LV.Theme.scaleMetric(12)))
    }

    function currentEditorLogicalLineNumber() {
        const plainTextValue = textBlock.currentEditorPlainText()
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        plainTextValue.length,
                        Number(blockEditor && blockEditor.cursorPosition !== undefined ? blockEditor.cursorPosition : 0) || 0))
        let lineNumber = 1
        for (let index = 0; index < cursorPosition; ++index) {
            if (plainTextValue.charAt(index) === "\n")
                lineNumber += 1
        }
        return Math.max(1, lineNumber)
    }

    function currentCursorRowRect() {
        const editorItem = blockEditor && blockEditor.editorItem ? blockEditor.editorItem : null
        const plainTextValue = textBlock.currentEditorPlainText()
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        plainTextValue.length,
                        Number(blockEditor && blockEditor.cursorPosition !== undefined ? blockEditor.cursorPosition : 0) || 0))
        if (!editorItem || editorItem.positionToRectangle === undefined)
            return ({
                        "contentHeight": Math.max(1, Number(blockEditor ? blockEditor.inputContentHeight : 0) || Math.round(LV.Theme.scaleMetric(12))),
                        "contentY": 0
                    })
        const rect = editorItem.positionToRectangle(cursorPosition)
        const mappedPoint = editorItem.mapToItem !== undefined
                ? editorItem.mapToItem(textBlock, 0, Number(rect.y) || 0)
                : ({ "x": 0, "y": Number(rect.y) || 0 })
        return {
            "contentHeight": Math.max(1, Number(rect.height) || Math.round(LV.Theme.scaleMetric(12))),
            "contentY": Math.max(0, Number(mappedPoint.y) || 0)
        }
    }

    function cursorOnFirstVisualRow() {
        const rowRect = textBlock.currentCursorRowRect()
        return Math.max(0, Number(rowRect.contentY) || 0) <= 1
    }

    function cursorOnLastVisualRow() {
        const rowRect = textBlock.currentCursorRowRect()
        const rowBottom = Math.max(
                    0,
                    (Number(rowRect.contentY) || 0) + (Number(rowRect.contentHeight) || 0))
        const contentHeight = Math.max(
                    1,
                    Number(blockEditor && blockEditor.inputContentHeight !== undefined ? blockEditor.inputContentHeight : 0)
                    || rowBottom)
        return rowBottom >= contentHeight - 1
    }

    function computePlainTextReplacementDelta(previousText, nextText) {
        const previous = previousText === undefined || previousText === null ? "" : String(previousText)
        const next = nextText === undefined || nextText === null ? "" : String(nextText)
        if (previous === next) {
            return {
                "insertedText": "",
                "previousEnd": 0,
                "start": 0,
                "valid": false
            }
        }

        let prefixLength = 0
        const prefixLimit = Math.min(previous.length, next.length)
        while (prefixLength < prefixLimit && previous.charAt(prefixLength) === next.charAt(prefixLength))
            ++prefixLength

        let suffixLength = 0
        const suffixLimit = Math.min(previous.length - prefixLength, next.length - prefixLength)
        while (suffixLength < suffixLimit
                && previous.charAt(previous.length - 1 - suffixLength) === next.charAt(next.length - 1 - suffixLength)) {
            ++suffixLength
        }

        const previousEnd = previous.length - suffixLength
        const nextEnd = next.length - suffixLength
        return {
            "insertedText": next.slice(prefixLength, nextEnd),
            "previousEnd": previousEnd,
            "start": prefixLength,
            "valid": true
        }
    }

    function focusEditor(cursorPosition) {
        blockEditor.forceActiveFocus()
        const numericCursorPosition = Number(cursorPosition)
        const targetCursorPosition = isFinite(numericCursorPosition)
                                   ? Math.max(0, Math.min(Math.floor(numericCursorPosition), Math.max(0, blockEditor.length || 0)))
                                   : Math.max(0, blockEditor.length || 0)
        if (blockEditor.setCursorPositionPreservingNativeInput !== undefined)
            blockEditor.setCursorPositionPreservingNativeInput(targetCursorPosition)
        else if (blockEditor.cursorPosition !== undefined)
            blockEditor.cursorPosition = targetCursorPosition
        textBlock.activated()
    }

    function restoreEditorSelection(selectionStart, selectionEnd, cursorPosition) {
        if (!blockEditor || blockEditor.restoreSelectionRange === undefined)
            return false
        return !!blockEditor.restoreSelectionRange(selectionStart, selectionEnd, cursorPosition)
    }

    function clearSelection(preserveFocusedEditor) {
        if (!blockEditor || blockEditor.clearSelection === undefined)
            return false
        if (preserveFocusedEditor === true && textBlock.focused)
            return false
        blockEditor.clearSelection()
        return true
    }

    function inlineFormatSelectionSnapshot() {
        if (!blockEditor)
            return ({})
        if (blockEditor.inlineFormatSelectionSnapshot !== undefined)
            return blockEditor.inlineFormatSelectionSnapshot()
        if (blockEditor.selectionSnapshot !== undefined)
            return blockEditor.selectionSnapshot()
        return ({})
    }

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const localCursorPosition = Number(safeRequest.localCursorPosition)
        const selectionStart = Number(safeRequest.selectionStart)
        const selectionEnd = Number(safeRequest.selectionEnd)
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (isFinite(localCursorPosition) && !isFinite(sourceOffset)) {
            textBlock.focusEditor(localCursorPosition)
            if (isFinite(selectionStart) && isFinite(selectionEnd) && selectionEnd > selectionStart)
                textBlock.restoreEditorSelection(selectionStart, selectionEnd, localCursorPosition)
            return true
        }
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < textBlock.sourceStart || sourceOffset > textBlock.sourceEnd)
            return false

        const resolvedCursorPosition = Math.max(0, StructuredCursorSupport.plainCursorForInlineTaggedSourceOffset(
                                                    textBlock.authoritativeSourceText(),
                                                    Math.floor(sourceOffset - textBlock.sourceStart)))
        textBlock.focusEditor(resolvedCursorPosition)
        if (isFinite(selectionStart) && isFinite(selectionEnd) && selectionEnd > selectionStart)
            textBlock.restoreEditorSelection(selectionStart, selectionEnd, resolvedCursorPosition)
        return true
    }

    function shortcutInsertionSourceOffset() {
        return Math.max(textBlock.sourceStart, Math.min(
                            textBlock.sourceEnd,
                            StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                                textBlock.authoritativeSourceText(),
                                Number(blockEditor.cursorPosition) || 0,
                                textBlock.sourceStart)))
    }

    ContentsTextFormatRenderer {
        id: inlineStyleRenderer

        paperPaletteEnabled: textBlock.paperPaletteEnabled
        sourceText: textBlock.inlineStyleOverlayVisible ? textBlock.authoritativeSourceText() : ""
    }

    ContentsInlineFormatEditor {
        id: blockEditor

        anchors.left: parent.left
        anchors.right: parent.right
        backgroundColor: "transparent"
        backgroundColorDisabled: "transparent"
        backgroundColorFocused: "transparent"
        backgroundColorHover: "transparent"
        backgroundColorPressed: "transparent"
        centeredTextHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
        cornerRadius: 0
        fieldMinHeight: Math.max(Math.round(LV.Theme.scaleMetric(12)), inputContentHeight)
        fontFamily: LV.Theme.fontBody
        fontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
        fontWeight: Font.Medium
        insetHorizontal: 0
        insetVertical: 0
        placeholderText: ""
        renderedText: textBlock.inlineStyleOverlayVisible ? inlineStyleRenderer.editorSurfaceHtml : ""
        selectByMouse: true
        selectedTextColor: LV.Theme.textPrimary
        selectionColor: LV.Theme.accent
        preferNativeInputHandling: true
        showRenderedOutput: textBlock.inlineStyleOverlayVisible
        showScrollBar: false
        text: textBlock.authoritativePlainText
        textColor: textBlock.editorTextColor
        wrapMode: TextEdit.Wrap

        onFocusedChanged: {
            if (focused)
                textBlock.activated()
        }
        onCursorPositionChanged: {
            if (focused)
                textBlock.activated()
        }
        onTextEdited: function (_surfaceText) {
            const previousSourceText = textBlock._hasLiveEditSnapshot
                    ? textBlock._liveEditSourceText
                    : textBlock.authoritativeSourceText()
            const previousPlainText = textBlock._hasLiveEditSnapshot
                    ? textBlock._liveEditPlainText
                    : textBlock.authoritativePlainText
            const nextPlainText = textBlock.currentEditorPlainText()
            if (previousPlainText === nextPlainText)
                return
            if (!inlineStyleRenderer || inlineStyleRenderer.applyPlainTextReplacementToSource === undefined)
                return
            const replacementDelta = textBlock.computePlainTextReplacementDelta(previousPlainText, nextPlainText)
            if (!replacementDelta.valid)
                return
            const replacementSourceStart = StructuredCursorSupport.sourceOffsetForInlineTaggedSelectionBoundary(
                        previousSourceText,
                        replacementDelta.start,
                        0)
            const replacementSourceEnd = StructuredCursorSupport.sourceOffsetForInlineTaggedSelectionBoundary(
                        previousSourceText,
                        replacementDelta.previousEnd,
                        0)
            const nextSourceText = String(inlineStyleRenderer.applyPlainTextReplacementToSource(
                                              previousSourceText,
                                              replacementSourceStart,
                                              replacementSourceEnd,
                                              replacementDelta.insertedText))
            if (nextSourceText === previousSourceText)
                return
            textBlock._liveEditSourceText = nextSourceText
            textBlock._liveEditPlainText = nextPlainText
            textBlock._hasLiveEditSnapshot = true
            textBlock.sourceMutationRequested(
                        nextSourceText,
                        {
                            "sourceOffset": StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                                                nextSourceText,
                                                Math.max(0, Number(blockEditor.cursorPosition) || 0),
                                                textBlock.sourceStart)
                        },
                        previousSourceText)
        }
    }

    onSourceTextChanged: syncLiveEditSnapshotFromHost()

    Component.onCompleted: syncLiveEditSnapshotFromHost()
}
