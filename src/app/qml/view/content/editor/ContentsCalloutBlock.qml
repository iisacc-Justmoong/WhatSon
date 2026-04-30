pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "../../../../models/editor/structure/ContentsLogicalLineLayoutSupport.js" as LogicalLineLayoutSupport
import "../../../../models/editor/structure/ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: calloutBlock

    required property var blockData
    property bool nativeTextInputPriority: false
    property bool paperPaletteEnabled: false
    property var tagManagementShortcutKeyPressHandler: null
    readonly property int editorMouseSelectionMode: Qt.platform.os === "ios"
                                                    ? TextEdit.SelectWords
                                                    : TextEdit.SelectCharacters

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

    function invokeDocumentTagManagementShortcut(event) {
        const handler = calloutBlock.tagManagementShortcutKeyPressHandler
        if (!handler || typeof handler !== "function")
            return false
        return !!handler(event)
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
                mouseSelectionMode: calloutBlock.editorMouseSelectionMode
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
                    if (calloutBlock.invokeDocumentTagManagementShortcut(event))
                        return true
                    return calloutBlockController.handleTagManagementKeyPress(event)
                }
                text: calloutBlock.calloutText
                textColor: calloutBlock.bodyTextColor
                wrapMode: TextEdit.Wrap

            }
        }
    }



Item {
    id: controller
    objectName: "contentsCalloutBlockController"

    property var calloutBlock: null
    property var calloutEditor: null
    property bool hasLiveTextSnapshot: false
    property string liveText: ""

    function currentEditorPlainText() {
        if (!controller.calloutBlock)
            return "";
        if (!controller.calloutEditor)
            return StructuredCursorSupport.normalizedPlainText(controller.calloutBlock.calloutText);
        if (controller.calloutEditor.currentPlainText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(controller.calloutEditor.currentPlainText());
        return StructuredCursorSupport.normalizedPlainText(controller.calloutBlock.calloutText);
    }

    function currentEditorLogicalLineNumber() {
        const textValue = controller.currentEditorPlainText();
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        textValue.length,
                        Number(controller.calloutEditor && controller.calloutEditor.cursorPosition !== undefined
                               ? controller.calloutEditor.cursorPosition
                               : 0) || 0));
        let lineNumber = 1;
        for (let index = 0; index < cursorPosition; ++index) {
            if (textValue.charAt(index) === "\n")
                lineNumber += 1;
        }
        return Math.max(1, lineNumber);
    }

    function currentCursorRowRect() {
        if (!controller.calloutBlock)
            return ({ "contentHeight": Math.round(LV.Theme.scaleMetric(12)), "contentY": 0 });
        const editorItem = controller.calloutEditor && controller.calloutEditor.editorItem ? controller.calloutEditor.editorItem : null;
        const textValue = controller.currentEditorPlainText();
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        textValue.length,
                        Number(controller.calloutEditor && controller.calloutEditor.cursorPosition !== undefined
                               ? controller.calloutEditor.cursorPosition
                               : 0) || 0));
        if (!editorItem || editorItem.positionToRectangle === undefined)
            return ({
                        "contentHeight": Math.max(
                                             1,
                                             Number(controller.calloutEditor ? controller.calloutEditor.inputContentHeight : 0)
                                             || Math.round(LV.Theme.scaleMetric(12))),
                        "contentY": 0
                    });
        const rect = editorItem.positionToRectangle(cursorPosition);
        const mappedPoint = editorItem.mapToItem !== undefined
                ? editorItem.mapToItem(controller.calloutBlock, 0, Number(rect.y) || 0)
                : ({ "x": 0, "y": Number(rect.y) || 0 });
        return {
            "contentHeight": Math.max(1, Number(rect.height) || Math.round(LV.Theme.scaleMetric(12))),
            "contentY": Math.max(0, Number(mappedPoint.y) || 0)
        };
    }

    function nativeCompositionActive() {
        if (!controller.calloutBlock)
            return false;
        const composing = controller.calloutBlock.inputMethodComposing !== undefined
                ? !!controller.calloutBlock.inputMethodComposing
                : false;
        const preeditText = controller.calloutBlock.preeditText === undefined || controller.calloutBlock.preeditText === null
                ? ""
                : String(controller.calloutBlock.preeditText);
        return composing || preeditText.length > 0;
    }

    function syncLiveTextFromHost() {
        if (!controller.calloutBlock)
            return;
        const hostText = StructuredCursorSupport.normalizedPlainText(controller.calloutBlock.calloutText);
        if (controller.calloutBlock.focused
                && controller.hasLiveTextSnapshot
                && hostText !== controller.liveText
                && controller.currentEditorPlainText() !== hostText) {
            return;
        }
        controller.liveText = hostText;
        controller.hasLiveTextSnapshot = true;
    }

    function logicalLineLayoutEntries() {
        if (!controller.calloutBlock)
            return [];
        const plainTextValue = controller.currentEditorPlainText();
        const editorItem = controller.calloutEditor && controller.calloutEditor.editorItem ? controller.calloutEditor.editorItem : null;
        const blockHeight = Math.max(
                    1,
                    Number(controller.calloutBlock.implicitHeight) || 0,
                    Number(controller.calloutEditor ? controller.calloutEditor.implicitHeight : 0) || 0,
                    Number(controller.calloutEditor ? controller.calloutEditor.height : 0) || 0);
        return LogicalLineLayoutSupport.buildEntries(
                    plainTextValue,
                    blockHeight,
                    editorItem,
                    controller.calloutBlock,
                    Math.round(LV.Theme.scaleMetric(12)));
    }

    function visiblePlainText() {
        return controller.currentEditorPlainText();
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText);
        return Math.max(0, normalizedLineText.length);
    }

    function cursorOnFirstVisualRow() {
        const rowRect = controller.currentCursorRowRect();
        return Math.max(0, Number(rowRect.contentY) || 0) <= 1;
    }

    function cursorOnLastVisualRow() {
        const rowRect = controller.currentCursorRowRect();
        const rowBottom = Math.max(
                    0,
                    (Number(rowRect.contentY) || 0) + (Number(rowRect.contentHeight) || 0));
        const contentHeight = Math.max(
                    1,
                    Number(controller.calloutEditor && controller.calloutEditor.inputContentHeight !== undefined
                           ? controller.calloutEditor.inputContentHeight
                           : 0)
                    || rowBottom);
        return rowBottom >= contentHeight - 1;
    }

    function focusEditor(cursorPosition) {
        if (!controller.calloutBlock || !controller.calloutEditor)
            return;
        controller.calloutEditor.forceActiveFocus();
        const numericCursorPosition = Number(cursorPosition);
        const targetCursorPosition = isFinite(numericCursorPosition)
                                   ? Math.max(
                                         0,
                                         Math.min(
                                             Math.floor(numericCursorPosition),
                                             Math.max(0, controller.calloutEditor.length || 0)))
                                   : Math.max(0, controller.calloutEditor.length || 0);
        if (controller.calloutEditor.setCursorPositionPreservingNativeInput !== undefined)
            controller.calloutEditor.setCursorPositionPreservingNativeInput(targetCursorPosition);
        else if (controller.calloutEditor.cursorPosition !== undefined)
            controller.calloutEditor.cursorPosition = targetCursorPosition;
        controller.calloutBlock.activated();
    }

    function clearSelection(preserveFocusedEditor) {
        if (!controller.calloutEditor || controller.calloutEditor.clearSelection === undefined)
            return false;
        if (preserveFocusedEditor === true && controller.calloutEditor.focused)
            return false;
        controller.calloutEditor.clearSelection();
        return true;
    }

    function applyFocusRequest(request) {
        if (!controller.calloutBlock)
            return false;
        const safeRequest = request && typeof request === "object" ? request : ({});
        const localCursorPosition = Number(safeRequest.localCursorPosition);
        const sourceOffset = Number(safeRequest.sourceOffset);
        const entryBoundary = safeRequest.entryBoundary === undefined || safeRequest.entryBoundary === null
                ? ""
                : String(safeRequest.entryBoundary).trim().toLowerCase();
        if (!isFinite(sourceOffset))
            return false;
        if (sourceOffset < controller.calloutBlock.sourceStart || sourceOffset > controller.calloutBlock.sourceEnd)
            return false;
        if (isFinite(localCursorPosition)) {
            controller.focusEditor(localCursorPosition);
            return true;
        }
        if (entryBoundary === "before" || sourceOffset <= controller.calloutBlock.contentStart) {
            controller.focusEditor(0);
            return true;
        }
        if (entryBoundary === "after" || sourceOffset >= controller.calloutBlock.contentEnd) {
            controller.focusEditor(controller.currentEditorPlainText().length);
            return true;
        }
        if (sourceOffset >= controller.calloutBlock.contentStart && sourceOffset <= controller.calloutBlock.contentEnd) {
            controller.focusEditor(
                        StructuredCursorSupport.plainCursorForSourceOffset(
                            controller.calloutBlock.calloutText,
                            sourceOffset - controller.calloutBlock.contentStart));
            return true;
        }
        controller.focusEditor();
        return true;
    }

    function shortcutInsertionSourceOffset() {
        if (!controller.calloutBlock)
            return 0;
        return controller.calloutBlock.sourceEnd;
    }

    function handleEditorFocusedChanged() {
        if (controller.calloutBlock && controller.calloutEditor && controller.calloutEditor.focused)
            controller.calloutBlock.activated();
    }

    function handleEditorCursorPositionChanged() {
        if (controller.calloutBlock && controller.calloutEditor && controller.calloutEditor.focused)
            controller.calloutBlock.cursorInteraction();
    }

    function handleTagManagementKeyPress(event) {
        if (!controller.calloutBlock || !controller.calloutEditor || !event)
            return false;
        const key = Number(event.key);
        const modifiers = Number(event.modifiers) || 0;
        const textModifiers = modifiers & ~Qt.KeypadModifier;
        if (key === Qt.Key_Backspace) {
            if (textModifiers !== Qt.NoModifier)
                return false;
            if (controller.nativeCompositionActive())
                return false;
            const plainText = controller.currentEditorPlainText();
            const cursorPosition = Math.max(0, Number(controller.calloutEditor.cursorPosition) || 0);
            const selectionStart = Math.max(0, Number(controller.calloutEditor.selectionStart) || 0);
            const selectionEnd = Math.max(selectionStart, Number(controller.calloutEditor.selectionEnd) || selectionStart);
            if (plainText.length !== 0 || cursorPosition !== 0 || selectionEnd > selectionStart)
                return false;
            controller.calloutBlock.blockDeletionRequested("backward");
            event.accepted = true;
            return true;
        }
        if (key !== Qt.Key_Return && key !== Qt.Key_Enter)
            return false;
        if (textModifiers === Qt.ShiftModifier)
            return false;
        if (textModifiers !== Qt.NoModifier)
            return false;
        if (controller.nativeCompositionActive())
            return false;

        const cursorPosition = Math.max(0, Number(controller.calloutEditor.cursorPosition) || 0);
        const sourceOffset = StructuredCursorSupport.sourceOffsetForPlainCursor(
                    controller.currentEditorPlainText(),
                    cursorPosition,
                    controller.calloutBlock.contentStart);
        controller.calloutBlock.enterExitRequested(
                    controller.calloutBlock.blockData,
                    Math.max(controller.calloutBlock.contentStart, sourceOffset));
        event.accepted = true;
        return true;
    }

    function handleEditorTextEdited(text) {
        if (!controller.calloutBlock || !controller.calloutEditor)
            return;
        const previousText = controller.hasLiveTextSnapshot
                ? controller.liveText
                : StructuredCursorSupport.normalizedPlainText(controller.calloutBlock.calloutText);
        const nextText = StructuredCursorSupport.normalizedPlainText(String(text || ""));
        if (previousText === nextText)
            return;
        controller.liveText = nextText;
        controller.hasLiveTextSnapshot = true;
        controller.calloutBlock.textChanged(
                    nextText,
                    Math.max(0, Number(controller.calloutEditor.cursorPosition) || 0),
                    previousText);
    }

    Connections {
        function onCalloutTextChanged() {
            controller.syncLiveTextFromHost();
        }

        target: controller.calloutBlock
    }

    Connections {
        function onCursorPositionChanged() {
            controller.handleEditorCursorPositionChanged();
        }

        function onFocusedChanged() {
            controller.handleEditorFocusedChanged();
        }

        function onTextEdited(text) {
            controller.handleEditorTextEdited(text);
        }

        target: controller.calloutEditor
    }

    Component.onCompleted: controller.syncLiveTextFromHost()
}
}
