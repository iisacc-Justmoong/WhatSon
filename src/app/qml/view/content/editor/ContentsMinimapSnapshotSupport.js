.pragma library

function normalizeTextValue(text) {
    return text === undefined || text === null ? "" : String(text);
}

function clampOffset(text, offset) {
    const value = normalizeTextValue(text);
    return Math.max(0, Math.min(value.length, Math.floor(Number(offset) || 0)));
}

function countLineBreaksBefore(text, offset) {
    const value = normalizeTextValue(text);
    const safeOffset = clampOffset(value, offset);
    let lineBreakCount = 0;
    for (let index = 0; index < safeOffset; ++index) {
        if (value.charAt(index) === "\n")
            ++lineBreakCount;
    }
    return lineBreakCount;
}

function lineNumberForOffset(text, offset) {
    return 1 + countLineBreaksBefore(text, offset);
}

function lineNumberForRangeEnd(text, endOffsetExclusive) {
    return 1 + countLineBreaksBefore(text, endOffsetExclusive);
}

function computeChangedLineRange(previousText, nextText) {
    const previous = normalizeTextValue(previousText);
    const next = normalizeTextValue(nextText);
    if (previous === next) {
        return {
            "nextEndLine": 1,
            "nextStartLine": 1,
            "previousEndLine": 1,
            "previousStartLine": 1,
            "valid": false
        };
    }

    let prefixLength = 0;
    const prefixLimit = Math.min(previous.length, next.length);
    while (prefixLength < prefixLimit
            && previous.charAt(prefixLength) === next.charAt(prefixLength)) {
        ++prefixLength;
    }

    let suffixLength = 0;
    const suffixLimit = Math.min(previous.length - prefixLength, next.length - prefixLength);
    while (suffixLength < suffixLimit
            && previous.charAt(previous.length - 1 - suffixLength) === next.charAt(next.length - 1 - suffixLength)) {
        ++suffixLength;
    }

    const previousEndOffsetExclusive = previous.length - suffixLength;
    const nextEndOffsetExclusive = next.length - suffixLength;
    const previousStartLine = lineNumberForOffset(previous, prefixLength);
    const nextStartLine = lineNumberForOffset(next, prefixLength);
    const previousEndLine = Math.max(previousStartLine, lineNumberForRangeEnd(previous, previousEndOffsetExclusive));
    const nextEndLine = Math.max(nextStartLine, lineNumberForRangeEnd(next, nextEndOffsetExclusive));

    return {
        "nextEndLine": nextEndLine,
        "nextStartLine": nextStartLine,
        "previousEndLine": previousEndLine,
        "previousStartLine": previousStartLine,
        "valid": true
    };
}

function cloneLineGroup(group) {
    return {
        "charCount": Math.max(0, Number(group && group.charCount) || 0),
        "contentHeight": Math.max(1, Number(group && group.contentHeight) || 1),
        "contentY": Number(group && group.contentY) || 0,
        "lineNumber": Math.max(1, Number(group && group.lineNumber) || 1),
        "rowCount": Math.max(1, Number(group && group.rowCount) || 1)
    };
}

function totalGroupHeight(groups) {
    const safeGroups = Array.isArray(groups) ? groups : [];
    let totalHeight = 0;
    for (let index = 0; index < safeGroups.length; ++index)
        totalHeight += Math.max(1, Number(safeGroups[index].contentHeight) || 1);
    return totalHeight;
}

function spliceLineGroups(existingGroups, replacementGroups, previousStartLine, previousEndLine) {
    const safeExistingGroups = Array.isArray(existingGroups) ? existingGroups : [];
    const safeReplacementGroups = Array.isArray(replacementGroups) ? replacementGroups : [];
    const safePreviousStartLine = Math.max(1, Math.floor(Number(previousStartLine) || 1));
    const safePreviousEndLine = Math.max(safePreviousStartLine, Math.floor(Number(previousEndLine) || safePreviousStartLine));
    const replaceStartIndex = Math.max(0, Math.min(safeExistingGroups.length, safePreviousStartLine - 1));
    const replaceEndIndexExclusive = Math.max(replaceStartIndex, Math.min(safeExistingGroups.length, safePreviousEndLine));
    const removedGroups = safeExistingGroups.slice(replaceStartIndex, replaceEndIndexExclusive);
    const lineNumberDelta = safeReplacementGroups.length - removedGroups.length;
    const contentYDelta = totalGroupHeight(safeReplacementGroups) - totalGroupHeight(removedGroups);

    const nextGroups = [];
    for (let prefixIndex = 0; prefixIndex < replaceStartIndex; ++prefixIndex)
        nextGroups.push(cloneLineGroup(safeExistingGroups[prefixIndex]));
    for (let replacementIndex = 0; replacementIndex < safeReplacementGroups.length; ++replacementIndex)
        nextGroups.push(cloneLineGroup(safeReplacementGroups[replacementIndex]));
    for (let suffixIndex = replaceEndIndexExclusive; suffixIndex < safeExistingGroups.length; ++suffixIndex) {
        const shiftedGroup = cloneLineGroup(safeExistingGroups[suffixIndex]);
        shiftedGroup.lineNumber += lineNumberDelta;
        shiftedGroup.contentY += contentYDelta;
        nextGroups.push(shiftedGroup);
    }
    return nextGroups;
}

function flattenLineGroups(lineGroups, fallbackLineHeight) {
    const safeLineGroups = Array.isArray(lineGroups) ? lineGroups : [];
    const safeLineHeight = Math.max(1, Number(fallbackLineHeight) || 1);
    const rows = [];
    if (safeLineGroups.length === 0) {
        rows.push({
            "charCount": 0,
            "contentHeight": safeLineHeight,
            "contentY": 0,
            "lineNumber": 1,
            "visualIndex": 0
        });
        return rows;
    }

    for (let groupIndex = 0; groupIndex < safeLineGroups.length; ++groupIndex) {
        const group = safeLineGroups[groupIndex];
        const charCount = Math.max(0, Number(group.charCount) || 0);
        const contentY = Number(group.contentY) || 0;
        const contentHeight = Math.max(safeLineHeight, Number(group.contentHeight) || safeLineHeight);
        const lineNumber = Math.max(1, Number(group.lineNumber) || 1);
        const rowCount = Math.max(1, Math.floor(Number(group.rowCount) || 1));

        for (let rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
            const segmentStart = Math.floor(rowIndex * charCount / rowCount);
            const segmentEnd = Math.ceil((rowIndex + 1) * charCount / rowCount);
            const rowStartY = contentY + (contentHeight * rowIndex / rowCount);
            const rowEndY = rowIndex + 1 < rowCount
                    ? contentY + (contentHeight * (rowIndex + 1) / rowCount)
                    : contentY + contentHeight;
            rows.push({
                "charCount": Math.max(0, segmentEnd - segmentStart),
                "contentHeight": Math.max(1, rowEndY - rowStartY),
                "contentY": rowStartY,
                "lineNumber": lineNumber,
                "visualIndex": rows.length
            });
        }
    }

    return rows;
}
