pragma ComponentBehavior: Bound

import QtQuick
import "../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace
import "../../../models/editor/display/ContentsMinimapSnapshotSupport.js" as MinimapSnapshotSupport

QtObject {
    id: model

    property var contentsView: null
    property var editorProjection: null
    property var editorTypingController: null
    property var gutterCoordinator: null
    property var minimapCoordinator: null
    property var structuredDocumentFlow: null
    property var structuredFlowCoordinator: null
    property var viewportCoordinator: null

    function activeLogicalTextSnapshot() {
        if (model.editorTypingController && model.editorTypingController.currentEditorPlainText !== undefined) {
            const livePlainText = model.editorTypingController.currentEditorPlainText();
            if (livePlainText !== undefined && livePlainText !== null)
                return String(livePlainText);
        }
        if (model.editorProjection && model.editorProjection.logicalText !== undefined && model.editorProjection.logicalText !== null)
            return String(model.editorProjection.logicalText);
        if (model.contentsView.documentPresentationSourceText !== undefined
                && model.contentsView.documentPresentationSourceText !== null) {
            return String(model.contentsView.documentPresentationSourceText);
        }
        return "";
    }

    function normalizedSnapshotEntries(rawEntries) {
        if (rawEntries === undefined || rawEntries === null)
            return [];
        const explicitLength = Math.floor(Number(rawEntries.length) || 0);
        if (explicitLength <= 0)
            return [];
        const normalized = [];
        for (let index = 0; index < explicitLength; ++index)
            normalized.push(rawEntries[index]);
        return normalized;
    }

    function minimapSnapshotToken(entry, fallbackIndex) {
        const safeEntry = entry && typeof entry === "object" ? entry : ({});
        if (safeEntry.snapshotToken !== undefined && safeEntry.snapshotToken !== null)
            return String(safeEntry.snapshotToken);
        if (safeEntry.token !== undefined && safeEntry.token !== null)
            return String(safeEntry.token);
        if (safeEntry.text !== undefined && safeEntry.text !== null)
            return "text|" + String(safeEntry.text);
        return "line|" + String(Math.max(0, Math.floor(Number(fallbackIndex) || 0)));
    }

    function normalizedMinimapSnapshotText(rawText) {
        const normalizedText = rawText === undefined || rawText === null ? "" : String(rawText);
        return normalizedText
                .replace(/\r\n/g, "\n")
                .replace(/\r/g, "\n")
                .replace(/\u2028/g, "\n")
                .replace(/\u2029/g, "\n");
    }

    function plainMinimapSnapshotEntries(rawText) {
        const normalizedText = model.normalizedMinimapSnapshotText(rawText);
        const lines = normalizedText.length > 0 ? normalizedText.split("\n") : [""];
        const entries = [];
        for (let lineIndex = 0; lineIndex < lines.length; ++lineIndex) {
            entries.push({
                "lineNumber": lineIndex + 1,
                "snapshotToken": "text|" + lines[lineIndex]
            });
        }
        return entries;
    }

    function hasStructuredLogicalLineGeometry() {
        return model.contentsView.structuredHostGeometryActive
                && model.effectiveStructuredLogicalLineEntries().length > 0;
    }

    function effectiveStructuredMinimapEntries() {
        if (!model.contentsView.structuredHostGeometryActive
                || !model.structuredDocumentFlow
                || model.structuredDocumentFlow.normalizedBlocks === undefined) {
            return [];
        }
        return model.normalizedSnapshotEntries(
                    model.minimapCoordinator.buildStructuredMinimapSnapshotEntries(
                        model.structuredDocumentFlow.normalizedBlocks()));
    }

    function hasStructuredMinimapEntries() {
        return model.contentsView.structuredHostGeometryActive
                && model.effectiveStructuredMinimapEntries().length > 0;
    }

    function currentMinimapSnapshotEntries() {
        if (model.hasStructuredMinimapEntries())
            return model.effectiveStructuredMinimapEntries();
        return model.plainMinimapSnapshotEntries(model.activeLogicalTextSnapshot());
    }

    function minimapSnapshotEntriesEqual(previousEntries, nextEntries) {
        const normalizedPrevious = model.normalizedSnapshotEntries(previousEntries);
        const normalizedNext = model.normalizedSnapshotEntries(nextEntries);
        if (normalizedPrevious.length !== normalizedNext.length)
            return false;
        for (let index = 0; index < normalizedPrevious.length; ++index) {
            if (model.minimapSnapshotToken(normalizedPrevious[index], index)
                    !== model.minimapSnapshotToken(normalizedNext[index], index)) {
                return false;
            }
        }
        return true;
    }

    function normalizedStructuredLogicalLineEntries() {
        if (!model.contentsView.structuredHostGeometryActive || !model.structuredDocumentFlow)
            return [];
        const rawEntries = model.structuredDocumentFlow.cachedLogicalLineEntries !== undefined
                ? model.structuredDocumentFlow.cachedLogicalLineEntries
                : (model.structuredDocumentFlow.logicalLineEntries !== undefined
                   ? model.structuredDocumentFlow.logicalLineEntries()
                   : []);
        return model.structuredFlowCoordinator.normalizeStructuredLogicalLineEntries(rawEntries);
    }

    function structuredLogicalLineEntryAt(lineNumber) {
        const lineEntries = model.normalizedStructuredLogicalLineEntries();
        if (lineEntries.length === 0)
            return null;
        const safeLineNumber = Math.floor(Number(lineNumber) || 0);
        if (safeLineNumber < 1 || safeLineNumber > lineEntries.length)
            return null;
        const entry = lineEntries[safeLineNumber - 1];
        return entry && typeof entry === "object" ? entry : null;
    }

    function effectiveStructuredLogicalLineEntries() {
        return model.normalizedStructuredLogicalLineEntries();
    }

    function currentStructuredGutterGeometrySignature() {
        return model.viewportCoordinator.structuredGutterGeometrySignature(
                    model.effectiveStructuredLogicalLineEntries());
    }

    function consumeStructuredGutterGeometryChange() {
        const state = model.structuredFlowCoordinator.evaluateStructuredLayoutState(
                    model.effectiveStructuredLogicalLineEntries());
        model.contentsView.structuredGutterGeometrySignature = String(state.signature || "");
        return !!state.geometryChanged;
    }

    function buildStructuredMinimapLineGroupsForRange(startLineNumber, endLineNumber) {
        const snapshotEntries = model.effectiveStructuredMinimapEntries();
        const groups = model.minimapCoordinator.buildStructuredMinimapLineGroupsForRange(
                    snapshotEntries,
                    Number(startLineNumber) || 1,
                    Number(endLineNumber) || Number(startLineNumber) || 1,
                    model.contentsView.editorOccupiedContentHeight());
        return groups.length > 0
                ? groups
                : model.buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber);
    }

    function buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber) {
        const safeStartLine = Math.max(1, Math.min(model.contentsView.logicalLineCount, Number(startLineNumber) || 1));
        const safeEndLine = Math.max(safeStartLine, Math.min(model.contentsView.logicalLineCount, Number(endLineNumber) || safeStartLine));
        const lineCharacterCounts = [];
        const lineDocumentYs = [];
        const lineVisualHeights = [];
        for (let lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber) {
            lineCharacterCounts.push(model.viewportCoordinator.logicalLineCharacterCountAt(
                                         lineNumber - 1,
                                         model.contentsView.logicalLineStartOffsets));
            lineDocumentYs.push(model.contentsView.lineDocumentY(lineNumber));
            lineVisualHeights.push(model.contentsView.lineVisualHeight(lineNumber, 1));
        }
        return model.minimapCoordinator.buildFallbackMinimapLineGroupsForRange(
                    lineCharacterCounts,
                    lineDocumentYs,
                    lineVisualHeights,
                    safeStartLine,
                    safeEndLine);
    }

    function buildMinimapLineGroupsForRange(startLineNumber, endLineNumber) {
        const safeStartLine = Math.max(1, Math.min(model.contentsView.logicalLineCount, Number(startLineNumber) || 1));
        const safeEndLine = Math.max(safeStartLine, Math.min(model.contentsView.logicalLineCount, Number(endLineNumber) || safeStartLine));
        if (model.hasStructuredLogicalLineGeometry())
            return model.buildStructuredMinimapLineGroupsForRange(safeStartLine, safeEndLine);
        const editorWidth = 0;
        const editorContentHeight = 0;
        const lineCharacterCounts = [];
        const lineStartOffsets = [];
        const fallbackLineDocumentYs = [];
        const fallbackLineVisualHeights = [];
        const editorRects = [];
        const logicalLength = model.contentsView.resolvedLogicalTextLength;
        for (let lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber) {
            const lineIndex = lineNumber - 1;
            const startOffset = model.viewportCoordinator.logicalLineStartOffsetAt(
                        lineIndex,
                        model.contentsView.logicalLineStartOffsets);
            lineCharacterCounts.push(model.viewportCoordinator.logicalLineCharacterCountAt(
                                         lineIndex,
                                         model.contentsView.logicalLineStartOffsets));
            lineStartOffsets.push(startOffset);
            fallbackLineDocumentYs.push(model.contentsView.lineDocumentY(lineNumber));
            fallbackLineVisualHeights.push(model.contentsView.lineVisualHeight(lineNumber, 1));
        }
        return model.minimapCoordinator.buildEditorMinimapLineGroupsForRange(
                    lineCharacterCounts,
                    lineStartOffsets,
                    fallbackLineDocumentYs,
                    fallbackLineVisualHeights,
                    editorRects,
                    logicalLength,
                    safeStartLine,
                    safeEndLine,
                    editorWidth,
                    editorContentHeight);
    }

    function activeLineGeometryNoteId() {
        return model.viewportCoordinator.normalizedNoteId(model.contentsView.selectedNoteId);
    }

    function nextMinimapLineGroupsForCurrentState(currentSnapshotEntries) {
        const currentNoteId = model.activeLineGeometryNoteId();
        const useStructuredMinimap = model.hasStructuredMinimapEntries();
        const structuredLineCount = useStructuredMinimap
                ? model.effectiveStructuredMinimapEntries().length
                : 0;
        const snapshotPlan = model.minimapCoordinator.buildNextMinimapSnapshotPlan(
                    model.contentsView.minimapLineGroups,
                    model.contentsView.minimapLineGroupsNoteId,
                    currentNoteId,
                    model.contentsView.minimapSnapshotEntries,
                    currentSnapshotEntries,
                    model.contentsView.minimapSnapshotForceFullRefresh,
                    model.hasPendingNoteEntryGutterRefresh(currentNoteId),
                    structuredLineCount,
                    model.contentsView.logicalLineCount);
        if (snapshotPlan.reuseExisting)
            return model.contentsView.minimapLineGroups;
        if (snapshotPlan.requiresFullRebuild) {
            return useStructuredMinimap
                    ? model.buildStructuredMinimapLineGroupsForRange(1, Math.max(1, structuredLineCount))
                    : model.buildMinimapLineGroupsForRange(1, model.contentsView.logicalLineCount);
        }

        const replacementGroups = useStructuredMinimap
                ? model.buildStructuredMinimapLineGroupsForRange(
                      Number(snapshotPlan.replacementStartLine) || 1,
                      Number(snapshotPlan.replacementEndLine) || 1)
                : model.buildMinimapLineGroupsForRange(
                      Number(snapshotPlan.replacementStartLine) || 1,
                      Number(snapshotPlan.replacementEndLine) || 1);
        const mergedGroups = MinimapSnapshotSupport.spliceLineGroups(
                    model.contentsView.minimapLineGroups,
                    replacementGroups,
                    Number(snapshotPlan.previousStartLine) || 1,
                    Number(snapshotPlan.previousEndLine) || 1);
        const expectedLineCount = useStructuredMinimap
                ? Math.max(1, structuredLineCount)
                : model.contentsView.logicalLineCount;
        if (!Array.isArray(mergedGroups) || mergedGroups.length !== expectedLineCount) {
            return useStructuredMinimap
                    ? model.buildStructuredMinimapLineGroupsForRange(1, expectedLineCount)
                    : model.buildMinimapLineGroupsForRange(1, expectedLineCount);
        }
        return mergedGroups;
    }

    function buildVisibleGutterLineEntries() {
        if (model.contentsView.structuredHostGeometryActive) {
            return model.structuredFlowCoordinator.buildVisibleStructuredGutterLineEntries(
                        model.effectiveStructuredLogicalLineEntries(),
                        model.contentsView.firstVisibleLogicalLine());
        }
        const firstVisibleLine = model.contentsView.firstVisibleLogicalLine();
        const gutterLineYs = [];
        const gutterLineHeights = [];
        for (let lineNumber = firstVisibleLine; lineNumber <= model.contentsView.logicalLineCount; ++lineNumber) {
            gutterLineYs.push(model.contentsView.gutterLineY(lineNumber));
            gutterLineHeights.push(model.contentsView.gutterLineVisualHeight(lineNumber, 1));
        }
        return model.gutterCoordinator.buildVisiblePlainGutterLineEntries(
                    firstVisibleLine,
                    gutterLineYs,
                    gutterLineHeights);
    }

    function clampUnit(value) {
        return Math.max(0, Math.min(1, Number(value) || 0));
    }

    function logicalLineOffsetsEqual(previousOffsets, nextOffsets) {
        const normalizedPrevious = Array.isArray(previousOffsets) && previousOffsets.length > 0 ? previousOffsets : [0];
        const normalizedNext = Array.isArray(nextOffsets) && nextOffsets.length > 0 ? nextOffsets : [0];
        if (normalizedPrevious.length !== normalizedNext.length)
            return false;
        for (let offsetIndex = 0; offsetIndex < normalizedPrevious.length; ++offsetIndex) {
            if ((Number(normalizedPrevious[offsetIndex]) || 0) !== (Number(normalizedNext[offsetIndex]) || 0))
                return false;
        }
        return true;
    }

    function applyLiveLogicalLineMetrics(logicalTextLength, lineStartOffsets, lineCount) {
        const normalizedLogicalTextLength = Math.max(0, Number(logicalTextLength) || 0);
        const normalizedLineStartOffsets = Array.isArray(lineStartOffsets) && lineStartOffsets.length > 0
                ? lineStartOffsets.slice(0)
                : [0];
        const normalizedLineCount = Math.max(1, Number(lineCount) || normalizedLineStartOffsets.length || 1);
        const lineCountChanged = model.contentsView.liveLogicalLineCount !== normalizedLineCount;
        const textLengthChanged = model.contentsView.liveLogicalTextLength !== normalizedLogicalTextLength;
        const lineOffsetsChanged = !model.logicalLineOffsetsEqual(
                    model.contentsView.liveLogicalLineStartOffsets,
                    normalizedLineStartOffsets);
        if (textLengthChanged)
            model.contentsView.liveLogicalTextLength = normalizedLogicalTextLength;
        if (lineOffsetsChanged)
            model.contentsView.liveLogicalLineStartOffsets = normalizedLineStartOffsets;
        if (lineCountChanged)
            model.contentsView.liveLogicalLineCount = normalizedLineCount;
        return lineCountChanged || textLengthChanged || lineOffsetsChanged;
    }

    function hasPendingNoteEntryGutterRefresh(noteId) {
        return model.viewportCoordinator.hasPendingNoteEntryGutterRefresh(
                    model.contentsView.pendingNoteEntryGutterRefreshNoteId,
                    noteId === undefined ? null : String(noteId));
    }

    function finalizePendingNoteEntryGutterRefresh(noteId, reason, refreshStructuredLayout) {
        const plan = model.viewportCoordinator.finalizePendingNoteEntryGutterRefresh(
                    model.contentsView.pendingNoteEntryGutterRefreshNoteId,
                    noteId === undefined || noteId === null ? "" : String(noteId),
                    model.contentsView.selectedNoteBodyLoading,
                    reason === undefined || reason === null ? "" : String(reason),
                    !!refreshStructuredLayout);
        if (!plan.clearPendingNoteId)
            return false;
        model.contentsView.pendingNoteEntryGutterRefreshNoteId = "";
        if (plan.refreshStructuredLayoutNow
                && model.structuredDocumentFlow
                && model.structuredDocumentFlow.refreshLayoutCache !== undefined) {
            model.structuredDocumentFlow.refreshLayoutCache();
        }
        if (plan.commitGutterRefresh)
            model.commitGutterRefresh();
        if (plan.scheduleViewportGutterRefresh)
            model.contentsView.scheduleViewportGutterRefresh();
        if (plan.scheduleMinimapSnapshotRefresh)
            model.contentsView.scheduleMinimapSnapshotRefresh(!!plan.scheduleMinimapSnapshotForceFull);
        if (plan.scheduleGutterRefresh) {
            model.contentsView.scheduleGutterRefresh(Number(plan.gutterPassCount) || 0,
                                               String(plan.gutterReason || ""));
        }
        return true;
    }

    function commitGutterRefresh() {
        model.contentsView.refreshLiveLogicalLineMetrics();
        model.contentsView.gutterRefreshRevision += 1;
        model.contentsView.visibleGutterLineEntries = model.buildVisibleGutterLineEntries();
        model.traceVisibleGutterSnapshot("commit");
    }

    function refreshVisibleGutterEntries() {
        model.contentsView.visibleGutterLineEntries = model.buildVisibleGutterLineEntries();
        model.traceVisibleGutterSnapshot("refresh");
    }

    function traceVisibleGutterSnapshot(reason) {
        if (!model.contentsView.hasSelectedNote)
            return;
        const startLine = 14;
        const endLine = Math.min(model.contentsView.logicalLineCount, 27);
        const parts = [];
        for (let lineNumber = startLine; lineNumber <= endLine; ++lineNumber) {
            const lineGroup = model.contentsView.incrementalLineGeometryAvailable()
                    ? model.contentsView.minimapLineGroups[lineNumber - 1]
                    : null;
            parts.push("L" + lineNumber
                       + "{lineY=" + Math.round(model.contentsView.lineY(lineNumber))
                       + ",gutterY=" + Math.round(model.contentsView.gutterLineY(lineNumber))
                       + ",lineH=" + Math.round(model.contentsView.lineVisualHeight(lineNumber, 1))
                       + ",gutterH=" + Math.round(model.contentsView.gutterLineVisualHeight(lineNumber, 1))
                       + ",rows=" + Math.max(1, Math.ceil(Number(lineGroup && lineGroup.rowCount !== undefined ? lineGroup.rowCount : 1) || 1))
                       + "}");
        }
        EditorTrace.trace("displayView",
                          "gutterSnapshot",
                          "reason=" + reason
                          + " count=" + model.contentsView.visibleGutterLineEntries.length
                          + " lines=[" + parts.join(", ") + "]",
                          model.contentsView);
    }
}
