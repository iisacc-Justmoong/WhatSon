pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property var contentEditor: null
    property var editorSession: null
    property var textFormatRenderer: null
    property var textMetricsBridge: null

    function continuedListInsertion(previousText, replacementDelta) {
        const previous = previousText === undefined || previousText === null ? "" : String(previousText);
        if (!replacementDelta || !replacementDelta.valid)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
        if (replacementDelta.insertedText !== "\n" || replacementDelta.previousEnd !== replacementDelta.start)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });

        const insertionStart = Math.max(0, Math.floor(Number(replacementDelta.start) || 0));
        const lineStart = insertionStart > 0 ? previous.lastIndexOf("\n", insertionStart - 1) + 1 : 0;
        const currentLine = previous.slice(lineStart, insertionStart);
        const unorderedMatch = /^([ \t]*)([-*+])(\s+)(.*)$/.exec(currentLine);
        if (unorderedMatch) {
            if (String(unorderedMatch[4] || "").trim().length === 0)
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
            const continuedText = "\n" + unorderedMatch[1] + unorderedMatch[2] + unorderedMatch[3];
            return ({
                    "applied": true,
                    "cursorPosition": insertionStart + continuedText.length,
                    "insertedText": continuedText
                });
        }

        const orderedMatch = /^([ \t]*)(\d+)([.)])(\s+)(.*)$/.exec(currentLine);
        if (orderedMatch) {
            if (String(orderedMatch[5] || "").trim().length === 0)
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
            const currentNumber = Number(orderedMatch[2]);
            if (!isFinite(currentNumber))
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
            const nextNumber = Math.max(1, Math.floor(currentNumber) + 1);
            const continuedText = "\n" + orderedMatch[1] + String(nextNumber) + orderedMatch[3] + orderedMatch[4];
            return ({
                    "applied": true,
                    "cursorPosition": insertionStart + continuedText.length,
                    "insertedText": continuedText
                });
        }

        return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
    }

    function normalizePlainText(text) {
        let normalizedText = text === undefined || text === null ? "" : String(text);
        normalizedText = normalizedText.replace(/\r\n/g, "\n");
        normalizedText = normalizedText.replace(/\r/g, "\n");
        normalizedText = normalizedText.replace(/\u2028/g, "\n");
        normalizedText = normalizedText.replace(/\u2029/g, "\n");
        normalizedText = normalizedText.replace(/\u00a0/g, " ");
        return normalizedText;
    }

    function authoritativeSourcePlainText() {
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalText !== undefined)
            return controller.normalizePlainText(controller.textMetricsBridge.logicalText);
        if (controller.view && controller.view.selectedNoteBodyText !== undefined)
            return controller.normalizePlainText(controller.view.selectedNoteBodyText);
        return "";
    }

    function currentEditorPlainText() {
        if (!controller.contentEditor || controller.contentEditor.getText === undefined)
            return controller.authoritativeSourcePlainText();
        const editorLength = controller.contentEditor.length !== undefined
                ? Math.max(0, Number(controller.contentEditor.length) || 0)
                : 0;
        return controller.normalizePlainText(controller.contentEditor.getText(0, editorLength));
    }

    function scheduleCursorPosition(position) {
        const targetPosition = Math.max(0, Math.floor(Number(position) || 0));
        Qt.callLater(function () {
            if (!controller.contentEditor)
                return;
            if (controller.contentEditor.cursorPosition !== undefined)
                controller.contentEditor.cursorPosition = targetPosition;
            if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.cursorPosition !== undefined)
                controller.contentEditor.editorItem.cursorPosition = targetPosition;
            if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.inputItem && controller.contentEditor.editorItem.inputItem.cursorPosition !== undefined)
                controller.contentEditor.editorItem.inputItem.cursorPosition = targetPosition;
        });
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

    function sourceOffsetForLogicalOffset(logicalOffset) {
        const safeOffset = Math.max(0, Math.floor(Number(logicalOffset) || 0));
        if (controller.textMetricsBridge && controller.textMetricsBridge.sourceOffsetForLogicalOffset !== undefined) {
            const mappedOffset = Number(controller.textMetricsBridge.sourceOffsetForLogicalOffset(safeOffset));
            if (isFinite(mappedOffset))
                return Math.max(0, Math.floor(mappedOffset));
        }
        const currentSourceText = controller.view && controller.view.editorText !== undefined && controller.view.editorText !== null
                ? String(controller.view.editorText)
                : "";
        return Math.max(0, Math.min(currentSourceText.length, safeOffset));
    }

    function handleEditorTextEdited() {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        if (!controller.contentEditor
                || !controller.textFormatRenderer
                || controller.textFormatRenderer.applyPlainTextReplacementToSource === undefined) {
            return false;
        }

        const previousPlainText = controller.authoritativeSourcePlainText();
        const nextPlainText = controller.currentEditorPlainText();
        if (previousPlainText === nextPlainText)
            return false;

        const replacementDelta = controller.computePlainTextReplacementDelta(previousPlainText, nextPlainText);
        if (!replacementDelta.valid)
            return false;
        const continuedListInsertion = controller.continuedListInsertion(previousPlainText, replacementDelta);
        const normalizedInsertedText = continuedListInsertion.applied
                ? continuedListInsertion.insertedText
                : replacementDelta.insertedText;

        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        const sourceStart = controller.sourceOffsetForLogicalOffset(replacementDelta.start);
        const sourceEnd = controller.sourceOffsetForLogicalOffset(replacementDelta.previousEnd);
        const nextSourceText = String(controller.textFormatRenderer.applyPlainTextReplacementToSource(
                                          currentSourceText,
                                          sourceStart,
                                          sourceEnd,
                                          normalizedInsertedText));
        if (controller.view.editorText !== nextSourceText)
            controller.view.editorText = nextSourceText;
        if (continuedListInsertion.applied)
            controller.scheduleCursorPosition(continuedListInsertion.cursorPosition);
        if (controller.view.syncingEditorTextFromModel)
            return true;
        if (controller.editorSession && controller.editorSession.markLocalEditorAuthority !== undefined)
            controller.editorSession.markLocalEditorAuthority();
        const saved = controller.view.persistEditorTextImmediately !== undefined
                ? !!controller.view.persistEditorTextImmediately(nextSourceText)
                : false;
        if (!saved && controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
            controller.editorSession.scheduleEditorPersistence();
        controller.view.editorTextEdited(nextSourceText);
        return true;
    }
}
