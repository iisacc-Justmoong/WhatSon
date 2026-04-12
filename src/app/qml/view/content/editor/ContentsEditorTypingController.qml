pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property var contentEditor: null
    property var editorSession: null
    property var textFormatRenderer: null
    property var textMetricsBridge: null
    property var agendaBackend: null
    property var calloutBackend: null
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

    function spliceSourceText(sourceText, sourceStart, sourceEnd, replacementSourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        const safeLength = normalizedSourceText.length;
        const boundedStart = Math.max(0, Math.min(safeLength, Math.floor(Number(sourceStart) || 0)));
        const boundedEnd = Math.max(boundedStart, Math.min(safeLength, Math.floor(Number(sourceEnd) || 0)));
        const replacementText = controller.normalizePlainText(replacementSourceText);
        return normalizedSourceText.slice(0, boundedStart)
                + replacementText
                + normalizedSourceText.slice(boundedEnd);
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
        if (controller.view
                && controller.view.selectedNoteBodyText !== undefined
                && controller.view.selectedNoteBodyNoteId !== undefined
                && controller.view.selectedNoteId !== undefined
                && String(controller.view.selectedNoteBodyNoteId) === String(controller.view.selectedNoteId))
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

    function breakShortcutInsertion(previousPlainText, replacementStart, replacementEnd, insertedText) {
        const previousText = controller.normalizePlainText(previousPlainText);
        const safeStart = Math.max(0, Math.min(previousText.length, Math.floor(Number(replacementStart) || 0)));
        const safeEnd = Math.max(safeStart, Math.min(previousText.length, Math.floor(Number(replacementEnd) || 0)));
        const normalizedInsertedText = controller.normalizePlainText(insertedText);
        if (normalizedInsertedText.indexOf("\n") >= 0)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });

        const candidateText = previousText.slice(0, safeStart)
                + normalizedInsertedText
                + previousText.slice(safeEnd);
        const candidateCursor = safeStart + normalizedInsertedText.length;
        const candidateLineAnchor = Math.max(0, candidateCursor - 1);
        const candidateLineStart = candidateLineAnchor > 0
                ? candidateText.lastIndexOf("\n", candidateLineAnchor) + 1
                : 0;
        const candidateLineEndIndex = candidateText.indexOf("\n", candidateLineStart);
        const candidateLineEnd = candidateLineEndIndex >= 0 ? candidateLineEndIndex : candidateText.length;
        const candidateLineText = candidateText.slice(candidateLineStart, candidateLineEnd);
        if (candidateLineText !== "---")
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });

        const previousLineAnchor = Math.max(0, safeStart - 1);
        const previousLineStart = previousLineAnchor > 0
                ? previousText.lastIndexOf("\n", previousLineAnchor) + 1
                : 0;
        const previousLineEndIndex = previousText.indexOf("\n", previousLineStart);
        const previousLineEnd = previousLineEndIndex >= 0 ? previousLineEndIndex : previousText.length;
        return ({
                "applied": true,
                "cursorPosition": previousLineStart + 1,
                "insertedText": "</break>",
                "replacementEnd": previousLineEnd,
                "replacementStart": previousLineStart
            });
    }

    function currentEditorSelectionSnapshot() {
        if (!controller.contentEditor) {
            return ({
                    "cursorPosition": NaN,
                    "selectionEnd": NaN,
                    "selectionStart": NaN
                });
        }
        if (controller.contentEditor.selectionSnapshot !== undefined) {
            const snapshot = controller.contentEditor.selectionSnapshot();
            if (snapshot)
                return snapshot;
        }
        return ({
                "cursorPosition": controller.contentEditor.cursorPosition,
                "selectionEnd": controller.contentEditor.selectionEnd,
                "selectionStart": controller.contentEditor.selectionStart
            });
    }

    function currentRawEditorSelectionRange() {
        const snapshot = controller.currentEditorSelectionSnapshot();
        const cursorPosition = Number(snapshot && snapshot.cursorPosition !== undefined ? snapshot.cursorPosition : NaN);
        const selectionStart = Number(snapshot && snapshot.selectionStart !== undefined ? snapshot.selectionStart : NaN);
        const selectionEnd = Number(snapshot && snapshot.selectionEnd !== undefined ? snapshot.selectionEnd : NaN);
        const logicalLength = controller.authoritativeSourcePlainText().length;
        if (isFinite(selectionStart) && isFinite(selectionEnd)) {
            return ({
                    "start": Math.max(0, Math.min(logicalLength, Math.floor(Math.min(selectionStart, selectionEnd)))),
                    "end": Math.max(0, Math.min(logicalLength, Math.floor(Math.max(selectionStart, selectionEnd))))
                });
        }
        const boundedCursor = isFinite(cursorPosition)
                ? Math.max(0, Math.min(logicalLength, Math.floor(cursorPosition)))
                : logicalLength;
        return ({
                "start": boundedCursor,
                "end": boundedCursor
            });
    }

    function queueAgendaShortcutInsertion() {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        return controller.insertStructuredShortcutSourceAtCursor("agenda");
    }

    function queueCalloutShortcutInsertion() {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        return controller.insertStructuredShortcutSourceAtCursor("callout");
    }

    function queueBreakShortcutInsertion() {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        return controller.insertStructuredShortcutSourceAtCursor("break");
    }

    function completeStructuredShortcutInsertionSpec(shortcutKind) {
        const normalizedKind = shortcutKind === undefined || shortcutKind === null
                ? ""
                : String(shortcutKind).toLowerCase();
        if (normalizedKind === "agenda") {
            if (!controller.agendaBackend
                    || controller.agendaBackend.buildAgendaInsertionPayload === undefined) {
                return ({
                        "applied": false,
                        "cursorSourceOffsetFromInsertionStart": 0,
                        "insertionSourceText": ""
                    });
            }
            const insertionPayload = controller.agendaBackend.buildAgendaInsertionPayload(false, "");
            const insertionSourceText = insertionPayload && insertionPayload.insertionSourceText !== undefined
                    ? String(insertionPayload.insertionSourceText)
                    : "";
            const canonicalAgendaPattern = /^<agenda\b[^>]*>\s*<task\b[^>]*>[\s\S]*<\/task>\s*<\/agenda>$/i;
            if (!canonicalAgendaPattern.test(insertionSourceText)) {
                return ({
                        "applied": false,
                        "cursorSourceOffsetFromInsertionStart": 0,
                        "insertionSourceText": ""
                    });
            }
            return ({
                    "applied": true,
                    "cursorSourceOffsetFromInsertionStart": insertionPayload && insertionPayload.cursorSourceOffsetFromInsertionStart !== undefined
                                                           ? Math.max(0, Math.floor(Number(insertionPayload.cursorSourceOffsetFromInsertionStart) || 0))
                                                           : 0,
                    "insertionSourceText": insertionSourceText
                });
        }

        if (normalizedKind === "callout") {
            if (!controller.calloutBackend
                    || controller.calloutBackend.buildCalloutInsertionPayload === undefined) {
                return ({
                        "applied": false,
                        "cursorSourceOffsetFromInsertionStart": 0,
                        "insertionSourceText": ""
                    });
            }
            const insertionPayload = controller.calloutBackend.buildCalloutInsertionPayload("");
            const insertionSourceText = insertionPayload && insertionPayload.insertionSourceText !== undefined
                    ? String(insertionPayload.insertionSourceText)
                    : "";
            const canonicalCalloutPattern = /^<callout>[\s\S]*<\/callout>$/i;
            if (!canonicalCalloutPattern.test(insertionSourceText)) {
                return ({
                        "applied": false,
                        "cursorSourceOffsetFromInsertionStart": 0,
                        "insertionSourceText": ""
                    });
            }
            return ({
                    "applied": true,
                    "cursorSourceOffsetFromInsertionStart": insertionPayload && insertionPayload.cursorSourceOffsetFromInsertionStart !== undefined
                                                           ? Math.max(0, Math.floor(Number(insertionPayload.cursorSourceOffsetFromInsertionStart) || 0))
                                                           : 0,
                    "insertionSourceText": insertionSourceText
                });
        }

        if (normalizedKind === "break") {
            return ({
                    "applied": true,
                    "cursorSourceOffsetFromInsertionStart": "</break>".length,
                    "insertionSourceText": "</break>"
                });
        }

        return ({
                "applied": false,
                "cursorSourceOffsetFromInsertionStart": 0,
                "insertionSourceText": ""
            });
    }

    function insertStructuredShortcutSourceAtCursor(shortcutKind) {
        const insertionSpec = controller.completeStructuredShortcutInsertionSpec(shortcutKind);
        if (!insertionSpec.applied)
            return false;
        return controller.insertRawShortcutSourceAtCursor(
                    String(insertionSpec.insertionSourceText || ""),
                    Math.max(0, Math.floor(Number(insertionSpec.cursorSourceOffsetFromInsertionStart) || 0)));
    }

    function currentLogicalCursorOffsetForShortcutInsertion() {
        const plainTextLength = controller.authoritativeSourcePlainText().length;
        if (!controller.contentEditor || controller.contentEditor.cursorPosition === undefined)
            return plainTextLength;
        const numericCursor = Number(controller.contentEditor.cursorPosition);
        if (!isFinite(numericCursor))
            return plainTextLength;
        return Math.max(0, Math.min(plainTextLength, Math.floor(numericCursor)));
    }

    function findEnclosingStructuredShortcutBlock(sourceText, sourceOffset) {
        const normalizedSourceText = controller.normalizePlainText(sourceText);
        const safeSourceOffset = Math.max(
                    0,
                    Math.min(
                        normalizedSourceText.length,
                        Math.floor(Number(sourceOffset) || 0)));
        const blockPatterns = [
            {
                "regex": /<agenda\b[^>]*>[\s\S]*?<\/agenda>/gi,
                "type": "agenda"
            },
            {
                "regex": /<callout\b[^>]*>[\s\S]*?<\/callout>/gi,
                "type": "callout"
            }
        ];

        for (let patternIndex = 0; patternIndex < blockPatterns.length; ++patternIndex) {
            const patternSpec = blockPatterns[patternIndex];
            const regex = patternSpec.regex;
            regex.lastIndex = 0;

            let match = regex.exec(normalizedSourceText);
            while (match) {
                const blockToken = String(match[0] || "");
                const blockStart = Math.max(0, Number(match.index) || 0);
                const blockEnd = blockStart + blockToken.length;
                if (safeSourceOffset > blockStart && safeSourceOffset < blockEnd) {
                    return ({
                            "end": blockEnd,
                            "found": true,
                            "start": blockStart,
                            "type": patternSpec.type
                        });
                }

                if (regex.lastIndex === match.index)
                    regex.lastIndex = match.index + Math.max(1, blockToken.length);
                match = regex.exec(normalizedSourceText);
            }
        }

        return ({
                "end": safeSourceOffset,
                "found": false,
                "start": safeSourceOffset,
                "type": ""
            });
    }

    function resolveStructuredShortcutInsertionSourceOffset(sourceText, sourceOffset) {
        const normalizedSourceText = controller.normalizePlainText(sourceText);
        const safeSourceOffset = Math.max(
                    0,
                    Math.min(
                        normalizedSourceText.length,
                        Math.floor(Number(sourceOffset) || 0)));
        const enclosingBlock = controller.findEnclosingStructuredShortcutBlock(
                    normalizedSourceText,
                    safeSourceOffset);
        if (enclosingBlock.found)
            return Math.max(0, Math.min(normalizedSourceText.length, Number(enclosingBlock.end) || 0));
        return safeSourceOffset;
    }

    function insertRawShortcutSourceAtCursor(rawSourceText, cursorSourceOffsetFromInsertionStart) {
        if (!controller.view)
            return false;
        const normalizedRawSourceText = controller.normalizePlainText(rawSourceText);
        if (normalizedRawSourceText.length === 0)
            return false;
        controller.ensureLiveEditingStateReady();
        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        const logicalCursor = controller.currentLogicalCursorOffsetForShortcutInsertion();
        const rawSourceCursorOffset = Math.max(
                    0,
                    Math.min(
                        currentSourceText.length,
                        Math.floor(Number(controller.sourceOffsetForLogicalOffset(logicalCursor)) || 0)));
        const sourceCursorOffset = controller.resolveStructuredShortcutInsertionSourceOffset(
                    currentSourceText,
                    rawSourceCursorOffset);
        const prefixNewline = sourceCursorOffset > 0 && currentSourceText.charAt(sourceCursorOffset - 1) !== "\n"
                ? "\n"
                : "";
        const suffixNewline = sourceCursorOffset < currentSourceText.length && currentSourceText.charAt(sourceCursorOffset) !== "\n"
                ? "\n"
                : "";
        const insertionSourceText = prefixNewline + normalizedRawSourceText + suffixNewline;
        const cursorOffsetInsideRawSource = Math.max(
                    0,
                    Math.min(
                        normalizedRawSourceText.length,
                        Math.floor(Number(cursorSourceOffsetFromInsertionStart) || 0)));
        const cursorSourceOffset = sourceCursorOffset + prefixNewline.length + cursorOffsetInsideRawSource;
        const nextSourceText = controller.spliceSourceText(
                    currentSourceText,
                    sourceCursorOffset,
                    sourceCursorOffset,
                    insertionSourceText);

        if (controller.view.editorText !== nextSourceText)
            controller.view.editorText = nextSourceText;
        if (controller.view.commitDocumentPresentationRefresh !== undefined)
            controller.view.commitDocumentPresentationRefresh();
        else
            controller.synchronizeLiveEditingStateFromPresentation();

        controller.scheduleCursorPosition(controller.logicalOffsetForSourceOffset(cursorSourceOffset));

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

    function agendaTodoShortcutInsertion(previousPlainText, replacementStart, replacementEnd, insertedText) {
        if (!controller.agendaBackend
                || controller.agendaBackend.detectTodoShortcutReplacement === undefined) {
            return ({ "applied": false });
        }
        return controller.agendaBackend.detectTodoShortcutReplacement(
                    previousPlainText,
                    replacementStart,
                    replacementEnd,
                    insertedText);
    }

    function agendaTaskEnterInsertion(currentSourceText, sourceStart, sourceEnd, insertedText) {
        if (!controller.agendaBackend
                || controller.agendaBackend.detectAgendaTaskEnterReplacement === undefined) {
            return ({ "applied": false });
        }
        return controller.agendaBackend.detectAgendaTaskEnterReplacement(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    insertedText);
    }

    function calloutEnterInsertion(currentSourceText, sourceStart, sourceEnd, insertedText) {
        if (!controller.calloutBackend
                || controller.calloutBackend.detectCalloutEnterReplacement === undefined) {
            return ({ "applied": false });
        }
        return controller.calloutBackend.detectCalloutEnterReplacement(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    insertedText);
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

    function logicalOffsetForSourceOffset(sourceOffset) {
        controller.ensureLiveEditingStateReady();
        const safeSourceOffset = Math.max(0, Math.floor(Number(sourceOffset) || 0));
        const offsets = Array.isArray(controller.liveLogicalToSourceOffsets)
                ? controller.liveLogicalToSourceOffsets
                : [];
        if (offsets.length === 0)
            return safeSourceOffset;
        for (let logicalIndex = 0; logicalIndex < offsets.length; ++logicalIndex) {
            const mappedSourceOffset = Math.max(0, Math.floor(Number(offsets[logicalIndex]) || 0));
            if (mappedSourceOffset >= safeSourceOffset)
                return logicalIndex;
        }
        return Math.max(0, offsets.length - 1);
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

        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        const previousPlainText = controller.authoritativeSourcePlainText();
        const nextPlainText = controller.currentEditorPlainText();
        if (previousPlainText === nextPlainText) {
            return false;
        }

        const replacementDelta = controller.computePlainTextReplacementDelta(previousPlainText, nextPlainText);
        if (!replacementDelta.valid) {
            return false;
        }
        const continuedListInsertion = controller.continuedListInsertion(replacementDelta, currentSourceText, nextPlainText);
        let normalizedInsertedText = continuedListInsertion.applied
                ? continuedListInsertion.insertedText
                : replacementDelta.insertedText;
        let logicalReplacementStart = continuedListInsertion.applied && continuedListInsertion.replacementStart !== undefined
                && Number(continuedListInsertion.replacementStart) >= 0
                ? Math.floor(Number(continuedListInsertion.replacementStart))
                : replacementDelta.start;
        let logicalReplacementEnd = continuedListInsertion.applied && continuedListInsertion.replacementEnd !== undefined
                && Number(continuedListInsertion.replacementEnd) >= 0
                ? Math.floor(Number(continuedListInsertion.replacementEnd))
                : replacementDelta.previousEnd;

        let rawSourceReplacementText = "";
        let rawReplacementEnabled = false;
        let cursorLogicalOverride = NaN;
        let cursorSourceOffsetOverride = NaN;

        const agendaTodoShortcut = controller.agendaTodoShortcutInsertion(
                    previousPlainText,
                    logicalReplacementStart,
                    logicalReplacementEnd,
                    normalizedInsertedText);
        if (agendaTodoShortcut.applied) {
            logicalReplacementStart = Math.max(0, Math.floor(Number(agendaTodoShortcut.replacementStart) || 0));
            logicalReplacementEnd = Math.max(logicalReplacementStart, Math.floor(Number(agendaTodoShortcut.replacementEnd) || 0));
            rawSourceReplacementText = String(agendaTodoShortcut.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(0, Math.floor(Number(agendaTodoShortcut.cursorSourceOffsetFromReplacementStart) || 0));
        }

        const breakShortcut = controller.breakShortcutInsertion(
                    previousPlainText,
                    logicalReplacementStart,
                    logicalReplacementEnd,
                    normalizedInsertedText);
        if (!rawReplacementEnabled && breakShortcut.applied) {
            normalizedInsertedText = breakShortcut.insertedText;
            logicalReplacementStart = Math.max(0, Math.floor(Number(breakShortcut.replacementStart) || 0));
            logicalReplacementEnd = Math.max(logicalReplacementStart, Math.floor(Number(breakShortcut.replacementEnd) || 0));
            rawSourceReplacementText = normalizedInsertedText;
            rawReplacementEnabled = true;
            cursorLogicalOverride = Math.max(0, Math.floor(Number(breakShortcut.cursorPosition) || 0));
        }
        const sourceStart = controller.sourceOffsetForLogicalOffset(logicalReplacementStart);
        const sourceEnd = controller.sourceOffsetForLogicalOffset(logicalReplacementEnd);

        const agendaTaskEnter = controller.agendaTaskEnterInsertion(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    normalizedInsertedText);
        let replacementSourceStart = sourceStart;
        let replacementSourceEnd = sourceEnd;
        if (agendaTaskEnter.applied) {
            replacementSourceStart = Math.max(0, Math.floor(Number(agendaTaskEnter.replacementSourceStart) || 0));
            replacementSourceEnd = Math.max(replacementSourceStart, Math.floor(Number(agendaTaskEnter.replacementSourceEnd) || 0));
            rawSourceReplacementText = String(agendaTaskEnter.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(0, Math.floor(Number(agendaTaskEnter.cursorSourceOffsetFromReplacementStart) || 0));
            cursorLogicalOverride = NaN;
        }
        const calloutEnter = agendaTaskEnter.applied
                ? ({ "applied": false })
                : controller.calloutEnterInsertion(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    normalizedInsertedText);
        if (calloutEnter.applied) {
            replacementSourceStart = Math.max(0, Math.floor(Number(calloutEnter.replacementSourceStart) || 0));
            replacementSourceEnd = Math.max(replacementSourceStart, Math.floor(Number(calloutEnter.replacementSourceEnd) || 0));
            rawSourceReplacementText = String(calloutEnter.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(0, Math.floor(Number(calloutEnter.cursorSourceOffsetFromReplacementStart) || 0));
            cursorLogicalOverride = NaN;
        }

        let nextSourceText = "";
        if (rawReplacementEnabled) {
            nextSourceText = controller.spliceSourceText(
                        currentSourceText,
                        replacementSourceStart,
                        replacementSourceEnd,
                        rawSourceReplacementText);
        } else {
            nextSourceText = String(controller.textFormatRenderer.applyPlainTextReplacementToSource(
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
        }

        if (nextSourceText === currentSourceText)
            return false;

        if (controller.view.editorText !== nextSourceText)
            controller.view.editorText = nextSourceText;
        if (rawReplacementEnabled) {
            if (controller.view.commitDocumentPresentationRefresh !== undefined)
                controller.view.commitDocumentPresentationRefresh();
            else
                controller.synchronizeLiveEditingStateFromPresentation();
        } else {
            controller.adoptLiveStateIntoBridge(nextSourceText);
        }

        if (continuedListInsertion.applied && !rawReplacementEnabled)
            controller.scheduleCursorPosition(continuedListInsertion.cursorPosition);
        else if (rawReplacementEnabled && isFinite(cursorSourceOffsetOverride))
            controller.scheduleCursorPosition(controller.logicalOffsetForSourceOffset(replacementSourceStart + cursorSourceOffsetOverride));
        else if (isFinite(cursorLogicalOverride))
            controller.scheduleCursorPosition(cursorLogicalOverride);

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
