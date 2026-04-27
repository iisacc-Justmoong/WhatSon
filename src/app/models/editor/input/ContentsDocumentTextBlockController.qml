pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "../structure/ContentsLogicalLineLayoutSupport.js" as LogicalLineLayoutSupport
import "../structure/ContentsStructuredCursorSupport.js" as StructuredCursorSupport

Item {
    id: controller
    objectName: "contentsDocumentTextBlockController"

    property var textBlock: null
    property var blockEditor: null
    property var inlineStyleRenderer: null
    property bool hasLiveEditSnapshot: false
    property string liveEditPlainText: ""
    property string liveEditSourceText: ""

    readonly property bool sourceContainsInlineStyleTags: /<\s*\/?\s*(?:bold|b|strong|italic|i|em|underline|u|strikethrough|strike|s|del|highlight|mark)\b/i.test(
                                                              controller.authoritativeSourceText())
    readonly property string authoritativePlainText: StructuredCursorSupport.plainTextFromInlineTaggedSource(
                                                        controller.authoritativeSourceText())

    function authoritativeSourceText() {
        if (!controller.textBlock)
            return "";
        return StructuredCursorSupport.normalizedPlainText(controller.textBlock.sourceText);
    }

    function visiblePlainText() {
        return controller.currentEditorPlainText();
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText);
        return Math.max(0, normalizedLineText.length);
    }

    function currentEditorPlainText() {
        if (!controller.textBlock)
            return "";
        if (!controller.blockEditor)
            return controller.authoritativePlainText;
        if (controller.blockEditor.currentPlainText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(controller.blockEditor.currentPlainText());
        if (controller.blockEditor.getText === undefined)
            return controller.authoritativePlainText;
        const editorLength = controller.blockEditor.length !== undefined
                ? Math.max(0, Number(controller.blockEditor.length) || 0)
                : 0;
        return StructuredCursorSupport.normalizedPlainText(controller.blockEditor.getText(0, editorLength));
    }

    function nativeCompositionActive() {
        if (!controller.textBlock)
            return false;
        const composing = controller.textBlock.inputMethodComposing !== undefined
                ? !!controller.textBlock.inputMethodComposing
                : false;
        const preeditText = controller.textBlock.preeditText === undefined || controller.textBlock.preeditText === null
                ? ""
                : String(controller.textBlock.preeditText);
        return composing || preeditText.length > 0;
    }

    function syncLiveEditSnapshotFromHost() {
        if (!controller.textBlock)
            return;
        const hostSourceText = controller.authoritativeSourceText();
        const hostPlainText = controller.authoritativePlainText;
        if (controller.textBlock.focused
                && controller.hasLiveEditSnapshot
                && hostSourceText !== controller.liveEditSourceText
                && controller.currentEditorPlainText() !== hostPlainText) {
            return;
        }
        controller.liveEditSourceText = hostSourceText;
        controller.liveEditPlainText = hostPlainText;
        controller.hasLiveEditSnapshot = true;
    }

    function logicalLineLayoutEntries() {
        if (!controller.textBlock)
            return [];
        const plainTextValue = controller.currentEditorPlainText();
        const editorItem = controller.blockEditor && controller.blockEditor.editorItem ? controller.blockEditor.editorItem : null;
        const blockHeight = Math.max(
                    1,
                    Number(controller.textBlock.implicitHeight) || 0,
                    Number(controller.blockEditor ? controller.blockEditor.implicitHeight : 0) || 0,
                    Number(controller.blockEditor ? controller.blockEditor.height : 0) || 0);
        return LogicalLineLayoutSupport.buildEntries(
                    plainTextValue,
                    blockHeight,
                    editorItem,
                    controller.textBlock,
                    Math.round(LV.Theme.scaleMetric(12)));
    }

    function currentEditorLogicalLineNumber() {
        const plainTextValue = controller.currentEditorPlainText();
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        plainTextValue.length,
                        Number(controller.blockEditor && controller.blockEditor.cursorPosition !== undefined
                               ? controller.blockEditor.cursorPosition
                               : 0) || 0));
        let lineNumber = 1;
        for (let index = 0; index < cursorPosition; ++index) {
            if (plainTextValue.charAt(index) === "\n")
                lineNumber += 1;
        }
        return Math.max(1, lineNumber);
    }

    function currentCursorRowRect() {
        if (!controller.textBlock)
            return ({ "contentHeight": Math.round(LV.Theme.scaleMetric(12)), "contentY": 0 });
        const editorItem = controller.blockEditor && controller.blockEditor.editorItem ? controller.blockEditor.editorItem : null;
        const plainTextValue = controller.currentEditorPlainText();
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        plainTextValue.length,
                        Number(controller.blockEditor && controller.blockEditor.cursorPosition !== undefined
                               ? controller.blockEditor.cursorPosition
                               : 0) || 0));
        if (!editorItem || editorItem.positionToRectangle === undefined)
            return ({
                        "contentHeight": Math.max(
                                             1,
                                             Number(controller.blockEditor ? controller.blockEditor.inputContentHeight : 0)
                                             || Math.round(LV.Theme.scaleMetric(12))),
                        "contentY": 0
                    });
        const rect = editorItem.positionToRectangle(cursorPosition);
        const mappedPoint = editorItem.mapToItem !== undefined
                ? editorItem.mapToItem(controller.textBlock, 0, Number(rect.y) || 0)
                : ({ "x": 0, "y": Number(rect.y) || 0 });
        return {
            "contentHeight": Math.max(1, Number(rect.height) || Math.round(LV.Theme.scaleMetric(12))),
            "contentY": Math.max(0, Number(mappedPoint.y) || 0)
        };
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
                    Number(controller.blockEditor && controller.blockEditor.inputContentHeight !== undefined
                           ? controller.blockEditor.inputContentHeight
                           : 0)
                    || rowBottom);
        return rowBottom >= contentHeight - 1;
    }

    function computePlainTextReplacementDelta(previousText, nextText) {
        const previous = previousText === undefined || previousText === null ? "" : String(previousText);
        const next = nextText === undefined || nextText === null ? "" : String(nextText);
        if (previous === next) {
            return {
                "insertedText": "",
                "previousEnd": 0,
                "start": 0,
                "valid": false
            };
        }

        let prefixLength = 0;
        const prefixLimit = Math.min(previous.length, next.length);
        while (prefixLength < prefixLimit && previous.charAt(prefixLength) === next.charAt(prefixLength))
            ++prefixLength;

        let suffixLength = 0;
        const suffixLimit = Math.min(previous.length - prefixLength, next.length - prefixLength);
        while (suffixLength < suffixLimit
                && previous.charAt(previous.length - 1 - suffixLength) === next.charAt(next.length - 1 - suffixLength)) {
            ++suffixLength;
        }

        const previousEnd = previous.length - suffixLength;
        const nextEnd = next.length - suffixLength;
        return {
            "insertedText": next.slice(prefixLength, nextEnd),
            "previousEnd": previousEnd,
            "start": prefixLength,
            "valid": true
        };
    }

    function currentPlainTextCursorSourceOffset(sourceText) {
        if (!controller.textBlock)
            return 0;
        const normalizedSourceText = StructuredCursorSupport.normalizedPlainText(sourceText);
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        normalizedSourceText.length,
                        Number(controller.blockEditor && controller.blockEditor.cursorPosition !== undefined
                               ? controller.blockEditor.cursorPosition
                               : normalizedSourceText.length) || 0));
        return controller.textBlock.sourceStart + cursorPosition;
    }

    function commitPlainTextRawMutation(nextPlainText, previousSourceText) {
        if (!controller.textBlock)
            return false;
        const nextSourceText = StructuredCursorSupport.normalizedPlainText(nextPlainText);
        if (nextSourceText === previousSourceText)
            return false;
        controller.liveEditSourceText = nextSourceText;
        controller.liveEditPlainText = nextSourceText;
        controller.hasLiveEditSnapshot = true;
        controller.textBlock.sourceMutationRequested(
                    nextSourceText,
                    {
                        "reason": "text-edit",
                        "sourceOffset": controller.currentPlainTextCursorSourceOffset(nextSourceText)
                    },
                    previousSourceText);
        return true;
    }

    function focusEditor(cursorPosition) {
        if (!controller.textBlock || !controller.blockEditor)
            return;
        controller.blockEditor.forceActiveFocus();
        const numericCursorPosition = Number(cursorPosition);
        const targetCursorPosition = isFinite(numericCursorPosition)
                                   ? Math.max(
                                         0,
                                         Math.min(
                                             Math.floor(numericCursorPosition),
                                             Math.max(0, controller.blockEditor.length || 0)))
                                   : Math.max(0, controller.blockEditor.length || 0);
        if (controller.blockEditor.setCursorPositionPreservingNativeInput !== undefined)
            controller.blockEditor.setCursorPositionPreservingNativeInput(targetCursorPosition);
        else if (controller.blockEditor.cursorPosition !== undefined)
            controller.blockEditor.cursorPosition = targetCursorPosition;
        controller.textBlock.activated();
    }

    function restoreEditorSelection(selectionStart, selectionEnd, cursorPosition) {
        if (!controller.blockEditor || controller.blockEditor.restoreSelectionRange === undefined)
            return false;
        return !!controller.blockEditor.restoreSelectionRange(selectionStart, selectionEnd, cursorPosition);
    }

    function clearSelection(preserveFocusedEditor) {
        if (!controller.textBlock || !controller.blockEditor || controller.blockEditor.clearSelection === undefined)
            return false;
        if (preserveFocusedEditor === true && controller.textBlock.focused)
            return false;
        controller.blockEditor.clearSelection();
        return true;
    }

    function inlineFormatSelectionSnapshot() {
        if (!controller.blockEditor)
            return ({});
        if (controller.blockEditor.inlineFormatSelectionSnapshot !== undefined)
            return controller.blockEditor.inlineFormatSelectionSnapshot();
        if (controller.blockEditor.selectionSnapshot !== undefined)
            return controller.blockEditor.selectionSnapshot();
        return ({});
    }

    function applyFocusRequest(request) {
        if (!controller.textBlock)
            return false;
        const safeRequest = request && typeof request === "object" ? request : ({});
        const localCursorPosition = Number(safeRequest.localCursorPosition);
        const selectionStart = Number(safeRequest.selectionStart);
        const selectionEnd = Number(safeRequest.selectionEnd);
        const sourceOffset = Number(safeRequest.sourceOffset);
        if (isFinite(localCursorPosition) && !isFinite(sourceOffset)) {
            controller.focusEditor(localCursorPosition);
            if (isFinite(selectionStart) && isFinite(selectionEnd) && selectionEnd > selectionStart)
                controller.restoreEditorSelection(selectionStart, selectionEnd, localCursorPosition);
            return true;
        }
        if (!isFinite(sourceOffset))
            return false;
        if (sourceOffset < controller.textBlock.sourceStart || sourceOffset > controller.textBlock.sourceEnd)
            return false;

        const resolvedCursorPosition = Math.max(0, StructuredCursorSupport.plainCursorForInlineTaggedSourceOffset(
                                                    controller.authoritativeSourceText(),
                                                    Math.floor(sourceOffset - controller.textBlock.sourceStart)));
        controller.focusEditor(resolvedCursorPosition);
        if (isFinite(selectionStart) && isFinite(selectionEnd) && selectionEnd > selectionStart)
            controller.restoreEditorSelection(selectionStart, selectionEnd, resolvedCursorPosition);
        return true;
    }

    function shortcutInsertionSourceOffset() {
        if (!controller.textBlock || !controller.blockEditor)
            return 0;
        return Math.max(controller.textBlock.sourceStart, Math.min(
                            controller.textBlock.sourceEnd,
                            StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                                controller.authoritativeSourceText(),
                                Number(controller.blockEditor.cursorPosition) || 0,
                                controller.textBlock.sourceStart)));
    }

    function handleEditorTextEdited(_surfaceText) {
        if (!controller.textBlock)
            return;
        const previousSourceText = controller.hasLiveEditSnapshot
                ? controller.liveEditSourceText
                : controller.authoritativeSourceText();
        const previousPlainText = controller.hasLiveEditSnapshot
                ? controller.liveEditPlainText
                : controller.authoritativePlainText;
        const nextPlainText = controller.currentEditorPlainText();
        if (previousPlainText === nextPlainText)
            return;
        if (!controller.sourceContainsInlineStyleTags) {
            controller.commitPlainTextRawMutation(nextPlainText, previousSourceText);
            return;
        }
        if (!controller.inlineStyleRenderer
                || controller.inlineStyleRenderer.applyPlainTextReplacementToSource === undefined)
            return;
        const replacementDelta = controller.computePlainTextReplacementDelta(previousPlainText, nextPlainText);
        if (!replacementDelta.valid)
            return;
        const replacementSourceStart = StructuredCursorSupport.sourceOffsetForInlineTaggedSelectionBoundary(
                    previousSourceText,
                    replacementDelta.start,
                    0);
        const replacementSourceEnd = StructuredCursorSupport.sourceOffsetForInlineTaggedSelectionBoundary(
                    previousSourceText,
                    replacementDelta.previousEnd,
                    0);
        const nextSourceText = String(controller.inlineStyleRenderer.applyPlainTextReplacementToSource(
                                          previousSourceText,
                                          replacementSourceStart,
                                          replacementSourceEnd,
                                          replacementDelta.insertedText));
        if (nextSourceText === previousSourceText)
            return;
        controller.liveEditSourceText = nextSourceText;
        controller.liveEditPlainText = nextPlainText;
        controller.hasLiveEditSnapshot = true;
        controller.textBlock.sourceMutationRequested(
                    nextSourceText,
                    {
                        "reason": "text-edit",
                        "sourceOffset": StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                                            nextSourceText,
                                            Math.max(0, Number(controller.blockEditor.cursorPosition) || 0),
                                            controller.textBlock.sourceStart)
                    },
                    previousSourceText);
    }

    function handleEditorFocusedChanged() {
        if (controller.textBlock && controller.blockEditor && controller.blockEditor.focused)
            controller.textBlock.activated();
    }

    function handleEditorCursorPositionChanged() {
        if (controller.textBlock && controller.blockEditor && controller.blockEditor.focused)
            controller.textBlock.cursorInteraction();
    }

    Connections {
        function onSourceTextChanged() {
            controller.syncLiveEditSnapshotFromHost();
        }

        target: controller.textBlock
    }

    Connections {
        function onCursorPositionChanged() {
            controller.handleEditorCursorPositionChanged();
        }

        function onFocusedChanged() {
            controller.handleEditorFocusedChanged();
        }

        function onTextEdited(surfaceText) {
            controller.handleEditorTextEdited(surfaceText);
        }

        target: controller.blockEditor
    }

    Component.onCompleted: controller.syncLiveEditSnapshotFromHost()
}
