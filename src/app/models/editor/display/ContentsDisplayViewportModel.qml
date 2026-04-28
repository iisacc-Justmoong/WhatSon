pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: model

    property var contentsView: null
    property var structuredDocumentFlow: null
    property var viewportCoordinator: null

    function isMinimapScrollable() {
        const flickable = model.contentsView.editorFlickable;
        if (!flickable)
            return false;
        return model.minimapContentHeight() > (Number(flickable.height) || 0);
    }

    function lineDocumentY(lineNumber) {
        if (model.contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, model.contentsView.effectiveStructuredLogicalLineEntries().length);
            const safeLineNumber = Math.max(1, Math.min(structuredLineCount, Number(lineNumber) || 1));
            const structuredEntry = model.contentsView.structuredLogicalLineEntryAt(safeLineNumber);
            if (structuredEntry && structuredEntry.contentY !== undefined)
                return Math.max(0, Number(structuredEntry.contentY) || 0);
            return Math.max(0, (safeLineNumber - 1) * model.contentsView.editorLineHeight);
        }
        const safeLineNumber = Math.max(1, Math.min(model.contentsView.logicalLineCount, Number(lineNumber) || 1));
        if (model.contentsView.incrementalLineGeometryAvailable()) {
            const lineGroup = model.contentsView.minimapLineGroups[safeLineNumber - 1];
            if (lineGroup && lineGroup.contentY !== undefined)
                return Math.max(0, (Number(lineGroup.contentY) || 0) - model.contentsView.editorDocumentStartY);
        }
        model.contentsView.ensureLogicalLineDocumentYCache();
        const cacheIndex = safeLineNumber - 1;
        if (Array.isArray(model.contentsView.logicalLineDocumentYCache) && cacheIndex >= 0 && cacheIndex < model.contentsView.logicalLineDocumentYCache.length)
            return Number(model.contentsView.logicalLineDocumentYCache[cacheIndex]) || 0;
        return Math.max(0, (safeLineNumber - 1) * model.contentsView.editorLineHeight);
    }

    function gutterLineDocumentY(lineNumber) {
        if (model.contentsView.structuredHostGeometryActive) {
            const structuredEntry = model.contentsView.structuredLogicalLineEntryAt(lineNumber);
            if (structuredEntry && structuredEntry.gutterContentY !== undefined)
                return Math.max(0, Number(structuredEntry.gutterContentY) || 0);
            if (structuredEntry && structuredEntry.contentY !== undefined)
                return Math.max(0, Number(structuredEntry.contentY) || 0);
        }
        model.contentsView.ensureLogicalLineGutterDocumentYCache();
        const safeLineNumber = Math.max(1, Math.min(model.contentsView.logicalLineCount, Number(lineNumber) || 1));
        const cacheIndex = safeLineNumber - 1;
        if (Array.isArray(model.contentsView.logicalLineGutterDocumentYCache)
                && cacheIndex >= 0
                && cacheIndex < model.contentsView.logicalLineGutterDocumentYCache.length) {
            return Math.max(0, Number(model.contentsView.logicalLineGutterDocumentYCache[cacheIndex]) || 0);
        }
        return model.lineDocumentY(safeLineNumber);
    }

    function gutterDocumentOccupiedBottomY() {
        if (model.contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, model.contentsView.effectiveStructuredLogicalLineEntries().length);
            const lastLineNumber = structuredLineCount;
            return Math.max(
                        model.contentsView.editorLineHeight,
                        model.gutterLineDocumentY(lastLineNumber)
                        + model.contentsView.singleLineGutterHeight(lastLineNumber));
        }
        if (model.contentsView.logicalLineCount <= 0)
            return model.contentsView.editorLineHeight;
        model.contentsView.ensureLogicalLineGutterDocumentYCache();
        const lastLineNumber = model.contentsView.logicalLineCount;
        return Math.max(
                    model.contentsView.editorLineHeight,
                    model.gutterLineDocumentY(lastLineNumber)
                    + model.contentsView.singleLineGutterHeight(lastLineNumber));
    }

    function lineVisualHeight(startLine, lineSpan) {
        const safeLineSpan = Math.max(1, Number(lineSpan) || 1);
        if (model.contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, model.contentsView.effectiveStructuredLogicalLineEntries().length);
            const safeStartLine = Math.max(1, Math.min(structuredLineCount, Number(startLine) || 1));
            const startEntry = model.contentsView.structuredLogicalLineEntryAt(safeStartLine);
            if (safeLineSpan === 1) {
                return Math.max(
                            1,
                            Number(startEntry && startEntry.contentHeight !== undefined
                                   ? startEntry.contentHeight
                                   : 0) || model.contentsView.editorLineHeight);
            }
            const safeEndLine = Math.max(
                        safeStartLine,
                        Math.min(
                            structuredLineCount,
                            safeStartLine + safeLineSpan - 1));
            const endEntry = model.contentsView.structuredLogicalLineEntryAt(safeEndLine);
            const startDocumentY = Math.max(0, Number(startEntry && startEntry.contentY !== undefined ? startEntry.contentY : 0) || 0);
            const endDocumentY = Math.max(
                        startDocumentY + model.contentsView.editorLineHeight,
                        Math.max(0, Number(endEntry && endEntry.contentY !== undefined ? endEntry.contentY : startDocumentY) || startDocumentY)
                        + Math.max(
                            1,
                            Number(endEntry && endEntry.contentHeight !== undefined
                                   ? endEntry.contentHeight
                                   : 0) || model.contentsView.editorLineHeight));
            return Math.max(model.contentsView.editorLineHeight, endDocumentY - startDocumentY);
        }
        const safeStartLine = Math.max(1, Math.min(model.contentsView.logicalLineCount, Number(startLine) || 1));
        if (safeLineSpan === 1 && model.contentsView.incrementalLineGeometryAvailable()) {
            const lineGroup = model.contentsView.minimapLineGroups[safeStartLine - 1];
            if (lineGroup && lineGroup.contentHeight !== undefined)
                return Math.max(1, Number(lineGroup.contentHeight) || model.contentsView.editorLineHeight);
        }
        const startDocumentY = model.lineDocumentY(safeStartLine);
        const nextLineNumber = safeStartLine + safeLineSpan;
        let endDocumentY = 0;
        if (nextLineNumber <= model.contentsView.logicalLineCount)
            endDocumentY = model.lineDocumentY(nextLineNumber);
        else
            endDocumentY = model.contentsView.documentOccupiedBottomY();
        return Math.max(model.contentsView.editorLineHeight, endDocumentY - startDocumentY);
    }

    function gutterLineVisualHeight(startLine, lineSpan) {
        const safeLineSpan = Math.max(1, Number(lineSpan) || 1);
        if (model.contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, model.contentsView.effectiveStructuredLogicalLineEntries().length);
            const safeStartLine = Math.max(1, Math.min(structuredLineCount, Number(startLine) || 1));
            if (safeLineSpan === 1)
                return model.contentsView.singleLineGutterHeight(safeStartLine);
            const startDocumentY = model.gutterLineDocumentY(safeStartLine);
            const nextLineNumber = safeStartLine + safeLineSpan;
            let endDocumentY = 0;
            if (nextLineNumber <= structuredLineCount)
                endDocumentY = model.gutterLineDocumentY(nextLineNumber);
            else
                endDocumentY = model.gutterDocumentOccupiedBottomY();
            return Math.max(model.contentsView.editorLineHeight, endDocumentY - startDocumentY);
        }
        const safeStartLine = Math.max(1, Math.min(model.contentsView.logicalLineCount, Number(startLine) || 1));
        if (safeLineSpan === 1)
            return model.contentsView.singleLineGutterHeight(safeStartLine);
        const startDocumentY = model.gutterLineDocumentY(safeStartLine);
        const nextLineNumber = safeStartLine + safeLineSpan;
        let endDocumentY = 0;
        if (nextLineNumber <= model.contentsView.logicalLineCount)
            endDocumentY = model.gutterLineDocumentY(nextLineNumber);
        else
            endDocumentY = model.gutterDocumentOccupiedBottomY();
        return Math.max(model.contentsView.editorLineHeight, endDocumentY - startDocumentY);
    }

    function lineY(lineNumber) {
        return model.contentsView.editorViewportYForDocumentY(model.lineDocumentY(lineNumber));
    }

    function gutterLineY(lineNumber) {
        return model.contentsView.editorViewportYForDocumentY(model.gutterLineDocumentY(lineNumber));
    }

    function logicalLineNumberForDocumentY(documentY) {
        if (model.contentsView.structuredHostGeometryActive) {
            const lineEntries = model.contentsView.effectiveStructuredLogicalLineEntries();
            if (lineEntries.length === 0)
                return 1;
            const safeDocumentY = Math.max(0, Number(documentY) || 0);
            let bestLineNumber = 1;
            for (let lineIndex = 0; lineIndex < lineEntries.length; ++lineIndex) {
                const entry = lineEntries[lineIndex] && typeof lineEntries[lineIndex] === "object"
                        ? lineEntries[lineIndex]
                        : ({});
                const lineTop = Math.max(0, Number(entry.contentY) || 0);
                const lineBottom = lineTop + Math.max(1, Number(entry.contentHeight) || model.contentsView.editorLineHeight);
                if (safeDocumentY < lineTop)
                    break;
                bestLineNumber = lineIndex + 1;
                if (safeDocumentY < lineBottom)
                    break;
            }
            return bestLineNumber;
        }
        if (model.contentsView.logicalLineCount <= 0)
            return 1;
        model.contentsView.ensureLogicalLineDocumentYCache();
        const safeDocumentY = Math.max(0, Number(documentY) || 0);
        let low = 0;
        let high = model.contentsView.logicalLineCount - 1;
        let best = 0;
        while (low <= high) {
            const middle = Math.floor((low + high) / 2);
            const middleY = model.lineDocumentY(middle + 1);
            if (middleY <= safeDocumentY) {
                best = middle;
                low = middle + 1;
            } else {
                high = middle - 1;
            }
        }
        return best + 1;
    }

    function markerColorForType(markerType) {
        const normalizedType = markerType === undefined || markerType === null ? "" : String(markerType).toLowerCase();
        if (normalizedType === "conflict")
            return model.contentsView.gutterMarkerConflictColor;
        if (normalizedType === "changed")
            return model.contentsView.gutterMarkerChangedColor;
        return model.contentsView.gutterMarkerChangedColor;
    }

    function markerHeight(markerSpec) {
        if (!markerSpec)
            return model.contentsView.editorLineHeight;
        return Math.max(1, model.gutterLineVisualHeight(markerSpec.startLine, markerSpec.lineSpan));
    }

    function markerY(markerSpec) {
        if (!markerSpec)
            return model.contentsView.editorDocumentStartY;
        const startLine = Math.max(1, Number(markerSpec.startLine) || 1);
        return model.gutterLineY(startLine);
    }

    function minimapContentHeight() {
        return Math.max(1, model.contentsView.editorOccupiedContentHeight());
    }

    function minimapContentYForLine(lineNumber) {
        const textStartY = model.contentsView.editorDocumentStartY;
        return textStartY + model.lineDocumentY(lineNumber);
    }

    function minimapCurrentVisualRow(rowsOverride) {
        const rows = Array.isArray(rowsOverride) ? rowsOverride : (Array.isArray(model.contentsView.minimapVisualRows) ? model.contentsView.minimapVisualRows : []);
        if (model.contentsView.structuredHostGeometryActive && rows.length > 0) {
            const resolvedBlockIndex = model.structuredDocumentFlow
                    && model.structuredDocumentFlow.normalizedResolvedInteractiveBlockIndex !== undefined
                    ? Number(model.structuredDocumentFlow.normalizedResolvedInteractiveBlockIndex())
                    : NaN;
            const fallbackBlockIndex = model.structuredDocumentFlow
                    && model.structuredDocumentFlow.activeBlockIndex !== undefined
                    ? Number(model.structuredDocumentFlow.activeBlockIndex)
                    : NaN;
            const safeBlockIndex = isFinite(resolvedBlockIndex) && resolvedBlockIndex >= 0
                    ? Math.floor(resolvedBlockIndex)
                    : (isFinite(fallbackBlockIndex) && fallbackBlockIndex >= 0
                       ? Math.floor(fallbackBlockIndex)
                       : -1);
            if (safeBlockIndex >= 0 && safeBlockIndex < rows.length)
                return rows[safeBlockIndex];
        }
        const textStartY = model.contentsView.editorDocumentStartY;
        const cursorRect = model.contentsView.currentCursorVisualRowRect();
        const cursorContentY = textStartY + (Number(cursorRect.y) || 0);
        for (let index = 0; index < rows.length; ++index) {
            const row = rows[index];
            const rowStart = Number(row.contentY) || 0;
            const rowEnd = rowStart + Math.max(1, Number(row.contentHeight) || model.contentsView.editorLineHeight);
            if (cursorContentY >= rowStart && cursorContentY < rowEnd)
                return row;
        }
        return rows.length > 0 ? rows[0] : ({
                "charCount": 0,
                "contentAvailableWidth": model.contentsView.minimapResolvedTrackWidth,
                "contentHeight": model.contentsView.editorLineHeight,
                "contentWidth": 0,
                "contentY": textStartY,
                "lineNumber": 1,
                "visualIndex": 0
            });
    }

    function minimapLineY(lineNumber) {
        const safeLineNumber = Math.max(1, Math.min(model.contentsView.logicalLineCount, Number(lineNumber) || 1));
        return model.viewportCoordinator.minimapTrackYForContentY(
                    model.minimapContentYForLine(safeLineNumber),
                    model.minimapContentHeight());
    }

    function minimapSilhouetteHeight(rowsOverride) {
        const rows = Array.isArray(rowsOverride) ? rowsOverride : (Array.isArray(model.contentsView.minimapVisualRows) ? model.contentsView.minimapVisualRows : []);
        if (rows.length === 0)
            return 1;
        const safeEditorLineHeight = Math.max(1, Number(model.contentsView.editorLineHeight) || 1);
        return Math.max(1, Math.ceil(model.minimapContentHeight() / safeEditorLineHeight));
    }

    function minimapVisualRowPaintHeight(rowSpec) {
        const safeContentHeight = model.minimapContentHeight();
        const safeRowContentHeight = Math.max(
                    1,
                    Number(rowSpec && rowSpec.contentHeight !== undefined ? rowSpec.contentHeight : 0)
                    || model.contentsView.editorLineHeight);
        return Math.max(
                    1,
                    model.viewportCoordinator.minimapTrackHeightForContentHeight(
                        safeRowContentHeight,
                        safeContentHeight));
    }

    function minimapVisualRowPaintY(rowSpec) {
        const safeContentHeight = model.minimapContentHeight();
        const safeRowContentY = Math.max(
                    0,
                    Number(rowSpec && rowSpec.contentY !== undefined ? rowSpec.contentY : 0) || 0);
        return model.viewportCoordinator.minimapTrackYForContentY(
                    safeRowContentY,
                    safeContentHeight);
    }
}
