.pragma library

function normalizedPlainText(value) {
    let text = value === undefined || value === null ? "" : String(value)
    text = text.replace(/\r\n/g, "\n")
    text = text.replace(/\r/g, "\n")
    text = text.replace(/\u2028/g, "\n")
    text = text.replace(/\u2029/g, "\n")
    text = text.replace(/\u00A0/g, " ")
    return text
}

function escapedSourceText(value) {
    let text = normalizedPlainText(value)
    text = text.replace(/&/g, "&amp;")
    text = text.replace(/</g, "&lt;")
    text = text.replace(/>/g, "&gt;")
    text = text.replace(/\"/g, "&quot;")
    text = text.replace(/'/g, "&#39;")
    return text
}

function clampedPlainCursor(text, cursorPosition) {
    const normalizedText = normalizedPlainText(text)
    const numericCursor = Number(cursorPosition)
    const fallbackCursor = normalizedText.length
    const resolvedCursor = isFinite(numericCursor) ? Math.floor(numericCursor) : fallbackCursor
    return Math.max(0, Math.min(normalizedText.length, resolvedCursor))
}

function replacementSourceText(value) {
    const normalizedText = normalizedPlainText(value)
    const escapedText = escapedSourceText(normalizedText)
    if (normalizedText.trim().length === 0)
        return " "
    return escapedText
}

function sourceOffsetForPlainCursor(value, cursorPosition, contentStart) {
    const normalizedText = normalizedPlainText(value)
    const boundedCursor = clampedPlainCursor(normalizedText, cursorPosition)
    const safeContentStart = Math.max(0, Math.floor(Number(contentStart) || 0))
    return safeContentStart + escapedSourceText(normalizedText.slice(0, boundedCursor)).length
}

function plainCursorForSourceOffset(value, sourceOffsetWithinContent) {
    const normalizedText = normalizedPlainText(value)
    const boundedOffset = Math.max(0, Math.floor(Number(sourceOffsetWithinContent) || 0))
    let escapedLength = 0
    for (let index = 0; index < normalizedText.length; ++index) {
        const nextEscapedLength = escapedLength + escapedSourceText(normalizedText.charAt(index)).length
        if (boundedOffset <= escapedLength)
            return index
        if (boundedOffset <= nextEscapedLength)
            return index + 1
        escapedLength = nextEscapedLength
    }
    return normalizedText.length
}

function decodedSourceEntityText(value) {
    const entityText = value === undefined || value === null ? "" : String(value)
    switch (entityText) {
    case "&amp;":
        return "&"
    case "&lt;":
        return "<"
    case "&gt;":
        return ">"
    case "&quot;":
        return "\""
    case "&#39;":
    case "&apos;":
        return "'"
    case "&nbsp;":
        return " "
    default:
        break
    }

    if (/^&#x[0-9a-f]+;$/i.test(entityText)) {
        const codePoint = parseInt(entityText.slice(3, -1), 16)
        if (isFinite(codePoint) && codePoint >= 0 && codePoint <= 0x10ffff) {
            if (String.fromCodePoint !== undefined)
                return String.fromCodePoint(codePoint)
            if (codePoint <= 0xffff)
                return String.fromCharCode(codePoint)
        }
    }
    if (/^&#[0-9]+;$/.test(entityText)) {
        const codePoint = parseInt(entityText.slice(2, -1), 10)
        if (isFinite(codePoint) && codePoint >= 0 && codePoint <= 0x10ffff) {
            if (String.fromCodePoint !== undefined)
                return String.fromCodePoint(codePoint)
            if (codePoint <= 0xffff)
                return String.fromCharCode(codePoint)
        }
    }
    return entityText
}

function normalizedInlineStyleTagName(rawTagName) {
    const normalizedTagName = rawTagName === undefined || rawTagName === null
            ? ""
            : String(rawTagName).trim().toLowerCase()
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

function inlineStyleSourceTagRangeAt(value, sourceOffset) {
    const sourceText = value === undefined || value === null ? "" : String(value)
    const boundedOffset = Math.max(0, Math.floor(Number(sourceOffset) || 0))
    if (boundedOffset >= sourceText.length || sourceText.charAt(boundedOffset) !== "<")
        return null
    const tagEnd = sourceText.indexOf(">", boundedOffset)
    if (tagEnd < 0)
        return null
    const token = sourceText.slice(boundedOffset, tagEnd + 1)
    const match = /^<\s*(\/?)\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>$/.exec(token)
    if (!match)
        return null
    const normalizedTagName = normalizedInlineStyleTagName(match[2])
    if (normalizedTagName.length === 0)
        return null
    return {
        "closing": match[1] === "/",
        "end": tagEnd + 1,
        "name": normalizedTagName,
        "start": boundedOffset
    }
}

function sourceEntityRangeAt(value, sourceOffset) {
    const sourceText = value === undefined || value === null ? "" : String(value)
    const boundedOffset = Math.max(0, Math.floor(Number(sourceOffset) || 0))
    if (boundedOffset >= sourceText.length || sourceText.charAt(boundedOffset) !== "&")
        return null
    const match = /^&(?:#[0-9]+|#x[0-9A-Fa-f]+|[A-Za-z][A-Za-z0-9]{0,31});/.exec(sourceText.slice(boundedOffset))
    if (!match)
        return null
    return {
        "end": boundedOffset + match[0].length,
        "start": boundedOffset
    }
}

function plainTextFromInlineTaggedSource(value) {
    const sourceText = value === undefined || value === null ? "" : String(value)
    let plainText = ""
    let sourceOffset = 0
    while (sourceOffset < sourceText.length) {
        const tagRange = inlineStyleSourceTagRangeAt(sourceText, sourceOffset)
        if (tagRange) {
            sourceOffset = tagRange.end
            continue
        }
        const entityRange = sourceEntityRangeAt(sourceText, sourceOffset)
        if (entityRange) {
            plainText += decodedSourceEntityText(sourceText.slice(entityRange.start, entityRange.end))
            sourceOffset = entityRange.end
            continue
        }
        plainText += sourceText.charAt(sourceOffset)
        sourceOffset += 1
    }
    return normalizedPlainText(plainText)
}

function sourceOffsetForInlineTaggedLogicalOffset(value, logicalOffset) {
    const sourceText = value === undefined || value === null ? "" : String(value)
    const boundedLogicalOffset = Math.max(0, Math.floor(Number(logicalOffset) || 0))
    let visibleCursor = 0
    let sourceOffset = 0
    while (sourceOffset < sourceText.length && visibleCursor < boundedLogicalOffset) {
        const tagRange = inlineStyleSourceTagRangeAt(sourceText, sourceOffset)
        if (tagRange) {
            sourceOffset = tagRange.end
            continue
        }
        const entityRange = sourceEntityRangeAt(sourceText, sourceOffset)
        if (entityRange) {
            sourceOffset = entityRange.end
            visibleCursor += 1
            continue
        }
        sourceOffset += 1
        visibleCursor += 1
    }
    return sourceOffset
}

function advanceSourceOffsetPastClosingInlineStyleTags(value, sourceOffset) {
    const sourceText = value === undefined || value === null ? "" : String(value)
    let boundedOffset = Math.max(0, Math.min(sourceText.length, Math.floor(Number(sourceOffset) || 0)))
    while (boundedOffset < sourceText.length) {
        const tagRange = inlineStyleSourceTagRangeAt(sourceText, boundedOffset)
        if (!tagRange || !tagRange.closing)
            break
        boundedOffset = tagRange.end
    }
    return boundedOffset
}

function sourceOffsetForInlineTaggedCursor(value, cursorPosition, contentStart) {
    const sourceText = value === undefined || value === null ? "" : String(value)
    const safeContentStart = Math.max(0, Math.floor(Number(contentStart) || 0))
    return safeContentStart + advanceSourceOffsetPastClosingInlineStyleTags(
                sourceText,
                sourceOffsetForInlineTaggedLogicalOffset(sourceText, cursorPosition))
}

function sourceOffsetForInlineTaggedSelectionBoundary(value, cursorPosition, contentStart) {
    const safeContentStart = Math.max(0, Math.floor(Number(contentStart) || 0))
    return safeContentStart + sourceOffsetForInlineTaggedLogicalOffset(value, cursorPosition)
}

function plainCursorForInlineTaggedSourceOffset(value, sourceOffsetWithinContent) {
    const sourceText = value === undefined || value === null ? "" : String(value)
    const boundedOffset = Math.max(0, Math.min(sourceText.length, Math.floor(Number(sourceOffsetWithinContent) || 0)))
    let visibleCursor = 0
    let sourceOffset = 0
    while (sourceOffset < sourceText.length && sourceOffset < boundedOffset) {
        const tagRange = inlineStyleSourceTagRangeAt(sourceText, sourceOffset)
        if (tagRange) {
            if (boundedOffset <= tagRange.end)
                return visibleCursor
            sourceOffset = tagRange.end
            continue
        }
        const entityRange = sourceEntityRangeAt(sourceText, sourceOffset)
        if (entityRange) {
            if (boundedOffset < entityRange.end)
                return visibleCursor + 1
            sourceOffset = entityRange.end
            visibleCursor += 1
            continue
        }
        sourceOffset += 1
        visibleCursor += 1
    }
    return visibleCursor
}
