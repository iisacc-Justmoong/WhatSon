pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../../../../models/editor/input" as EditorInputModel

FocusScope {
    id: calloutBlock

    required property var blockData
    property bool nativeTextInputPriority: false
    property bool paperPaletteEnabled: false

    signal activated()
    signal boundaryNavigationRequested(string axis, string side)
    signal blockDeletionRequested(string direction)
    signal cursorInteraction()
    signal enterExitRequested(var blockData, int sourceOffset)
    signal textChanged(string text, int cursorPosition, string expectedPreviousText)

    readonly property int currentLogicalLineNumber: calloutBlockController.currentEditorLogicalLineNumber()
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
    implicitHeight: calloutFrame.implicitHeight
    width: parent ? parent.width : implicitWidth

    function currentEditorLogicalLineNumber() {
        return calloutBlockController.currentEditorLogicalLineNumber()
    }

    function currentCursorRowRect() {
        return calloutBlockController.currentCursorRowRect()
    }

    function currentEditorPlainText() {
        return calloutBlockController.currentEditorPlainText()
    }

    function nativeCompositionActive() {
        return calloutBlockController.nativeCompositionActive()
    }

    function syncLiveTextFromHost() {
        calloutBlockController.syncLiveTextFromHost()
    }

    function logicalLineLayoutEntries() {
        return calloutBlockController.logicalLineLayoutEntries()
    }

    function visiblePlainText() {
        return calloutBlockController.visiblePlainText()
    }

    function representativeCharCount(lineText) {
        return calloutBlockController.representativeCharCount(lineText)
    }

    function cursorOnFirstVisualRow() {
        return calloutBlockController.cursorOnFirstVisualRow()
    }

    function cursorOnLastVisualRow() {
        return calloutBlockController.cursorOnLastVisualRow()
    }

    function focusEditor(cursorPosition) {
        calloutBlockController.focusEditor(cursorPosition)
    }

    function clearSelection(preserveFocusedEditor) {
        return calloutBlockController.clearSelection(preserveFocusedEditor)
    }

    function applyFocusRequest(request) {
        return calloutBlockController.applyFocusRequest(request)
    }

    function shortcutInsertionSourceOffset() {
        return calloutBlockController.shortcutInsertionSourceOffset()
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
                inputMethodHints: Qt.ImhNone
                mouseSelectionMode: TextEdit.SelectCharacters
                overwriteMode: false
                persistentSelection: true
                placeholderText: ""
                selectByKeyboard: true
                selectByMouse: true
                selectedTextColor: LV.Theme.textPrimary
                selectionColor: LV.Theme.accent
                preferNativeInputHandling: true
                showRenderedOutput: false
                showScrollBar: false
                tagManagementKeyPressHandler: function (event) {
                    return calloutBlockController.handleTagManagementKeyPress(event)
                }
                text: calloutBlock.calloutText
                textColor: calloutBlock.bodyTextColor
                wrapMode: TextEdit.Wrap

            }
        }
    }

    EditorInputModel.ContentsCalloutBlockController {
        id: calloutBlockController

        calloutBlock: calloutBlock
        calloutEditor: calloutEditor
    }
}
