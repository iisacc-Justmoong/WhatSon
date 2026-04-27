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

function safeNonNegativeNumber(value, fallbackValue) {
    const numericValue = Number(value)
    if (isFinite(numericValue) && numericValue >= 0)
        return numericValue
    return Math.max(0, Number(fallbackValue) || 0)
}

function safeMappedPoint(editorItem, mapTarget, rawX, rawY, fallbackX, fallbackY) {
    const numericFallbackX = safeNonNegativeNumber(fallbackX, 0)
    const numericFallbackY = safeNonNegativeNumber(fallbackY, 0)
    const numericRawX = safeNonNegativeNumber(rawX, numericFallbackX)
    const numericRawY = safeNonNegativeNumber(rawY, numericFallbackY)
    if (!editorItem || editorItem.mapToItem === undefined || !mapTarget) {
        return {
            "x": numericRawX,
            "y": numericRawY
        }
    }
    const mappedPoint = editorItem.mapToItem(mapTarget, numericRawX, numericRawY)
    const mappedX = Number(mappedPoint && mappedPoint.x !== undefined ? mappedPoint.x : numericFallbackX)
    const mappedY = Number(mappedPoint && mappedPoint.y !== undefined ? mappedPoint.y : numericFallbackY)
    return {
        "x": Math.max(0, isFinite(mappedX) ? mappedX : numericFallbackX),
        "y": Math.max(0, isFinite(mappedY) ? mappedY : numericFallbackY)
    }
}

function safeMappedY(editorItem, mapTarget, rawY, fallbackY) {
    return safeMappedPoint(editorItem, mapTarget, 0, rawY, 0, fallbackY).y
}

function safeAvailableWidth(editorItem, mapTarget, fallbackLineHeight, plainTextValue) {
    const safeLineHeight = safePositiveNumber(fallbackLineHeight, 1)
    const textValue = plainTextValue === undefined || plainTextValue === null ? "" : String(plainTextValue)
    const editorWidth = Number(editorItem && editorItem.width !== undefined ? editorItem.width : 0)
    const targetWidth = Number(mapTarget && mapTarget.width !== undefined ? mapTarget.width : 0)
    const textFallbackWidth = Math.max(1, textValue.length * safeLineHeight * 0.58)
    return Math.max(
                1,
                isFinite(editorWidth) ? editorWidth : 0,
                isFinite(targetWidth) ? targetWidth : 0,
                textFallbackWidth)
}

function fallbackLineWidth(lineText, availableWidth, fallbackLineHeight) {
    const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
    if (normalizedLineText.length <= 0)
        return 0
    const safeAvailableWidth = safePositiveNumber(availableWidth, 1)
    const safeLineHeight = safePositiveNumber(fallbackLineHeight, 1)
    return Math.min(
                safeAvailableWidth,
                Math.max(1, normalizedLineText.length * safeLineHeight * 0.58))
}

function sampledLineWidthEntry(
        safeText,
        lineText,
        lineStartOffset,
        lineEndOffset,
        editorItem,
        mapTarget,
        availableWidth,
        fallbackLineHeight,
        fallbackContentY) {
    const fallbackWidth = fallbackLineWidth(lineText, availableWidth, fallbackLineHeight)
    const safeAvailable = safePositiveNumber(availableWidth, fallbackWidth > 0 ? fallbackWidth : 1)
    if (!editorItem || editorItem.positionToRectangle === undefined) {
        return {
            "contentAvailableWidth": safeAvailable,
            "contentWidth": fallbackWidth,
            "visualRowWidths": [ fallbackWidth ]
        }
    }

    const safeLineStart = Math.max(0, Math.min(String(safeText).length, Math.floor(Number(lineStartOffset) || 0)))
    const safeLineEnd = Math.max(safeLineStart, Math.min(String(safeText).length, Math.floor(Number(lineEndOffset) || safeLineStart)))
    const rows = []
    let previousSample = null

    for (let position = safeLineStart; position <= safeLineEnd; ++position) {
        const rect = editorItem.positionToRectangle(position)
        const rawHeight = safePositiveNumber(rect && rect.height !== undefined ? rect.height : 0, fallbackLineHeight)
        const rawX = safeNonNegativeNumber(rect && rect.x !== undefined ? rect.x : 0, 0)
        const fallbackY = safeNonNegativeNumber(fallbackContentY, 0)
        const rawY = safeNonNegativeNumber(rect && rect.y !== undefined ? rect.y : fallbackY, fallbackY)
        const mappedPoint = safeMappedPoint(editorItem, mapTarget, rawX, rawY, rawX, fallbackY)
        let row = rows.length > 0 ? rows[rows.length - 1] : null
        const rowTolerance = Math.max(1, rawHeight * 0.5)
        if (!row || mappedPoint.y > row.contentY + rowTolerance) {
            row = {
                "contentY": mappedPoint.y,
                "hasLineEndBoundary": position === safeLineEnd,
                "maxPositiveAdvance": 0,
                "maxX": mappedPoint.x,
                "minX": mappedPoint.x
            }
            rows.push(row)
        } else {
            if (previousSample && previousSample.row === row) {
                const positiveAdvance = mappedPoint.x - previousSample.x
                if (positiveAdvance > 0)
                    row.maxPositiveAdvance = Math.max(row.maxPositiveAdvance, positiveAdvance)
            }
            row.hasLineEndBoundary = row.hasLineEndBoundary || position === safeLineEnd
            row.maxX = Math.max(row.maxX, mappedPoint.x)
            row.minX = Math.min(row.minX, mappedPoint.x)
        }
        previousSample = {
            "row": row,
            "x": mappedPoint.x
        }
    }

    if (rows.length === 0) {
        return {
            "contentAvailableWidth": safeAvailable,
            "contentWidth": fallbackWidth,
            "visualRowWidths": [ fallbackWidth ]
        }
    }

    const fallbackAdvance = Math.max(1, safePositiveNumber(fallbackLineHeight, 1) * 0.58)
    const visualRowWidths = []
    let maxWidth = 0
    for (let rowIndex = 0; rowIndex < rows.length; ++rowIndex) {
        const row = rows[rowIndex]
        let rowWidth = Math.max(0, row.maxX - row.minX)
        if (String(lineText || "").length > 0 && !row.hasLineEndBoundary)
            rowWidth += row.maxPositiveAdvance > 0 ? row.maxPositiveAdvance : fallbackAdvance
        if (String(lineText || "").length > 0 && rowWidth <= 0)
            rowWidth = fallbackWidth
        rowWidth = Math.min(safeAvailable, Math.max(0, rowWidth))
        visualRowWidths.push(rowWidth)
        maxWidth = Math.max(maxWidth, rowWidth)
    }

    return {
        "contentAvailableWidth": safeAvailable,
        "contentWidth": maxWidth,
        "visualRowWidths": visualRowWidths.length > 0 ? visualRowWidths : [ fallbackWidth ]
    }
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
        const lineEndOffset = Math.min(safeText.length, lineStartOffset + lineText.length)

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

        const availableWidth = safeAvailableWidth(editorItem, mapTarget, safeFallbackLineHeight, lineText)
        const widthEntry = sampledLineWidthEntry(
                    safeText,
                    lineText,
                    lineStartOffset,
                    lineEndOffset,
                    editorItem,
                    mapTarget,
                    availableWidth,
                    safeFallbackLineHeight,
                    startY)
        entries.push({
            "contentAvailableWidth": widthEntry.contentAvailableWidth,
            "contentHeight": Math.max(1, endY - startY),
            "contentWidth": widthEntry.contentWidth,
            "contentY": startY,
            "visualRowWidths": widthEntry.visualRowWidths
        })
        lineStartOffset = nextLineStartOffset
    }

    return entries
}
