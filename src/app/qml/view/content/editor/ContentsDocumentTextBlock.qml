pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import WhatSon.App.Internal 1.0

FocusScope {
    id: textBlock

    required property var blockData
    property var focusRequest: ({})

    signal activated()
    signal sourceMutationRequested(string nextBlockSourceText, var focusRequest)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || 0)
    readonly property string sourceText: normalizedBlock.sourceText !== undefined ? String(normalizedBlock.sourceText) : ""

    implicitHeight: blockEditor.implicitHeight
    width: parent ? parent.width : implicitWidth

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < textBlock.sourceStart || sourceOffset > textBlock.sourceEnd)
            return false

        blockEditor.forceActiveFocus()
        const localOffset = Math.max(0, Math.min(textBlock.sourceEnd - textBlock.sourceStart, Math.floor(sourceOffset - textBlock.sourceStart)))
        if (blockEditor.setCursorPositionPreservingInputMethod !== undefined)
            blockEditor.setCursorPositionPreservingInputMethod(localOffset)
        else if (blockEditor.cursorPosition !== undefined)
            blockEditor.cursorPosition = localOffset
        return true
    }

    function shortcutInsertionSourceOffset() {
        const localCursorPosition = Math.max(0, Math.min(Number(blockEditor.length) || 0, Number(blockEditor.cursorPosition) || 0))
        if (blockEditor.getFormattedText !== undefined
                && blockRenderer
                && blockRenderer.normalizeEditorSurfaceTextToSource !== undefined) {
            const formattedPrefix = String(blockEditor.getFormattedText(0, localCursorPosition) || "")
            const prefixSourceText = String(blockRenderer.normalizeEditorSurfaceTextToSource(formattedPrefix) || "")
            return textBlock.sourceStart + prefixSourceText.length
        }
        return textBlock.sourceStart + localCursorPosition
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
        fieldMinHeight: Math.max(Math.round(LV.Theme.scaleMetric(28)), inputContentHeight)
        fontFamily: LV.Theme.fontBody
        fontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
        fontWeight: Font.Normal
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
        onTextEdited: function (surfaceText) {
            textBlock.sourceMutationRequested(
                        blockRenderer.normalizeEditorSurfaceTextToSource(String(surfaceText || "")),
                        {
                            "sourceOffset": textBlock.sourceStart + Math.max(0, Number(blockEditor.cursorPosition) || 0)
                        })
        }
    }

    onFocusRequestChanged: {
        Qt.callLater(function () {
            textBlock.applyFocusRequest(textBlock.focusRequest)
        })
    }
}
