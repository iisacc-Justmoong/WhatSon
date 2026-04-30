pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import WhatSon.App.Internal 1.0
import "../../../../models/editor/structure/ContentsLogicalLineLayoutSupport.js" as LogicalLineLayoutSupport
import "../../../../models/editor/structure/ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: textBlock

    required property var blockData
    property bool nativeTextInputPriority: false
    property bool paperPaletteEnabled: false
    property var tagManagementShortcutKeyPressHandler: null

    signal activated()
    signal adjacentAtomicBlockDeleteRequested(string side)
    signal boundaryNavigationRequested(string axis, string side)
    signal blockDeletionRequested(string direction)
    signal cursorInteraction()
    signal inlineFormatRequested(string tagName, var selectionSnapshot)
    signal paragraphSplitRequested(int sourceOffset)
    signal sourceMutationRequested(string nextBlockSourceText, var focusRequest, string expectedPreviousSourceText)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property int currentLogicalLineNumber: documentTextBlockController.currentEditorLogicalLineNumber()
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
    readonly property bool sourceContainsInlineStyleTags: documentTextBlockController.sourceContainsInlineStyleTags
    readonly property bool inlineStyleOverlayVisible: textBlock.sourceContainsInlineStyleTags
                                                      && inlineStyleRenderer
                                                      && inlineStyleRenderer.htmlOverlayVisible !== undefined
                                                      && !!inlineStyleRenderer.htmlOverlayVisible
    readonly property string authoritativePlainText: documentTextBlockController.authoritativePlainText
    readonly property string preeditText: blockEditor.preeditText === undefined || blockEditor.preeditText === null
                                          ? ""
                                          : String(blockEditor.preeditText)
    readonly property int editorMouseSelectionMode: Qt.platform.os === "ios"
                                                    ? TextEdit.SelectWords
                                                    : TextEdit.SelectCharacters
    implicitHeight: blockEditor.implicitHeight
    width: parent ? parent.width : implicitWidth

    function authoritativeSourceText() {
        return documentTextBlockController.authoritativeSourceText()
    }

    function visiblePlainText() {
        return documentTextBlockController.visiblePlainText()
    }

    function representativeCharCount(lineText) {
        return documentTextBlockController.representativeCharCount(lineText)
    }

    function currentEditorPlainText() {
        return documentTextBlockController.currentEditorPlainText()
    }

    function nativeCompositionActive() {
        return documentTextBlockController.nativeCompositionActive()
    }

    function syncLiveEditSnapshotFromHost() {
        documentTextBlockController.syncLiveEditSnapshotFromHost()
    }

    function logicalLineLayoutEntries() {
        return documentTextBlockController.logicalLineLayoutEntries()
    }

    function currentEditorLogicalLineNumber() {
        return documentTextBlockController.currentEditorLogicalLineNumber()
    }

    function currentCursorRowRect() {
        return documentTextBlockController.currentCursorRowRect()
    }

    function cursorOnFirstVisualRow() {
        return documentTextBlockController.cursorOnFirstVisualRow()
    }

    function cursorOnLastVisualRow() {
        return documentTextBlockController.cursorOnLastVisualRow()
    }

    function computePlainTextReplacementDelta(previousText, nextText) {
        return documentTextBlockController.computePlainTextReplacementDelta(previousText, nextText)
    }

    function focusEditor(cursorPosition) {
        documentTextBlockController.focusEditor(cursorPosition)
    }

    function restoreEditorSelection(selectionStart, selectionEnd, cursorPosition) {
        return documentTextBlockController.restoreEditorSelection(selectionStart, selectionEnd, cursorPosition)
    }

    function clearSelection(preserveFocusedEditor) {
        return documentTextBlockController.clearSelection(preserveFocusedEditor)
    }

    function inlineFormatSelectionSnapshot() {
        return documentTextBlockController.inlineFormatSelectionSnapshot()
    }

    function applyFocusRequest(request) {
        return documentTextBlockController.applyFocusRequest(request)
    }

    function shortcutInsertionSourceOffset() {
        return documentTextBlockController.shortcutInsertionSourceOffset()
    }

    function invokeTagManagementShortcut(event) {
        const handler = textBlock.tagManagementShortcutKeyPressHandler
        if (handler && typeof handler === "function") {
            const handled = !!handler(event)
            if (handled || (event && event.accepted))
                return true
        }
        if (documentTextBlockController.handleBoundaryDeletionKeyPress(event))
            return true
        const tagName = blockEditor.inlineFormatShortcutTag !== undefined
                ? blockEditor.inlineFormatShortcutTag(event)
                : ""
        if (tagName.length === 0)
            return false
        textBlock.inlineFormatRequested(tagName, textBlock.inlineFormatSelectionSnapshot())
        if (event)
            event.accepted = true
        return true
    }

    ContentsInlineStyleOverlayRenderer {
        id: inlineStyleRenderer

        paperPaletteEnabled: textBlock.paperPaletteEnabled
        sourceText: textBlock.sourceContainsInlineStyleTags ? textBlock.authoritativeSourceText() : ""
    }

    ContentsPlainTextSourceMutator {
        id: plainTextSourceMutator
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
        inputMethodHints: Qt.ImhNone
        mouseSelectionMode: textBlock.editorMouseSelectionMode
        overwriteMode: false
        persistentSelection: true
        placeholderText: ""
        renderedText: textBlock.inlineStyleOverlayVisible ? inlineStyleRenderer.editorSurfaceHtml : ""
        selectByKeyboard: true
        selectByMouse: true
        selectedTextColor: LV.Theme.textPrimary
        selectionColor: LV.Theme.accent
        preferNativeInputHandling: true
        showRenderedOutput: textBlock.inlineStyleOverlayVisible
        showScrollBar: false
        tagManagementKeyPressHandler: function (event) {
            return textBlock.invokeTagManagementShortcut(event)
        }
        text: textBlock.authoritativePlainText
        textColor: textBlock.editorTextColor
        wrapMode: TextEdit.Wrap
    }



ContentsDocumentTextBlockController {
    id: documentTextBlockController
    objectName: "contentsDocumentTextBlockController"
    textBlock: textBlock
    blockEditor: blockEditor
    inlineStyleRenderer: inlineStyleRenderer
    plainTextSourceMutator: plainTextSourceMutator
}
}
