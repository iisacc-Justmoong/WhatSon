pragma ComponentBehavior: Bound

import QtQuick
import "ContentsEditorDebugTrace.js" as EditorTrace

QtObject {
    id: controller
    objectName: "contentsEditorTypingController"

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
    property bool pendingCursorPositionRequest: false
    property int pendingCursorPosition: 0

    function normalizePlainText(text) {
        let normalizedText = text === undefined || text === null ? "" : String(text);
        normalizedText = normalizedText.replace(/\r\n/g, "\n");
        normalizedText = normalizedText.replace(/\r/g, "\n");
        normalizedText = normalizedText.replace(/\u2028/g, "\n");
        normalizedText = normalizedText.replace(/\u2029/g, "\n");
        normalizedText = normalizedText.replace(/\uFFFC/g, "");
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

    function currentSourceTextSnapshot() {
        if (controller.view && controller.view.editorText !== undefined)
            return controller.view.editorText === undefined || controller.view.editorText === null
                    ? ""
                    : String(controller.view.editorText);
        return controller.presentationSourceText();
    }

    function presentationSnapshotStale() {
        return controller.currentSourceTextSnapshot() !== controller.presentationSourceText();
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
        controller.liveSnapshotSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
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
        EditorTrace.trace(
                    "typingController",
                    "synchronizeLiveEditingStateFromPresentation",
                    "source=" + EditorTrace.describeText(snapshotSourceText)
                    + " logical=" + EditorTrace.describeText(presentationLogicalText),
                    controller)
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
        const liveStateValid = currentOffsets.length === controller.liveAuthoritativePlainText.length + 1
                && currentLineStartOffsets.length > 0;
        EditorTrace.trace(
                    "typingController",
                    "ensureLiveEditingStateReady",
                    "snapshotChanged=" + (controller.liveSnapshotSourceText !== snapshotSourceText)
                    + " liveStateValid=" + liveStateValid
                    + " localAuthority=" + (controller.editorSession && controller.editorSession.localEditorAuthority !== undefined
                                            ? controller.editorSession.localEditorAuthority
                                            : false),
                    controller)
        if (controller.presentationSnapshotStale()
                && liveStateValid
                && controller.editorSession
                && controller.editorSession.localEditorAuthority !== undefined
                && controller.editorSession.localEditorAuthority) {
            return;
        }
        if (controller.liveSnapshotSourceText !== snapshotSourceText
                || !liveStateValid) {
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
        if (!controller.contentEditor)
            return controller.authoritativeSourcePlainText();
        if (controller.contentEditor.currentPlainText !== undefined)
            return controller.normalizePlainText(controller.contentEditor.currentPlainText());
        if (controller.contentEditor.getText === undefined)
            return controller.authoritativeSourcePlainText();
        const editorLength = controller.contentEditor.length !== undefined
                ? Math.max(0, Number(controller.contentEditor.length) || 0)
                : 0;
        return controller.normalizePlainText(controller.contentEditor.getText(0, editorLength));
    }

    function nativeCompositionActive() {
        if (!controller.contentEditor)
            return false;
        const activePreeditText = controller.contentEditor.preeditText !== undefined
                ? String(controller.contentEditor.preeditText === undefined || controller.contentEditor.preeditText === null
                             ? ""
                             : controller.contentEditor.preeditText)
                : "";
        return (controller.contentEditor.inputMethodComposing !== undefined
                && controller.contentEditor.inputMethodComposing)
                || activePreeditText.length > 0;
    }

    function applyEditorCursorPosition(position) {
        if (!controller.contentEditor || controller.nativeCompositionActive())
            return false;
        const targetPosition = Math.max(0, Math.floor(Number(position) || 0));
        if (controller.contentEditor.setCursorPositionPreservingNativeInput !== undefined)
            return !!controller.contentEditor.setCursorPositionPreservingNativeInput(targetPosition);
        if (controller.contentEditor.cursorPosition !== undefined) {
            controller.contentEditor.cursorPosition = targetPosition;
            return true;
        }
        return false;
    }

    function applyPendingCursorPositionIfInputSettled() {
        if (!controller.pendingCursorPositionRequest || controller.nativeCompositionActive())
            return false;
        const targetPosition = Math.max(0, Math.floor(Number(controller.pendingCursorPosition) || 0));
        controller.pendingCursorPositionRequest = false;
        return controller.applyEditorCursorPosition(targetPosition);
    }

    function scheduleCursorPosition(position) {
        const targetPosition = Math.max(0, Math.floor(Number(position) || 0));
        Qt.callLater(function () {
            if (!controller.contentEditor)
                return;
            if (controller.nativeCompositionActive()) {
                controller.pendingCursorPosition = targetPosition;
                controller.pendingCursorPositionRequest = true;
                return;
            }
            controller.pendingCursorPositionRequest = false;
            controller.applyEditorCursorPosition(targetPosition);
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

    function sourceTagRanges(sourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        const ranges = [];
        const tagPattern = /<!--[\s\S]*?-->|<\s*\/?\s*[A-Za-z_][A-Za-z0-9_.:-]*\b[^>]*?>/g;
        let match = tagPattern.exec(normalizedSourceText);
        while (match) {
            const token = String(match[0] || "");
            const start = Math.max(0, Number(match.index) || 0);
            const end = start + token.length;
            if (end > start) {
                ranges.push({
                        "end": end,
                        "start": start,
                        "token": token
                    });
            }
            if (tagPattern.lastIndex === match.index)
                tagPattern.lastIndex = match.index + Math.max(1, token.length);
            match = tagPattern.exec(normalizedSourceText);
        }
        return ranges;
    }

    function normalizedSourceTagName(tagToken) {
        const normalizedToken = tagToken === undefined || tagToken === null ? "" : String(tagToken);
        if (normalizedToken.length === 0)
            return "";
        const match = normalizedToken.match(/^<\s*\/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b/i);
        if (!match || match.length < 2)
            return "";
        return String(match[1] || "").trim().toLowerCase();
    }
    function isClosingSourceTagToken(tagToken) {
        const normalizedToken = tagToken === undefined || tagToken === null ? "" : String(tagToken);
        return /^<\s*\//.test(normalizedToken);
    }
    function isInlineStyleTagName(tagName) {
        const normalizedTagName = tagName === undefined || tagName === null ? "" : String(tagName).trim().toLowerCase();
        return normalizedTagName === "bold"
                || normalizedTagName === "b"
                || normalizedTagName === "strong"
                || normalizedTagName === "italic"
                || normalizedTagName === "i"
                || normalizedTagName === "em"
                || normalizedTagName === "underline"
                || normalizedTagName === "u"
                || normalizedTagName === "strikethrough"
                || normalizedTagName === "strike"
                || normalizedTagName === "s"
                || normalizedTagName === "del"
                || normalizedTagName === "highlight"
                || normalizedTagName === "mark";
    }
    function sourceTagRangeStartingAt(sourceText, sourceOffset) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        const boundedOffset = Math.max(
                    0,
                    Math.min(
                        normalizedSourceText.length,
                        Math.floor(Number(sourceOffset) || 0)));
        if (boundedOffset >= normalizedSourceText.length || normalizedSourceText.charAt(boundedOffset) !== "<")
            return null;
        const tagEnd = normalizedSourceText.indexOf(">", boundedOffset + 1);
        if (tagEnd <= boundedOffset)
            return null;
        return {
            "end": tagEnd + 1,
            "start": boundedOffset,
            "token": normalizedSourceText.slice(boundedOffset, tagEnd + 1)
        };
    }
    function advanceSourceOffsetPastClosingInlineStyleTags(sourceText, sourceOffset) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        let nextOffset = Math.max(
                    0,
                    Math.min(
                        normalizedSourceText.length,
                        Math.floor(Number(sourceOffset) || 0)));
        while (nextOffset < normalizedSourceText.length) {
            const tagRange = controller.sourceTagRangeStartingAt(normalizedSourceText, nextOffset);
            if (!tagRange)
                break;
            const normalizedTagName = controller.normalizedSourceTagName(tagRange.token);
            if (!controller.isClosingSourceTagToken(tagRange.token)
                    || !controller.isInlineStyleTagName(normalizedTagName)) {
                break;
            }
            nextOffset = Math.max(nextOffset, Number(tagRange.end) || nextOffset);
        }
        return nextOffset;
    }
    function sourceOffsetForCollapsedLogicalInsertion(sourceText, logicalOffset) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        const baseSourceOffset = controller.sourceOffsetForLogicalOffset(logicalOffset);
        return controller.advanceSourceOffsetPastClosingInlineStyleTags(
                    normalizedSourceText,
                    baseSourceOffset);
    }

    function sourceRangeTouchesNamedTag(sourceTagRanges, sourceStart, sourceEnd, normalizedTagName) {
        const ranges = Array.isArray(sourceTagRanges) ? sourceTagRanges : [];
        const targetTagName = normalizedTagName === undefined || normalizedTagName === null
                ? ""
                : String(normalizedTagName).trim().toLowerCase();
        if (targetTagName.length === 0)
            return false;
        const safeStart = Math.max(0, Math.floor(Number(sourceStart) || 0));
        const safeEnd = Math.max(safeStart, Math.floor(Number(sourceEnd) || 0));
        for (let index = 0; index < ranges.length; ++index) {
            const range = ranges[index];
            if (controller.normalizedSourceTagName(range.token) !== targetTagName)
                continue;
            const rangeStart = Math.max(0, Math.floor(Number(range.start) || 0));
            const rangeEnd = Math.max(rangeStart, Math.floor(Number(range.end) || 0));
            if (safeEnd > safeStart) {
                if (rangeEnd > safeStart && rangeStart < safeEnd)
                    return true;
                continue;
            }
            if (safeStart > rangeStart && safeStart < rangeEnd)
                return true;
        }
        return false;
    }

    function logicalRangeCollapsesToSingleSourceOffset(logicalStart, logicalEnd) {
        const safeLogicalStart = Math.max(0, Math.floor(Number(logicalStart) || 0));
        const safeLogicalEnd = Math.max(safeLogicalStart, Math.floor(Number(logicalEnd) || 0));
        if (safeLogicalEnd <= safeLogicalStart)
            return false;
        return controller.sourceOffsetForLogicalOffset(safeLogicalStart)
                === controller.sourceOffsetForLogicalOffset(safeLogicalEnd);
    }

    function restoreEditorSurfaceFromSourcePresentation() {
        if (controller.view && controller.view.restoreEditorSurfaceFromPresentation !== undefined) {
            controller.view.restoreEditorSurfaceFromPresentation();
        } else if (controller.view && controller.view.commitDocumentPresentationRefresh !== undefined) {
            controller.view.commitDocumentPresentationRefresh();
        } else {
            controller.synchronizeLiveEditingStateFromPresentation();
            return;
        }
        controller.synchronizeLiveEditingStateFromPresentation();
    }
    function resourceTagLossDetectedForMutation(currentSourceText, nextSourceText) {
        if (!controller.view || controller.view.resourceTagLossDetected === undefined)
            return false;
        return !!controller.view.resourceTagLossDetected(currentSourceText, nextSourceText);
    }

    function collapsedDeletionTagRange(sourceTagRanges, sourceOffset, deleteForward) {
        const ranges = Array.isArray(sourceTagRanges) ? sourceTagRanges : [];
        const safeOffset = Math.max(0, Math.floor(Number(sourceOffset) || 0));
        if (deleteForward) {
            for (let index = 0; index < ranges.length; ++index) {
                const range = ranges[index];
                const rangeStart = Math.max(0, Math.floor(Number(range.start) || 0));
                const rangeEnd = Math.max(rangeStart, Math.floor(Number(range.end) || 0));
                if (safeOffset >= rangeStart && safeOffset < rangeEnd)
                    return range;
            }
            return null;
        }
        for (let index = ranges.length - 1; index >= 0; --index) {
            const range = ranges[index];
            const rangeStart = Math.max(0, Math.floor(Number(range.start) || 0));
            const rangeEnd = Math.max(rangeStart, Math.floor(Number(range.end) || 0));
            if (safeOffset > rangeStart && safeOffset <= rangeEnd)
                return range;
        }
        return null;
    }

    function expandedDeletionSourceRange(sourceTagRanges, sourceStart, sourceEnd) {
        const ranges = Array.isArray(sourceTagRanges) ? sourceTagRanges : [];
        let expandedStart = Math.max(0, Math.floor(Number(sourceStart) || 0));
        let expandedEnd = Math.max(expandedStart, Math.floor(Number(sourceEnd) || 0));
        if (expandedEnd <= expandedStart) {
            return ({
                    "start": expandedStart,
                    "end": expandedEnd
                });
        }

        let changed = true;
        while (changed) {
            changed = false;
            for (let index = 0; index < ranges.length; ++index) {
                const range = ranges[index];
                const rangeStart = Math.max(0, Math.floor(Number(range.start) || 0));
                const rangeEnd = Math.max(rangeStart, Math.floor(Number(range.end) || 0));
                const intersects = rangeEnd > expandedStart && rangeStart < expandedEnd;
                if (!intersects)
                    continue;
                if (rangeStart < expandedStart) {
                    expandedStart = rangeStart;
                    changed = true;
                }
                if (rangeEnd > expandedEnd) {
                    expandedEnd = rangeEnd;
                    changed = true;
                }
            }
        }

        return ({
                "start": expandedStart,
                "end": expandedEnd
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
        return controller.insertRawSourceTextAtCursor(
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

    function insertRawSourceTextAtCursor(rawSourceText, cursorSourceOffsetFromInsertionStart) {
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
                        Math.floor(Number(controller.sourceOffsetForCollapsedLogicalInsertion(
                                             currentSourceText,
                                             logicalCursor)) || 0)));
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
        if (controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
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

    function commitExplicitSourceMutation(nextSourceText) {
        EditorTrace.trace(
                    "typingController",
                    "commitExplicitSourceMutation",
                    EditorTrace.describeText(nextSourceText),
                    controller)
        if (!controller.view)
            return false;
        if (controller.view.applyDocumentSourceMutation !== undefined)
            return controller.view.applyDocumentSourceMutation(nextSourceText);

        const normalizedNextSourceText = nextSourceText === undefined || nextSourceText === null
                ? ""
                : String(nextSourceText);
        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        if (normalizedNextSourceText === currentSourceText)
            return false;
        if (controller.resourceTagLossDetectedForMutation(currentSourceText, normalizedNextSourceText)) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
        }

        if (controller.view.editorText !== normalizedNextSourceText)
            controller.view.editorText = normalizedNextSourceText;
        if (controller.view.commitDocumentPresentationRefresh !== undefined)
            controller.view.commitDocumentPresentationRefresh();
        else
            controller.synchronizeLiveEditingStateFromPresentation();

        if (controller.editorSession && controller.editorSession.markLocalEditorAuthority !== undefined)
            controller.editorSession.markLocalEditorAuthority();
        if (controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
            controller.editorSession.scheduleEditorPersistence();
        controller.view.editorTextEdited(normalizedNextSourceText);
        return true;
    }

    function applyExplicitPlainTextLogicalReplacement(logicalReplacementStart, logicalReplacementEnd, insertedText) {
        EditorTrace.trace(
                    "typingController",
                    "applyExplicitPlainTextLogicalReplacement",
                    "logicalStart=" + logicalReplacementStart
                    + " logicalEnd=" + logicalReplacementEnd
                    + " inserted=" + EditorTrace.describeText(insertedText),
                    controller)
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
        const plainTextLength = previousPlainText.length;
        const boundedLogicalStart = Math.max(
                    0,
                    Math.min(
                        plainTextLength,
                        Math.floor(Number(logicalReplacementStart) || 0)));
        const boundedLogicalEnd = Math.max(
                    boundedLogicalStart,
                    Math.min(
                        plainTextLength,
                        Math.floor(Number(logicalReplacementEnd) || 0)));
        const normalizedRequestedText = controller.normalizePlainText(insertedText);
        const syntheticNextPlainText = previousPlainText.slice(0, boundedLogicalStart)
                + normalizedRequestedText
                + previousPlainText.slice(boundedLogicalEnd);
        let normalizedInsertedText = normalizedRequestedText;
        let resolvedLogicalReplacementStart = boundedLogicalStart;
        let resolvedLogicalReplacementEnd = boundedLogicalEnd;

        let rawSourceReplacementText = "";
        let rawReplacementEnabled = false;
        let cursorLogicalOverride = Math.max(
                    0,
                    Math.min(
                        syntheticNextPlainText.length,
                        resolvedLogicalReplacementStart + normalizedInsertedText.length));
        let cursorSourceOffsetOverride = NaN;

        const breakShortcut = controller.breakShortcutInsertion(
                    previousPlainText,
                    resolvedLogicalReplacementStart,
                    resolvedLogicalReplacementEnd,
                    normalizedInsertedText);
        if (!rawReplacementEnabled && breakShortcut.applied) {
            normalizedInsertedText = breakShortcut.insertedText;
            resolvedLogicalReplacementStart = Math.max(0, Math.floor(Number(breakShortcut.replacementStart) || 0));
            resolvedLogicalReplacementEnd = Math.max(
                        resolvedLogicalReplacementStart,
                        Math.floor(Number(breakShortcut.replacementEnd) || 0));
            rawSourceReplacementText = normalizedInsertedText;
            rawReplacementEnabled = true;
            cursorLogicalOverride = Math.max(0, Math.floor(Number(breakShortcut.cursorPosition) || 0));
        }

        const collapsedLogicalInsertion = resolvedLogicalReplacementEnd === resolvedLogicalReplacementStart;
        const sourceStart = collapsedLogicalInsertion
                ? controller.sourceOffsetForCollapsedLogicalInsertion(
                    currentSourceText,
                    resolvedLogicalReplacementStart)
                : controller.sourceOffsetForLogicalOffset(resolvedLogicalReplacementStart);
        const sourceEnd = collapsedLogicalInsertion
                ? sourceStart
                : controller.sourceOffsetForLogicalOffset(resolvedLogicalReplacementEnd);
        const currentSourceTagRanges = controller.sourceTagRanges(currentSourceText);
        const targetsVirtualSourceOnly = controller.logicalRangeCollapsesToSingleSourceOffset(
                    resolvedLogicalReplacementStart,
                    resolvedLogicalReplacementEnd);

        const agendaTaskEnter = controller.agendaTaskEnterInsertion(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    normalizedInsertedText);
        let replacementSourceStart = sourceStart;
        let replacementSourceEnd = sourceEnd;
        if (agendaTaskEnter.applied) {
            replacementSourceStart = Math.max(0, Math.floor(Number(agendaTaskEnter.replacementSourceStart) || 0));
            replacementSourceEnd = Math.max(
                        replacementSourceStart,
                        Math.floor(Number(agendaTaskEnter.replacementSourceEnd) || 0));
            rawSourceReplacementText = String(agendaTaskEnter.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(
                        0,
                        Math.floor(Number(agendaTaskEnter.cursorSourceOffsetFromReplacementStart) || 0));
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
            replacementSourceEnd = Math.max(
                        replacementSourceStart,
                        Math.floor(Number(calloutEnter.replacementSourceEnd) || 0));
            rawSourceReplacementText = String(calloutEnter.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(
                        0,
                        Math.floor(Number(calloutEnter.cursorSourceOffsetFromReplacementStart) || 0));
            cursorLogicalOverride = NaN;
        }

        const touchesResourceTag = controller.sourceRangeTouchesNamedTag(
                    currentSourceTagRanges,
                    replacementSourceStart,
                    replacementSourceEnd,
                    "resource");
        if (touchesResourceTag || targetsVirtualSourceOnly) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
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
        }
        if (nextSourceText === currentSourceText)
            return false;

        const committed = controller.commitExplicitSourceMutation(nextSourceText);
        if (!committed)
            return false;

        if (rawReplacementEnabled && isFinite(cursorSourceOffsetOverride))
            controller.scheduleCursorPosition(
                        controller.logicalOffsetForSourceOffset(replacementSourceStart + cursorSourceOffsetOverride));
        else if (isFinite(cursorLogicalOverride))
            controller.scheduleCursorPosition(cursorLogicalOverride);
        return true;
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
        EditorTrace.trace(
                    "typingController",
                    "handleEditorTextEdited",
                    "selectedNoteId=" + (controller.view ? String(controller.view.selectedNoteId || "") : "")
                    + " structured=" + (controller.view && controller.view.showStructuredDocumentFlow !== undefined
                                        ? controller.view.showStructuredDocumentFlow
                                        : false),
                    controller)
        if (!controller.view
                || !controller.view.hasSelectedNote
                || (controller.view.showStructuredDocumentFlow !== undefined
                    && controller.view.showStructuredDocumentFlow)
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        if (controller.view.resourceDropEditorSurfaceGuardActive !== undefined
                && controller.view.resourceDropEditorSurfaceGuardActive) {
            return false;
        }
        if ((controller.view.programmaticEditorSurfaceSyncActive !== undefined
             && controller.view.programmaticEditorSurfaceSyncActive)
                || (controller.editorSession
                    && controller.editorSession.syncingEditorTextFromModel !== undefined
                    && controller.editorSession.syncingEditorTextFromModel)) {
            return false;
        }
        const boundNoteId = controller.editorSession
                && controller.editorSession.editorBoundNoteId !== undefined
                && controller.editorSession.editorBoundNoteId !== null
                ? String(controller.editorSession.editorBoundNoteId).trim()
                : "";
        const selectedNoteId = controller.view.selectedNoteId !== undefined
                && controller.view.selectedNoteId !== null
                ? String(controller.view.selectedNoteId).trim()
                : "";
        if (boundNoteId.length > 0
                && selectedNoteId.length > 0
                && boundNoteId !== selectedNoteId) {
            return false;
        }
        const hasReadableEditorSurface = controller.contentEditor
                && (controller.contentEditor.currentPlainText !== undefined
                    || controller.contentEditor.getText !== undefined);
        if (!hasReadableEditorSurface) {
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
        let normalizedInsertedText = replacementDelta.insertedText;
        let logicalReplacementStart = replacementDelta.start;
        let logicalReplacementEnd = replacementDelta.previousEnd;

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
        const collapsedLogicalInsertion = logicalReplacementEnd === logicalReplacementStart;
        const sourceStart = collapsedLogicalInsertion
                ? controller.sourceOffsetForCollapsedLogicalInsertion(
                    currentSourceText,
                    logicalReplacementStart)
                : controller.sourceOffsetForLogicalOffset(logicalReplacementStart);
        const sourceEnd = collapsedLogicalInsertion
                ? sourceStart
                : controller.sourceOffsetForLogicalOffset(logicalReplacementEnd);
        const currentSourceTagRanges = controller.sourceTagRanges(currentSourceText);
        const targetsVirtualSourceOnly = controller.logicalRangeCollapsesToSingleSourceOffset(
                    logicalReplacementStart,
                    logicalReplacementEnd);

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

        const touchesResourceTag = controller.sourceRangeTouchesNamedTag(
                    currentSourceTagRanges,
                    replacementSourceStart,
                    replacementSourceEnd,
                    "resource");
        if (touchesResourceTag || targetsVirtualSourceOnly) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
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
        if (controller.resourceTagLossDetectedForMutation(currentSourceText, nextSourceText)) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
        }

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

        if (rawReplacementEnabled && isFinite(cursorSourceOffsetOverride))
            controller.scheduleCursorPosition(controller.logicalOffsetForSourceOffset(replacementSourceStart + cursorSourceOffsetOverride));
        else if (isFinite(cursorLogicalOverride))
            controller.scheduleCursorPosition(cursorLogicalOverride);

        if (controller.editorSession && controller.editorSession.markLocalEditorAuthority !== undefined)
            controller.editorSession.markLocalEditorAuthority();
        if (controller.editorSession && controller.editorSession.scheduleEditorPersistence !== undefined)
            controller.editorSession.scheduleEditorPersistence();
        controller.view.editorTextEdited(nextSourceText);
        return true;
    }

    Component.onCompleted: {
        EditorTrace.trace("typingController", "mount", "", controller)
    }

    Component.onDestruction: {
        EditorTrace.trace("typingController", "unmount", "", controller)
    }
}
