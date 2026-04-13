pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import WhatSon.App.Internal 1.0
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: textBlock

    required property var blockData

    signal activated()
    signal sourceMutationRequested(string nextBlockSourceText, var focusRequest)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
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

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const localCursorPosition = Number(safeRequest.localCursorPosition)
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (isFinite(localCursorPosition) && !isFinite(sourceOffset)) {
            textBlock.focusEditor(localCursorPosition)
            return true
        }
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < textBlock.sourceStart || sourceOffset > textBlock.sourceEnd)
            return false

        textBlock.focusEditor(
                    StructuredCursorSupport.plainCursorForInlineTaggedSourceOffset(
                        textBlock.sourceText,
                        Math.floor(sourceOffset - textBlock.sourceStart)))
        return true
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
        selectByMouse: true
        selectedTextColor: LV.Theme.textPrimary
        selectionColor: LV.Theme.accent
        showRenderedOutput: true
        showScrollBar: false
        text: blockRenderer.editorSurfaceHtml
        textColor: LV.Theme.bodyColor
        textFormat: TextEdit.RichText
        wrapMode: TextEdit.Wrap

        onFocusedChanged: {
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
