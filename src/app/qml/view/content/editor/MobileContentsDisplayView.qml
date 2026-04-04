pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: contentsView

    property string activeDrawerMode: contentsView.drawerModeQuickNote
    readonly property int activeEditorViewModeValue: {
        const viewModeModel = contentsView.resolvedEditorViewModeViewModel;
        if (viewModeModel && viewModeModel.activeViewMode !== undefined) {
            const activeValue = Number(viewModeModel.activeViewMode);
            if (isFinite(activeValue))
                return Math.max(0, Math.floor(activeValue));
        }
        return contentsView.plainEditorViewModeValue;
    }
    readonly property color activeLineNumberColor: "#9DA0A8"
    readonly property bool contentPersistenceContractAvailable: selectionBridge.contentPersistenceContractAvailable
    property var contentViewModel: null
    property alias contextMenuSelectionEnd: editorSelectionController.contextMenuSelectionEnd
    property alias contextMenuSelectionStart: editorSelectionController.contextMenuSelectionStart
    readonly property int currentCursorLineNumber: textMetricsBridge.logicalLineNumberForOffset(Number(contentEditor.cursorPosition) || 0)
    readonly property color decorativeMarkerYellow: "#FFF567"
    readonly property int desktopEditorFontPixelSize: 12
    property color displayColor: "transparent"
    property color drawerColor: "transparent"
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property string drawerModeQuickNote: "QuickNote"
    property string drawerQuickNoteText: ""
    property bool drawerVisible: true
    readonly property int editorBottomInset: 16
    property alias editorBoundNoteId: editorSession.editorBoundNoteId
    readonly property real editorContentOffsetY: {
        if (!contentEditor.editorItem || !contentEditor.editorItem.parent)
            return 0;
        return Number(contentEditor.editorItem.parent.y) || 0;
    }
    readonly property real editorDocumentStartY: {
        if (contentsView.showPrintEditorLayout)
            return 0;
        return contentsView.effectiveEditorTopInset;
    }
    readonly property int editorDocumentTopPadding: 0
    readonly property var editorFlickable: contentsView.resolveEditorFlickable()
    readonly property int editorHorizontalInset: 16
    readonly property bool editorInputFocused: {
        if (contentEditor && contentEditor.focused !== undefined && contentEditor.focused)
            return true;
        if (contentEditor && contentEditor.activeFocus !== undefined && contentEditor.activeFocus)
            return true;
        if (contentEditor && contentEditor.editorItem && contentEditor.editorItem.activeFocus !== undefined && contentEditor.editorItem.activeFocus)
            return true;
        if (contentEditor && contentEditor.editorItem && contentEditor.editorItem.inputItem && contentEditor.editorItem.inputItem.activeFocus !== undefined && contentEditor.editorItem.inputItem.activeFocus)
            return true;
        if (contentEditor && contentEditor.editorItem && contentEditor.editorItem.inputItem && contentEditor.editorItem.inputItem.focused !== undefined && contentEditor.editorItem.inputItem.focused)
            return true;
        return false;
    }
    readonly property real editorLineHeight: contentsView.editorTextLineBoxHeight
    readonly property int editorMobileFontPixelSizeOffset: 2
    property alias editorSelectionContextMenuItems: editorSelectionController.contextMenuItems
    readonly property real editorSurfaceHeight: Math.max(0, contentsView.editorViewportHeight - contentsView.editorDocumentStartY)
    property alias editorText: editorSession.editorText
    readonly property int editorTextLineBoxHeight: contentsView.effectiveEditorFontPixelSize
    readonly property int editorTopInset: LV.Theme.gap4
    property int editorTopInsetOverride: -1
    property var editorViewModeViewModel: null
    readonly property real editorViewportHeight: editorViewport ? Number(editorViewport.height) || 0 : 0
    readonly property int effectiveEditorFontPixelSize: contentsView.desktopEditorFontPixelSize + contentsView.editorMobileFontPixelSizeOffset
    readonly property int effectiveEditorTopInset: contentsView.editorTopInsetOverride >= 0 ? contentsView.editorTopInsetOverride : contentsView.editorTopInset
    readonly property int effectiveFrameHorizontalInset: contentsView.frameHorizontalInsetOverride >= 0 ? contentsView.frameHorizontalInsetOverride : contentsView.frameHorizontalInset
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
    readonly property int effectiveGutterWidth: contentsView.showEditorGutter ? (contentsView.gutterWidthOverride >= 0 ? contentsView.gutterWidthOverride : contentsView.gutterWidth) : 0
    readonly property int effectiveLineNumberColumnLeft: contentsView.lineNumberColumnLeftOverride >= 0 ? contentsView.lineNumberColumnLeftOverride : contentsView.lineNumberColumnLeft
    readonly property int effectiveLineNumberColumnTextWidth: contentsView.lineNumberColumnTextWidthOverride >= 0 ? contentsView.lineNumberColumnTextWidthOverride : contentsView.lineNumberColumnTextWidth
    readonly property int frameHorizontalInset: 2
    property int frameHorizontalInsetOverride: -1
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
    readonly property real gutterViewportHeight: contentsView.editorViewportHeight
    readonly property int gutterWidth: 74
    property int gutterWidthOverride: -1
    readonly property bool hasSelectedNote: contentsView.selectedNoteId.length > 0
    property var libraryHierarchyViewModel: null
    readonly property color lineNumberColor: "#4E5157"
    readonly property int lineNumberColumnLeft: 14
    property int lineNumberColumnLeftOverride: -1
    readonly property int lineNumberColumnTextWidth: contentsView.gutterWidth - contentsView.lineNumberColumnLeft - contentsView.lineNumberRightInset
    property int lineNumberColumnTextWidthOverride: -1
    readonly property int lineNumberColumnWidth: 26
    readonly property int lineNumberRightInset: contentsView.editorHorizontalInset
    readonly property int logicalLineCount: Math.max(1, Number(textMetricsBridge.logicalLineCount) || 1)
    property var logicalLineDocumentYCache: []
    property int logicalLineDocumentYCacheLineCount: 0
    property int logicalLineDocumentYCacheRevision: -1
    readonly property var logicalLineStartOffsets: textMetricsBridge.logicalLineStartOffsets
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    readonly property int minEditorHeight: LV.Theme.gap20 * 12
    readonly property real minimapAvailableTrackHeight: Math.max(1, contentsView.editorViewportHeight - 16)
    readonly property color minimapCurrentLineColor: contentsView.activeLineNumberColor
    readonly property color minimapLineColor: contentsView.lineNumberColor
    readonly property int minimapOuterWidth: 56
    property real minimapResolvedCurrentLineHeight: 1
    property real minimapResolvedCurrentLineWidth: 0
    property real minimapResolvedCurrentLineY: 0
    property real minimapResolvedSilhouetteHeight: 1
    property real minimapResolvedTrackHeight: 1
    readonly property real minimapResolvedTrackWidth: contentsView.minimapTrackWidth
    property real minimapResolvedViewportHeight: 0
    property real minimapResolvedViewportY: 0
    readonly property int minimapRowGap: 1
    property bool minimapScrollable: false
    property bool minimapSnapshotRefreshQueued: false
    readonly property int minimapTrackInset: 8
    readonly property int minimapTrackWidth: 36
    readonly property color minimapViewportFillColor: "#149DA0A8"
    readonly property int minimapViewportMinHeight: 28
    property bool minimapVisible: true
    property var minimapVisualRows: []
    readonly property var normalizedExternalGutterMarkers: gutterMarkerBridge.normalizedExternalGutterMarkers
    readonly property bool noteCountContractAvailable: selectionBridge.noteCountContractAvailable
    property var noteListModel: null
    readonly property bool noteSelectionContractAvailable: selectionBridge.noteSelectionContractAvailable
    readonly property bool noteSnapshotRefreshEnabled: contentsView.visible && contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer && contentsView.noteSelectionContractAvailable
    readonly property int noteSnapshotRefreshIntervalMs: 1200
    readonly property int pageEditorViewModeValue: 1
    property var panelViewModel: null
    property alias pendingBodySave: editorSession.pendingBodySave
    property string pendingEditorFocusNoteId: ""
    readonly property int plainEditorViewModeValue: 0
    readonly property color printCanvasColor: "#F1F3F6"
    readonly property int printEditorViewModeValue: 2
    readonly property int printGuideHorizontalInset: LV.Theme.gap12 * 2
    readonly property int printGuideVerticalInset: LV.Theme.gap12 * 2
    readonly property real printPaperAspectRatio: 210 / 297
    readonly property color printPaperBorderColor: "#19000000"
    readonly property color printPaperColor: "#FFFFFF"
    readonly property int printPaperHorizontalMargin: LV.Theme.gap12
    readonly property int printPaperMaxWidth: 880
    readonly property int printPaperPaddingHorizontal: LV.Theme.gap12
    readonly property int printPaperPaddingVertical: LV.Theme.gap8
    readonly property color printPaperTextColor: "#000000"
    readonly property int printPaperVerticalMargin: LV.Theme.gap4
    readonly property string renderedEditorText: contentsView.normalizeBodySourceForRichTextEditor(contentsView.editorText)
    readonly property var resolvedEditorViewModeViewModel: {
        if (contentsView.editorViewModeViewModel)
            return contentsView.editorViewModeViewModel;
        if (LV.ViewModels && LV.ViewModels.get !== undefined)
            return LV.ViewModels.get("editorViewModeViewModel");
        return null;
    }
    property bool resourceDropActive: false
    readonly property color resourceRenderBorderColor: "#334E5157"
    readonly property color resourceRenderCardColor: "#E61A1D22"
    readonly property int resourceRenderDisplayLimit: 3
    property var resourcesImportViewModel: null
    readonly property string richTextHighlightOpenTag: "<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">"
    readonly property int saveDebounceMs: 120
    readonly property string selectedNoteBodyText: selectionBridge.selectedNoteBodyText
    readonly property string selectedNoteId: selectionBridge.selectedNoteId
    readonly property bool selectedNoteIsResourcePackage: contentsView.selectedNoteId.trim().toLowerCase().endsWith(".wsresource")
    readonly property var selectedResourceEntry: {
        if (contentsView.showDedicatedResourceViewer && contentsView.noteListModel && contentsView.noteListModel.currentResourceEntry !== undefined) {
            const currentEntry = contentsView.noteListModel.currentResourceEntry;
            if (currentEntry && typeof currentEntry === "object") {
                const entrySource = currentEntry.source !== undefined ? String(currentEntry.source).trim() : "";
                const entryResolvedPath = currentEntry.resolvedPath !== undefined ? String(currentEntry.resolvedPath).trim() : "";
                const entryRenderMode = currentEntry.renderMode !== undefined ? String(currentEntry.renderMode).trim() : "";
                if (entrySource.length > 0 || entryResolvedPath.length > 0 || entryRenderMode.length > 0)
                    return currentEntry;
            }
        }

        const resources = bodyResourceRenderer.renderedResources;
        if (!Array.isArray(resources) || resources.length === 0)
            return ({});
        const entry = resources[0];
        return entry && typeof entry === "object" ? entry : ({});
    }
    readonly property bool showCurrentLineMarker: contentsView.hasSelectedNote || contentsView.editorText.length > 0 || contentsView.editorInputFocused
    readonly property bool showDedicatedResourceViewer: contentsView.selectedNoteIsResourcePackage
    readonly property bool showEditorGutter: false
    readonly property bool showFormattedTextRenderer: false
    readonly property bool showPageEditorLayout: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && contentsView.activeEditorViewModeValue === contentsView.pageEditorViewModeValue
    readonly property bool showPrintEditorLayout: contentsView.showPageEditorLayout || contentsView.showPrintModeActive
    readonly property bool showPrintMarginGuides: contentsView.showPrintModeActive
    readonly property bool showPrintModeActive: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && contentsView.activeEditorViewModeValue === contentsView.printEditorViewModeValue
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property alias syncingEditorTextFromModel: editorSession.syncingEditorTextFromModel
    readonly property real textOriginY: {
        return contentsView.editorDocumentStartY + contentsView.editorContentOffsetY;
    }
    property var visibleGutterLineEntries: [
        {
            "lineNumber": 1,
            "y": 0
        }
    ]
    readonly property int visibleNoteCount: selectionBridge.visibleNoteCount

    signal drawerHeightDragRequested(int value)
    signal editorTextEdited(string text)
    signal viewHookRequested

    function applyEditorRichTextSurface() {
        editorSelectionController.applyEditorRichTextSurface();
    }
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
    function buildVisibleGutterLineEntries() {
        const visibleLines = [];
        const firstVisibleLine = contentsView.firstVisibleLogicalLine();
        for (let lineNumber = firstVisibleLine; lineNumber <= contentsView.logicalLineCount; ++lineNumber) {
            const resolvedLineY = contentsView.lineY(lineNumber);
            if (resolvedLineY > contentsView.gutterViewportHeight)
                break;
            if (resolvedLineY + contentsView.lineVisualHeight(lineNumber, 1) < 0)
                continue;
            visibleLines.push({
                "lineNumber": lineNumber,
                "y": resolvedLineY
            });
        }
        if (visibleLines.length === 0) {
            visibleLines.push({
                "lineNumber": firstVisibleLine,
                "y": contentsView.lineY(firstVisibleLine)
            });
        }
        return visibleLines;
    }
    function canAcceptResourceDropUrls(urls) {
        if (!contentsView.hasSelectedNote || !contentsView.resourcesImportViewModel)
            return false;
        if (contentsView.showDedicatedResourceViewer || contentsView.showFormattedTextRenderer)
            return false;
        if (!Array.isArray(urls) || urls.length === 0)
            return false;
        if (contentsView.resourcesImportViewModel.canImportUrls === undefined)
            return false;
        return !!contentsView.resourcesImportViewModel.canImportUrls(urls);
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
        contentsView.visibleGutterLineEntries = contentsView.buildVisibleGutterLineEntries();
        contentsView.refreshMinimapSnapshot();
        if (minimapLayer)
            minimapLayer.requestRepaint();
    }
    function contextMenuEditorSelectionRange() {
        return editorSelectionController.contextMenuEditorSelectionRange();
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
        const safeOffset = Math.max(0, Math.min(contentsView.logicalTextLength(), Number(contentEditor.cursorPosition) || 0));
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
    function currentEditorCursorPosition() {
        return editorSelectionController.currentEditorCursorPosition();
    }
    function currentSelectedEditorText() {
        return editorSelectionController.currentSelectedEditorText();
    }
    function documentOccupiedBottomY() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        contentsView.ensureLogicalLineDocumentYCache();
        const cachedLastLineDocumentY = contentsView.logicalLineCount > 0 ? contentsView.lineDocumentY(contentsView.logicalLineCount) : 0;
        const cachedBottom = Math.max(contentsView.editorLineHeight, cachedLastLineDocumentY + contentsView.editorLineHeight);
        const logicalLength = contentsView.logicalTextLength();
        if (contentEditor.editorItem && contentEditor.editorItem.positionToRectangle !== undefined) {
            const rect = contentEditor.editorItem.positionToRectangle(Math.max(0, logicalLength));
            const rectY = Number(rect.y);
            const rectHeight = Number(rect.height);
            if (isFinite(rectY))
                return Math.max(cachedBottom, rectY + Math.max(contentsView.editorLineHeight, isFinite(rectHeight) ? rectHeight : contentsView.editorLineHeight));
        }
        return cachedBottom;
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
    function ensureLogicalLineDocumentYCache() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const lineCount = contentsView.logicalLineCount;
        if (contentsView.logicalLineDocumentYCacheRevision === refreshRevision && contentsView.logicalLineDocumentYCacheLineCount === lineCount && Array.isArray(contentsView.logicalLineDocumentYCache) && contentsView.logicalLineDocumentYCache.length === lineCount) {
            return;
        }
        const cachedYValues = [];
        let previousDocumentY = 0;
        for (let lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
            const startOffset = Math.max(0, Number(textMetricsBridge.logicalLineStartOffsetAt(lineIndex)) || 0);
            const rawDocumentY = Math.max(0, Number(contentsView.documentYForOffset(startOffset)) || 0);
            const minimumDocumentY = lineIndex === 0 ? 0 : previousDocumentY + Math.max(1, contentsView.editorLineHeight);
            const resolvedDocumentY = lineIndex === 0 ? rawDocumentY : Math.max(rawDocumentY, minimumDocumentY);
            cachedYValues.push(resolvedDocumentY);
            previousDocumentY = resolvedDocumentY;
        }
        contentsView.logicalLineDocumentYCache = cachedYValues;
        contentsView.logicalLineDocumentYCacheRevision = refreshRevision;
        contentsView.logicalLineDocumentYCacheLineCount = lineCount;
    }
    function extractResourceDropUrls(drop) {
        const urls = [];
        if (drop && drop.urls !== undefined && drop.urls !== null) {
            if (Array.isArray(drop.urls)) {
                for (let index = 0; index < drop.urls.length; ++index)
                    urls.push(drop.urls[index]);
            } else if (drop.urls.length !== undefined) {
                for (let listIndex = 0; listIndex < drop.urls.length; ++listIndex)
                    urls.push(drop.urls[listIndex]);
            } else {
                urls.push(drop.urls);
            }
        }
        if (urls.length > 0)
            return urls;
        if (!drop || drop.getDataAsString === undefined)
            return urls;
        const rawUriList = String(drop.getDataAsString("text/uri-list") || "").trim();
        if (rawUriList.length === 0)
            return urls;
        const uriLines = rawUriList.split(/\r?\n/);
        for (let lineIndex = 0; lineIndex < uriLines.length; ++lineIndex) {
            const line = String(uriLines[lineIndex] || "").trim();
            if (line.length === 0 || line.charAt(0) === "#")
                continue;
            urls.push(line);
        }

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
    function flushEditorStateAfterInputSettles(attempt) {
        const retryCount = Math.max(0, Number(attempt) || 0);
        const activePreeditText = contentEditor && contentEditor.preeditText !== undefined ? String(contentEditor.preeditText === undefined || contentEditor.preeditText === null ? "" : contentEditor.preeditText) : "";
        const inputMethodBusy = !!(contentEditor && ((contentEditor.inputMethodComposing !== undefined && contentEditor.inputMethodComposing) || activePreeditText.length > 0));
        if (inputMethodBusy && retryCount < 6) {
            Qt.callLater(function () {
                contentsView.flushEditorStateAfterInputSettles(retryCount + 1);
            });
            return;
        }
        editorTypingController.handleEditorTextEdited();
        editorSession.flushPendingEditorText();
    }
    function focusEditorForPendingNote() {
        const pendingNoteId = contentsView.pendingEditorFocusNoteId === undefined || contentsView.pendingEditorFocusNoteId === null ? "" : String(contentsView.pendingEditorFocusNoteId).trim();
        const selectedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null ? "" : String(contentsView.selectedNoteId).trim();
        if (pendingNoteId.length === 0 || pendingNoteId !== selectedNoteId || !contentsView.hasSelectedNote)
            return;

        contentsView.pendingEditorFocusNoteId = "";
        Qt.callLater(function () {
            const activeNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null ? "" : String(contentsView.selectedNoteId).trim();
            if (activeNoteId !== pendingNoteId)
                return;

            const cursorPosition = Math.max(0, contentsView.logicalTextLength());
            contentEditor.forceActiveFocus();
            if (contentEditor.editorItem && contentEditor.editorItem.forceActiveFocus !== undefined)
                contentEditor.editorItem.forceActiveFocus();
            if (contentEditor.cursorPosition !== undefined)
                contentEditor.cursorPosition = cursorPosition;
            if (contentEditor.editorItem && contentEditor.editorItem.cursorPosition !== undefined)
                contentEditor.editorItem.cursorPosition = cursorPosition;
        });
    }
    function handleInlineFormatShortcutKeyPress(event) {
        return editorSelectionController.handleInlineFormatShortcutKeyPress(event);
    }
    function handleSelectionContextMenuEvent(eventName) {
        editorSelectionController.handleSelectionContextMenuEvent(eventName);
    }
    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition) {
        return editorSelectionController.inferSelectionRangeFromSelectedText(selectedText, cursorPosition);
    }
    function inlineStyleWrapTags(styleTag) {
        return editorSelectionController.inlineStyleWrapTags(styleTag);
    }
    function insertImportedResourceTags(importedEntries) {
        if (!Array.isArray(importedEntries) || importedEntries.length === 0)
            return false;
        const currentText = contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText);
        const cursorPosition = Math.max(0, Math.min(currentText.length, contentEditor && contentEditor.cursorPosition !== undefined ? Number(contentEditor.cursorPosition) || 0 : currentText.length));
        const tagTexts = [];
        for (let index = 0; index < importedEntries.length; ++index) {
            const tagText = contentsView.resourceTagTextForImportedEntry(importedEntries[index]);
            if (tagText.length > 0)
                tagTexts.push(tagText);
        }
        if (tagTexts.length === 0)
            return false;
        const prefix = cursorPosition > 0 && currentText.charAt(cursorPosition - 1) !== "\n" ? "\n" : "";
        const suffix = cursorPosition < currentText.length && currentText.charAt(cursorPosition) !== "\n" ? "\n" : "";
        const insertionText = prefix + tagTexts.join("\n") + suffix;
        const nextText = currentText.slice(0, cursorPosition) + insertionText + currentText.slice(cursorPosition);
        contentsView.editorText = nextText;
        editorSession.markLocalEditorAuthority();
        const saved = contentsView.persistEditorTextImmediately(nextText);
        if (!saved)
            editorSession.scheduleEditorPersistence();
        if (contentEditor && contentEditor.cursorPosition !== undefined)
            contentEditor.cursorPosition = cursorPosition + insertionText.length;
        contentsView.editorTextEdited(nextText);
        bodyResourceRenderer.requestRenderRefresh();
        return true;
    }
    function isMinimapScrollable() {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return false;
        return contentsView.minimapContentHeight() > (Number(flickable.height) || 0);
    }
    function lineDocumentY(lineNumber) {
        contentsView.ensureLogicalLineDocumentYCache();
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        const cacheIndex = safeLineNumber - 1;
        if (Array.isArray(contentsView.logicalLineDocumentYCache) && cacheIndex >= 0 && cacheIndex < contentsView.logicalLineDocumentYCache.length) {
            return Number(contentsView.logicalLineDocumentYCache[cacheIndex]) || 0;
        }
        return Math.max(0, (safeLineNumber - 1) * contentsView.editorLineHeight);
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
        if (contentsView.logicalLineCount <= 0)
            return 1;
        contentsView.ensureLogicalLineDocumentYCache();
        const safeDocumentY = Math.max(0, Number(documentY) || 0);
        let low = 0;
        let high = contentsView.logicalLineCount - 1;
        let best = 0;
        while (low <= high) {
            const middle = Math.floor((low + high) / 2);
            const middleY = contentsView.lineDocumentY(middle + 1);
            if (middleY <= safeDocumentY) {
                best = middle;
                low = middle + 1;
            } else {
                high = middle - 1;
            }
        }
        return best + 1;
    }
    function logicalTextLength() {
        const safeLineCount = Math.max(1, Number(textMetricsBridge.logicalLineCount) || 1);
        const lastLineIndex = safeLineCount - 1;
        const lastStartOffset = Math.max(0, Number(textMetricsBridge.logicalLineStartOffsetAt(lastLineIndex)) || 0);
        const lastLineCharacterCount = Math.max(0, Number(textMetricsBridge.logicalLineCharacterCountAt(lastLineIndex)) || 0);
        return lastStartOffset + lastLineCharacterCount;
    }
    function markerColorForType(markerType) {
        const normalizedType = markerType === undefined || markerType === null ? "" : String(markerType).toLowerCase();
        if (normalizedType === "conflict")
            return contentsView.gutterMarkerConflictColor;
        if (normalizedType === "changed")
            return contentsView.gutterMarkerChangedColor;
        if (normalizedType === "current")
            return contentsView.gutterMarkerCurrentColor;
        return contentsView.gutterMarkerChangedColor;
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
        const maxWidth = Math.max(6, contentsView.minimapResolvedTrackWidth - 1);
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
    function minimapCurrentVisualRow(rowsOverride) {
        const rows = Array.isArray(rowsOverride) ? rowsOverride : (Array.isArray(contentsView.minimapVisualRows) ? contentsView.minimapVisualRows : []);
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
    function minimapSilhouetteHeight(rowsOverride) {
        const rows = Array.isArray(rowsOverride) ? rowsOverride : (Array.isArray(contentsView.minimapVisualRows) ? contentsView.minimapVisualRows : []);
        if (rows.length === 0)
            return contentsView.minimapVisualRowPaintHeight(null);
        return rows.length * contentsView.minimapVisualRowPaintHeight(rows[0]) + Math.max(0, rows.length - 1) * contentsView.minimapRowGap;
    }
    function minimapTrackHeightForContentHeight(segmentHeight) {
        const safeSegmentHeight = Math.max(0, Number(segmentHeight) || 0);
        const contentHeight = contentsView.minimapContentHeight();
        if (contentHeight <= 0)
            return 0;
        return Math.max(1, (safeSegmentHeight / contentHeight) * contentsView.minimapResolvedTrackHeight);
    }
    function minimapTrackYForContentY(contentY) {
        const contentHeight = contentsView.minimapContentHeight();
        if (contentHeight <= 0)
            return 0;
        const safeContentY = Math.max(0, Math.min(contentHeight, Number(contentY) || 0));
        return (safeContentY / contentHeight) * contentsView.minimapResolvedTrackHeight;
    }
    function minimapViewportHeight(trackHeightOverride) {
        const flickable = contentsView.editorFlickable;
        const trackHeight = Math.max(1, Number(trackHeightOverride) || contentsView.minimapResolvedTrackHeight);
        if (!flickable)
            return trackHeight;
        const contentHeight = Math.max(1, contentsView.minimapContentHeight());
        const viewportHeight = Math.max(0, contentsView.editorViewportHeight);
        if (contentHeight <= viewportHeight)
            return trackHeight;
        const proportionalHeight = Math.max(1, (viewportHeight / contentHeight) * trackHeight);
        return Math.min(trackHeight, Math.max(contentsView.minimapViewportMinHeight, proportionalHeight));
    }
    function minimapViewportY(trackHeightOverride, viewportHeightOverride) {
        const flickable = contentsView.editorFlickable;
        const trackHeight = Math.max(1, Number(trackHeightOverride) || contentsView.minimapResolvedTrackHeight);
        const viewportHeight = Math.max(0, Number(viewportHeightOverride) || contentsView.minimapResolvedViewportHeight);
        if (!flickable)
            return 0;
        const contentHeight = Math.max(1, contentsView.minimapContentHeight());
        const editorViewportHeight = Math.max(0, contentsView.editorViewportHeight);
        const maxContentY = Math.max(0, contentHeight - editorViewportHeight);
        if (maxContentY <= 0)
            return 0;
        const contentY = Math.max(0, Math.min(maxContentY, Number(flickable.contentY) || 0));
        const maxTrackY = Math.max(0, trackHeight - viewportHeight);
        return maxTrackY * (contentY / maxContentY);
    }
    function minimapVisualRowPaintHeight(rowSpec) {
        return 1;
    }
    function minimapVisualRowPaintY(rowSpec) {
        const visualIndex = rowSpec && rowSpec.visualIndex !== undefined ? Math.max(0, Number(rowSpec.visualIndex) || 0) : 0;
        return visualIndex * (contentsView.minimapRowGap + contentsView.minimapVisualRowPaintHeight(rowSpec));
    }
    function normalizeBodySourceForRichTextEditor(sourceText) {
        return editorSelectionController.normalizeBodySourceForRichTextEditor(sourceText);
    }
    function normalizeInlineStyleTag(tagName) {
        return editorSelectionController.normalizeInlineStyleTag(tagName);
    }
    function normalizeResourceFormat(formatValue) {
        let normalizedFormat = formatValue === undefined || formatValue === null ? "" : String(formatValue).trim().toLowerCase();
        if (normalizedFormat.length === 0)
            normalizedFormat = ".bin";
        if (normalizedFormat.charAt(0) !== ".")
            normalizedFormat = "." + normalizedFormat;
        return normalizedFormat;
    }
    function normalizeResourceType(typeValue) {
        const normalizedType = typeValue === undefined || typeValue === null ? "" : String(typeValue).trim().toLowerCase();
        return normalizedType.length > 0 ? normalizedType : "other";
    }
    function openEditorSelectionContextMenu(localX, localY) {
        return editorSelectionController.openEditorSelectionContextMenu(localX, localY);
    }
    function persistEditorTextImmediately(nextText) {
        return editorSelectionController.persistEditorTextImmediately(nextText);
    }
    function pollSelectedNoteSnapshot() {
        if (!selectionBridge || selectionBridge.refreshSelectedNoteSnapshot === undefined)
            return;
        selectionBridge.refreshSelectedNoteSnapshot();
        contentsView.scheduleGutterRefresh(2);
    }
    function queueInlineFormatWrap(tagName) {
        return editorSelectionController.queueInlineFormatWrap(tagName);
    }
    function refreshMinimapSnapshot() {
        const nextRows = contentsView.buildMinimapVisualRows(contentsView.editorText, Number(contentEditor ? contentEditor.width : 0), Number(contentEditor ? contentEditor.contentHeight : 0));
        const nextSilhouetteHeight = contentsView.minimapSilhouetteHeight(nextRows);
        const nextTrackHeight = Math.min(contentsView.minimapAvailableTrackHeight, nextSilhouetteHeight);
        const nextScrollable = contentsView.isMinimapScrollable();
        const nextViewportHeight = contentsView.minimapViewportHeight(nextTrackHeight);
        const nextViewportY = contentsView.minimapViewportY(nextTrackHeight, nextViewportHeight);
        const nextCurrentVisualRow = contentsView.minimapCurrentVisualRow(nextRows);

        contentsView.minimapVisualRows = nextRows;
        contentsView.minimapResolvedSilhouetteHeight = nextSilhouetteHeight;
        contentsView.minimapResolvedTrackHeight = nextTrackHeight;
        contentsView.minimapScrollable = nextScrollable;
        contentsView.minimapResolvedViewportHeight = nextViewportHeight;
        contentsView.minimapResolvedViewportY = nextViewportY;
        contentsView.minimapResolvedCurrentLineHeight = contentsView.minimapVisualRowPaintHeight(nextCurrentVisualRow);
        contentsView.minimapResolvedCurrentLineWidth = contentsView.minimapBarWidth(nextCurrentVisualRow.charCount);
        contentsView.minimapResolvedCurrentLineY = contentsView.minimapVisualRowPaintY(nextCurrentVisualRow);
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function resetEditorSelectionCache() {
        editorSelectionController.resetEditorSelectionCache();
    }
    function resolveEditorFlickable() {
        if (!contentEditor.editorItem || !contentEditor.editorItem.parent)
            return null;
        const candidate = contentEditor.editorItem.parent.parent;
        if (!candidate || candidate.contentY === undefined || candidate.contentHeight === undefined || candidate.height === undefined)
            return null;


    }
    function resourceTagTextForImportedEntry(entry) {
        const resourceEntry = entry && typeof entry === "object" ? entry : ({});
        const resourcePath = resourceEntry.resourcePath !== undefined ? String(resourceEntry.resourcePath).trim() : "";
        if (resourcePath.length === 0)
            return "";
        const resourceType = contentsView.normalizeResourceType(resourceEntry.type);
        const resourceFormat = contentsView.normalizeResourceFormat(resourceEntry.format);
        const pathExpression = /\s/.test(resourcePath) ? "path=\"" + resourcePath + "\"" : "path=" + resourcePath;
        return "<resource type=\"" + resourceType + "\" format=\"" + resourceFormat + "\" " + pathExpression + ">";
    }
    function scheduleEditorFocusForNote(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length === 0)
            return;

        contentsView.pendingEditorFocusNoteId = normalizedNoteId;
        contentsView.focusEditorForPendingNote();
    }
    function scheduleEditorRichTextSurfaceSync() {
        editorSelectionController.scheduleEditorRichTextSurfaceSync();
    }
    function scheduleGutterRefresh(passCount) {
        const requestedPassCount = Math.max(1, Number(passCount) || 1);
        contentsView.gutterRefreshPassesRemaining = Math.max(contentsView.gutterRefreshPassesRemaining, requestedPassCount);
        if (!gutterRefreshTimer.running)
            gutterRefreshTimer.start();
    }
    function scheduleMinimapSnapshotRefresh() {
        if (contentsView.minimapSnapshotRefreshQueued)
            return;
        contentsView.minimapSnapshotRefreshQueued = true;
        Qt.callLater(function () {
            contentsView.minimapSnapshotRefreshQueued = false;
            contentsView.refreshMinimapSnapshot();
        });
    }
    function scrollEditorViewportToMinimapPosition(localY) {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return;
        const contentHeight = Math.max(1, contentsView.editorOccupiedContentHeight());
        const viewportHeight = Math.max(0, contentsView.editorViewportHeight);
        const maxContentY = Math.max(0, contentHeight - viewportHeight);
        if (maxContentY <= 0) {
            flickable.contentY = 0;
            return;
        }
        const trackRatio = contentsView.clampUnit((Number(localY) || 0) / Math.max(1, contentsView.minimapResolvedTrackHeight));
        const documentY = contentHeight * trackRatio;
        const nextContentY = Math.max(0, Math.min(maxContentY, documentY - viewportHeight / 2));
        flickable.contentY = nextContentY;
    }
    function selectedEditorRange() {
        return editorSelectionController.selectedEditorRange();
    }
    function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {
        return editorSelectionController.wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange);
    }

    clip: true

    Component.onCompleted: {
        contentsView.resetEditorSelectionCache();
        editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);
        contentsView.scheduleEditorRichTextSurfaceSync();
        contentsView.scheduleMinimapSnapshotRefresh();
        contentsView.scheduleGutterRefresh(4);
    }
    Component.onDestruction: {
        editorTypingController.handleEditorTextEdited();
        editorSession.flushPendingEditorText();
    }
    onEditorFlickableChanged: {
        contentsView.scheduleMinimapSnapshotRefresh();
        contentsView.scheduleGutterRefresh(2);
    }
    onEditorTextChanged: {
        contentsView.scheduleEditorRichTextSurfaceSync();
        contentsView.scheduleMinimapSnapshotRefresh();
        if (minimapLayer)
            minimapLayer.requestRepaint();
    }
    onHeightChanged: {
        contentsView.scheduleMinimapSnapshotRefresh();
        contentsView.scheduleGutterRefresh(2);
    }
    onSelectedNoteBodyTextChanged: {
        const normalizedBodyText = contentsView.selectedNoteBodyText;
        if (editorSession.shouldAcceptModelBodyText(contentsView.selectedNoteId, normalizedBodyText)) {
            editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, normalizedBodyText);
        } else {
            editorSession.scheduleEditorPersistence();
        }
        contentsView.scheduleEditorRichTextSurfaceSync();
        contentsView.scheduleMinimapSnapshotRefresh();
        contentsView.scheduleGutterRefresh(4);
    }
    onSelectedNoteIdChanged: {
        contentsView.resetEditorSelectionCache();
        if (contentsView.pendingBodySave && contentsView.editorBoundNoteId !== contentsView.selectedNoteId)
            editorSession.flushPendingEditorText();
        editorSession.syncEditorTextFromSelection(contentsView.selectedNoteId, contentsView.selectedNoteBodyText);
        contentsView.scheduleEditorRichTextSurfaceSync();
        contentsView.scheduleMinimapSnapshotRefresh();
        contentsView.focusEditorForPendingNote();
        contentsView.scheduleGutterRefresh(4);
    }
    onVisibleChanged: {
        if (visible) {
            contentsView.scheduleMinimapSnapshotRefresh();
            contentsView.scheduleGutterRefresh(4);
        }
    }
    onWidthChanged: {
        contentsView.scheduleMinimapSnapshotRefresh();
        contentsView.scheduleGutterRefresh(2);
    }

    ContentsEditorSelectionController {
        id: editorSelectionController

        contentEditor: contentEditor
        editorSession: editorSession
        editorViewport: editorViewport
        selectionBridge: selectionBridge
        selectionContextMenu: editorSelectionContextMenu
        textFormatRenderer: textFormatRenderer
        textMetricsBridge: textMetricsBridge
        view: contentsView
    }
    ContentsEditorSelectionBridge {
        id: selectionBridge

        contentViewModel: contentsView.contentViewModel
        noteListModel: contentsView.noteListModel
    }
    ContentsEditorTypingController {
        id: editorTypingController

        contentEditor: contentEditor
        editorSession: editorSession
        textFormatRenderer: textFormatRenderer
        textMetricsBridge: textMetricsBridge
        view: contentsView
    }
    ContentsBodyResourceRenderer {
        id: bodyResourceRenderer

        contentViewModel: contentsView.contentViewModel
        maxRenderCount: contentsView.resourceRenderDisplayLimit
        noteId: contentsView.selectedNoteId
    }
    ContentsTextFormatRenderer {
        id: textFormatRenderer

        sourceText: contentsView.editorText
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
    Timer {
        id: noteSnapshotRefreshTimer

        interval: contentsView.noteSnapshotRefreshIntervalMs
        repeat: true
        running: contentsView.noteSnapshotRefreshEnabled

        onTriggered: contentsView.pollSelectedNoteSnapshot()
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
            contentsView.scheduleMinimapSnapshotRefresh();
            contentsView.scheduleGutterRefresh(2);
        }
        function onCursorPositionChanged() {
            contentsView.scheduleMinimapSnapshotRefresh();
            if (minimapLayer)
                minimapLayer.requestRepaint();
        }
        function onLineCountChanged() {
            contentsView.scheduleMinimapSnapshotRefresh();
            contentsView.scheduleGutterRefresh(2);
        }

        ignoreUnknownSignals: true
        target: contentEditor
    }
    Connections {
        function onContentYChanged() {
            contentsView.scheduleMinimapSnapshotRefresh();
            if (minimapLayer)
                minimapLayer.requestRepaint();
        }

        ignoreUnknownSignals: true
        target: contentsView.editorFlickable
    }
    Connections {
        function onCursorPositionChanged() {
            contentsView.scheduleEditorRichTextSurfaceSync();
        }

        ignoreUnknownSignals: true
        target: contentEditor && contentEditor.editorItem ? contentEditor.editorItem : null
    }
    Connections {
        function onCursorPositionChanged() {
            contentsView.scheduleEditorRichTextSurfaceSync();
        }

        ignoreUnknownSignals: true
        target: contentEditor && contentEditor.editorItem && contentEditor.editorItem.inputItem ? contentEditor.editorItem.inputItem : null
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
    ColumnLayout {
        id: drawerView

        readonly property real availableDisplayHeight: Math.max(contentsView.minDisplayHeight, (Number(contentsView.height) || 0) - drawerView.drawerReservedHeight)
        readonly property real drawerReservedHeight: contentsView.drawerVisible ? drawerView.effectiveDrawerHeight + contentsView.splitterThickness : 0
        readonly property real effectiveDrawerHeight: contentsView.drawerVisible ? contentsView.clampDrawerHeight(contentsView.drawerHeight) : 0

        anchors.fill: parent
        spacing: LV.Theme.gapNone

        Rectangle {
            id: contentsDisplayView

            Layout.fillWidth: true
            Layout.minimumHeight: contentsView.minDisplayHeight
            Layout.preferredHeight: drawerView.availableDisplayHeight
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
                    visible: contentsView.showEditorGutter
                    visibleLineNumbersModel: contentsView.visibleGutterLineEntries
                }
                Item {
                    id: editorViewport

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumHeight: contentsView.minEditorHeight
                    clip: true

                    Rectangle {
                        id: printEditorCanvas

                        anchors.fill: parent
                        clip: true
                        color: contentsView.printCanvasColor
                        visible: contentsView.showPrintEditorLayout
                        z: 0

                        Rectangle {
                            id: printEditorPage

                            readonly property real availableWidth: Math.max(0, Number(parent ? parent.width : 0) - contentsView.printPaperHorizontalMargin * 2)
                            readonly property real fittedWidth: Math.max(0, Math.min(contentsView.printPaperMaxWidth, printEditorPage.availableWidth))

                            border.color: contentsView.printPaperBorderColor
                            border.width: 1
                            color: contentsView.printPaperColor
                            height: contentsView.printPaperAspectRatio > 0 ? printEditorPage.width / contentsView.printPaperAspectRatio : 0
                            radius: LV.Theme.radiusSm
                            width: printEditorPage.fittedWidth
                            x: Math.max(0, (Number(parent ? parent.width : 0) - printEditorPage.width) / 2)
                            y: contentsView.printPaperVerticalMargin

                            Canvas {
                                anchors.fill: parent
                                visible: contentsView.showPrintMarginGuides

                                onHeightChanged: requestPaint()
                                onPaint: {
                                    const ctx = getContext("2d");
                                    ctx.clearRect(0, 0, width, height);
                                    if (!visible)
                                        return;

                                    const leftInset = Math.max(1, Number(contentsView.printGuideHorizontalInset) || 1);
                                    const rightInset = Math.max(1, Number(contentsView.printGuideHorizontalInset) || 1);
                                    const topInset = Math.max(1, Number(contentsView.printGuideVerticalInset) || 1);
                                    const bottomInset = Math.max(1, Number(contentsView.printGuideVerticalInset) || 1);
                                    const left = leftInset;
                                    const top = topInset;
                                    const right = Math.max(left + 1, width - rightInset);
                                    const bottom = Math.max(top + 1, height - bottomInset);

                                    ctx.lineWidth = 1;
                                    ctx.strokeStyle = "#66727D";
                                    ctx.beginPath();
                                    const segment = 6;
                                    const gap = 4;

                                    for (let x = left; x < right; x += segment + gap) {
                                        const x2 = Math.min(right, x + segment);
                                        ctx.moveTo(x, top);
                                        ctx.lineTo(x2, top);
                                        ctx.moveTo(x, bottom);
                                        ctx.lineTo(x2, bottom);
                                    }
                                    for (let y = top; y < bottom; y += segment + gap) {
                                        const y2 = Math.min(bottom, y + segment);
                                        ctx.moveTo(left, y);
                                        ctx.lineTo(left, y2);
                                        ctx.moveTo(right, y);
                                        ctx.lineTo(right, y2);
                                    }
                                    ctx.stroke();
                                }
                                onVisibleChanged: {
                                    if (visible)
                                        requestPaint();
                                }
                                onWidthChanged: requestPaint()
                            }
                        }
                    }
                    ContentsInlineFormatEditor {
                        id: contentEditor

                        anchors.bottomMargin: contentsView.showPrintEditorLayout ? Math.max(0, (Number(parent ? parent.height : 0) || 0) - ((Number(printEditorPage.y) || 0) + (Number(printEditorPage.height) || 0) - contentsView.printGuideVerticalInset)) : 0
                        anchors.fill: parent
                        anchors.leftMargin: contentsView.showPrintEditorLayout ? Math.max(0, (Number(printEditorPage.x) || 0) + contentsView.printGuideHorizontalInset) : 0
                        anchors.rightMargin: contentsView.showPrintEditorLayout ? Math.max(0, (Number(parent ? parent.width : 0) || 0) - ((Number(printEditorPage.x) || 0) + (Number(printEditorPage.width) || 0) - contentsView.printGuideHorizontalInset)) : 0
                        anchors.topMargin: contentsView.showPrintEditorLayout ? Math.max(0, (Number(printEditorPage.y) || 0) + contentsView.printGuideVerticalInset) : contentsView.editorDocumentStartY
                        autoFocusOnPress: true
                        backgroundColor: contentsView.showPrintEditorLayout ? "transparent" : contentsView.displayColor
                        backgroundColorDisabled: contentsView.showPrintEditorLayout ? "transparent" : contentsView.displayColor
                        backgroundColorFocused: contentsView.showPrintEditorLayout ? "transparent" : contentsView.displayColor
                        backgroundColorHover: contentsView.showPrintEditorLayout ? "transparent" : contentsView.displayColor
                        backgroundColorPressed: contentsView.showPrintEditorLayout ? "transparent" : contentsView.displayColor
                        centeredTextHeight: contentsView.editorTextLineBoxHeight
                        cornerRadius: 0
                        editorHeight: contentsView.showPrintEditorLayout ? Math.max(0, (Number(printEditorPage.height) || 0) - contentsView.printGuideVerticalInset * 2) : contentsView.editorSurfaceHeight
                        enforceModeDefaults: false
                        fieldMinHeight: contentsView.showPrintEditorLayout ? Math.max(1, (Number(printEditorPage.height) || 0) - contentsView.printGuideVerticalInset * 2) : Math.max(contentsView.minEditorHeight, contentsView.editorSurfaceHeight)
                        fontFamily: LV.Theme.fontBody
                        fontLetterSpacing: 0
                        fontPixelSize: contentsView.effectiveEditorFontPixelSize
                        fontWeight: Font.Medium
                        insetHorizontal: contentsView.showPrintEditorLayout ? 0 : contentsView.editorHorizontalInset
                        insetVertical: contentsView.showPrintEditorLayout ? 0 : contentsView.editorBottomInset
                        placeholderText: ""
                        selectByMouse: true
                        selectedTextColor: LV.Theme.textPrimary
                        selectionColor: LV.Theme.accent
                        shapeStyle: shapeRoundRect
                        showRenderedOutput: true
                        showScrollBar: false
                        text: contentsView.renderedEditorText
                        textColor: contentsView.showPrintEditorLayout ? contentsView.printPaperTextColor : LV.Theme.bodyColor
                        textFormat: TextEdit.RichText
                        visible: !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        wrapMode: TextEdit.Wrap

                        Keys.onPressed: function (event) {
                            contentsView.handleInlineFormatShortcutKeyPress(event);
                        }
                        onFocusedChanged: {
                            if (!focused)
                                contentsView.flushEditorStateAfterInputSettles(0);
                        }
                        onTextEdited: function (_text) {
                            editorTypingController.handleEditorTextEdited();
                        }
                    }
                    Flickable {
                        id: formattedPreviewViewport

                        readonly property real bottomInset: contentsView.showPrintEditorLayout ? contentsView.printGuideVerticalInset : contentsView.editorBottomInset
                        readonly property real horizontalInset: contentsView.showPrintEditorLayout ? (Number(printEditorPage.x) || 0) + contentsView.printGuideHorizontalInset : contentsView.editorHorizontalInset
                        readonly property real textWidth: {
                            if (contentsView.showPrintEditorLayout) {
                                const pageWidth = Number(printEditorPage.width) || 0;
                                return Math.max(0, pageWidth - contentsView.printGuideHorizontalInset * 2);
                            }
                            return Math.max(0, formattedPreviewViewport.width - contentsView.editorHorizontalInset * 2);
                        }
                        readonly property real topInset: contentsView.showPrintEditorLayout ? (Number(printEditorPage.y) || 0) + contentsView.printGuideVerticalInset : contentsView.editorDocumentStartY

                        anchors.fill: parent
                        clip: true
                        contentHeight: Math.max(height, Math.max(0, Number(formattedPreviewText.paintedHeight) || 0) + formattedPreviewViewport.topInset + formattedPreviewViewport.bottomInset)
                        contentWidth: width
                        interactive: contentHeight > height
                        visible: contentsView.showFormattedTextRenderer
                        z: 1

                        Text {
                            id: formattedPreviewText

                            color: contentsView.showPrintEditorLayout ? contentsView.printPaperTextColor : LV.Theme.bodyColor
                            font.family: LV.Theme.fontBody
                            font.letterSpacing: 0
                            font.pixelSize: contentsView.effectiveEditorFontPixelSize
                            font.weight: Font.Medium
                            text: textFormatRenderer.renderedHtml
                            textFormat: Text.RichText
                            width: formattedPreviewViewport.textWidth
                            wrapMode: Text.Wrap
                            x: formattedPreviewViewport.horizontalInset
                            y: formattedPreviewViewport.topInset
                        }
                    }
                    Rectangle {
                        anchors.fill: parent
                        border.color: LV.Theme.primary
                        border.width: contentsView.resourceDropActive ? 1 : 0
                        color: contentsView.resourceDropActive ? "#1A9DA0A8" : "transparent"
                        radius: LV.Theme.radiusSm
                        visible: contentsView.resourceDropActive && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        z: 4
                    }
                    DropArea {
                        anchors.fill: parent
                        enabled: !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        keys: ["text/uri-list"]
                        z: 5

                        onDropped: function (drop) {
                            const dropUrls = contentsView.extractResourceDropUrls(drop);
                            if (!contentsView.canAcceptResourceDropUrls(dropUrls)) {
                                if (drop)
                                    drop.accepted = false;
                                contentsView.resourceDropActive = false;
                                return;
                            }
                            const importedEntries = contentsView.resourcesImportViewModel.importUrlsForEditor(dropUrls);
                            const inserted = contentsView.insertImportedResourceTags(importedEntries);
                            if (drop)
                                drop.accepted = inserted;
                            contentsView.resourceDropActive = false;
                        }
                        onEntered: function (drag) {
                            const dropUrls = contentsView.extractResourceDropUrls(drag);
                            const accepted = contentsView.canAcceptResourceDropUrls(dropUrls);
                            if (drag)
                                drag.accepted = accepted;
                            contentsView.resourceDropActive = accepted;
                        }
                        onExited: {
                            contentsView.resourceDropActive = false;
                        }
                        onPositionChanged: function (drag) {
                            const dropUrls = contentsView.extractResourceDropUrls(drag);
                            const accepted = contentsView.canAcceptResourceDropUrls(dropUrls);
                            if (drag)
                                drag.accepted = accepted;
                            contentsView.resourceDropActive = accepted;
                        }
                    }
                    MouseArea {
                        property real lastPressX: 0
                        property real lastPressY: 0

                        acceptedButtons: Qt.RightButton
                        anchors.fill: parent
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        hoverEnabled: false
                        preventStealing: false
                        propagateComposedEvents: true
                        z: 6

                        onClicked: function (mouse) {
                            if (mouse.button !== Qt.RightButton)
                                return;
                            Qt.callLater(function () {
                                contentsView.openEditorSelectionContextMenu(lastPressX, lastPressY);
                            });
                        }
                        onPressed: function (mouse) {
                            if (mouse.button !== Qt.RightButton)
                                return;
                            lastPressX = mouse.x;
                            lastPressY = mouse.y;
                        }
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+B"

                        onActivated: contentsView.queueInlineFormatWrap("bold")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Ctrl+B"

                        onActivated: contentsView.queueInlineFormatWrap("bold")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+I"

                        onActivated: contentsView.queueInlineFormatWrap("italic")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Ctrl+I"

                        onActivated: contentsView.queueInlineFormatWrap("italic")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+U"

                        onActivated: contentsView.queueInlineFormatWrap("underline")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Ctrl+U"

                        onActivated: contentsView.queueInlineFormatWrap("underline")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+Shift+X"

                        onActivated: contentsView.queueInlineFormatWrap("strikethrough")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Ctrl+Shift+X"

                        onActivated: contentsView.queueInlineFormatWrap("strikethrough")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+Shift+E"

                        onActivated: contentsView.queueInlineFormatWrap("highlight")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Ctrl+Shift+E"

                        onActivated: contentsView.queueInlineFormatWrap("highlight")
                    }
                    LV.ContextMenu {
                        id: editorSelectionContextMenu

                        autoCloseOnTrigger: true
                        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
                        items: contentsView.editorSelectionContextMenuItems
                        modal: false
                        parent: Controls.Overlay.overlay

                        onItemEventTriggered: function (eventName, payload, index, item) {
                            contentsView.handleSelectionContextMenuEvent(eventName);
                        }
                    }
                    // Shared desktop/mobile contract: the outer editor surface owns the 16px top spacer, so LVRS top centering stays disabled.
                    Binding {
                        property: "topPadding"
                        target: contentEditor.editorItem
                        value: contentsView.editorDocumentTopPadding
                    }
                    ContentsResourceViewer {
                        id: dedicatedResourceViewer

                        anchors.fill: parent
                        resourceEntry: contentsView.selectedResourceEntry
                        visible: contentsView.showDedicatedResourceViewer
                        z: 3
                    }
                    Item {
                        id: bodyResourceRenderLayer

                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: contentsView.editorBottomInset
                        anchors.left: parent.left
                        anchors.leftMargin: contentsView.editorHorizontalInset
                        anchors.right: parent.right
                        anchors.rightMargin: contentsView.editorHorizontalInset
                        enabled: visible
                        visible: contentsView.hasSelectedNote && bodyResourceRenderer.resourceCount > 0 && !contentsView.showPrintEditorLayout && !contentsView.showDedicatedResourceViewer
                        z: 2

                        LV.VStack {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            spacing: LV.Theme.gap2

                            Repeater {
                                model: bodyResourceRenderer.renderedResources

                                delegate: Rectangle {
                                    id: resourceRenderCard

                                    readonly property string resourceDisplayName: resourceEntry.displayName !== undefined ? String(resourceEntry.displayName) : ""
                                    readonly property var resourceEntry: modelData && typeof modelData === "object" ? modelData : ({})
                                    readonly property string resourceFormat: resourceEntry.format !== undefined ? String(resourceEntry.format) : ""
                                    readonly property string resourceModeTitle: {
                                        if (resourceRenderCard.resourceRenderMode === "image")
                                            return "Image Resource";
                                        if (resourceRenderCard.resourceRenderMode === "video")
                                            return "Video Resource";
                                        if (resourceRenderCard.resourceRenderMode === "audio")
                                            return "Audio Resource";
                                        if (resourceRenderCard.resourceRenderMode === "pdf")
                                            return "PDF Resource";
                                        if (resourceRenderCard.resourceRenderMode === "text")
                                            return "Text Resource";
                                        return "Document Resource";
                                    }
                                    readonly property bool resourceOpenable: resourceRenderCard.resourceSource.length > 0
                                    readonly property string resourcePath: resourceEntry.resourcePath !== undefined ? String(resourceEntry.resourcePath) : ""
                                    readonly property string resourcePreviewText: resourceEntry.previewText !== undefined ? String(resourceEntry.previewText) : ""
                                    readonly property string resourceRenderMode: resourceEntry.renderMode !== undefined ? String(resourceEntry.renderMode) : ""
                                    readonly property string resourceSource: resourceEntry.source !== undefined ? String(resourceEntry.source) : ""
                                    readonly property string resourceType: resourceEntry.type !== undefined ? String(resourceEntry.type) : ""

                                    border.color: contentsView.resourceRenderBorderColor
                                    border.width: 1
                                    color: contentsView.resourceRenderCardColor
                                    implicitHeight: resourceRow.implicitHeight + LV.Theme.gap2 * 2
                                    radius: LV.Theme.radiusSm
                                    width: parent ? parent.width : 0

                                    RowLayout {
                                        id: resourceRow

                                        anchors.fill: parent
                                        anchors.margins: LV.Theme.gap2
                                        spacing: LV.Theme.gap2

                                        Rectangle {
                                            Layout.preferredHeight: resourceRenderCard.resourceRenderMode === "text" ? 96 : 72
                                            Layout.preferredWidth: 120
                                            color: "#CC0F141A"
                                            radius: LV.Theme.radiusSm
                                            visible: resourceRenderCard.resourceRenderMode === "image" || resourceRenderCard.resourceRenderMode === "text"

                                            Image {
                                                anchors.fill: parent
                                                anchors.margins: 1
                                                asynchronous: true
                                                cache: true
                                                fillMode: Image.PreserveAspectFit
                                                source: resourceRenderCard.resourceSource
                                                visible: resourceRenderCard.resourceRenderMode === "image" && resourceRenderCard.resourceSource.length > 0
                                            }
                                            LV.Label {
                                                anchors.fill: parent
                                                anchors.margins: LV.Theme.gap2
                                                color: LV.Theme.textSecondary
                                                elide: Text.ElideRight
                                                style: body
                                                text: resourceRenderCard.resourcePreviewText.length > 0 ? resourceRenderCard.resourcePreviewText : "No text preview"
                                                visible: resourceRenderCard.resourceRenderMode === "text"
                                                wrapMode: Text.Wrap
                                            }
                                        }
                                        LV.VStack {
                                            Layout.fillWidth: true
                                            spacing: LV.Theme.gap2

                                            LV.Label {
                                                color: LV.Theme.textPrimary
                                                style: body
                                                text: resourceRenderCard.resourceDisplayName.length > 0 ? resourceRenderCard.resourceDisplayName : resourceRenderCard.resourceModeTitle
                                            }
                                            LV.Label {
                                                color: LV.Theme.textSecondary
                                                style: body
                                                text: resourceRenderCard.resourceModeTitle
                                            }
                                            LV.Label {
                                                color: LV.Theme.textSecondary
                                                style: body
                                                text: "type=" + resourceRenderCard.resourceType + "  format=" + resourceRenderCard.resourceFormat
                                            }
                                            LV.Label {
                                                color: LV.Theme.textTertiary
                                                elide: Text.ElideMiddle
                                                style: body
                                                text: resourceRenderCard.resourcePath
                                            }
                                            LV.IconButton {
                                                iconName: "generalshow"
                                                iconSize: 14
                                                visible: resourceRenderCard.resourceOpenable

                                                onClicked: Qt.openUrlExternally(resourceRenderCard.resourceSource)
                                            }
                                        }
                                    }
                                }
                            }
                        }
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
                    minimapCurrentLineColor: contentsView.minimapCurrentLineColor
                    minimapCurrentLineHeight: contentsView.minimapResolvedCurrentLineHeight
                    minimapCurrentLineWidth: contentsView.minimapResolvedCurrentLineWidth
                    minimapCurrentLineY: contentsView.minimapResolvedCurrentLineY
                    minimapLineColor: contentsView.minimapLineColor
                    minimapScrollable: contentsView.minimapScrollable
                    minimapSilhouetteHeight: contentsView.minimapResolvedSilhouetteHeight
                    minimapTrackInset: contentsView.minimapTrackInset
                    minimapTrackWidth: contentsView.minimapTrackWidth
                    minimapViewportFillColor: contentsView.minimapViewportFillColor
                    minimapViewportHeight: contentsView.minimapResolvedViewportHeight
                    minimapViewportY: contentsView.minimapResolvedViewportY
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
                    visible: contentsView.minimapVisible && !contentsView.showDedicatedResourceViewer && !contentsView.showPrintEditorLayout && !contentsView.showFormattedTextRenderer
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
            Layout.preferredHeight: drawerView.effectiveDrawerHeight
            clip: true
            color: contentsView.drawerColor
            visible: contentsView.drawerVisible

            ColumnLayout {
                anchors.fill: parent
                spacing: LV.Theme.gapNone

                DrawerMenuBar {
                    Layout.fillWidth: true
                    activeDrawerMode: contentsView.activeDrawerMode

                    onDrawerModeRequested: function (modeName) {
                        if (modeName !== contentsView.drawerModeQuickNote)
                            return;
                        if (contentsView.activeDrawerMode !== modeName)
                            contentsView.activeDrawerMode = modeName;
                    }
                    onViewHookRequested: function (reason) {
                        contentsView.requestViewHook(reason);
                    }
                }
                DrawerContents {
                    id: drawerContents

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    activeDrawerMode: contentsView.activeDrawerMode
                    quickNoteText: contentsView.drawerQuickNoteText

                    onQuickNoteTextEdited: function (text) {
                        contentsView.drawerQuickNoteText = text;
                    }
                    onViewHookRequested: function (reason) {
                        contentsView.requestViewHook(reason);
                    }
                }
                DrawerToolbar {
                    Layout.fillWidth: true

                    onViewHookRequested: function (reason) {
                        contentsView.requestViewHook(reason);
                    }
                }
            }
        }
    }
}
