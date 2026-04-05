pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property var contentEditor: null
    property var editorSession: null
    property var textFormatRenderer: null
    property var textMetricsBridge: null

    function continuedListInsertion(replacementDelta, currentSourceText, nextPlainText) {
        const sourceText = currentSourceText === undefined || currentSourceText === null ? "" : String(currentSourceText);
        const plainText = nextPlainText === undefined || nextPlainText === null ? "" : String(nextPlainText);
        if (!replacementDelta || !replacementDelta.valid)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });

        const insertedText = replacementDelta.insertedText === undefined || replacementDelta.insertedText === null
                ? ""
                : String(replacementDelta.insertedText);
        if (!insertedText.endsWith("\n"))
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
        if ((insertedText.match(/\n/g) || []).length !== 1)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });

        const insertionStart = Math.max(0, Math.floor(Number(replacementDelta.start) || 0));
        const sourceInsertionStart = controller.sourceOffsetForLogicalOffset(insertionStart);
        const sourceLineStart = sourceInsertionStart > 0 ? sourceText.lastIndexOf("\n", sourceInsertionStart - 1) + 1 : 0;
        const sourceLinePrefix = sourceText.slice(sourceLineStart, sourceInsertionStart);
        const sourceLineEnd = sourceText.indexOf("\n", sourceInsertionStart);
        const sourceCurrentLine = sourceText.slice(sourceLineStart, sourceLineEnd >= 0 ? sourceLineEnd : sourceText.length);
        const logicalInsertionEnd = insertionStart + insertedText.length;
        if (logicalInsertionEnd <= 0 || logicalInsertionEnd > plainText.length || plainText.charAt(logicalInsertionEnd - 1) !== "\n")
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
        const logicalLineEnd = logicalInsertionEnd - 1;
        const logicalLineStart = logicalLineEnd > 0 ? plainText.lastIndexOf("\n", logicalLineEnd - 1) + 1 : 0;
        const logicalLine = plainText.slice(logicalLineStart, logicalLineEnd);
        const replacesWholeLogicalLine = replacementDelta.start === logicalLineStart
                && insertedText.slice(0, Math.max(0, insertedText.length - 1)) === logicalLine;
        if (replacementDelta.previousEnd !== replacementDelta.start && !replacesWholeLogicalLine)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
        const sourceLineForMarkerLookup = replacesWholeLogicalLine ? sourceCurrentLine : sourceLinePrefix;

        const unorderedMatch = /^([ \t]*)([-*+\u2022])(\s+)(.*)$/.exec(logicalLine);
        if (unorderedMatch) {
            if (String(unorderedMatch[4] || "").trim().length === 0)
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
            const sourceUnorderedMatch = /^([ \t]*)([-*+])(\s*)/.exec(sourceLineForMarkerLookup);
            const continuedIndent = sourceUnorderedMatch ? sourceUnorderedMatch[1] : unorderedMatch[1];
            const continuedMarker = sourceUnorderedMatch
                    ? sourceUnorderedMatch[2]
                    : (unorderedMatch[2] === "\u2022" ? "-" : unorderedMatch[2]);
            const continuedSeparator = sourceUnorderedMatch && String(sourceUnorderedMatch[3] || "").length > 0
                    ? sourceUnorderedMatch[3]
                    : unorderedMatch[3];
            const normalizedInsertedText = replacesWholeLogicalLine
                    ? unorderedMatch[1] + continuedMarker + continuedSeparator + unorderedMatch[4] + "\n"
                    : insertedText;
            const continuedText = normalizedInsertedText + continuedIndent + continuedMarker + continuedSeparator;
            return ({
                    "applied": true,
                    "cursorPosition": insertionStart + continuedText.length,
                    "insertedText": continuedText
                });
        }

        const orderedMatch = /^([ \t]*)(\d+)([.)])([ \t]*)(.*)$/.exec(logicalLine);
        if (orderedMatch) {
            const orderedSpacing = String(orderedMatch[4] || "");
            const orderedContent = String(orderedMatch[5] || "");
            if (orderedContent.trim().length === 0)
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
            if (orderedSpacing.length === 0 && /^[0-9]/.test(orderedContent))
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
            const sourceOrderedMatch = /^([ \t]*)(\d+)([.)])([ \t]*)/.exec(sourceLineForMarkerLookup);
            const currentNumber = Number(orderedMatch[2]);
            if (!isFinite(currentNumber))
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "" });
            const nextNumber = Math.max(1, Math.floor(currentNumber) + 1);
            const continuedIndent = sourceOrderedMatch ? sourceOrderedMatch[1] : orderedMatch[1];
            const continuedDelimiter = sourceOrderedMatch ? sourceOrderedMatch[3] : orderedMatch[3];
            const continuedSeparator = sourceOrderedMatch && String(sourceOrderedMatch[4] || "").length > 0
                    ? sourceOrderedMatch[4]
                    : (orderedSpacing.length > 0 ? orderedSpacing : " ");
            const continuedText = insertedText + continuedIndent + String(nextNumber) + continuedDelimiter + continuedSeparator;
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
        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        const continuedListInsertion = controller.continuedListInsertion(replacementDelta, currentSourceText, nextPlainText);
        const normalizedInsertedText = continuedListInsertion.applied
                ? continuedListInsertion.insertedText
                : replacementDelta.insertedText;
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
