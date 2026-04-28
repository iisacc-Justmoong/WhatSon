function normalizeSourceText(sourceText) {
    let normalizedText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
    normalizedText = normalizedText.replace(/\r\n/g, "\n");
    normalizedText = normalizedText.replace(/\r/g, "\n");
    normalizedText = normalizedText.replace(/\u2028/g, "\n");
    normalizedText = normalizedText.replace(/\u2029/g, "\n");
    normalizedText = normalizedText.replace(/\uFFFC/g, "");
    normalizedText = normalizedText.replace(/\u00A0/g, " ");
    return normalizedText;
}

function escapeSourceLiteral(text) {
    let escapedText = normalizeSourceText(text);
    escapedText = escapedText.replace(/&/g, "&amp;");
    escapedText = escapedText.replace(/</g, "&lt;");
    escapedText = escapedText.replace(/>/g, "&gt;");
    escapedText = escapedText.replace(/"/g, "&quot;");
    escapedText = escapedText.replace(/'/g, "&#39;");
    return escapedText;
}

function notAppliedPayload(reason) {
    return {
        "applied": false,
        "reason": reason
    };
}

function boundedOffset(sourceText, offset) {
    const safeOffset = Math.floor(Number(offset) || 0);
    return Math.max(0, Math.min(normalizeSourceText(sourceText).length, safeOffset));
}

function spliceSourceText(sourceText, sourceStart, sourceEnd, replacementSourceText) {
    const normalizedText = normalizeSourceText(sourceText);
    const boundedStart = boundedOffset(normalizedText, sourceStart);
    const boundedEnd = Math.max(boundedStart, Math.min(normalizedText.length, Math.floor(Number(sourceEnd) || 0)));
    return normalizedText.slice(0, boundedStart)
            + normalizeSourceText(replacementSourceText)
            + normalizedText.slice(boundedEnd);
}

function todayIsoDate() {
    const currentDate = new Date();
    const year = String(currentDate.getFullYear());
    const monthNumber = currentDate.getMonth() + 1;
    const dayNumber = currentDate.getDate();
    const month = monthNumber < 10 ? "0" + String(monthNumber) : String(monthNumber);
    const day = dayNumber < 10 ? "0" + String(dayNumber) : String(dayNumber);
    return year + "-" + month + "-" + day;
}

function agendaInsertionSpec() {
    const taskBodyText = " ";
    const agendaOpenTag = "<agenda date=\"" + todayIsoDate() + "\"><task done=\"false\">";
    const agendaCloseTag = "</task></agenda>";
    return {
        "applied": true,
        "cursorSourceOffsetFromInsertionStart": agendaOpenTag.length,
        "insertionSourceText": agendaOpenTag + taskBodyText + agendaCloseTag,
        "tagKind": "agenda"
    };
}

function calloutInsertionSpec() {
    const calloutOpenTag = "<callout>";
    const calloutCloseTag = "</callout>";
    return {
        "applied": true,
        "cursorSourceOffsetFromInsertionStart": calloutOpenTag.length,
        "insertionSourceText": calloutOpenTag + " " + calloutCloseTag,
        "tagKind": "callout"
    };
}

function breakInsertionSpec() {
    const sourceText = "</break>";
    return {
        "applied": true,
        "cursorSourceOffsetFromInsertionStart": sourceText.length,
        "insertionSourceText": sourceText,
        "tagKind": "break"
    };
}

function structuredShortcutInsertionSpec(shortcutKind) {
    const normalizedShortcutKind = String(shortcutKind || "").trim().toLowerCase();
    if (normalizedShortcutKind === "agenda")
        return agendaInsertionSpec();
    if (normalizedShortcutKind === "callout")
        return calloutInsertionSpec();
    if (normalizedShortcutKind === "break")
        return breakInsertionSpec();
    return notAppliedPayload("unsupported-tag-kind");
}

function sourceRangeOverlapsStructuredBlock(sourceText, selectionStart, selectionEnd) {
    const pattern = /<(?:agenda|callout)\b[^>]*>[\s\S]*?<\/(?:agenda|callout)>/ig;
    let match = null;
    while ((match = pattern.exec(sourceText)) !== null) {
        const blockStart = Math.max(0, Number(match.index) || 0);
        const blockEnd = Math.max(blockStart, blockStart + String(match[0] || "").length);
        if (selectionStart < blockEnd && selectionEnd > blockStart)
            return true;
    }
    return false;
}

function sourceTextContainsDocumentBlockTagToken(sourceText) {
    return /<\s*\/?\s*(?:agenda|task|callout|resource|break|hr)\b[^>]*>/i.test(sourceText);
}

function resolveStructuredTagInsertionOffset(sourceText, requestedInsertionOffset) {
    const normalizedText = normalizeSourceText(sourceText);
    const safeRequestedOffset = boundedOffset(normalizedText, requestedInsertionOffset);
    const pattern = /<(?:agenda|callout)\b[^>]*>[\s\S]*?<\/(?:agenda|callout)>/ig;
    let match = null;
    while ((match = pattern.exec(normalizedText)) !== null) {
        const blockStart = Math.max(0, Number(match.index) || 0);
        const blockEnd = Math.max(blockStart, blockStart + String(match[0] || "").length);
        if (safeRequestedOffset > blockStart && safeRequestedOffset < blockEnd)
            return blockEnd;
    }
    return safeRequestedOffset;
}

function buildRawSourceInsertionPayload(sourceText,
                                        requestedInsertionOffset,
                                        rawSourceText,
                                        cursorSourceOffsetFromInsertionStart) {
    const currentSourceText = normalizeSourceText(sourceText);
    const normalizedRawSourceText = normalizeSourceText(rawSourceText).trim();
    if (normalizedRawSourceText.length === 0)
        return notAppliedPayload("empty-tag-source");

    const insertionOffset = resolveStructuredTagInsertionOffset(
                currentSourceText,
                requestedInsertionOffset);
    const prefixNewline = insertionOffset > 0 && currentSourceText.charAt(insertionOffset - 1) !== "\n"
            ? "\n"
            : "";
    const suffixNewline = insertionOffset < currentSourceText.length
            && currentSourceText.charAt(insertionOffset) !== "\n"
            ? "\n"
            : "";
    const insertedSourceText = prefixNewline + normalizedRawSourceText + suffixNewline;
    const cursorOffsetInsideRawSource = Math.max(
                0,
                Math.min(
                    normalizedRawSourceText.length,
                    Math.floor(Number(cursorSourceOffsetFromInsertionStart) || 0)));
    const sourceOffset = insertionOffset + prefixNewline.length + cursorOffsetInsideRawSource;
    const nextSourceText = spliceSourceText(
                currentSourceText,
                insertionOffset,
                insertionOffset,
                insertedSourceText);

    return {
        "applied": nextSourceText !== currentSourceText,
        "insertedSourceText": insertedSourceText,
        "nextSourceText": nextSourceText,
        "rawSourceText": normalizedRawSourceText,
        "requestedInsertionOffset": boundedOffset(currentSourceText, requestedInsertionOffset),
        "resolvedInsertionOffset": insertionOffset,
        "sourceOffset": sourceOffset
    };
}

function buildStructuredShortcutInsertionPayload(sourceText, requestedInsertionOffset, shortcutKind) {
    const insertionSpec = structuredShortcutInsertionSpec(shortcutKind);
    if (!insertionSpec.applied)
        return insertionSpec;

    const payload = buildRawSourceInsertionPayload(
                sourceText,
                requestedInsertionOffset,
                insertionSpec.insertionSourceText,
                insertionSpec.cursorSourceOffsetFromInsertionStart);
    payload.tagKind = insertionSpec.tagKind;
    payload.cursorSourceOffsetFromInsertionStart = insertionSpec.cursorSourceOffsetFromInsertionStart;
    return payload;
}

function buildCalloutRangeWrappingPayload(sourceText, selectionSourceStart, selectionSourceEnd) {
    const currentSourceText = normalizeSourceText(sourceText);
    const sourceLength = currentSourceText.length;
    const rangeStart = Math.max(
                0,
                Math.min(
                    sourceLength,
                    Math.min(
                        Math.floor(Number(selectionSourceStart) || 0),
                        Math.floor(Number(selectionSourceEnd) || 0))));
    const rangeEnd = Math.max(
                rangeStart,
                Math.min(
                    sourceLength,
                    Math.max(
                        Math.floor(Number(selectionSourceStart) || 0),
                        Math.floor(Number(selectionSourceEnd) || 0))));
    if (rangeEnd <= rangeStart)
        return notAppliedPayload("empty-callout-range");
    if (sourceRangeOverlapsStructuredBlock(currentSourceText, rangeStart, rangeEnd))
        return notAppliedPayload("callout-range-overlaps-structured-block");

    const selectedSourceText = currentSourceText.slice(rangeStart, rangeEnd);
    if (sourceTextContainsDocumentBlockTagToken(selectedSourceText))
        return notAppliedPayload("callout-range-contains-document-block-tag");

    const calloutOpenTag = "<callout>";
    const calloutCloseTag = "</callout>";
    const wrappedCalloutSourceText = calloutOpenTag + selectedSourceText + calloutCloseTag;
    const prefixNewline = rangeStart > 0 && currentSourceText.charAt(rangeStart - 1) !== "\n" ? "\n" : "";
    const suffixNewline = rangeEnd < sourceLength && currentSourceText.charAt(rangeEnd) !== "\n" ? "\n" : "";
    const replacementSourceText = prefixNewline + wrappedCalloutSourceText + suffixNewline;
    const nextSourceText = spliceSourceText(
                currentSourceText,
                rangeStart,
                rangeEnd,
                replacementSourceText);
    const sourceOffset = rangeStart + prefixNewline.length + calloutOpenTag.length + selectedSourceText.length;

    return {
        "applied": nextSourceText !== currentSourceText,
        "nextSourceText": nextSourceText,
        "replacementSourceText": replacementSourceText,
        "requestedRangeEnd": Math.max(0, Math.floor(Number(selectionSourceEnd) || 0)),
        "requestedRangeStart": Math.max(0, Math.floor(Number(selectionSourceStart) || 0)),
        "resolvedRangeEnd": rangeEnd,
        "resolvedRangeStart": rangeStart,
        "sourceOffset": sourceOffset,
        "tagKind": "callout",
        "wrappedSourceText": selectedSourceText
    };
}
