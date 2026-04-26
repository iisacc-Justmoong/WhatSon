pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "ContentsLogicalLineLayoutSupport.js" as LogicalLineLayoutSupport
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: calloutBlock

    required property var blockData
    property bool nativeTextInputPriority: false
    property bool paperPaletteEnabled: false

    signal activated()
    signal boundaryNavigationRequested(string axis, string side)
    signal cursorInteraction()
    signal enterExitRequested(var blockData)
    signal textChanged(string text, int cursorPosition, string expectedPreviousText)

    readonly property int currentLogicalLineNumber: calloutBlock.currentEditorLogicalLineNumber()
    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property bool focused: calloutEditor.focused
    readonly property bool inputMethodComposing: calloutEditor.inputMethodComposing !== undefined
                                                       && calloutEditor.inputMethodComposing
    readonly property bool textEditable: true
    readonly property bool atomicBlock: false
    readonly property bool gutterCollapsed: false
    readonly property string minimapVisualKind: "text"
    readonly property int minimapRepresentativeCharCount: 0
    readonly property int contentStart: Math.max(0, Number(normalizedBlock.contentStart) || 0)
    readonly property int contentEnd: Math.max(contentStart, Number(normalizedBlock.contentEnd) || contentStart)
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || 0)
    readonly property string calloutText: normalizedBlock.text !== undefined ? String(normalizedBlock.text) : ""
    readonly property string preeditText: calloutEditor.preeditText === undefined || calloutEditor.preeditText === null
                                          ? ""
                                          : String(calloutEditor.preeditText)
    readonly property color frameColor: paperPaletteEnabled ? "#F7F3EA" : "#262728"
    readonly property color dividerColor: paperPaletteEnabled ? "#B7A58A" : "#D9D9D9"
    readonly property color bodyTextColor: paperPaletteEnabled ? "#111111" : "#FFFFFF"
    property bool _hasLiveTextSnapshot: false
    property string _liveText: ""

    implicitHeight: calloutFrame.implicitHeight
    width: parent ? parent.width : implicitWidth

    function currentEditorLogicalLineNumber() {
        const textValue = calloutBlock.currentEditorPlainText()
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        textValue.length,
                        Number(calloutEditor && calloutEditor.cursorPosition !== undefined ? calloutEditor.cursorPosition : 0) || 0))
        let lineNumber = 1
        for (let index = 0; index < cursorPosition; ++index) {
            if (textValue.charAt(index) === "\n")
                lineNumber += 1
        }
        return Math.max(1, lineNumber)
    }

    function currentCursorRowRect() {
        const editorItem = calloutEditor && calloutEditor.editorItem ? calloutEditor.editorItem : null
        const textValue = calloutBlock.currentEditorPlainText()
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        textValue.length,
                        Number(calloutEditor && calloutEditor.cursorPosition !== undefined ? calloutEditor.cursorPosition : 0) || 0))
        if (!editorItem || editorItem.positionToRectangle === undefined)
            return ({
                        "contentHeight": Math.max(1, Number(calloutEditor ? calloutEditor.inputContentHeight : 0) || Math.round(LV.Theme.scaleMetric(12))),
                        "contentY": 0
                    })
        const rect = editorItem.positionToRectangle(cursorPosition)
        const mappedPoint = editorItem.mapToItem !== undefined
                ? editorItem.mapToItem(calloutBlock, 0, Number(rect.y) || 0)
                : ({ "x": 0, "y": Number(rect.y) || 0 })
        return {
            "contentHeight": Math.max(1, Number(rect.height) || Math.round(LV.Theme.scaleMetric(12))),
            "contentY": Math.max(0, Number(mappedPoint.y) || 0)
        }
    }

    function currentEditorPlainText() {
        if (!calloutEditor)
            return StructuredCursorSupport.normalizedPlainText(calloutBlock.calloutText)
        if (calloutEditor.currentPlainText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(calloutEditor.currentPlainText())
        return StructuredCursorSupport.normalizedPlainText(calloutBlock.calloutText)
    }

    function nativeCompositionActive() {
        return calloutBlock.inputMethodComposing || calloutBlock.preeditText.length > 0
    }

    function syncLiveTextFromHost() {
        const hostText = StructuredCursorSupport.normalizedPlainText(calloutBlock.calloutText)
        if (calloutBlock.focused
                && calloutBlock._hasLiveTextSnapshot
                && hostText !== calloutBlock._liveText
                && calloutBlock.currentEditorPlainText() !== hostText) {
            return
        }
        calloutBlock._liveText = hostText
        calloutBlock._hasLiveTextSnapshot = true
    }

    function logicalLineLayoutEntries() {
        const plainTextValue = calloutBlock.currentEditorPlainText()
        const editorItem = calloutEditor && calloutEditor.editorItem ? calloutEditor.editorItem : null
        const blockHeight = Math.max(
                    1,
                    Number(calloutBlock.implicitHeight) || 0,
                    Number(calloutEditor ? calloutEditor.implicitHeight : 0) || 0,
                    Number(calloutEditor ? calloutEditor.height : 0) || 0)
        return LogicalLineLayoutSupport.buildEntries(
                    plainTextValue,
                    blockHeight,
                    editorItem,
                    calloutBlock,
                    Math.round(LV.Theme.scaleMetric(12)))
    }

    function visiblePlainText() {
        return calloutBlock.currentEditorPlainText()
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
        return Math.max(0, normalizedLineText.length)
    }

    function cursorOnFirstVisualRow() {
        const rowRect = calloutBlock.currentCursorRowRect()
        return Math.max(0, Number(rowRect.contentY) || 0) <= 1
    }

    function cursorOnLastVisualRow() {
        const rowRect = calloutBlock.currentCursorRowRect()
        const rowBottom = Math.max(
                    0,
                    (Number(rowRect.contentY) || 0) + (Number(rowRect.contentHeight) || 0))
        const contentHeight = Math.max(
                    1,
                    Number(calloutEditor && calloutEditor.inputContentHeight !== undefined
                           ? calloutEditor.inputContentHeight
                           : 0)
                    || rowBottom)
        return rowBottom >= contentHeight - 1
    }

    function focusEditor(cursorPosition) {
        calloutEditor.forceActiveFocus()
        const numericCursorPosition = Number(cursorPosition)
        const targetCursorPosition = isFinite(numericCursorPosition)
                                   ? Math.max(0, Math.min(Math.floor(numericCursorPosition), Math.max(0, calloutEditor.length || 0)))
                                   : Math.max(0, calloutEditor.length || 0)
        if (calloutEditor.setCursorPositionPreservingNativeInput !== undefined)
            calloutEditor.setCursorPositionPreservingNativeInput(targetCursorPosition)
        else if (calloutEditor.cursorPosition !== undefined)
            calloutEditor.cursorPosition = targetCursorPosition
        calloutBlock.activated()
    }

    function clearSelection(preserveFocusedEditor) {
        if (!calloutEditor || calloutEditor.clearSelection === undefined)
            return false
        if (preserveFocusedEditor === true && calloutEditor.focused)
            return false
        calloutEditor.clearSelection()
        return true
    }

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const localCursorPosition = Number(safeRequest.localCursorPosition)
        const sourceOffset = Number(safeRequest.sourceOffset)
        const entryBoundary = safeRequest.entryBoundary === undefined || safeRequest.entryBoundary === null
                ? ""
                : String(safeRequest.entryBoundary).trim().toLowerCase()
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < calloutBlock.sourceStart || sourceOffset > calloutBlock.sourceEnd)
            return false
        if (isFinite(localCursorPosition)) {
            calloutBlock.focusEditor(localCursorPosition)
            return true
        }
        if (entryBoundary === "before" || sourceOffset <= calloutBlock.contentStart) {
            calloutBlock.focusEditor(0)
            return true
        }
        if (entryBoundary === "after" || sourceOffset >= calloutBlock.contentEnd) {
            calloutBlock.focusEditor(calloutBlock.currentEditorPlainText().length)
            return true
        }
        if (sourceOffset >= calloutBlock.contentStart && sourceOffset <= calloutBlock.contentEnd) {
            calloutBlock.focusEditor(
                        StructuredCursorSupport.plainCursorForSourceOffset(
                            calloutBlock.calloutText,
                            sourceOffset - calloutBlock.contentStart))
            return true
        }
        calloutBlock.focusEditor()
        return true
    }

    function shortcutInsertionSourceOffset() {
        // Agenda/callout shortcuts must stay block-scoped here so new wrappers do not nest inside callout content.
        return calloutBlock.sourceEnd
    }

    Rectangle {
        id: calloutFrame

        anchors.left: parent.left
        anchors.right: parent.right
        color: calloutBlock.frameColor
        implicitHeight: Math.max(Math.round(LV.Theme.scaleMetric(22)), calloutEditor.implicitHeight + Math.round(LV.Theme.scaleMetric(8)))
        radius: 0

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Math.round(LV.Theme.scaleMetric(4))
            anchors.rightMargin: Math.round(LV.Theme.scaleMetric(4))
            anchors.topMargin: Math.round(LV.Theme.scaleMetric(4))
            anchors.bottomMargin: Math.round(LV.Theme.scaleMetric(4))
            spacing: Math.round(LV.Theme.scaleMetric(12))

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: calloutBlock.dividerColor
                radius: 1
            }

            ContentsInlineFormatEditor {
                id: calloutEditor

                Layout.fillWidth: true
                backgroundColor: "transparent"
                backgroundColorDisabled: "transparent"
                backgroundColorFocused: "transparent"
                backgroundColorHover: "transparent"
                backgroundColorPressed: "transparent"
                centeredTextHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
                cornerRadius: 0
                fieldMinHeight: Math.max(Math.round(LV.Theme.scaleMetric(18)), inputContentHeight)
                fontFamily: LV.Theme.fontBody
                fontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
                fontWeight: Font.Medium
                insetHorizontal: 0
                insetVertical: 0
                placeholderText: ""
                selectByMouse: true
                selectedTextColor: LV.Theme.textPrimary
                selectionColor: LV.Theme.accent
                preferNativeInputHandling: true
                showRenderedOutput: false
                showScrollBar: false
                text: calloutBlock.calloutText
                textColor: calloutBlock.bodyTextColor
                wrapMode: TextEdit.Wrap

                onFocusedChanged: {
                    if (focused)
                        calloutBlock.activated()
                }
                onCursorPositionChanged: {
                    if (focused)
                        calloutBlock.cursorInteraction()
                }
                onTextEdited: function (text) {
                    const previousText = calloutBlock._hasLiveTextSnapshot
                            ? calloutBlock._liveText
                            : StructuredCursorSupport.normalizedPlainText(calloutBlock.calloutText)
                    const nextText = StructuredCursorSupport.normalizedPlainText(String(text || ""))
                    if (previousText === nextText)
                        return
                    calloutBlock._liveText = nextText
                    calloutBlock._hasLiveTextSnapshot = true
                    calloutBlock.textChanged(
                                nextText,
                                Math.max(0, Number(calloutEditor.cursorPosition) || 0),
                                previousText)
                }
            }
        }
    }

    onCalloutTextChanged: syncLiveTextFromHost()

    Component.onCompleted: syncLiveTextFromHost()
}
