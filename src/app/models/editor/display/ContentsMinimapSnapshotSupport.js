.pragma library

function normalizeVisualRowWidths(rawWidths, fallbackWidth) {
    const widths = [];
    if (Array.isArray(rawWidths)) {
        for (let index = 0; index < rawWidths.length; ++index)
            widths.push(Math.max(0, Number(rawWidths[index]) || 0));
    } else {
        const explicitLength = Number(rawWidths && rawWidths.length);
        if (isFinite(explicitLength) && explicitLength >= 0) {
            for (let index = 0; index < Math.floor(explicitLength); ++index)
                widths.push(Math.max(0, Number(rawWidths[index]) || 0));
        }
    }
    if (widths.length === 0)
        widths.push(Math.max(0, Number(fallbackWidth) || 0));
    return widths;
}

function flattenLineGroups(lineGroups, fallbackLineHeight) {
    const safeLineGroups = Array.isArray(lineGroups) ? lineGroups : [];
    const safeLineHeight = Math.max(1, Number(fallbackLineHeight) || 1);
    const rows = [];
    if (safeLineGroups.length === 0) {
        rows.push({
            "charCount": 0,
            "contentAvailableWidth": 0,
            "contentHeight": safeLineHeight,
            "contentWidth": 0,
            "contentY": 0,
            "lineNumber": 1,
            "visualIndex": 0
        });
        return rows;
    }

    for (let groupIndex = 0; groupIndex < safeLineGroups.length; ++groupIndex) {
        const group = safeLineGroups[groupIndex];
        const charCount = Math.max(0, Number(group.charCount) || 0);
        const contentWidth = Math.max(0, Number(group.contentWidth) || 0);
        const contentAvailableWidth = Math.max(
                    contentWidth,
                    Number(group.contentAvailableWidth) || 0);
        const contentY = Number(group.contentY) || 0;
        const contentHeight = Math.max(safeLineHeight, Number(group.contentHeight) || safeLineHeight);
        const lineNumber = Math.max(1, Number(group.lineNumber) || 1);
        const minimapRowCharCount = Math.max(0, Number(group.minimapRowCharCount) || 0);
        const minimapVisualKind = group && group.minimapVisualKind !== undefined
                ? String(group.minimapVisualKind)
                : "text";
        const rowCount = Math.max(1, Math.floor(Number(group.rowCount) || 1));
        const visualRowWidths = normalizeVisualRowWidths(group.visualRowWidths, contentWidth);

        for (let rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
            const segmentStart = Math.floor(rowIndex * charCount / rowCount);
            const segmentEnd = Math.ceil((rowIndex + 1) * charCount / rowCount);
            const rowStartY = contentY + (contentHeight * rowIndex / rowCount);
            const rowEndY = rowIndex + 1 < rowCount
                    ? contentY + (contentHeight * (rowIndex + 1) / rowCount)
                    : contentY + contentHeight;
            rows.push({
                "charCount": minimapRowCharCount > 0
                             ? minimapRowCharCount
                             : Math.max(0, segmentEnd - segmentStart),
                "contentAvailableWidth": contentAvailableWidth,
                "contentHeight": Math.max(1, rowEndY - rowStartY),
                "contentWidth": rowIndex < visualRowWidths.length
                                ? Math.max(0, Number(visualRowWidths[rowIndex]) || 0)
                                : contentWidth,
                "contentY": rowStartY,
                "lineNumber": lineNumber,
                "minimapVisualKind": minimapVisualKind,
                "visualIndex": rows.length
            });
        }
    }

    return rows;
}
