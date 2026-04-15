pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import WhatSon.App.Internal 1.0
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: textBlock

    required property var blockData

    signal activated()
    signal adjacentAtomicBlockDeleteRequested(string side)
    signal boundaryNavigationRequested(string axis, string side)
    signal sourceMutationRequested(string nextBlockSourceText, var focusRequest)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property int currentLogicalLineNumber: textBlock.currentEditorLogicalLineNumber()
    readonly property bool focused: blockEditor.focused
    readonly property bool hasInlineStyleMarkup: /<\s*\/?\s*(bold|italic|underline|strikethrough|highlight)\b/i.test(textBlock.sourceText)
    property bool hasAdjacentAtomicBlockAfter: false
    property bool hasAdjacentAtomicBlockBefore: false
    property bool hasAdjacentBlockAfter: false
    property bool hasAdjacentBlockBefore: false
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || 0)
    readonly property string sourceText: normalizedBlock.sourceText !== undefined ? String(normalizedBlock.sourceText) : ""

    implicitHeight: blockEditor.implicitHeight
    width: parent ? parent.width : implicitWidth

    function authoritativePlainText() {
        return StructuredCursorSupport.plainTextFromInlineTaggedSource(textBlock.sourceText)
    }

    function currentEditorPlainText() {
        if (!blockEditor)
            return textBlock.authoritativePlainText()
        if (blockEditor.currentPlainText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(blockEditor.currentPlainText())
        if (blockEditor.getText === undefined)
            return textBlock.authoritativePlainText()
        const editorLength = blockEditor.length !== undefined
                ? Math.max(0, Number(blockEditor.length) || 0)
                : 0
        return StructuredCursorSupport.normalizedPlainText(blockEditor.getText(0, editorLength))
    }

    function logicalLineLayoutEntries() {
        const plainText = textBlock.currentEditorPlainText()
        const logicalLines = plainText.length > 0 ? plainText.split("\n") : [""]
        const editorItem = blockEditor && blockEditor.editorItem ? blockEditor.editorItem : null
        const blockHeight = Math.max(
                    1,
                    Number(textBlock.implicitHeight) || 0,
                    Number(blockEditor ? blockEditor.implicitHeight : 0) || 0,
                    Number(blockEditor ? blockEditor.height : 0) || 0)
        const entries = []
        let lineStartOffset = 0

        for (let index = 0; index < logicalLines.length; ++index) {
            const lineText = String(logicalLines[index] || "")
            const startRect = editorItem && editorItem.positionToRectangle !== undefined
                    ? editorItem.positionToRectangle(lineStartOffset)
                    : ({})
            const fallbackStartY = blockHeight * index / Math.max(1, logicalLines.length)
            const startY = Math.max(
                        0,
                        startRect && startRect.y !== undefined
                        ? Number(startRect.y) || 0
                        : fallbackStartY)
            const startHeight = Math.max(
                        1,
                        startRect && startRect.height !== undefined
                        ? Number(startRect.height) || 0
                        : 0)
            const nextLineStartOffset = index + 1 < logicalLines.length
                    ? Math.min(plainText.length, lineStartOffset + lineText.length + 1)
                    : plainText.length
            let endY = Math.max(startY + startHeight, blockHeight)
            if (index + 1 < logicalLines.length) {
                const nextRect = editorItem && editorItem.positionToRectangle !== undefined
                        ? editorItem.positionToRectangle(nextLineStartOffset)
                        : ({})
                const fallbackEndY = blockHeight * (index + 1) / Math.max(1, logicalLines.length)
                endY = Math.max(
                            startY + startHeight,
                            nextRect && nextRect.y !== undefined
                            ? Number(nextRect.y) || 0
                            : fallbackEndY)
            }
            entries.push({
                "contentHeight": Math.max(1, endY - startY),
                "contentY": startY
            })
            lineStartOffset = nextLineStartOffset
        }

        return entries
    }

    function currentEditorLogicalLineNumber() {
        const plainText = textBlock.currentEditorPlainText()
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        plainText.length,
                        Number(blockEditor && blockEditor.cursorPosition !== undefined ? blockEditor.cursorPosition : 0) || 0))
        let lineNumber = 1
        for (let index = 0; index < cursorPosition; ++index) {
            if (plainText.charAt(index) === "\n")
                lineNumber += 1
        }
        return Math.max(1, lineNumber)
    }

    function currentCursorRowRect() {
        const editorItem = blockEditor && blockEditor.editorItem ? blockEditor.editorItem : null
        const plainText = textBlock.currentEditorPlainText()
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        plainText.length,
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

    function normalizeInlineStyleTag(tagName) {
        const normalizedTagName = tagName === undefined || tagName === null ? "" : String(tagName).trim().toLowerCase()
        if (normalizedTagName === "bold" || normalizedTagName === "b" || normalizedTagName === "strong")
            return "bold"
        if (normalizedTagName === "italic" || normalizedTagName === "i" || normalizedTagName === "em")
            return "italic"
        if (normalizedTagName === "underline" || normalizedTagName === "u")
            return "underline"
        if (normalizedTagName === "strikethrough" || normalizedTagName === "strike" || normalizedTagName === "s" || normalizedTagName === "del")
            return "strikethrough"
        if (normalizedTagName === "highlight" || normalizedTagName === "mark")
            return "highlight"
        if (normalizedTagName === "plain" || normalizedTagName === "clear" || normalizedTagName === "none")
            return "plain"
        return ""
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
        if (blockEditor.setCursorPositionPreservingInputMethod !== undefined)
            blockEditor.setCursorPositionPreservingInputMethod(targetCursorPosition)
        else if (blockEditor.cursorPosition !== undefined)
            blockEditor.cursorPosition = targetCursorPosition
        textBlock.activated()
    }

    function restoreEditorSelection(selectionStart, selectionEnd, cursorPosition) {
        if (!blockEditor || blockEditor.restoreSelectionRange === undefined)
            return false
        return !!blockEditor.restoreSelectionRange(selectionStart, selectionEnd, cursorPosition)
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

        const resolvedCursorPosition = StructuredCursorSupport.plainCursorForInlineTaggedSourceOffset(
                    textBlock.sourceText,
                    Math.floor(sourceOffset - textBlock.sourceStart))
        textBlock.focusEditor(resolvedCursorPosition)
        if (isFinite(selectionStart) && isFinite(selectionEnd) && selectionEnd > selectionStart)
            textBlock.restoreEditorSelection(selectionStart, selectionEnd, resolvedCursorPosition)
        return true
    }

    function applyInlineFormatToSelection(tagName, explicitSelectionSnapshot) {
        const normalizedTagName = textBlock.normalizeInlineStyleTag(tagName)
        if (normalizedTagName.length === 0)
            return false
        if (!blockRenderer || blockRenderer.applyInlineStyleToLogicalSelectionSource === undefined)
            return false
        const selectionSnapshot = explicitSelectionSnapshot && typeof explicitSelectionSnapshot === "object"
                ? explicitSelectionSnapshot
                : textBlock.inlineFormatSelectionSnapshot()
        const plainText = textBlock.currentEditorPlainText()
        const selectionStart = Math.max(0, Math.min(plainText.length, Math.floor(Number(selectionSnapshot.selectionStart) || 0)))
        const selectionEnd = Math.max(selectionStart, Math.min(plainText.length, Math.floor(Number(selectionSnapshot.selectionEnd) || 0)))
        if (selectionEnd <= selectionStart)
            return false
        const nextBlockSourceText = blockRenderer.applyInlineStyleToLogicalSelectionSource(
                    textBlock.sourceText,
                    selectionStart,
                    selectionEnd,
                    normalizedTagName)
        if (nextBlockSourceText === undefined || nextBlockSourceText === null)
            return false
        const normalizedNextBlockSourceText = String(nextBlockSourceText)
        const cursorPosition = Math.max(
                    selectionStart,
                    Math.min(
                        selectionEnd,
                        Math.floor(Number(selectionSnapshot.cursorPosition) || selectionEnd)))
        textBlock.sourceMutationRequested(
                    normalizedNextBlockSourceText,
                    {
                        "localCursorPosition": cursorPosition,
                        "selectionEnd": selectionEnd,
                        "selectionStart": selectionStart,
                        "sourceOffset": StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                                            normalizedNextBlockSourceText,
                                            cursorPosition,
                                            textBlock.sourceStart)
                    })
        return true
    }

    function handleAtomicBlockBoundaryKeyPress(event) {
        if (!event)
            return false
        const key = Number(event.key)
        const deleteBackward = key === Qt.Key_Backspace
        const deleteForward = key === Qt.Key_Delete
        const moveBackward = key === Qt.Key_Left
        const moveForward = key === Qt.Key_Right
        const moveUp = key === Qt.Key_Up
        const moveDown = key === Qt.Key_Down
        if (!deleteBackward
                && !deleteForward
                && !moveBackward
                && !moveForward
                && !moveUp
                && !moveDown) {
            return false
        }
        const modifiers = Number(event.modifiers) || 0
        if ((modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier)) !== 0)
            return false
        const selectionStart = Math.max(0, Math.floor(Number(blockEditor.selectionStart) || 0))
        const selectionEnd = Math.max(selectionStart, Math.floor(Number(blockEditor.selectionEnd) || 0))
        if (selectionEnd > selectionStart)
            return false
        if (blockEditor.inputMethodComposing !== undefined && blockEditor.inputMethodComposing)
            return false
        const preeditText = blockEditor.preeditText !== undefined && blockEditor.preeditText !== null
                ? String(blockEditor.preeditText)
                : ""
        if (preeditText.length > 0)
            return false

        const cursorPosition = Math.max(0, Math.floor(Number(blockEditor.cursorPosition) || 0))
        const plainTextLength = Math.max(
                    0,
                    blockEditor.length !== undefined
                    ? Math.floor(Number(blockEditor.length) || 0)
                    : textBlock.currentEditorPlainText().length)
        if (deleteBackward && cursorPosition === 0 && textBlock.hasAdjacentAtomicBlockBefore) {
            textBlock.adjacentAtomicBlockDeleteRequested("before")
            event.accepted = true
            return true
        }
        if (deleteForward && cursorPosition === plainTextLength && textBlock.hasAdjacentAtomicBlockAfter) {
            textBlock.adjacentAtomicBlockDeleteRequested("after")
            event.accepted = true
            return true
        }
        if ((modifiers & Qt.ShiftModifier) !== 0)
            return false
        if (moveBackward && cursorPosition === 0 && textBlock.hasAdjacentBlockBefore) {
            textBlock.boundaryNavigationRequested("horizontal", "before")
            event.accepted = true
            return true
        }
        if (moveForward && cursorPosition === plainTextLength && textBlock.hasAdjacentBlockAfter) {
            textBlock.boundaryNavigationRequested("horizontal", "after")
            event.accepted = true
            return true
        }
        if (moveUp && textBlock.cursorOnFirstVisualRow() && textBlock.hasAdjacentBlockBefore) {
            textBlock.boundaryNavigationRequested("vertical", "before")
            event.accepted = true
            return true
        }
        if (moveDown && textBlock.cursorOnLastVisualRow() && textBlock.hasAdjacentBlockAfter) {
            textBlock.boundaryNavigationRequested("vertical", "after")
            event.accepted = true
            return true
        }
        return false
    }

    function shortcutInsertionSourceOffset() {
        return StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                    textBlock.sourceText,
                    Number(blockEditor.cursorPosition) || 0,
                    textBlock.sourceStart)
    }

    ContentsTextFormatRenderer {
        id: blockRenderer

        sourceText: textBlock.sourceText
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
        renderedText: textBlock.hasInlineStyleMarkup ? String(blockRenderer.editorSurfaceHtml || "") : ""
        selectByMouse: true
        selectedTextColor: LV.Theme.textPrimary
        selectionColor: LV.Theme.accent
        showRenderedOutput: textBlock.hasInlineStyleMarkup
        showScrollBar: false
        shortcutKeyPressHandler: function (event) {
            return textBlock.handleAtomicBlockBoundaryKeyPress(event)
        }
        text: textBlock.authoritativePlainText()
        textColor: LV.Theme.bodyColor
        textFormat: TextEdit.PlainText
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
            // Live typing mutates RAW block source spans only; the RichText surface is presentation input, not source authority.
            const previousPlainText = textBlock.authoritativePlainText()
            const nextPlainText = textBlock.currentEditorPlainText()
            if (previousPlainText === nextPlainText)
                return

            const replacementDelta = textBlock.computePlainTextReplacementDelta(previousPlainText, nextPlainText)
            if (!replacementDelta.valid)
                return

            const collapsedInsertion = replacementDelta.previousEnd === replacementDelta.start
            const replacementSourceStart = collapsedInsertion
                    ? StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                        textBlock.sourceText,
                        replacementDelta.start,
                        0)
                    : StructuredCursorSupport.sourceOffsetForInlineTaggedSelectionBoundary(
                        textBlock.sourceText,
                        replacementDelta.start,
                        0)
            const replacementSourceEnd = collapsedInsertion
                    ? replacementSourceStart
                    : StructuredCursorSupport.sourceOffsetForInlineTaggedSelectionBoundary(
                        textBlock.sourceText,
                        replacementDelta.previousEnd,
                        0)
            const nextBlockSourceText = blockRenderer.applyPlainTextReplacementToSource(
                        textBlock.sourceText,
                        replacementSourceStart,
                        replacementSourceEnd,
                        replacementDelta.insertedText)
            textBlock.sourceMutationRequested(
                        nextBlockSourceText,
                        {
                            "sourceOffset": StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                                                nextBlockSourceText,
                                                Number(blockEditor.cursorPosition) || 0,
                                                textBlock.sourceStart)
                        })
        }
    }
}
