pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property var contentEditor: null
    property var editorSession: null
    property var textFormatRenderer: null
    property var textMetricsBridge: null
    property string liveAuthoritativePlainText: ""
    property var liveLogicalLineStartOffsets: [0]
    property var liveLogicalToSourceOffsets: [0]
    property string liveSnapshotSourceText: ""

    function shouldDeferImmediatePersistence() {
        return !!(controller.view
                  && controller.view.deferImmediateEditorPersistence !== undefined
                  && controller.view.deferImmediateEditorPersistence
                  && controller.editorSession
                  && controller.editorSession.scheduleEditorPersistence !== undefined);
    }

    function markerOnlyListBreakInsertion(logicalLineStart, logicalLineEnd) {
        const replacementStart = Math.max(0, Math.floor(Number(logicalLineStart) || 0));
        const replacementEnd = Math.max(replacementStart, Math.floor(Number(logicalLineEnd) || 0));
        return ({
                "applied": true,
                "cursorPosition": replacementStart + 1,
                "insertedText": "\n",
                "replacementEnd": replacementEnd,
                "replacementStart": replacementStart
            });
    }

    function continuedListInsertion(replacementDelta, currentSourceText, nextPlainText) {
        const sourceText = currentSourceText === undefined || currentSourceText === null ? "" : String(currentSourceText);
        const plainText = nextPlainText === undefined || nextPlainText === null ? "" : String(nextPlainText);
        if (!replacementDelta || !replacementDelta.valid)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });

        const insertedText = replacementDelta.insertedText === undefined || replacementDelta.insertedText === null
                ? ""
                : String(replacementDelta.insertedText);
        if (!insertedText.endsWith("\n"))
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });
        if ((insertedText.match(/\n/g) || []).length !== 1)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });

        const insertionStart = Math.max(0, Math.floor(Number(replacementDelta.start) || 0));
        const sourceInsertionStart = controller.sourceOffsetForLogicalOffset(insertionStart);
        const sourceLineStart = sourceInsertionStart > 0 ? sourceText.lastIndexOf("\n", sourceInsertionStart - 1) + 1 : 0;
        const sourceLinePrefix = sourceText.slice(sourceLineStart, sourceInsertionStart);
        const sourceLineEnd = sourceText.indexOf("\n", sourceInsertionStart);
        const sourceCurrentLine = sourceText.slice(sourceLineStart, sourceLineEnd >= 0 ? sourceLineEnd : sourceText.length);
        const logicalInsertionEnd = insertionStart + insertedText.length;
        if (logicalInsertionEnd <= 0 || logicalInsertionEnd > plainText.length || plainText.charAt(logicalInsertionEnd - 1) !== "\n")
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });
        const logicalLineEnd = logicalInsertionEnd - 1;
        const logicalLineStart = logicalLineEnd > 0 ? plainText.lastIndexOf("\n", logicalLineEnd - 1) + 1 : 0;
        const logicalLine = plainText.slice(logicalLineStart, logicalLineEnd);
        const replacesWholeLogicalLine = replacementDelta.start === logicalLineStart
                && insertedText.slice(0, Math.max(0, insertedText.length - 1)) === logicalLine;
        if (replacementDelta.previousEnd !== replacementDelta.start && !replacesWholeLogicalLine)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });
        const sourceLineForMarkerLookup = replacesWholeLogicalLine ? sourceCurrentLine : sourceLinePrefix;

        const unorderedMatch = /^([ \t]*)([-*+\u2022])(\s+)(.*)$/.exec(logicalLine);
        if (unorderedMatch) {
            if (String(unorderedMatch[4] || "").trim().length === 0)
                return controller.markerOnlyListBreakInsertion(logicalLineStart, logicalLineEnd);
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
                return controller.markerOnlyListBreakInsertion(logicalLineStart, logicalLineEnd);
            if (orderedSpacing.length === 0 && /^[0-9]/.test(orderedContent))
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });
            const sourceOrderedMatch = /^([ \t]*)(\d+)([.)])([ \t]*)/.exec(sourceLineForMarkerLookup);
            const currentNumber = Number(orderedMatch[2]);
            if (!isFinite(currentNumber))
                return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });
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

        return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });
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

    function presentationSourceText() {
        if (controller.view && controller.view.documentPresentationSourceText !== undefined)
            return controller.view.documentPresentationSourceText === undefined || controller.view.documentPresentationSourceText === null
                    ? ""
                    : String(controller.view.documentPresentationSourceText);
        if (controller.view && controller.view.editorText !== undefined)
            return controller.view.editorText === undefined || controller.view.editorText === null
                    ? ""
                    : String(controller.view.editorText);
        return "";
    }

    function identityOffsetArray(sourceLength) {
        const safeLength = Math.max(0, Math.floor(Number(sourceLength) || 0));
        const offsets = new Array(safeLength + 1);
        for (let index = 0; index <= safeLength; ++index)
            offsets[index] = index;
        return offsets;
    }

    function computeLineStartOffsets(text) {
        const normalizedText = controller.normalizePlainText(text);
        const offsets = [0];
        for (let index = 0; index < normalizedText.length; ++index) {
            if (normalizedText.charAt(index) === "\n")
                offsets.push(index + 1);
        }
        return offsets;
    }

    function normalizeLineStartOffsets(rawOffsets, logicalText) {
        const normalizedText = controller.normalizePlainText(logicalText);
        const rawLength = rawOffsets && rawOffsets.length !== undefined
                ? Math.max(0, Math.floor(Number(rawOffsets.length) || 0))
                : 0;
        if (rawLength > 0) {
            const offsets = new Array(rawLength);
            for (let index = 0; index < rawLength; ++index) {
                const numericOffset = Number(rawOffsets[index]);
                offsets[index] = isFinite(numericOffset) ? Math.max(0, Math.floor(numericOffset)) : 0;
            }
            return offsets;
        }
        return controller.computeLineStartOffsets(normalizedText);
    }

    function normalizeOffsetArray(rawOffsets, logicalLength, fallbackSourceText) {
        const safeLogicalLength = Math.max(0, Math.floor(Number(logicalLength) || 0));
        const rawLength = rawOffsets && rawOffsets.length !== undefined
                ? Math.max(0, Math.floor(Number(rawOffsets.length) || 0))
                : 0;
        if (rawLength === safeLogicalLength + 1) {
            const offsets = new Array(rawLength);
            for (let index = 0; index < rawLength; ++index) {
                const numericOffset = Number(rawOffsets[index]);
                offsets[index] = isFinite(numericOffset) ? Math.max(0, Math.floor(numericOffset)) : 0;
            }
            return offsets;
        }
        if (controller.textMetricsBridge && controller.textMetricsBridge.sourceOffsetForLogicalOffset !== undefined) {
            const offsets = new Array(safeLogicalLength + 1);
            for (let index = 0; index <= safeLogicalLength; ++index) {
                const numericOffset = Number(controller.textMetricsBridge.sourceOffsetForLogicalOffset(index));
                offsets[index] = isFinite(numericOffset) ? Math.max(0, Math.floor(numericOffset)) : 0;
            }
            return offsets;
        }
        return controller.identityOffsetArray(safeLogicalLength);
    }

    function lastLineIndexForOffset(lineStartOffsets, offset) {
        const offsets = Array.isArray(lineStartOffsets) && lineStartOffsets.length > 0 ? lineStartOffsets : [0];
        const safeOffset = Math.max(0, Math.floor(Number(offset) || 0));
        let bestIndex = 0;
        for (let index = 0; index < offsets.length; ++index) {
            const lineOffset = Math.max(0, Number(offsets[index]) || 0);
            if (lineOffset > safeOffset)
                break;
            bestIndex = index;
        }
        return bestIndex;
    }

    function firstLineIndexAtOrAfterOffset(lineStartOffsets, offset, minimumIndex) {
        const offsets = Array.isArray(lineStartOffsets) && lineStartOffsets.length > 0 ? lineStartOffsets : [0];
        const safeOffset = Math.max(0, Math.floor(Number(offset) || 0));
        const safeMinimumIndex = Math.max(0, Math.floor(Number(minimumIndex) || 0));
        for (let index = safeMinimumIndex; index < offsets.length; ++index) {
            const lineOffset = Math.max(0, Number(offsets[index]) || 0);
            if (lineOffset >= safeOffset)
                return index;
        }
        return offsets.length;
    }

    function escapedSourceLengthForCharacter(ch) {
        const safeChar = ch === undefined || ch === null ? "" : String(ch);
        if (safeChar === "&")
            return 5;
        if (safeChar === "<" || safeChar === ">")
            return 4;
        if (safeChar === "\"")
            return 6;
        if (safeChar === "'")
            return 5;
        return 1;
    }

    function buildReplacementSourceOffsets(text) {
        const normalizedText = controller.normalizePlainText(text);
        const offsets = [0];
        let sourceOffset = 0;
        for (let index = 0; index < normalizedText.length; ++index) {
            sourceOffset += controller.escapedSourceLengthForCharacter(normalizedText.charAt(index));
            offsets.push(sourceOffset);
        }
        return offsets;
    }

    function buildReplacementLineStartOffsets(currentLineStart, text) {
        const baseStart = Math.max(0, Math.floor(Number(currentLineStart) || 0));
        const normalizedText = controller.normalizePlainText(text);
        const offsets = [baseStart];
        for (let index = 0; index < normalizedText.length; ++index) {
            if (normalizedText.charAt(index) === "\n")
                offsets.push(baseStart + index + 1);
        }
        return offsets;
    }

    function adoptLiveStateIntoBridge(sourceText) {
        if (!controller.textMetricsBridge
                || controller.textMetricsBridge.adoptIncrementalState === undefined) {
            return;
        }
        controller.textMetricsBridge.adoptIncrementalState(
                    sourceText,
                    controller.liveAuthoritativePlainText,
                    controller.liveLogicalLineStartOffsets,
                    controller.liveLogicalToSourceOffsets);
    }

    function synchronizeLiveEditingStateFromPresentation() {
        const snapshotSourceText = controller.presentationSourceText();
        const presentationLogicalText = controller.textMetricsBridge && controller.textMetricsBridge.logicalText !== undefined
                ? controller.normalizePlainText(controller.textMetricsBridge.logicalText)
                : controller.normalizePlainText(snapshotSourceText);
        let lineStartOffsets = [];
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalLineStartOffsets !== undefined)
            lineStartOffsets = controller.textMetricsBridge.logicalLineStartOffsets;
        let sourceOffsets = [];
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalToSourceOffsets !== undefined)
            sourceOffsets = controller.textMetricsBridge.logicalToSourceOffsets();
        controller.liveSnapshotSourceText = snapshotSourceText;
        controller.liveAuthoritativePlainText = presentationLogicalText;
        controller.liveLogicalLineStartOffsets = controller.normalizeLineStartOffsets(
                    lineStartOffsets,
                    presentationLogicalText);
        controller.liveLogicalToSourceOffsets = controller.normalizeOffsetArray(
                    sourceOffsets,
                    presentationLogicalText.length,
                    snapshotSourceText);
    }

    function ensureLiveEditingStateReady() {
        const snapshotSourceText = controller.presentationSourceText();
        const currentOffsets = Array.isArray(controller.liveLogicalToSourceOffsets)
                ? controller.liveLogicalToSourceOffsets
                : [];
        const currentLineStartOffsets = Array.isArray(controller.liveLogicalLineStartOffsets)
                ? controller.liveLogicalLineStartOffsets
                : [];
        if (controller.liveSnapshotSourceText !== snapshotSourceText
                || currentOffsets.length !== controller.liveAuthoritativePlainText.length + 1
                || currentLineStartOffsets.length === 0) {
            controller.synchronizeLiveEditingStateFromPresentation();
        }
    }

    function authoritativeSourcePlainText() {
        controller.ensureLiveEditingStateReady();
        if (controller.liveAuthoritativePlainText.length > 0
                || controller.liveSnapshotSourceText.length > 0) {
            return controller.liveAuthoritativePlainText;
        }
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

    function scheduleCursorPosition(position, attempt) {
        const targetPosition = Math.max(0, Math.floor(Number(position) || 0));
        const retryCount = Math.max(0, Math.floor(Number(attempt) || 0));
        Qt.callLater(function () {
            if (!controller.contentEditor)
                return;
            const activePreeditText = controller.contentEditor.preeditText !== undefined
                    ? String(controller.contentEditor.preeditText === undefined || controller.contentEditor.preeditText === null
                                 ? ""
                                 : controller.contentEditor.preeditText)
                    : "";
            const inputMethodBusy = (controller.contentEditor.inputMethodComposing !== undefined
                                     && controller.contentEditor.inputMethodComposing)
                    || activePreeditText.length > 0;
            if (inputMethodBusy && retryCount < 6) {
                controller.scheduleCursorPosition(targetPosition, retryCount + 1);
                return;
            }
            if (controller.contentEditor.setCursorPositionPreservingInputMethod !== undefined) {
                controller.contentEditor.setCursorPositionPreservingInputMethod(targetPosition);
                return;
            }
            if (controller.contentEditor.cursorPosition !== undefined)
                controller.contentEditor.cursorPosition = targetPosition;
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
        controller.ensureLiveEditingStateReady();
        const safeOffset = Math.max(0, Math.floor(Number(logicalOffset) || 0));
        const offsets = Array.isArray(controller.liveLogicalToSourceOffsets)
                ? controller.liveLogicalToSourceOffsets
                : [];
        if (offsets.length > 0) {
            const boundedOffset = Math.max(0, Math.min(offsets.length - 1, safeOffset));
            const mappedOffset = Number(offsets[boundedOffset]);
            if (isFinite(mappedOffset))
                return Math.max(0, Math.floor(mappedOffset));
        }
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

    function applyLiveEditingStateReplacement(logicalStart, logicalEnd, replacementText, sourceStart, sourceEnd) {
        controller.ensureLiveEditingStateReady();
        const previousLogicalText = controller.liveAuthoritativePlainText;
        const boundedLogicalStart = Math.max(0, Math.min(previousLogicalText.length, Math.floor(Number(logicalStart) || 0)));
        const boundedLogicalEnd = Math.max(boundedLogicalStart, Math.min(previousLogicalText.length, Math.floor(Number(logicalEnd) || 0)));
        const boundedSourceStart = Math.max(0, Math.floor(Number(sourceStart) || 0));
        const boundedSourceEnd = Math.max(boundedSourceStart, Math.floor(Number(sourceEnd) || 0));
        const insertedText = controller.normalizePlainText(replacementText);
        const insertedSourceOffsets = controller.buildReplacementSourceOffsets(insertedText);
        const previousOffsets = Array.isArray(controller.liveLogicalToSourceOffsets)
                ? controller.liveLogicalToSourceOffsets
                : controller.identityOffsetArray(previousLogicalText.length);
        const previousLogicalLength = previousLogicalText.length;
        const logicalInsertedLength = insertedText.length;
        const sourceRemovedLength = boundedSourceEnd - boundedSourceStart;
        const sourceInsertedLength = Math.max(0, Number(insertedSourceOffsets[insertedSourceOffsets.length - 1]) || 0);
        const sourceDelta = sourceInsertedLength - sourceRemovedLength;
        const nextLogicalText = previousLogicalText.slice(0, boundedLogicalStart)
                + insertedText
                + previousLogicalText.slice(boundedLogicalEnd);
        const nextLineStartOffsets = controller.computeLineStartOffsets(nextLogicalText);
        const nextOffsets = new Array(nextLogicalText.length + 1);

        for (let index = 0; index < boundedLogicalStart; ++index)
            nextOffsets[index] = Math.max(0, Number(previousOffsets[index]) || 0);
        nextOffsets[boundedLogicalStart] = boundedSourceStart;
        for (let index = 1; index <= logicalInsertedLength; ++index)
            nextOffsets[boundedLogicalStart + index] = boundedSourceStart + Math.max(0, Number(insertedSourceOffsets[index]) || 0);
        for (let previousIndex = boundedLogicalEnd; previousIndex <= previousLogicalLength; ++previousIndex) {
            const nextIndex = boundedLogicalStart + logicalInsertedLength + (previousIndex - boundedLogicalEnd);
            nextOffsets[nextIndex] = Math.max(0, Number(previousOffsets[previousIndex]) || 0) + sourceDelta;
        }

        controller.liveAuthoritativePlainText = nextLogicalText;
        controller.liveLogicalLineStartOffsets = nextLineStartOffsets;
        controller.liveLogicalToSourceOffsets = nextOffsets;
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
        const logicalReplacementStart = continuedListInsertion.applied && continuedListInsertion.replacementStart !== undefined
                && Number(continuedListInsertion.replacementStart) >= 0
                ? Math.floor(Number(continuedListInsertion.replacementStart))
                : replacementDelta.start;
        const logicalReplacementEnd = continuedListInsertion.applied && continuedListInsertion.replacementEnd !== undefined
                && Number(continuedListInsertion.replacementEnd) >= 0
                ? Math.floor(Number(continuedListInsertion.replacementEnd))
                : replacementDelta.previousEnd;
        const sourceStart = controller.sourceOffsetForLogicalOffset(logicalReplacementStart);
        const sourceEnd = controller.sourceOffsetForLogicalOffset(logicalReplacementEnd);
        const nextSourceText = String(controller.textFormatRenderer.applyPlainTextReplacementToSource(
                                          currentSourceText,
                                          sourceStart,
                                          sourceEnd,
                                          normalizedInsertedText));
        controller.applyLiveEditingStateReplacement(
                    logicalReplacementStart,
                    logicalReplacementEnd,
                    normalizedInsertedText,
                    sourceStart,
                    sourceEnd);
        if (controller.view.editorText !== nextSourceText)
            controller.view.editorText = nextSourceText;
        controller.adoptLiveStateIntoBridge(nextSourceText);
        if (continuedListInsertion.applied)
            controller.scheduleCursorPosition(continuedListInsertion.cursorPosition);
        if (controller.editorSession && controller.editorSession.markLocalEditorAuthority !== undefined)
            controller.editorSession.markLocalEditorAuthority();
        if (controller.shouldDeferImmediatePersistence()) {
            controller.editorSession.scheduleEditorPersistence();
            controller.view.editorTextEdited(nextSourceText);
            return true;
        }
        const saved = controller.view.persistEditorTextImmediately !== undefined
                ? !!controller.view.persistEditorTextImmediately(nextSourceText)
                : false;
        if (!saved && controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
            controller.editorSession.scheduleEditorPersistence();
        controller.view.editorTextEdited(nextSourceText);
        return true;
    }
}
