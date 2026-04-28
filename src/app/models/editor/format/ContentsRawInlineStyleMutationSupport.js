.pragma library

function normalizeSourceText(sourceText) {
    let normalizedText = sourceText === undefined || sourceText === null ? "" : String(sourceText)
    normalizedText = normalizedText.replace(/\r\n/g, "\n")
    normalizedText = normalizedText.replace(/\r/g, "\n")
    normalizedText = normalizedText.replace(/\u2028/g, "\n")
    normalizedText = normalizedText.replace(/\u2029/g, "\n")
    normalizedText = normalizedText.replace(/\u00A0/g, " ")
    return normalizedText
}

function normalizedInlineStyleTag(rawStyleTag) {
    const normalizedTagName = rawStyleTag === undefined || rawStyleTag === null
            ? ""
            : String(rawStyleTag).trim().toLowerCase()
    if (normalizedTagName === "plain" || normalizedTagName === "clear" || normalizedTagName === "none")
        return "plain"
    if (normalizedTagName === "bold" || normalizedTagName === "b" || normalizedTagName === "strong")
        return "bold"
    if (normalizedTagName === "italic" || normalizedTagName === "i" || normalizedTagName === "em")
        return "italic"
    if (normalizedTagName === "underline" || normalizedTagName === "u")
        return "underline"
    if (normalizedTagName === "strikethrough" || normalizedTagName === "strike" || normalizedTagName === "s" || normalizedTagName === "del")
        return "strikethrough"
    if (normalizedTagName === "highlight" || normalizedTagName === "mark")
        return "highlight"
    return ""
}

function isLogicalBreakTagName(normalizedTagName) {
    return normalizedTagName === "br"
            || normalizedTagName === "break"
            || normalizedTagName === "hr"
}

function sourceTagRangeAt(sourceText, sourceOffset) {
    const normalizedSourceText = normalizeSourceText(sourceText)
    const boundedOffset = Math.max(0, Math.floor(Number(sourceOffset) || 0))
    if (boundedOffset >= normalizedSourceText.length || normalizedSourceText.charAt(boundedOffset) !== "<")
        return null
    const tagEnd = normalizedSourceText.indexOf(">", boundedOffset)
    if (tagEnd < 0)
        return null

    const token = normalizedSourceText.slice(boundedOffset, tagEnd + 1)
    const match = /^<\s*(\/?)\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>$/.exec(token)
    if (!match)
        return null

    const normalizedTagName = String(match[2] || "").trim().toLowerCase()
    return {
        "closing": match[1] === "/",
        "end": tagEnd + 1,
        "inlineStyleName": normalizedInlineStyleTag(normalizedTagName),
        "logicalBreak": isLogicalBreakTagName(normalizedTagName),
        "name": normalizedTagName,
        "selfClosing": token.trim().endsWith("/>"),
        "start": boundedOffset
    }
}

function htmlEntityRangeAt(sourceText, sourceOffset) {
    const normalizedSourceText = normalizeSourceText(sourceText)
    const boundedOffset = Math.max(0, Math.floor(Number(sourceOffset) || 0))
    if (boundedOffset >= normalizedSourceText.length || normalizedSourceText.charAt(boundedOffset) !== "&")
        return null
    const match = /^&(?:#[0-9]+|#x[0-9A-Fa-f]+|[A-Za-z][A-Za-z0-9]{0,31});/.exec(normalizedSourceText.slice(boundedOffset))
    if (!match)
        return null
    return {
        "end": boundedOffset + match[0].length,
        "start": boundedOffset
    }
}

function logicalLengthForSource(sourceText) {
    const normalizedSourceText = normalizeSourceText(sourceText)
    let logicalLength = 0
    let sourceOffset = 0

    while (sourceOffset < normalizedSourceText.length) {
        const tagRange = sourceTagRangeAt(normalizedSourceText, sourceOffset)
        if (tagRange) {
            sourceOffset = tagRange.end
            if (tagRange.logicalBreak)
                logicalLength += 1
            continue
        }

        const entityRange = htmlEntityRangeAt(normalizedSourceText, sourceOffset)
        if (entityRange) {
            sourceOffset = entityRange.end
            logicalLength += 1
            continue
        }

        sourceOffset += 1
        logicalLength += 1
    }

    return logicalLength
}

function notAppliedPayload(reason, sourceText, selectionStart, selectionEnd, styleTag) {
    return {
        "applied": false,
        "logicalLength": logicalLengthForSource(sourceText),
        "nextSourceText": normalizeSourceText(sourceText),
        "reason": reason,
        "selectionEnd": Math.max(0, Math.floor(Number(selectionEnd) || 0)),
        "selectionStart": Math.max(0, Math.floor(Number(selectionStart) || 0)),
        "styleTag": normalizedInlineStyleTag(styleTag)
    }
}

function advanceSourceOffsetPastOpeningWrappers(sourceText, sourceOffset) {
    const normalizedSourceText = normalizeSourceText(sourceText)
    let boundedOffset = Math.max(0, Math.min(normalizedSourceText.length, Math.floor(Number(sourceOffset) || 0)))

    while (boundedOffset < normalizedSourceText.length) {
        const tagRange = sourceTagRangeAt(normalizedSourceText, boundedOffset)
        if (!tagRange
                || tagRange.closing
                || tagRange.selfClosing
                || tagRange.logicalBreak) {
            break
        }
        boundedOffset = tagRange.end
    }

    return boundedOffset
}

function sourceOffsetForDirectWrapBoundary(sourceText, logicalOffset) {
    const normalizedSourceText = normalizeSourceText(sourceText)
    const boundedLogicalOffset = Math.max(0, Math.floor(Number(logicalOffset) || 0))
    let visibleCursor = 0
    let sourceOffset = 0

    while (sourceOffset < normalizedSourceText.length && visibleCursor < boundedLogicalOffset) {
        const tagRange = sourceTagRangeAt(normalizedSourceText, sourceOffset)
        if (tagRange) {
            sourceOffset = tagRange.end
            if (tagRange.logicalBreak)
                visibleCursor += 1
            continue
        }

        const entityRange = htmlEntityRangeAt(normalizedSourceText, sourceOffset)
        if (entityRange) {
            sourceOffset = entityRange.end
            visibleCursor += 1
            continue
        }

        sourceOffset += 1
        visibleCursor += 1
    }

    return advanceSourceOffsetPastOpeningWrappers(normalizedSourceText, sourceOffset)
}

function inlineStyleOpeningTagEndingAt(sourceText, boundaryOffset) {
    const normalizedSourceText = normalizeSourceText(sourceText)
    const boundedOffset = Math.max(0, Math.min(normalizedSourceText.length, Math.floor(Number(boundaryOffset) || 0)))
    if (boundedOffset <= 0)
        return null

    const tagStart = normalizedSourceText.lastIndexOf("<", boundedOffset - 1)
    if (tagStart < 0)
        return null

    const tagRange = sourceTagRangeAt(normalizedSourceText, tagStart)
    if (!tagRange
            || tagRange.end !== boundedOffset
            || tagRange.closing
            || tagRange.inlineStyleName.length === 0) {
        return null
    }

    return tagRange
}

function inlineStyleClosingTagStartingAt(sourceText, boundaryOffset) {
    const tagRange = sourceTagRangeAt(sourceText, boundaryOffset)
    if (!tagRange || !tagRange.closing || tagRange.inlineStyleName.length === 0)
        return null
    return tagRange
}

function unwrapDirectSelectionWrappers(sourceText, selectionSourceStart, selectionSourceEnd) {
    let nextSourceText = normalizeSourceText(sourceText)
    let currentSelectionStart = Math.max(0, Math.floor(Number(selectionSourceStart) || 0))
    let currentSelectionEnd = Math.max(currentSelectionStart, Math.floor(Number(selectionSourceEnd) || 0))
    let removedAny = false

    while (currentSelectionEnd > currentSelectionStart) {
        const openingTag = inlineStyleOpeningTagEndingAt(nextSourceText, currentSelectionStart)
        const closingTag = inlineStyleClosingTagStartingAt(nextSourceText, currentSelectionEnd)
        if (!openingTag || !closingTag || openingTag.inlineStyleName !== closingTag.inlineStyleName)
            break

        const selectedSourceText = nextSourceText.slice(currentSelectionStart, currentSelectionEnd)
        nextSourceText = nextSourceText.slice(0, openingTag.start)
                + selectedSourceText
                + nextSourceText.slice(closingTag.end)
        currentSelectionStart = openingTag.start
        currentSelectionEnd = currentSelectionStart + selectedSourceText.length
        removedAny = true
    }

    return {
        "nextSourceText": nextSourceText,
        "removedAny": removedAny
    }
}

function buildInlineStyleSelectionPayload(sourceText, selectionStart, selectionEnd, styleTag) {
    const normalizedSourceText = normalizeSourceText(sourceText)
    if (normalizedSourceText.length === 0)
        return notAppliedPayload("empty-source", normalizedSourceText, selectionStart, selectionEnd, styleTag)

    const normalizedStyleTag = normalizedInlineStyleTag(styleTag)
    if (normalizedStyleTag.length === 0)
        return notAppliedPayload("unsupported-style-tag", normalizedSourceText, selectionStart, selectionEnd, styleTag)

    const logicalLength = logicalLengthForSource(normalizedSourceText)
    const boundedStart = Math.max(
                0,
                Math.min(
                    logicalLength,
                    Math.floor(Math.min(Number(selectionStart) || 0, Number(selectionEnd) || 0))))
    const boundedEnd = Math.max(
                0,
                Math.min(
                    logicalLength,
                    Math.floor(Math.max(Number(selectionStart) || 0, Number(selectionEnd) || 0))))
    if (boundedEnd <= boundedStart)
        return notAppliedPayload("empty-selection", normalizedSourceText, boundedStart, boundedEnd, normalizedStyleTag)

    const selectionSourceStart = sourceOffsetForDirectWrapBoundary(normalizedSourceText, boundedStart)
    const selectionSourceEnd = sourceOffsetForDirectWrapBoundary(normalizedSourceText, boundedEnd)
    if (selectionSourceEnd < selectionSourceStart)
        return notAppliedPayload("invalid-selection-boundary", normalizedSourceText, boundedStart, boundedEnd, normalizedStyleTag)

    let nextSourceText = normalizedSourceText
    let applied = false
    if (normalizedStyleTag === "plain") {
        const unwrapPayload = unwrapDirectSelectionWrappers(
                    normalizedSourceText,
                    selectionSourceStart,
                    selectionSourceEnd)
        nextSourceText = unwrapPayload.nextSourceText
        applied = unwrapPayload.removedAny
    } else {
        const openTag = "<" + normalizedStyleTag + ">"
        const closeTag = "</" + normalizedStyleTag + ">"
        nextSourceText = normalizedSourceText.slice(0, selectionSourceStart)
                + openTag
                + normalizedSourceText.slice(selectionSourceStart, selectionSourceEnd)
                + closeTag
                + normalizedSourceText.slice(selectionSourceEnd)
        applied = nextSourceText !== normalizedSourceText
    }

    return {
        "applied": applied,
        "logicalLength": logicalLength,
        "nextSourceText": nextSourceText,
        "reason": applied ? "" : "no-op",
        "selectionEnd": boundedEnd,
        "selectionStart": boundedStart,
        "styleTag": normalizedStyleTag
    }
}
