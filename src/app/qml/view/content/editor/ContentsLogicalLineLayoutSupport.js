.pragma library

function normalizedPlainTextLines(plainTextValue) {
    const safeText = plainTextValue === undefined || plainTextValue === null
            ? ""
            : String(plainTextValue)
    return safeText.length > 0 ? safeText.split("\n") : [""]
}

function safePositiveNumber(value, fallbackValue) {
    const numericValue = Number(value)
    if (isFinite(numericValue) && numericValue > 0)
        return numericValue
    return Math.max(1, Number(fallbackValue) || 1)
}

function safeMappedY(editorItem, mapTarget, rawY, fallbackY) {
    const numericFallbackY = Math.max(0, Number(fallbackY) || 0)
    const numericRawY = Math.max(0, Number(rawY) || 0)
    if (!editorItem || editorItem.mapToItem === undefined || !mapTarget)
        return numericRawY
    const mappedPoint = editorItem.mapToItem(mapTarget, 0, numericRawY)
    const mappedY = Number(mappedPoint && mappedPoint.y !== undefined ? mappedPoint.y : numericFallbackY)
    return Math.max(0, isFinite(mappedY) ? mappedY : numericFallbackY)
}

function buildEntries(plainTextValue, blockHeight, editorItem, mapTarget, fallbackLineHeight) {
    const safeText = plainTextValue === undefined || plainTextValue === null
            ? ""
            : String(plainTextValue)
    const logicalLines = normalizedPlainTextLines(safeText)
    const safeFallbackLineHeight = safePositiveNumber(fallbackLineHeight, 1)
    const safeBlockHeight = Math.max(
                safeFallbackLineHeight,
                Number(blockHeight) || 0,
                logicalLines.length * safeFallbackLineHeight)
    const entries = []
    let lineStartOffset = 0

    for (let index = 0; index < logicalLines.length; ++index) {
        const lineText = String(logicalLines[index] || "")
        const startRect = editorItem && editorItem.positionToRectangle !== undefined
                ? editorItem.positionToRectangle(lineStartOffset)
                : ({})
        const fallbackStartY = safeBlockHeight * index / Math.max(1, logicalLines.length)
        const rawStartY = Number(startRect && startRect.y !== undefined ? startRect.y : fallbackStartY)
        const startY = safeMappedY(editorItem, mapTarget, rawStartY, fallbackStartY)
        const startHeight = safePositiveNumber(
                    startRect && startRect.height !== undefined ? startRect.height : 0,
                    safeFallbackLineHeight)
        const nextLineStartOffset = index + 1 < logicalLines.length
                ? Math.min(safeText.length, lineStartOffset + lineText.length + 1)
                : safeText.length

        let endY = Math.max(startY + startHeight, safeBlockHeight)
        if (index + 1 < logicalLines.length) {
            const nextRect = editorItem && editorItem.positionToRectangle !== undefined
                    ? editorItem.positionToRectangle(nextLineStartOffset)
                    : ({})
            const fallbackEndY = safeBlockHeight * (index + 1) / Math.max(1, logicalLines.length)
            const rawNextY = Number(nextRect && nextRect.y !== undefined ? nextRect.y : fallbackEndY)
            const mappedNextY = safeMappedY(editorItem, mapTarget, rawNextY, fallbackEndY)
            endY = Math.max(startY + startHeight, mappedNextY)
        }

        entries.push({
            "contentHeight": Math.max(1, endY - startY),
            "contentY": startY
        })
        lineStartOffset = nextLineStartOffset
    }

    return entries
}
