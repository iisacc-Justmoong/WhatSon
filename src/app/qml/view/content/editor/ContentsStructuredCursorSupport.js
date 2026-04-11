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
