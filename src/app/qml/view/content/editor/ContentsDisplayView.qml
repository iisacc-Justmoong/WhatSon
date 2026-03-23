pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: contentsView

    readonly property color activeLineNumberColor: "#9DA0A8"
    readonly property bool contentPersistenceContractAvailable: selectionBridge.contentPersistenceContractAvailable
    property var contentViewModel: null
    readonly property int currentCursorLineNumber: textMetricsBridge.logicalLineNumberForOffset(Number(contentEditor.cursorPosition) || 0)
    readonly property color decorativeMarkerYellow: "#FFF567"
    property color displayColor: "transparent"
    property bool drawerVisible: true
    property color drawerColor: "transparent"
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property int editorBottomInset: 16
    property alias editorBoundNoteId: editorSession.editorBoundNoteId
    readonly property real editorContentOffsetY: {
        if (!contentEditor.editorItem || !contentEditor.editorItem.parent)
            return 0;
        return Number(contentEditor.editorItem.parent.y) || 0;
    }
    readonly property real editorDocumentStartY: {
        return contentsView.effectiveEditorTopInset;
    }
    readonly property int editorDocumentTopPadding: 0
    readonly property var editorFlickable: contentsView.resolveEditorFlickable()
    readonly property int editorHorizontalInset: 16
    readonly property real editorLineHeight: contentsView.editorTextLineBoxHeight
    property alias editorText: editorSession.editorText
    readonly property int editorTextLineBoxHeight: 12
    readonly property int editorTopInset: LV.Theme.gap4
    property int editorTopInsetOverride: -1
    readonly property int effectiveEditorTopInset: contentsView.editorTopInsetOverride >= 0 ? contentsView.editorTopInsetOverride : contentsView.editorTopInset
    readonly property var effectiveGutterMarkers: {
        const normalizedMarkers = [];
        if (contentsView.showCurrentLineMarker) {
            normalizedMarkers.push({
                "color": contentsView.gutterMarkerCurrentColor,
                "lineSpan": 1,
                "startLine": contentsView.currentCursorLineNumber,
                "type": "current"
            });
        }
        const externalMarkers = Array.isArray(contentsView.normalizedExternalGutterMarkers) ? contentsView.normalizedExternalGutterMarkers : [];
        for (let i = 0; i < externalMarkers.length; ++i) {
            const marker = externalMarkers[i];
            normalizedMarkers.push({
                "color": contentsView.markerColorForType(marker.type),
                "lineSpan": marker.lineSpan,
                "startLine": marker.startLine,
                "type": marker.type
            });
        }
        return normalizedMarkers;
    }
    readonly property int frameHorizontalInset: 2
    property int frameHorizontalInsetOverride: -1
    readonly property int effectiveFrameHorizontalInset: contentsView.frameHorizontalInsetOverride >= 0 ? contentsView.frameHorizontalInsetOverride : contentsView.frameHorizontalInset
    property color gutterColor: "transparent"
    readonly property int gutterCommentMarkerOffset: 2
    readonly property int gutterCommentRailLeft: 4
    readonly property int gutterCommentRailWidth: 10
    readonly property int gutterIconRailLeft: 40
    readonly property int gutterIconRailWidth: 18
    readonly property color gutterMarkerChangedColor: contentsView.decorativeMarkerYellow
    readonly property color gutterMarkerConflictColor: LV.Theme.danger
    readonly property color gutterMarkerCurrentColor: LV.Theme.primary
    property var gutterMarkers: []
    property int gutterRefreshPassesRemaining: 0
    property int gutterRefreshRevision: 0
    readonly property real gutterViewportHeight: editorViewport ? Number(editorViewport.height) || 0 : 0
    readonly property int gutterWidth: 74
    property int gutterWidthOverride: -1
    readonly property int effectiveGutterWidth: contentsView.gutterWidthOverride >= 0 ? contentsView.gutterWidthOverride : contentsView.gutterWidth
    readonly property bool hasSelectedNote: contentsView.selectedNoteId.length > 0
    readonly property color lineNumberColor: "#4E5157"
    readonly property int lineNumberColumnLeft: 14
    readonly property int lineNumberColumnTextWidth: contentsView.gutterWidth - contentsView.lineNumberColumnLeft - contentsView.lineNumberRightInset
    property int lineNumberColumnLeftOverride: -1
    property int lineNumberColumnTextWidthOverride: -1
    readonly property int effectiveLineNumberColumnLeft: contentsView.lineNumberColumnLeftOverride >= 0 ? contentsView.lineNumberColumnLeftOverride : contentsView.lineNumberColumnLeft
    readonly property int effectiveLineNumberColumnTextWidth: contentsView.lineNumberColumnTextWidthOverride >= 0 ? contentsView.lineNumberColumnTextWidthOverride : contentsView.lineNumberColumnTextWidth
    readonly property int lineNumberColumnWidth: 26
    readonly property int lineNumberRightInset: contentsView.editorHorizontalInset
    readonly property int logicalLineCount: Math.max(1, Number(textMetricsBridge.logicalLineCount) || 1)
    readonly property var logicalLineStartOffsets: textMetricsBridge.logicalLineStartOffsets
    property var libraryHierarchyViewModel: null
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    readonly property int minEditorHeight: LV.Theme.gap20 * 12
    readonly property color minimapCurrentLineColor: contentsView.activeLineNumberColor
    readonly property color minimapLineColor: contentsView.lineNumberColor
    readonly property int minimapOuterWidth: 56
    property bool minimapVisible: true
    readonly property int minimapRowGap: 1
    readonly property int minimapTrackInset: 8
    readonly property int minimapTrackWidth: 36
    readonly property real minimapTrackRuntimeHeight: minimapLayer ? minimapLayer.trackHeight : 0
    readonly property real minimapTrackRuntimeWidth: minimapLayer ? minimapLayer.trackWidth : contentsView.minimapTrackWidth
    readonly property color minimapViewportFillColor: "#149DA0A8"
    readonly property int minimapViewportMinHeight: 28
    readonly property var minimapVisualRows: contentsView.buildMinimapVisualRows(contentsView.editorText, Number(contentEditor ? contentEditor.width : 0), Number(contentEditor ? contentEditor.contentHeight : 0))
    readonly property var normalizedExternalGutterMarkers: gutterMarkerBridge.normalizedExternalGutterMarkers
    readonly property bool noteCountContractAvailable: selectionBridge.noteCountContractAvailable
    property var noteListModel: null
    readonly property bool noteSelectionContractAvailable: selectionBridge.noteSelectionContractAvailable
    property var panelViewModel: null
    property string pendingEditorFocusNoteId: ""
    property alias pendingBodySave: editorSession.pendingBodySave
    readonly property int saveDebounceMs: 120
    readonly property string selectedNoteBodyText: selectionBridge.selectedNoteBodyText
    readonly property string selectedNoteId: selectionBridge.selectedNoteId
    readonly property bool showCurrentLineMarker: contentsView.hasSelectedNote || contentsView.editorText.length > 0 || contentEditor.focused
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property alias syncingEditorTextFromModel: editorSession.syncingEditorTextFromModel
    readonly property real textOriginY: {
        return contentsView.editorDocumentStartY + contentsView.editorContentOffsetY;
    }
    readonly property int visibleNoteCount: selectionBridge.visibleNoteCount

    signal drawerHeightDragRequested(int value)
    signal editorTextEdited(string text)
    signal viewHookRequested

    function buildFallbackMinimapVisualRows(textStartY) {
        const rows = [];
        for (let lineNumber = 1; lineNumber <= contentsView.logicalLineCount; ++lineNumber) {
            const characterCount = textMetricsBridge.logicalLineCharacterCountAt(lineNumber - 1);
            const rowCount = Math.max(1, Math.round(contentsView.lineVisualHeight(lineNumber, 1) / Math.max(1, contentsView.editorLineHeight)));
            for (let rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
                const segmentStart = Math.floor(rowIndex * characterCount / rowCount);
                const segmentEnd = Math.ceil((rowIndex + 1) * characterCount / rowCount);
                rows.push({
                    "charCount": Math.max(0, segmentEnd - segmentStart),
                    "contentHeight": contentsView.editorLineHeight,
                    "contentY": textStartY + contentsView.lineDocumentY(lineNumber) + rowIndex * contentsView.editorLineHeight,
                    "lineNumber": lineNumber,
                    "visualIndex": rows.length
                });
            }
        }
        if (rows.length === 0) {
            rows.push({
                "charCount": 0,
                "contentHeight": contentsView.editorLineHeight,
                "contentY": textStartY,
                "lineNumber": 1,
                "visualIndex": 0
            });
        }

        return rows;
    }
    function buildMinimapVisualRows(text, editorWidth, editorContentHeight) {
        const _editorWidth = Number(editorWidth) || 0;
        const _editorContentHeight = Number(editorContentHeight) || 0;
        const value = text === undefined || text === null ? "" : String(text);
        const textStartY = contentsView.editorDocumentStartY;
        if (!contentEditor.editorItem || contentEditor.editorItem.positionToRectangle === undefined || _editorWidth <= 0 || _editorContentHeight <= 0)
            return contentsView.buildFallbackMinimapVisualRows(textStartY);

        const rows = [];
        if (value.length === 0) {
            rows.push({
                "charCount": 0,
                "contentHeight": contentsView.editorLineHeight,
                "contentY": textStartY,
                "lineNumber": 1,
                "visualIndex": 0
            });
            return rows;
        }

        let segmentStartOffset = 0;
        let segmentStartY = Number(contentEditor.editorItem.positionToRectangle(0).y) || 0;
        for (let offset = 1; offset <= value.length; ++offset) {
            let nextY = segmentStartY + contentsView.editorLineHeight;
            if (offset < value.length) {
                const nextRect = contentEditor.editorItem.positionToRectangle(offset);
                const nextRectY = Number(nextRect.y);
                if (isFinite(nextRectY))
                    nextY = nextRectY;
            }
            const rowBreak = offset === value.length || nextY !== segmentStartY;
            if (!rowBreak)
                continue;

            let characterCount = offset - segmentStartOffset;
            if (offset > segmentStartOffset && value.charAt(offset - 1) === "\n")
                characterCount -= 1;

            rows.push({
                "charCount": Math.max(0, characterCount),
                "contentHeight": Math.max(1, nextY - segmentStartY),
                "contentY": textStartY + segmentStartY,
                "lineNumber": textMetricsBridge.logicalLineNumberForOffset(segmentStartOffset),
                "visualIndex": rows.length
            });
            segmentStartOffset = offset;
            segmentStartY = nextY;
        }

        return rows.length > 0 ? rows : contentsView.buildFallbackMinimapVisualRows(textStartY);
    }
    function clampDrawerHeight(value) {
        var maxDrawer = Math.max(contentsView.minDrawerHeight, contentsView.height - contentsView.minDisplayHeight - contentsView.splitterThickness);
        return Math.max(contentsView.minDrawerHeight, Math.min(maxDrawer, value));
    }
    function clampUnit(value) {
        return Math.max(0, Math.min(1, Number(value) || 0));
    }
    function commitGutterRefresh() {
        contentsView.gutterRefreshRevision += 1;
        if (minimapLayer)
            minimapLayer.requestRepaint();
    }
    function currentCursorVisualLineHeight() {
        const rect = contentsView.currentCursorVisualRowRect();
        return Math.max(1, Number(rect.height) || contentsView.editorLineHeight);
    }
    function currentCursorVisualLineY() {
        const rect = contentsView.currentCursorVisualRowRect();
        return contentsView.editorViewportYForDocumentY(Number(rect.y) || 0);
    }
    function currentCursorVisualRowRect() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const safeOffset = Math.max(0, Math.min(contentsView.editorText.length, Number(contentEditor.cursorPosition) || 0));
        if (contentEditor.editorItem && contentEditor.editorItem.positionToRectangle !== undefined) {
            const rect = contentEditor.editorItem.positionToRectangle(safeOffset);
            return {
                "height": Math.max(1, Number(rect.height) || contentsView.editorLineHeight),
                "width": Math.max(0, Number(rect.width) || 0),
                "y": Number(rect.y) || 0
            };
        }
        return {
            "height": contentsView.editorLineHeight,
            "width": 0,
            "y": contentsView.documentYForOffset(safeOffset)
        };
    }
    function documentOccupiedBottomY() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const text = contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText);
        if (contentEditor.editorItem && contentEditor.editorItem.positionToRectangle !== undefined) {
            const rect = contentEditor.editorItem.positionToRectangle(Math.max(0, text.length));
            const rectY = Number(rect.y);
            const rectHeight = Number(rect.height);
            if (isFinite(rectY))
                return Math.max(contentsView.editorLineHeight, rectY + Math.max(contentsView.editorLineHeight, isFinite(rectHeight) ? rectHeight : contentsView.editorLineHeight));
        }
        return Math.max(contentsView.editorLineHeight, contentsView.logicalLineCount * contentsView.editorLineHeight);
    }
    function documentYForOffset(offset) {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const safeOffset = Math.max(0, Number(offset) || 0);
        if (!contentEditor.editorItem || contentEditor.editorItem.positionToRectangle === undefined) {
            const fallbackLineNumber = textMetricsBridge.logicalLineNumberForOffset(safeOffset);
            return (fallbackLineNumber - 1) * contentsView.editorLineHeight;
        }
        const rect = contentEditor.editorItem.positionToRectangle(safeOffset);
        return Number(rect.y) || 0;
    }
    function editorOccupiedContentHeight() {
        const textStartY = contentsView.editorDocumentStartY;
        return Math.max(textStartY + contentsView.documentOccupiedBottomY(), textStartY + contentsView.editorLineHeight);
    }
    function editorViewportYForDocumentY(documentY) {
        const editorY = contentsView.editorDocumentStartY;
        return editorY + documentY + contentsView.editorContentOffsetY;
    }
    function firstVisibleLogicalLine() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return 1;
        const contentY = Math.max(0, Number(flickable.contentY) || 0);
        const firstVisibleDocumentY = Math.max(0, contentY - contentsView.editorDocumentStartY);
        return Math.max(1, Math.min(contentsView.logicalLineCount, contentsView.logicalLineNumberForDocumentY(firstVisibleDocumentY)));
    }
    function lineDocumentY(lineNumber) {
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        return contentsView.documentYForOffset(textMetricsBridge.logicalLineStartOffsetAt(safeLineNumber - 1));
    }
    function lineVisualHeight(startLine, lineSpan) {
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLine) || 1));
        const safeLineSpan = Math.max(1, Number(lineSpan) || 1);
        const startDocumentY = contentsView.lineDocumentY(safeStartLine);
        const nextLineNumber = safeStartLine + safeLineSpan;
        let endDocumentY = 0;
        if (nextLineNumber <= contentsView.logicalLineCount) {
            endDocumentY = contentsView.lineDocumentY(nextLineNumber);
        } else {
            endDocumentY = contentsView.documentOccupiedBottomY();
        }
        return Math.max(contentsView.editorLineHeight, endDocumentY - startDocumentY);
    }
    function lineY(lineNumber) {
        return contentsView.editorViewportYForDocumentY(contentsView.lineDocumentY(lineNumber));
    }
    function logicalLineNumberForDocumentY(documentY) {
        const offsets = contentsView.logicalLineStartOffsets;
        if (!offsets || offsets.length === undefined || offsets.length === 0)
            return 1;
        const safeDocumentY = Math.max(0, Number(documentY) || 0);
        let low = 0;
        let high = offsets.length - 1;
        let best = 0;
        while (low <= high) {
            const middle = Math.floor((low + high) / 2);
            const middleY = contentsView.documentYForOffset(offsets[middle]);
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
            return contentsView.gutterMarkerConflictColor;
        if (normalizedType === "changed")
            return contentsView.gutterMarkerChangedColor;
        if (normalizedType === "current")
            return contentsView.gutterMarkerCurrentColor;

    }
    function markerHeight(markerSpec) {
        const markerType = markerSpec && markerSpec.type !== undefined ? String(markerSpec.type).toLowerCase() : "";
        if (markerType === "current")
            return contentsView.currentCursorVisualLineHeight();
        if (!markerSpec)
            return contentsView.editorLineHeight;
        return Math.max(1, contentsView.lineVisualHeight(markerSpec.startLine, markerSpec.lineSpan));
    }
    function markerY(markerSpec) {
        const markerType = markerSpec && markerSpec.type !== undefined ? String(markerSpec.type).toLowerCase() : "";
        if (markerType === "current")
            return contentsView.currentCursorVisualLineY();
        if (!markerSpec)
            return contentsView.editorDocumentStartY;
        const startLine = Math.max(1, Number(markerSpec.startLine) || 1);
        return contentsView.lineY(startLine);
    }
    function minimapBarWidth(characterCount) {
        const safeCount = Math.max(0, Number(characterCount) || 0);
        const maxWidth = Math.max(6, contentsView.minimapTrackRuntimeWidth - 1);
        if (safeCount <= 0)
            return Math.max(2, maxWidth * 0.08);
        const widthRatio = contentsView.clampUnit(0.08 + Math.log(safeCount + 1) / Math.log(160));
        return Math.max(4, maxWidth * widthRatio);
    }
    function minimapContentHeight() {
        return Math.max(1, contentsView.editorOccupiedContentHeight());
    }
    function minimapContentYForLine(lineNumber) {
        const textStartY = contentsView.editorDocumentStartY;
        return textStartY + contentsView.lineDocumentY(lineNumber);
    }
    function minimapCurrentLineHeight() {
        return contentsView.minimapVisualRowPaintHeight(contentsView.minimapCurrentVisualRow());
    }
    function minimapCurrentLineWidth() {
        const visualRow = contentsView.minimapCurrentVisualRow();
        return contentsView.minimapBarWidth(visualRow.charCount);
    }
    function minimapCurrentLineY() {
        return contentsView.minimapVisualRowPaintY(contentsView.minimapCurrentVisualRow());
    }
    function minimapCurrentVisualRow() {
        const rows = Array.isArray(contentsView.minimapVisualRows) ? contentsView.minimapVisualRows : [];
        const textStartY = contentsView.editorDocumentStartY;
        const cursorRect = contentsView.currentCursorVisualRowRect();
        const cursorContentY = textStartY + (Number(cursorRect.y) || 0);
        for (let index = 0; index < rows.length; ++index) {
            const row = rows[index];
            const rowStart = Number(row.contentY) || 0;
            const rowEnd = rowStart + Math.max(1, Number(row.contentHeight) || contentsView.editorLineHeight);
            if (cursorContentY >= rowStart && cursorContentY < rowEnd)
                return row;
        }
        return rows.length > 0 ? rows[0] : ({
                "charCount": 0,
                "contentHeight": contentsView.editorLineHeight,
                "contentY": textStartY,
                "lineNumber": 1,
                "visualIndex": 0
            });
    }
    function minimapLineY(lineNumber) {
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        return contentsView.minimapTrackYForContentY(contentsView.minimapContentYForLine(safeLineNumber));
    }
    function minimapSilhouetteHeight() {
        const rows = Array.isArray(contentsView.minimapVisualRows) ? contentsView.minimapVisualRows : [];
        if (rows.length === 0)
            return contentsView.minimapVisualRowPaintHeight(null);
        return rows.length * contentsView.minimapVisualRowPaintHeight(rows[0]) + Math.max(0, rows.length - 1) * contentsView.minimapRowGap;
    }
    function minimapTrackHeightForContentHeight(segmentHeight) {
        const safeSegmentHeight = Math.max(0, Number(segmentHeight) || 0);
        const contentHeight = contentsView.minimapContentHeight();
        if (contentHeight <= 0)
            return 0;
        return Math.max(1, (safeSegmentHeight / contentHeight) * contentsView.minimapTrackRuntimeHeight);
    }
    function minimapTrackYForContentY(contentY) {
        const contentHeight = contentsView.minimapContentHeight();
        if (contentHeight <= 0)
            return 0;
        const safeContentY = Math.max(0, Math.min(contentHeight, Number(contentY) || 0));
        return (safeContentY / contentHeight) * contentsView.minimapTrackRuntimeHeight;
    }
    function minimapViewportHeight() {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return contentsView.minimapTrackRuntimeHeight;
        const contentHeight = Math.max(1, contentsView.minimapContentHeight());
        const viewportHeight = Math.max(0, Number(flickable.height) || 0);
        if (contentHeight <= viewportHeight)
            return contentsView.minimapTrackRuntimeHeight;
        const proportionalHeight = contentsView.minimapTrackHeightForContentHeight(viewportHeight);
        return Math.min(contentsView.minimapTrackRuntimeHeight, Math.max(contentsView.minimapViewportMinHeight, proportionalHeight));
    }
    function minimapViewportY() {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return 0;
        const contentHeight = Math.max(1, contentsView.minimapContentHeight());
        const viewportHeight = Math.max(0, Number(flickable.height) || 0);
        const maxContentY = Math.max(0, contentHeight - viewportHeight);
        if (maxContentY <= 0)
            return 0;
        const contentY = Math.max(0, Math.min(maxContentY, Number(flickable.contentY) || 0));
        const maxTrackY = Math.max(0, contentsView.minimapTrackRuntimeHeight - contentsView.minimapViewportHeight());
        return maxTrackY * (contentY / maxContentY);
    }
    function minimapVisualRowPaintHeight(rowSpec) {
        return 1;
    }
    function minimapVisualRowPaintY(rowSpec) {
        const visualIndex = rowSpec && rowSpec.visualIndex !== undefined ? Math.max(0, Number(rowSpec.visualIndex) || 0) : 0;
        return visualIndex * (contentsView.minimapRowGap + contentsView.minimapVisualRowPaintHeight(rowSpec));
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function focusEditorForPendingNote() {
        const pendingNoteId = contentsView.pendingEditorFocusNoteId === undefined || contentsView.pendingEditorFocusNoteId === null
                ? ""
                : String(contentsView.pendingEditorFocusNoteId).trim();
        const selectedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null
                ? ""
                : String(contentsView.selectedNoteId).trim();
        if (pendingNoteId.length === 0 || pendingNoteId !== selectedNoteId || !contentsView.hasSelectedNote)
            return;

        contentsView.pendingEditorFocusNoteId = "";
        Qt.callLater(function () {
            const activeNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null
                    ? ""
                    : String(contentsView.selectedNoteId).trim();
            if (activeNoteId !== pendingNoteId)
                return;

            const cursorPosition = contentsView.editorText === undefined || contentsView.editorText === null
                    ? 0
                    : String(contentsView.editorText).length;
            contentEditor.forceActiveFocus();
            if (contentEditor.editorItem && contentEditor.editorItem.forceActiveFocus !== undefined)
                contentEditor.editorItem.forceActiveFocus();
            if (contentEditor.cursorPosition !== undefined)
                contentEditor.cursorPosition = cursorPosition;
            if (contentEditor.editorItem && contentEditor.editorItem.cursorPosition !== undefined)
                contentEditor.editorItem.cursorPosition = cursorPosition;
        });
    }
    function scheduleEditorFocusForNote(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length === 0)
            return;

        contentsView.pendingEditorFocusNoteId = normalizedNoteId;
        contentsView.focusEditorForPendingNote();
    }
    function resolveEditorFlickable() {
        if (!contentEditor.editorItem || !contentEditor.editorItem.parent)
            return null;
        const candidate = contentEditor.editorItem.parent.parent;
        if (!candidate || candidate.contentY === undefined || candidate.contentHeight === undefined || candidate.height === undefined)
            return null;

        return candidate;
    }
    function scheduleGutterRefresh(passCount) {
        const requestedPassCount = Math.max(1, Number(passCount) || 1);
        contentsView.gutterRefreshPassesRemaining = Math.max(contentsView.gutterRefreshPassesRemaining, requestedPassCount);
        if (!gutterRefreshTimer.running)
            gutterRefreshTimer.start();
    }
    function scrollEditorViewportToMinimapPosition(localY) {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return;
        const contentHeight = Math.max(1, Number(flickable.contentHeight) || 0);
        const viewportHeight = Math.max(0, Number(flickable.height) || 0);
        const maxContentY = Math.max(0, contentHeight - viewportHeight);
        if (maxContentY <= 0) {
            flickable.contentY = 0;
            return;
        }
        const trackRatio = contentsView.clampUnit((Number(localY) || 0) / Math.max(1, contentsView.minimapTrackRuntimeHeight));
        const documentY = contentHeight * trackRatio;
        const nextContentY = Math.max(0, Math.min(maxContentY, documentY - viewportHeight / 2));
        flickable.contentY = nextContentY;
    }
    function visibleLineNumbers() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const visibleLines = [];
        const firstVisibleLine = contentsView.firstVisibleLogicalLine();
        for (let lineNumber = firstVisibleLine; lineNumber <= contentsView.logicalLineCount; ++lineNumber) {
            const lineY = contentsView.lineY(lineNumber);
            if (lineY > contentsView.gutterViewportHeight)
                break;
            if (lineY + contentsView.lineVisualHeight(lineNumber, 1) < 0)
                continue;
            visibleLines.push(lineNumber);
        }
        if (visibleLines.length === 0)
            visibleLines.push(firstVisibleLine);

        return visibleLines;
    }

    clip: true

    Component.onCompleted: {
        editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);
        contentsView.scheduleGutterRefresh(4);
    }
    Component.onDestruction: editorSession.flushPendingEditorText()
    onEditorFlickableChanged: contentsView.scheduleGutterRefresh(2)
    onEditorTextChanged: {
        if (minimapLayer)
            minimapLayer.requestRepaint();
    }
    onHeightChanged: contentsView.scheduleGutterRefresh(2)
    onSelectedNoteBodyTextChanged: {
        if (editorSession.shouldAcceptModelBodyText(contentsView.selectedNoteId, contentsView.selectedNoteBodyText)) {
            editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);
        } else {
            editorSession.scheduleEditorPersistence();
        }
        contentsView.scheduleGutterRefresh(4);
    }
    onSelectedNoteIdChanged: {
        if (contentsView.pendingBodySave && contentsView.editorBoundNoteId !== contentsView.selectedNoteId)
            editorSession.flushPendingEditorText();
        editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);
        contentsView.focusEditorForPendingNote();
        contentsView.scheduleGutterRefresh(4);
    }
    onVisibleChanged: {
        if (visible)
            contentsView.scheduleGutterRefresh(4);
    }
    onWidthChanged: {
        contentsView.scheduleGutterRefresh(2);
    }

    ContentsEditorSelectionBridge {
        id: selectionBridge

        contentViewModel: contentsView.contentViewModel
        noteListModel: contentsView.noteListModel
    }
    ContentsLogicalTextBridge {
        id: textMetricsBridge

        text: contentsView.editorText
    }
    ContentsGutterMarkerBridge {
        id: gutterMarkerBridge

        gutterMarkers: contentsView.gutterMarkers
    }
    ContentsEditorSession {
        id: editorSession

        saveDebounceMs: contentsView.saveDebounceMs
        selectionBridge: selectionBridge
    }
    Timer {
        id: gutterRefreshTimer

        interval: 16
        repeat: true

        onTriggered: {
            contentsView.commitGutterRefresh();
            if (contentsView.gutterRefreshPassesRemaining <= 1) {
                contentsView.gutterRefreshPassesRemaining = 0;
                stop();
                return;
            }
            contentsView.gutterRefreshPassesRemaining -= 1;
        }
    }
    Connections {
        function onEmptyNoteCreated(noteId) {
            contentsView.scheduleEditorFocusForNote(noteId);
        }

        ignoreUnknownSignals: true
        target: contentsView.libraryHierarchyViewModel
    }
    Connections {
        function onEditorTextSynchronized() {
            contentsView.scheduleGutterRefresh(4);
        }

        target: editorSession
    }
    Connections {
        function onContentHeightChanged() {
            contentsView.scheduleGutterRefresh(2);
        }
        function onLineCountChanged() {
            contentsView.scheduleGutterRefresh(2);
        }

        target: contentEditor
    }
    Connections {
        function onContentYChanged() {
            contentsView.scheduleGutterRefresh(1);
        }
        function onHeightChanged() {
            contentsView.scheduleGutterRefresh(2);
        }
        function onWidthChanged() {
            contentsView.scheduleGutterRefresh(2);
        }

        target: contentsView.editorFlickable
    }
    LV.VStack {
        id: drawerView

        anchors.fill: parent
        spacing: LV.Theme.gapNone

        Rectangle {
            id: contentsDisplayView

            Layout.fillHeight: true
            Layout.fillWidth: true
            color: contentsView.displayColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: contentsView.effectiveFrameHorizontalInset
                anchors.rightMargin: contentsView.effectiveFrameHorizontalInset
                spacing: 0
                visible: contentsView.hasSelectedNote

                ContentsGutterLayer {
                    id: gutterLayer

                    Layout.fillHeight: true
                    Layout.preferredWidth: contentsView.effectiveGutterWidth
                    activeLineNumberColor: contentsView.activeLineNumberColor
                    currentCursorLineNumber: contentsView.currentCursorLineNumber
                    editorLineHeight: contentsView.editorLineHeight
                    effectiveGutterMarkers: contentsView.effectiveGutterMarkers
                    gutterColor: contentsView.gutterColor
                    gutterCommentMarkerOffset: contentsView.gutterCommentMarkerOffset
                    gutterCommentRailLeft: contentsView.gutterCommentRailLeft
                    gutterIconRailLeft: contentsView.gutterIconRailLeft
                    gutterIconRailWidth: contentsView.gutterIconRailWidth
                    lineNumberColor: contentsView.lineNumberColor
                    lineNumberColumnLeft: contentsView.effectiveLineNumberColumnLeft
                    lineNumberColumnTextWidth: contentsView.effectiveLineNumberColumnTextWidth
                    lineYResolver: function (lineNumber) {
                        return contentsView.lineY(lineNumber);
                    }
                    markerHeightResolver: function (markerSpec) {
                        return contentsView.markerHeight(markerSpec);
                    }
                    markerYResolver: function (markerSpec) {
                        return contentsView.markerY(markerSpec);
                    }
                    visibleLineNumbersModel: contentsView.visibleLineNumbers()
                }
                Item {
                    id: editorViewport

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumHeight: contentsView.minEditorHeight

                    LV.TextEditor {
                        id: contentEditor

                        anchors.fill: parent
                        autoFocusOnPress: true
                        backgroundColor: contentsView.displayColor
                        backgroundColorDisabled: contentsView.displayColor
                        backgroundColorFocused: contentsView.displayColor
                        backgroundColorHover: contentsView.displayColor
                        backgroundColorPressed: contentsView.displayColor
                        centeredTextHeight: contentsView.editorTextLineBoxHeight
                        cornerRadius: 0
                        editorHeight: editorViewport.height
                        enforceModeDefaults: false
                        fieldMinHeight: Math.max(contentsView.minEditorHeight, editorViewport.height)
                        fontFamily: LV.Theme.fontBody
                        fontLetterSpacing: 0
                        fontPixelSize: 12
                        fontWeight: Font.Medium
                        insetHorizontal: contentsView.editorHorizontalInset
                        insetVertical: contentsView.editorBottomInset
                        placeholderText: ""
                        selectByMouse: true
                        selectedTextColor: LV.Theme.textPrimary
                        selectionColor: LV.Theme.accent
                        shapeStyle: shapeRoundRect
                        showRenderedOutput: false
                        showScrollBar: false
                        text: contentsView.editorText
                        textColor: LV.Theme.bodyColor
                        textFormat: TextEdit.PlainText
                        wrapMode: TextEdit.Wrap

                        onFocusedChanged: {
                            if (!focused)
                                editorSession.flushPendingEditorText();
                        }
                        onTextEdited: function (text) {
                            if (contentsView.editorText !== text)
                                contentsView.editorText = text;
                            if (contentsView.syncingEditorTextFromModel)
                                return;
                            editorSession.markLocalEditorAuthority();
                            editorSession.scheduleEditorPersistence();
                            contentsView.editorTextEdited(text);
                        }
                    }
                    Binding {
                        property: "y"
                        target: contentEditor.editorItem
                        value: contentsView.editorDocumentStartY
                    }
                    // Shared desktop/mobile contract: outer inset owns document origin, so LVRS top centering stays disabled.
                    Binding {
                        property: "topPadding"
                        target: contentEditor.editorItem
                        value: contentsView.editorDocumentTopPadding
                    }
                    LV.Label {
                        anchors.left: parent.left
                        anchors.leftMargin: contentsView.editorHorizontalInset
                        anchors.top: parent.top
                        anchors.topMargin: contentsView.editorDocumentStartY
                        color: LV.Theme.textTertiary
                        style: body
                        text: "Start typing here"
                        visible: contentEditor.empty && contentsView.hasSelectedNote
                    }
                }
                ContentsMinimapLayer {
                    id: minimapLayer

                    Layout.fillHeight: true
                    Layout.preferredWidth: contentsView.minimapOuterWidth
                    editorFlickable: contentsView.editorFlickable
                    minimapBarWidthResolver: function (characterCount) {
                        return contentsView.minimapBarWidth(characterCount);
                    }
                    minimapContentHeightResolver: function () {
                        return contentsView.minimapContentHeight();
                    }
                    minimapCurrentLineColor: contentsView.minimapCurrentLineColor
                    minimapCurrentLineHeightResolver: function () {
                        return contentsView.minimapCurrentLineHeight();
                    }
                    minimapCurrentLineWidthResolver: function () {
                        return contentsView.minimapCurrentLineWidth();
                    }
                    minimapCurrentLineYResolver: function () {
                        return contentsView.minimapCurrentLineY();
                    }
                    minimapLineColor: contentsView.minimapLineColor
                    minimapSilhouetteHeightResolver: function () {
                        return contentsView.minimapSilhouetteHeight();
                    }
                    minimapTrackInset: contentsView.minimapTrackInset
                    minimapTrackWidth: contentsView.minimapTrackWidth
                    minimapViewportFillColor: contentsView.minimapViewportFillColor
                    minimapViewportHeightResolver: function () {
                        return contentsView.minimapViewportHeight();
                    }
                    minimapViewportYResolver: function () {
                        return contentsView.minimapViewportY();
                    }
                    minimapVisualRowPaintHeightResolver: function (row) {
                        return contentsView.minimapVisualRowPaintHeight(row);
                    }
                    minimapVisualRowPaintYResolver: function (row) {
                        return contentsView.minimapVisualRowPaintY(row);
                    }
                    minimapVisualRows: contentsView.minimapVisualRows
                    scrollToMinimapPositionHandler: function (localY) {
                        contentsView.scrollEditorViewportToMinimapPosition(localY);
                    }
                    visible: contentsView.minimapVisible
                }
            }
        }
        ContentsDrawerSplitter {
            id: drawerSplitter

            Layout.fillWidth: true
            Layout.preferredHeight: contentsView.splitterThickness
            clampDrawerHeightResolver: function (value) {
                return contentsView.clampDrawerHeight(value);
            }
            drawerHeight: contentsView.drawerHeight
            splitterColor: contentsView.splitterColor
            splitterHandleThickness: contentsView.splitterHandleThickness
            visible: contentsView.drawerVisible

            onDrawerHeightDragRequested: function (value) {
                contentsView.drawerHeightDragRequested(value);
            }
        }
        Rectangle {
            id: drawer

            Layout.fillWidth: true
            Layout.preferredHeight: contentsView.clampDrawerHeight(contentsView.drawerHeight)
            color: contentsView.drawerColor
            visible: contentsView.drawerVisible
        }
    }
}
