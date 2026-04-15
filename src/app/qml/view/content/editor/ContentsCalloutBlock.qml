pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: calloutBlock

    required property var blockData

    signal activated()
    signal enterExitRequested(var blockData)
    signal textChanged(string text, int cursorPosition)

    readonly property int currentLogicalLineNumber: calloutBlock.currentEditorLogicalLineNumber()
    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property bool focused: calloutEditor.focused
    readonly property int contentStart: Math.max(0, Number(normalizedBlock.contentStart) || 0)
    readonly property int contentEnd: Math.max(contentStart, Number(normalizedBlock.contentEnd) || contentStart)
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || 0)
    readonly property string calloutText: normalizedBlock.text !== undefined ? String(normalizedBlock.text) : ""

    implicitHeight: calloutFrame.implicitHeight
    width: parent ? parent.width : implicitWidth

    function currentEditorLogicalLineNumber() {
        const textValue = StructuredCursorSupport.normalizedPlainText(calloutBlock.calloutText)
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

    function focusEditor(cursorPosition) {
        calloutEditor.forceActiveFocus()
        const numericCursorPosition = Number(cursorPosition)
        const targetCursorPosition = isFinite(numericCursorPosition)
                                   ? Math.max(0, Math.min(Math.floor(numericCursorPosition), Math.max(0, calloutEditor.length || 0)))
                                   : Math.max(0, calloutEditor.length || 0)
        if (calloutEditor.setCursorPositionPreservingInputMethod !== undefined)
            calloutEditor.setCursorPositionPreservingInputMethod(targetCursorPosition)
        else if (calloutEditor.cursorPosition !== undefined)
            calloutEditor.cursorPosition = targetCursorPosition
        calloutBlock.activated()
    }

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const localCursorPosition = Number(safeRequest.localCursorPosition)
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < calloutBlock.sourceStart || sourceOffset > calloutBlock.sourceEnd)
            return false
        if (isFinite(localCursorPosition)) {
            calloutBlock.focusEditor(localCursorPosition)
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
        color: "#262728"
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
                color: "#D9D9D9"
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
                shortcutKeyPressHandler: function (event) {
                    const noModifiers = (event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier | Qt.ShiftModifier)) === 0
                    if (!noModifiers)
                        return false
                    if (event.key !== Qt.Key_Return && event.key !== Qt.Key_Enter)
                        return false
                    const cursorPosition = Math.max(0, Number(calloutEditor.cursorPosition) || 0)
                    const textValue = String(calloutEditor.getText(0, Math.max(0, calloutEditor.length || 0)) || "")
                    if (cursorPosition === textValue.length && textValue.endsWith("\n")) {
                        event.accepted = true
                        calloutBlock.enterExitRequested(calloutBlock.blockData)
                        return true
                    }
                    return false
                }
                showRenderedOutput: false
                showScrollBar: false
                text: calloutBlock.calloutText
                textColor: "#FFFFFF"
                textFormat: TextEdit.PlainText
                wrapMode: TextEdit.Wrap

                onFocusedChanged: {
                    if (focused)
                        calloutBlock.activated()
                }
                onCursorPositionChanged: {
                    if (focused)
                        calloutBlock.activated()
                }
                onTextEdited: function (text) {
                    calloutBlock.textChanged(
                                String(text || ""),
                                Math.max(0, Number(calloutEditor.cursorPosition) || 0))
                }
            }
        }
    }
}
