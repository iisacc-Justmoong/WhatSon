pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "ContentsEditorDebugTrace.js" as EditorTrace
import "ContentsMinimapSnapshotSupport.js" as MinimapSnapshotSupport

Item {
    id: contentsView
    objectName: contentsView.mobileHost ? "mobileContentsDisplayView" : "contentsDisplayView"

    readonly property int activeEditorViewModeValue: {
        const viewModeModel = contentsView.resolvedEditorViewModeViewModel;
        if (viewModeModel && viewModeModel.activeViewMode !== undefined) {
            const activeValue = Number(viewModeModel.activeViewMode);
            if (isFinite(activeValue))
                return Math.max(0, Math.floor(activeValue));
        }
        return contentsView.plainEditorViewModeValue;
    }
    readonly property color activeLineNumberColor: LV.Theme.accentGray
    readonly property bool contentPersistenceContractAvailable: selectionBridge.contentPersistenceContractAvailable
    property var contentViewModel: null
    property bool mobileHost: false
    readonly property var contentEditor: contentEditorLoader.item ? contentEditorLoader.item : contentEditorProxy
    property alias contextMenuSelectionEnd: editorSelectionController.contextMenuSelectionEnd
    property alias contextMenuSelectionStart: editorSelectionController.contextMenuSelectionStart
    property int structuredContextMenuBlockIndex: -1
    property var structuredContextMenuSelectionSnapshot: ({})
    readonly property int currentCursorLineNumber: contentsView.structuredHostGeometryActive
                                                   ? (structuredDocumentFlow && structuredDocumentFlow.currentLogicalLineNumber !== undefined
                                                      ? Math.max(1, Number(structuredDocumentFlow.currentLogicalLineNumber) || 1)
                                                      : 1)
                                                   : contentsView.showStructuredDocumentFlow
                                                     ? 1
                                                   : contentsView.logicalLineNumberForOffset(Number(contentEditor.cursorPosition) || 0)
    readonly property color decorativeMarkerYellow: LV.Theme.warning
    readonly property int desktopEditorFontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property int editorFontWeight: modePolicy.editorFontWeight
    property color displayColor: "transparent"
    readonly property int editorBottomInset: Math.max(
                                                LV.Theme.gap16,
                                                Math.round(contentsView.editorLineHeight * 6),
                                                Math.round(contentsView.editorSurfaceHeight * 0.5))
    property alias editorBoundNoteId: editorSession.editorBoundNoteId
    readonly property real editorContentOffsetY: {
        const flickable = contentsView.editorFlickable;
        if (flickable && flickable.contentY !== undefined)
            return -(Number(flickable.contentY) || 0);
        if (contentEditor && contentEditor.contentOffsetY !== undefined)
            return Number(contentEditor.contentOffsetY) || 0;
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
    readonly property int editorHorizontalInset: modePolicy.editorHorizontalInset
    readonly property bool editorInputFocused: {
        if (structuredDocumentFlow && structuredDocumentFlow.focused !== undefined && structuredDocumentFlow.focused)
            return true;
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
    property alias editorSelectionContextMenuItems: editorSelectionController.contextMenuItems
    readonly property real editorSurfaceHeight: Math.max(0, contentsView.editorViewportHeight - contentsView.editorDocumentStartY)
    property alias editorText: editorSession.editorText
    readonly property int editorTextLineBoxHeight: contentsView.effectiveEditorFontPixelSize
    readonly property int editorTopInset: LV.Theme.gap4
    property int editorTopInsetOverride: -1
    property var editorViewModeViewModel: null
    readonly property real editorViewportHeight: editorViewport ? Number(editorViewport.height) || 0 : 0
    readonly property int effectiveEditorFontPixelSize: contentsView.desktopEditorFontPixelSize
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
    readonly property int frameHorizontalInset: LV.Theme.gap2
    property int frameHorizontalInsetOverride: -1
    property color gutterColor: "transparent"
    readonly property int gutterCommentMarkerOffset: LV.Theme.gap2
    readonly property int gutterCommentRailLeft: LV.Theme.gap4
    readonly property int gutterCommentRailWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(10)))
    readonly property int gutterIconRailLeft: Math.max(0, Math.round(LV.Theme.scaleMetric(40)))
    readonly property int gutterIconRailWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(18)))
    readonly property color gutterMarkerChangedColor: contentsView.decorativeMarkerYellow
    readonly property color gutterMarkerConflictColor: LV.Theme.danger
    readonly property color gutterMarkerCurrentColor: LV.Theme.primary
    property var gutterMarkers: []
    property int gutterRefreshPassesRemaining: 0
    property int gutterRefreshRevision: 0
    readonly property real gutterViewportHeight: contentsView.editorViewportHeight
    readonly property int gutterWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(74)))
    readonly property int gutterBodyGap: Math.max(0, Math.round(LV.Theme.scaleMetric(8)))
    property int gutterWidthOverride: -1
    readonly property bool hasSelectedNote: contentsView.selectedNoteId.length > 0
    property var libraryHierarchyViewModel: null
    readonly property color lineNumberColor: LV.Theme.descriptionColor
    readonly property int lineNumberColumnLeft: Math.max(0, Math.round(LV.Theme.scaleMetric(14)))
    property int lineNumberColumnLeftOverride: -1
    readonly property int lineNumberColumnTextWidth: contentsView.gutterWidth - contentsView.lineNumberColumnLeft - contentsView.lineNumberRightInset
    property int lineNumberColumnTextWidthOverride: -1
    readonly property int lineNumberColumnWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(26)))
    readonly property int lineNumberRightInset: contentsView.gutterBodyGap
    property int liveLogicalLineCount: 1
    readonly property int logicalLineCount: Math.max(1, Number(contentsView.liveLogicalLineCount) || 1)
    property var logicalLineDocumentYCache: []
    property int logicalLineDocumentYCacheLineCount: 0
    property int logicalLineDocumentYCacheRevision: -1
    property var logicalLineGutterDocumentYCache: []
    property int logicalLineGutterDocumentYCacheLineCount: 0
    property int logicalLineGutterDocumentYCacheRevision: -1
    property string structuredGutterGeometrySignature: ""
    property var liveLogicalLineStartOffsets: [0]
    readonly property var logicalLineStartOffsets: contentsView.liveLogicalLineStartOffsets
    property int liveLogicalTextLength: 0
    readonly property bool lineGeometryRefreshEnabled: modePolicy.lineGeometryRefreshEnabled
    readonly property int minEditorHeight: LV.Theme.gap20 * 12
    readonly property real minimapAvailableTrackHeight: Math.max(1, contentsView.editorViewportHeight - contentsView.minimapTrackInset * 2)
    readonly property color minimapCurrentLineColor: contentsView.activeLineNumberColor
    readonly property color minimapLineColor: contentsView.lineNumberColor
    readonly property int minimapOuterWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(56)))
    property real minimapResolvedCurrentLineHeight: 1
    property real minimapResolvedCurrentLineWidth: 0
    property real minimapResolvedCurrentLineY: 0
    property real minimapResolvedSilhouetteHeight: 1
    property real minimapResolvedTrackHeight: 1
    readonly property real minimapResolvedTrackWidth: contentsView.minimapTrackWidth
    property real minimapResolvedViewportHeight: 0
    property real minimapResolvedViewportY: 0
    readonly property int minimapRowGap: Math.max(1, Math.round(LV.Theme.scaleMetric(1)))
    property var minimapLineGroups: []
    property string minimapLineGroupsNoteId: ""
    property bool minimapScrollable: false
    property bool minimapSnapshotForceFullRefresh: true
    property bool cursorDrivenUiRefreshQueued: false
    property bool typingViewportCorrectionQueued: false
    property bool typingViewportForceCorrectionRequested: false
    property bool viewportGutterRefreshQueued: false
    property bool minimapSnapshotRefreshQueued: false
    property string minimapSnapshotSourceText: ""
    readonly property int minimapTrackInset: LV.Theme.gap8
    readonly property int minimapTrackWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(36)))
    readonly property color minimapViewportFillColor: LV.Theme.accentTransparent
    readonly property int minimapViewportMinHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(28)))
    property bool minimapVisible: true
    readonly property bool showMinimapRail: modePolicy.showMinimapRail
    readonly property bool minimapRefreshEnabled: modePolicy.minimapRefreshEnabled
    property var minimapVisualRows: []
    readonly property var normalizedExternalGutterMarkers: gutterMarkerBridge.normalizedExternalGutterMarkers
    readonly property bool noteCountContractAvailable: selectionBridge.noteCountContractAvailable
    property var noteListModel: null
    readonly property bool noteSelectionContractAvailable: selectionBridge.noteSelectionContractAvailable
    readonly property bool typingSessionSyncProtected: editorSession && editorSession.isTypingSessionActive !== undefined && editorSession.isTypingSessionActive()
    readonly property bool noteSnapshotRefreshEnabled: contentsView.visible && contentsView.hasSelectedNote && !contentsView.selectedNoteBodyLoading && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer && contentsView.noteSelectionContractAvailable && !contentsView.editorInputFocused && !contentsView.typingSessionSyncProtected && !contentsView.pendingBodySave
    readonly property int noteSnapshotRefreshIntervalMs: 1200
    readonly property int pageEditorViewModeValue: pagePrintLayoutRenderer.pageViewModeValue
    property var panelViewModel: null
    property alias pendingBodySave: editorSession.pendingBodySave
    readonly property string editorEntrySnapshotComparedNoteId: selectionSyncCoordinator.comparedSnapshotNoteId
    readonly property string editorEntrySnapshotPendingNoteId: selectionSyncCoordinator.pendingSnapshotNoteId
    property string pendingNoteEntryGutterRefreshNoteId: ""
    readonly property string pendingEditorFocusNoteId: selectionSyncCoordinator.pendingEditorFocusNoteId
    readonly property int plainEditorViewModeValue: 0
    readonly property color printCanvasColor: pagePrintLayoutRenderer.canvasColor
    readonly property int printEditorViewModeValue: pagePrintLayoutRenderer.printViewModeValue
    readonly property real printGuideHorizontalInset: pagePrintLayoutRenderer.guideHorizontalInset
    readonly property real printGuideVerticalInset: pagePrintLayoutRenderer.guideVerticalInset
    readonly property real printPaperAspectRatio: pagePrintLayoutRenderer.paperAspectRatio
    readonly property color printPaperBorderColor: pagePrintLayoutRenderer.paperBorderColor
    readonly property color printPaperColor: pagePrintLayoutRenderer.paperColor
    readonly property color printPaperHighlightColor: pagePrintLayoutRenderer.paperHighlightColor
    readonly property color printPaperShadeColor: pagePrintLayoutRenderer.paperShadeColor
    readonly property color printPaperSeparatorColor: pagePrintLayoutRenderer.paperSeparatorColor
    readonly property color printPaperShadowColor: pagePrintLayoutRenderer.paperShadowColor
    readonly property real printPaperShadowOffsetX: pagePrintLayoutRenderer.paperShadowOffsetX
    readonly property real printPaperShadowOffsetY: pagePrintLayoutRenderer.paperShadowOffsetY
    readonly property real printPaperDocumentHeight: pagePrintLayoutRenderer.paperDocumentHeight
    readonly property real printPaperHorizontalMargin: pagePrintLayoutRenderer.paperHorizontalMargin
    readonly property real printPaperMaxWidth: pagePrintLayoutRenderer.paperMaxWidth
    readonly property real printPaperSeparatorThickness: pagePrintLayoutRenderer.paperSeparatorThickness
    readonly property color printPaperTextColor: pagePrintLayoutRenderer.paperTextColor
    readonly property real printPaperTextHeight: pagePrintLayoutRenderer.paperTextHeight
    readonly property real printPaperTextWidth: pagePrintLayoutRenderer.paperTextWidth
    readonly property real printPaperVerticalMargin: pagePrintLayoutRenderer.paperVerticalMargin
    readonly property int printDocumentPageCount: pagePrintLayoutRenderer.documentPageCount
    readonly property real printDocumentSurfaceHeight: pagePrintLayoutRenderer.documentSurfaceHeight
    readonly property real printPaperResolvedHeight: pagePrintLayoutRenderer.paperResolvedHeight
    readonly property real printPaperResolvedWidth: pagePrintLayoutRenderer.paperResolvedWidth
    readonly property int documentPresentationRefreshIntervalMs: 120
    readonly property string documentPresentationSourceText: contentsView.resolvedDocumentPresentationSourceText()
    readonly property bool documentPresentationRefreshPendingWhileFocused: presentationRefreshController.pendingWhileFocused
    readonly property string nativeInputDisplayText: modePolicy.nativeInputDisplayText
    property string renderedEditorHtml: ""
    readonly property var resolvedEditorViewModeViewModel: {
        if (contentsView.editorViewModeViewModel)
            return contentsView.editorViewModeViewModel;
        if (LV.ViewModels && LV.ViewModels.get !== undefined)
            return LV.ViewModels.get("editorViewModeViewModel");
        return null;
    }
    property bool resourceDropActive: false
    readonly property bool resourceDropEditorSurfaceGuardActive: resourceImportController.resourceDropEditorSurfaceGuardActive
    readonly property int resourceImportConflictPolicyAbort: 0
    readonly property int resourceImportConflictPolicyOverwrite: 1
    readonly property int resourceImportConflictPolicyKeepBoth: 2
    readonly property int resourceImportModeNone: 0
    readonly property int resourceImportModeUrls: 1
    readonly property int resourceImportModeClipboard: 2
    readonly property bool resourceImportConflictAlertOpen: resourceImportController.resourceImportConflictAlertOpen
    readonly property color resourceRenderBorderColor: "#334E5157"
    readonly property color resourceRenderCardColor: "#E61A1D22"
    readonly property int resourceRenderDisplayLimit: 0
    property var resourcesImportViewModel: null
    property var sidebarHierarchyViewModel: null
    readonly property bool preferNativeInputHandling: contentsView.parsedStructuredFlowRequested
    readonly property bool documentPresentationProjectionEnabled: !contentsView.preferNativeInputHandling
                                                                    || contentsView.showFormattedTextRenderer
    readonly property bool inlineHtmlImageRenderingEnabled: !contentsView.preferNativeInputHandling
    readonly property int resourceEditorPlaceholderLineCount: 1
    readonly property int editorIdleSyncThresholdMs: 1000
    readonly property string selectedNoteBodyText: selectionBridge.selectedNoteBodyText
    readonly property string selectedNoteBodyNoteId: selectionBridge.selectedNoteBodyNoteId
    readonly property bool selectedNoteBodyLoading: selectionBridge.selectedNoteBodyLoading
    readonly property string selectedNoteId: selectionBridge.selectedNoteId
    readonly property bool showCurrentLineMarker: contentsView.hasSelectedNote || contentsView.editorText.length > 0 || contentsView.editorInputFocused
    readonly property bool editorSessionBoundToSelectedNote: editorSession.editorBoundNoteId === contentsView.selectedNoteId
    readonly property string structuredFlowSourceText: contentsView.documentPresentationSourceText
    readonly property bool liveResourceStructuredFlowRequested: contentsView.sourceContainsCanonicalResourceTag(
                                                                    contentsView.documentPresentationSourceText)
    readonly property bool parsedStructuredFlowRequested: contentsView.hasSelectedNote
    readonly property bool structuredDocumentFlowEnabled: contentsView.parsedStructuredFlowRequested
    readonly property bool resourceResolverNeedsLiveEditorSource: contentsView.showStructuredDocumentFlow
                                                                  || contentsView.liveResourceStructuredFlowRequested
    readonly property bool legacyInlineEditorActive: !contentsView.showStructuredDocumentFlow
                                                     && !contentsView.showDedicatedResourceViewer
                                                     && !contentsView.showFormattedTextRenderer
    readonly property bool resourceBlocksRenderedInlineByHtmlProjection: contentsView.legacyInlineEditorActive
    readonly property bool programmaticEditorSurfaceSyncActive: resourceImportController.programmaticEditorSurfaceSyncActive
    readonly property bool showDedicatedResourceViewer: false
    readonly property bool showEditorGutter: modePolicy.showEditorGutter
    readonly property bool showFormattedTextRenderer: false
    readonly property bool showStructuredDocumentFlow: contentsView.structuredDocumentFlowEnabled
                                                       && !contentsView.showDedicatedResourceViewer
                                                       && !contentsView.showFormattedTextRenderer
    readonly property bool structuredHostGeometryActive: modePolicy.structuredHostGeometryActive
    readonly property bool showPageEditorLayout: pagePrintLayoutRenderer.showPageEditorLayout
    readonly property bool showPrintEditorLayout: pagePrintLayoutRenderer.showPrintEditorLayout
    readonly property bool showPrintMarginGuides: pagePrintLayoutRenderer.showPrintMarginGuides
    readonly property bool showPrintModeActive: pagePrintLayoutRenderer.showPrintModeActive
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

    signal editorTextEdited(string text)
    signal viewHookRequested

    function resolvedDocumentPresentationSourceText() {
        return sessionCoordinator.resolvedDocumentPresentationSourceText();
    }
    function activeLogicalTextSnapshot() {
        if (editorTypingController && editorTypingController.currentEditorPlainText !== undefined) {
            const livePlainText = editorTypingController.currentEditorPlainText();
            if (livePlainText !== undefined && livePlainText !== null)
                return String(livePlainText);
        }
        if (editorProjection && editorProjection.logicalText !== undefined && editorProjection.logicalText !== null)
            return String(editorProjection.logicalText);
        return "";
    }
    function normalizedStructuredLogicalLineEntries() {
        if (!contentsView.structuredHostGeometryActive || !structuredDocumentFlow)
            return [];
        const rawEntries = structuredDocumentFlow.cachedLogicalLineEntries !== undefined
                ? structuredDocumentFlow.cachedLogicalLineEntries
                : (structuredDocumentFlow.logicalLineEntries !== undefined
                   ? structuredDocumentFlow.logicalLineEntries()
                   : []);
        return structuredFlowCoordinator.normalizeStructuredLogicalLineEntries(rawEntries);
    }
    function structuredLogicalLineEntryAt(lineNumber) {
        const lineEntries = contentsView.normalizedStructuredLogicalLineEntries();
        if (lineEntries.length === 0)
            return null;
        const safeLineNumber = Math.floor(Number(lineNumber) || 0);
        if (safeLineNumber < 1 || safeLineNumber > lineEntries.length)
            return null;
        const entry = lineEntries[safeLineNumber - 1];
        return entry && typeof entry === "object" ? entry : null;
    }
    function effectiveStructuredLogicalLineEntries() {
        return contentsView.normalizedStructuredLogicalLineEntries();
    }
    function currentStructuredGutterGeometrySignature() {
        return viewportCoordinator.structuredGutterGeometrySignature(
                    contentsView.effectiveStructuredLogicalLineEntries());
    }
    function consumeStructuredGutterGeometryChange() {
        const state = structuredFlowCoordinator.evaluateStructuredLayoutState(
                    contentsView.effectiveStructuredLogicalLineEntries());
        contentsView.structuredGutterGeometrySignature = String(state.signature || "");
        return !!state.geometryChanged;
    }
    function buildStructuredMinimapLineGroupsForRange(startLineNumber, endLineNumber) {
        const lineEntries = contentsView.effectiveStructuredLogicalLineEntries();
        const groups = minimapCoordinator.buildStructuredMinimapLineGroupsForRange(
                    lineEntries,
                    Number(startLineNumber) || 1,
                    Number(endLineNumber) || Number(startLineNumber) || 1);
        return groups.length > 0 ? groups : contentsView.buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber);
    }
    function buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber) {
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLineNumber) || 1));
        const safeEndLine = Math.max(safeStartLine, Math.min(contentsView.logicalLineCount, Number(endLineNumber) || safeStartLine));
        const lineCharacterCounts = [];
        const lineDocumentYs = [];
        const lineVisualHeights = [];
        for (let lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber) {
            lineCharacterCounts.push(contentsView.logicalLineCharacterCountAt(lineNumber - 1));
            lineDocumentYs.push(contentsView.lineDocumentY(lineNumber));
            lineVisualHeights.push(contentsView.lineVisualHeight(lineNumber, 1));
        }
        return minimapCoordinator.buildFallbackMinimapLineGroupsForRange(
                    lineCharacterCounts,
                    lineDocumentYs,
                    lineVisualHeights,
                    safeStartLine,
                    safeEndLine);
    }
    function buildMinimapLineGroupsForRange(startLineNumber, endLineNumber) {
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLineNumber) || 1));
        const safeEndLine = Math.max(safeStartLine, Math.min(contentsView.logicalLineCount, Number(endLineNumber) || safeStartLine));
        if (contentsView.structuredHostGeometryActive)
            return contentsView.buildStructuredMinimapLineGroupsForRange(safeStartLine, safeEndLine);
        const editorItem = contentEditor ? contentEditor.editorItem : null;
        const editorWidth = Number(contentEditor ? contentEditor.width : 0) || 0;
        const editorContentHeight = Number(contentEditor ? contentEditor.contentHeight : 0) || 0;
        const lineCharacterCounts = [];
        const lineStartOffsets = [];
        const fallbackLineDocumentYs = [];
        const fallbackLineVisualHeights = [];
        const editorRects = [];
        const logicalLength = contentsView.logicalTextLength();
        for (let lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber) {
            const lineIndex = lineNumber - 1;
            const startOffset = contentsView.logicalLineStartOffsetAt(lineIndex);
            lineCharacterCounts.push(contentsView.logicalLineCharacterCountAt(lineIndex));
            lineStartOffsets.push(startOffset);
            fallbackLineDocumentYs.push(contentsView.lineDocumentY(lineNumber));
            fallbackLineVisualHeights.push(contentsView.lineVisualHeight(lineNumber, 1));
            if (editorItem && editorItem.positionToRectangle !== undefined)
                editorRects.push(editorItem.positionToRectangle(startOffset));
        }
        if (editorItem && editorItem.positionToRectangle !== undefined)
            editorRects.push(editorItem.positionToRectangle(Math.max(0, logicalLength)));
        return minimapCoordinator.buildEditorMinimapLineGroupsForRange(
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
    function currentMinimapSourceText() {
        return sessionCoordinator.currentMinimapSourceText(contentsView.structuredHostGeometryActive);
    }
    function activeLineGeometryNoteId() {
        return viewportCoordinator.normalizedNoteId(contentsView.selectedNoteId);
    }
    function nextMinimapLineGroupsForCurrentState(currentSourceText) {
        const currentNoteId = contentsView.activeLineGeometryNoteId();
        const snapshotPlan = minimapCoordinator.buildNextMinimapSnapshotPlan(
                    contentsView.minimapLineGroups,
                    contentsView.minimapLineGroupsNoteId,
                    currentNoteId,
                    contentsView.minimapSnapshotSourceText,
                    currentSourceText,
                    contentsView.minimapSnapshotForceFullRefresh,
                    contentsView.hasPendingNoteEntryGutterRefresh(currentNoteId),
                    contentsView.effectiveStructuredLogicalLineEntries().length,
                    contentsView.logicalLineCount);
        if (snapshotPlan.reuseExisting)
            return contentsView.minimapLineGroups;
        if (snapshotPlan.requiresFullRebuild) {
            return contentsView.structuredHostGeometryActive
                    ? contentsView.buildStructuredMinimapLineGroupsForRange(1, Math.max(1, contentsView.effectiveStructuredLogicalLineEntries().length))
                    : contentsView.buildMinimapLineGroupsForRange(1, contentsView.logicalLineCount);
        }

        const replacementGroups = contentsView.structuredHostGeometryActive
                ? contentsView.buildStructuredMinimapLineGroupsForRange(
                      Number(snapshotPlan.replacementStartLine) || 1,
                      Number(snapshotPlan.replacementEndLine) || 1)
                : contentsView.buildMinimapLineGroupsForRange(
                      Number(snapshotPlan.replacementStartLine) || 1,
                      Number(snapshotPlan.replacementEndLine) || 1);
        const mergedGroups = MinimapSnapshotSupport.spliceLineGroups(
                    contentsView.minimapLineGroups,
                    replacementGroups,
                    Number(snapshotPlan.previousStartLine) || 1,
                    Number(snapshotPlan.previousEndLine) || 1);
        const expectedLineCount = contentsView.structuredHostGeometryActive
                ? Math.max(1, contentsView.effectiveStructuredLogicalLineEntries().length)
                : contentsView.logicalLineCount;
        if (!Array.isArray(mergedGroups) || mergedGroups.length !== expectedLineCount) {
            return contentsView.structuredHostGeometryActive
                    ? contentsView.buildStructuredMinimapLineGroupsForRange(1, expectedLineCount)
                    : contentsView.buildMinimapLineGroupsForRange(1, expectedLineCount);
        }
        return mergedGroups;
    }
    function buildVisibleGutterLineEntries() {
        if (contentsView.structuredHostGeometryActive) {
            return structuredFlowCoordinator.buildVisibleStructuredGutterLineEntries(
                        contentsView.effectiveStructuredLogicalLineEntries(),
                        contentsView.firstVisibleLogicalLine());
        }
        const firstVisibleLine = contentsView.firstVisibleLogicalLine();
        const gutterLineYs = [];
        const gutterLineHeights = [];
        for (let lineNumber = firstVisibleLine; lineNumber <= contentsView.logicalLineCount; ++lineNumber) {
            gutterLineYs.push(contentsView.gutterLineY(lineNumber));
            gutterLineHeights.push(contentsView.gutterLineVisualHeight(lineNumber, 1));
        }
        return gutterCoordinator.buildVisiblePlainGutterLineEntries(
                    firstVisibleLine,
                    gutterLineYs,
                    gutterLineHeights);
    }
    function canAcceptResourceDropUrls(urls) {
        return resourceImportController.canAcceptResourceDropUrls(urls);
    }
    function clearPendingResourceImportConflict() {
        resourceImportController.clearPendingResourceImportConflict();
    }
    function normalizedResourceImportConflict(conflict) {
        return resourceImportController.normalizedResourceImportConflict(conflict);
    }
    function resourceImportConflictAlertMessage() {
        return resourceImportController.resourceImportConflictAlertMessage();
    }
    function scheduleResourceImportConflictPrompt(importMode, urls, conflict) {
        return resourceImportController.scheduleResourceImportConflictPrompt(importMode, urls, conflict);
    }
    function finalizeInsertedImportedResources(importedEntries) {
        return resourceImportController.finalizeInsertedImportedResources(importedEntries);
    }
    function cancelPendingResourceImportConflict() {
        resourceImportController.cancelPendingResourceImportConflict();
    }
    function executePendingResourceImportWithPolicy(conflictPolicy) {
        return resourceImportController.executePendingResourceImportWithPolicy(conflictPolicy);
    }
    function importUrlsAsResourcesWithPrompt(urls) {
        return resourceImportController.importUrlsAsResourcesWithPrompt(urls);
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
        const lineCountChanged = contentsView.liveLogicalLineCount !== normalizedLineCount;
        const textLengthChanged = contentsView.liveLogicalTextLength !== normalizedLogicalTextLength;
        const lineOffsetsChanged = !contentsView.logicalLineOffsetsEqual(
                    contentsView.liveLogicalLineStartOffsets,
                    normalizedLineStartOffsets);
        if (textLengthChanged)
            contentsView.liveLogicalTextLength = normalizedLogicalTextLength;
        if (lineOffsetsChanged)
            contentsView.liveLogicalLineStartOffsets = normalizedLineStartOffsets;
        if (lineCountChanged)
            contentsView.liveLogicalLineCount = normalizedLineCount;
        return lineCountChanged || textLengthChanged || lineOffsetsChanged;
    }
    function hasPendingNoteEntryGutterRefresh(noteId) {
        return viewportCoordinator.hasPendingNoteEntryGutterRefresh(
                    contentsView.pendingNoteEntryGutterRefreshNoteId,
                    noteId === undefined ? null : String(noteId));
    }
    function finalizePendingNoteEntryGutterRefresh(noteId, reason, refreshStructuredLayout) {
        const plan = viewportCoordinator.finalizePendingNoteEntryGutterRefresh(
                    contentsView.pendingNoteEntryGutterRefreshNoteId,
                    noteId === undefined || noteId === null ? "" : String(noteId),
                    contentsView.selectedNoteBodyLoading,
                    reason === undefined || reason === null ? "" : String(reason),
                    !!refreshStructuredLayout);
        if (!plan.clearPendingNoteId)
            return false;
        contentsView.pendingNoteEntryGutterRefreshNoteId = "";
        if (plan.refreshStructuredLayoutNow
                && structuredDocumentFlow
                && structuredDocumentFlow.refreshLayoutCache !== undefined)
            structuredDocumentFlow.refreshLayoutCache();
        if (plan.commitGutterRefresh)
            contentsView.commitGutterRefresh();
        if (plan.scheduleViewportGutterRefresh)
            contentsView.scheduleViewportGutterRefresh();
        if (plan.scheduleMinimapSnapshotRefresh)
            contentsView.scheduleMinimapSnapshotRefresh(!!plan.scheduleMinimapSnapshotForceFull);
        if (plan.scheduleGutterRefresh)
            contentsView.scheduleGutterRefresh(Number(plan.gutterPassCount) || 0,
                                               String(plan.gutterReason || ""));
        return true;
    }
    function commitGutterRefresh() {
        contentsView.refreshLiveLogicalLineMetrics();
        contentsView.gutterRefreshRevision += 1;
        contentsView.visibleGutterLineEntries = contentsView.buildVisibleGutterLineEntries();
        contentsView.traceVisibleGutterSnapshot("commit");
    }
    function refreshVisibleGutterEntries() {
        contentsView.visibleGutterLineEntries = contentsView.buildVisibleGutterLineEntries();
        contentsView.traceVisibleGutterSnapshot("refresh");
    }
    function traceVisibleGutterSnapshot(reason) {
        if (!contentsView.hasSelectedNote)
            return;
        const startLine = 14;
        const endLine = Math.min(contentsView.logicalLineCount, 27);
        const parts = [];
        for (let lineNumber = startLine; lineNumber <= endLine; ++lineNumber) {
            const lineGroup = contentsView.incrementalLineGeometryAvailable()
                    ? contentsView.minimapLineGroups[lineNumber - 1]
                    : null;
            parts.push("L" + lineNumber
                       + "{lineY=" + Math.round(contentsView.lineY(lineNumber))
                       + ",gutterY=" + Math.round(contentsView.gutterLineY(lineNumber))
                       + ",lineH=" + Math.round(contentsView.lineVisualHeight(lineNumber, 1))
                       + ",gutterH=" + Math.round(contentsView.gutterLineVisualHeight(lineNumber, 1))
                       + ",rows=" + Math.max(1, Math.ceil(Number(lineGroup && lineGroup.rowCount !== undefined ? lineGroup.rowCount : 1) || 1))
                       + "}");
        }
        EditorTrace.trace("displayView",
                          "gutterSnapshot",
                          "reason=" + reason
                          + " count=" + contentsView.visibleGutterLineEntries.length
                          + " lines=[" + parts.join(", ") + "]",
                          contentsView);
    }
    function contextMenuEditorSelectionRange() {
        return editorSelectionController.contextMenuEditorSelectionRange();
    }
    function currentCursorVisualLineHeight() {
        const rect = contentsView.currentCursorVisualRowRect();
        return Math.max(1, Number(rect.height) || contentsView.editorLineHeight);
    }
    function currentCursorGutterLineHeight() {
        return contentsView.currentCursorVisualLineHeight();
    }
    function currentCursorVisualLineY() {
        const rect = contentsView.currentCursorVisualRowRect();
        return contentsView.editorViewportYForDocumentY(Number(rect.y) || 0);
    }
    function currentCursorGutterLineY() {
        return contentsView.currentCursorVisualLineY();
    }
    function currentCursorVisualRowRect() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        if (contentsView.structuredHostGeometryActive) {
            const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, contentsView.currentCursorLineNumber));
            const structuredRect = structuredDocumentFlow && structuredDocumentFlow.currentCursorVisualRowRect !== undefined
                    ? structuredDocumentFlow.currentCursorVisualRowRect()
                    : ({ });
            return minimapCoordinator.currentCursorVisualRowRectFromStructuredRect(
                        structuredRect,
                        safeLineNumber,
                        contentsView.lineDocumentY(safeLineNumber),
                        contentsView.lineVisualHeight(safeLineNumber, 1));
        }
        const safeOffset = Math.max(0, Math.min(contentsView.logicalTextLength(), Number(contentEditor.cursorPosition) || 0));
        const rect = contentEditor.editorItem && contentEditor.editorItem.positionToRectangle !== undefined
                ? contentEditor.editorItem.positionToRectangle(safeOffset)
                : ({ });
        return minimapCoordinator.currentCursorVisualRowRectFromTextRect(
                    rect,
                    safeOffset,
                    contentsView.documentYForOffset(safeOffset));
    }
    function typingViewportBandTop(cursorHeight) {
        const safeCursorHeight = Math.max(1, Number(cursorHeight) || contentsView.editorLineHeight);
        const surfaceHeight = Math.max(safeCursorHeight, Number(contentsView.editorSurfaceHeight) || 0);
        return contentsView.editorDocumentStartY + Math.max(
                    safeCursorHeight,
                    Math.round(surfaceHeight * 0.18));
    }
    function typingViewportBandBottom(cursorHeight) {
        const safeCursorHeight = Math.max(1, Number(cursorHeight) || contentsView.editorLineHeight);
        const surfaceHeight = Math.max(safeCursorHeight, Number(contentsView.editorSurfaceHeight) || 0);
        return contentsView.editorDocumentStartY + Math.max(
                    safeCursorHeight * 2,
                    Math.round(surfaceHeight * 0.56));
    }
    function typingViewportAnchorCenter(cursorHeight) {
        const safeCursorHeight = Math.max(1, Number(cursorHeight) || contentsView.editorLineHeight);
        const surfaceHeight = Math.max(safeCursorHeight, Number(contentsView.editorSurfaceHeight) || 0);
        return contentsView.editorDocumentStartY + Math.max(
                    safeCursorHeight / 2,
                    Math.round(surfaceHeight * 0.5));
    }
    function currentEditorCursorPosition() {
        return editorSelectionController.currentEditorCursorPosition();
    }
    function currentSelectedEditorText() {
        return editorSelectionController.currentSelectedEditorText();
    }
    function documentOccupiedBottomY() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        if (contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, contentsView.effectiveStructuredLogicalLineEntries().length);
            const lastLineNumber = structuredLineCount;
            return Math.max(
                        contentsView.editorLineHeight,
                        contentsView.lineDocumentY(lastLineNumber) + contentsView.lineVisualHeight(lastLineNumber, 1));
        }
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
        if (contentsView.structuredHostGeometryActive)
            return contentsView.lineDocumentY(contentsView.logicalLineNumberForOffset(safeOffset));
        if (!contentEditor.editorItem || contentEditor.editorItem.positionToRectangle === undefined) {
            const fallbackLineNumber = contentsView.logicalLineNumberForOffset(safeOffset);
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
    function incrementalLineGeometryAvailable() {
        return !contentsView.hasPendingNoteEntryGutterRefresh()
                && Array.isArray(contentsView.minimapLineGroups)
                && contentsView.minimapLineGroupsNoteId === contentsView.activeLineGeometryNoteId()
                && contentsView.minimapLineGroups.length === contentsView.logicalLineCount
                && contentsView.minimapLineGroups.length > 0;
    }
    function ensureLogicalLineDocumentYCache() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const lineCount = contentsView.logicalLineCount;
        if (contentsView.incrementalLineGeometryAvailable()) {
            contentsView.logicalLineDocumentYCacheRevision = refreshRevision;
            contentsView.logicalLineDocumentYCacheLineCount = lineCount;
            return;
        }
        if (contentsView.logicalLineDocumentYCacheRevision === refreshRevision && contentsView.logicalLineDocumentYCacheLineCount === lineCount && Array.isArray(contentsView.logicalLineDocumentYCache) && contentsView.logicalLineDocumentYCache.length === lineCount) {
            return;
        }
        const cachedYValues = [];
        let previousDocumentY = 0;
        for (let lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
            const startOffset = contentsView.logicalLineStartOffsetAt(lineIndex);
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
    function visualRowCountForLine(lineNumber) {
        if (contentsView.structuredHostGeometryActive) {
            const structuredEntry = contentsView.structuredLogicalLineEntryAt(lineNumber);
            if (structuredEntry && structuredEntry.rowCount !== undefined)
                return Math.max(1, Math.ceil(Number(structuredEntry.rowCount) || 1));
            return Math.max(
                        1,
                        Math.ceil(
                            contentsView.lineVisualHeight(lineNumber, 1)
                            / Math.max(1, contentsView.editorLineHeight)));
        }
        if (contentsView.incrementalLineGeometryAvailable()) {
            const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
            const lineGroup = contentsView.minimapLineGroups[safeLineNumber - 1];
            if (lineGroup && lineGroup.rowCount !== undefined)
                return Math.max(1, Math.ceil(Number(lineGroup.rowCount) || 1));
        }
        return Math.max(
                    1,
                    Math.ceil(
                        contentsView.lineVisualHeight(lineNumber, 1)
                        / Math.max(1, contentsView.editorLineHeight)));
    }
    function singleLineGutterHeight(lineNumber) {
        if (contentsView.structuredHostGeometryActive) {
            const structuredEntry = contentsView.structuredLogicalLineEntryAt(lineNumber);
            const explicitGutterHeight = Number(
                        structuredEntry && structuredEntry.gutterContentHeight !== undefined
                        ? structuredEntry.gutterContentHeight
                        : 0);
            if (isFinite(explicitGutterHeight) && explicitGutterHeight > 0)
                return Math.max(contentsView.editorLineHeight, explicitGutterHeight);
        }
        const visualRowCount = contentsView.visualRowCountForLine(lineNumber);
        const rowDrivenHeight = Math.max(
                    contentsView.editorLineHeight,
                    visualRowCount * contentsView.editorLineHeight);
        return Math.max(rowDrivenHeight, contentsView.lineVisualHeight(lineNumber, 1));
    }
    function ensureLogicalLineGutterDocumentYCache() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const lineCount = contentsView.logicalLineCount;
        if (contentsView.logicalLineGutterDocumentYCacheRevision === refreshRevision
                && contentsView.logicalLineGutterDocumentYCacheLineCount === lineCount
                && Array.isArray(contentsView.logicalLineGutterDocumentYCache)
                && contentsView.logicalLineGutterDocumentYCache.length === lineCount) {
            return;
        }
        const cachedYValues = [];
        let accumulatedWrapOffsetY = 0;
        for (let lineIndex = 0; lineIndex < lineCount; ++lineIndex) {
            const lineNumber = lineIndex + 1;
            const baseDocumentY = Math.max(0, Number(contentsView.lineDocumentY(lineNumber)) || 0);
            const baseLogicalY = lineIndex * contentsView.editorLineHeight;
            const actualWrapOffsetY = Math.max(0, baseDocumentY - baseLogicalY);
            const missingWrapOffsetY = Math.max(0, accumulatedWrapOffsetY - actualWrapOffsetY);
            const gutterDocumentY = Math.max(0, baseDocumentY + missingWrapOffsetY);
            const gutterHeight = contentsView.singleLineGutterHeight(lineNumber);
            cachedYValues.push(gutterDocumentY);
            accumulatedWrapOffsetY += Math.max(0, gutterHeight - contentsView.editorLineHeight);
        }
        contentsView.logicalLineGutterDocumentYCache = cachedYValues;
        contentsView.logicalLineGutterDocumentYCacheRevision = refreshRevision;
        contentsView.logicalLineGutterDocumentYCacheLineCount = lineCount;
    }
    function extractResourceDropUrls(drop) {
        return resourceImportController.extractResourceDropUrls(drop);
    }
    function firstVisibleLogicalLine() {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return 1;
        const contentY = Math.max(0, Number(flickable.contentY) || 0);
        const firstVisibleDocumentY = Math.max(0, contentY - contentsView.editorDocumentStartY);
        if (contentsView.structuredHostGeometryActive) {
            const lineEntries = contentsView.effectiveStructuredLogicalLineEntries();
            for (let lineIndex = 0; lineIndex < lineEntries.length; ++lineIndex) {
                const entry = lineEntries[lineIndex] && typeof lineEntries[lineIndex] === "object"
                        ? lineEntries[lineIndex]
                        : ({});
                const lineTop = Math.max(0, Number(entry.contentY) || 0);
                const lineBottom = lineTop + Math.max(1, Number(entry.contentHeight) || contentsView.editorLineHeight);
                if (lineBottom > firstVisibleDocumentY)
                    return lineIndex + 1;
            }
            return Math.max(1, lineEntries.length);
        }
        return Math.max(1, Math.min(contentsView.logicalLineCount, contentsView.logicalLineNumberForDocumentY(firstVisibleDocumentY)));
    }
    function shouldFlushBlurredEditorState(scheduledNoteId) {
        const normalizedScheduledNoteId = scheduledNoteId === undefined || scheduledNoteId === null
                ? ""
                : String(scheduledNoteId).trim();
        if (normalizedScheduledNoteId.length === 0 || !editorSession)
            return false;
        const currentBoundNoteId = editorSession.editorBoundNoteId !== undefined && editorSession.editorBoundNoteId !== null
                ? String(editorSession.editorBoundNoteId).trim()
                : "";
        const currentSelectedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null
                ? ""
                : String(contentsView.selectedNoteId).trim();
        if (currentBoundNoteId !== normalizedScheduledNoteId
                || currentSelectedNoteId !== normalizedScheduledNoteId) {
            return false;
        }
        const hasLocalEditorAuthority = editorSession.localEditorAuthority !== undefined
                && !!editorSession.localEditorAuthority;
        const hasPendingBodySave = editorSession.pendingBodySave !== undefined
                && !!editorSession.pendingBodySave;
        return hasLocalEditorAuthority || hasPendingBodySave;
    }
    function flushEditorStateAfterInputSettles(attempt, scheduledNoteId) {
        const retryCount = Math.max(0, Number(attempt) || 0);
        const normalizedScheduledNoteId = scheduledNoteId === undefined || scheduledNoteId === null
                ? ""
                : String(scheduledNoteId).trim();
        if (!contentsView.shouldFlushBlurredEditorState(normalizedScheduledNoteId))
            return;
        const activePreeditText = contentEditor && contentEditor.preeditText !== undefined ? String(contentEditor.preeditText === undefined || contentEditor.preeditText === null ? "" : contentEditor.preeditText) : "";
        const inputMethodBusy = !!(contentEditor && ((contentEditor.inputMethodComposing !== undefined && contentEditor.inputMethodComposing) || activePreeditText.length > 0));
        if (inputMethodBusy && retryCount < 6) {
            Qt.callLater(function () {
                contentsView.flushEditorStateAfterInputSettles(retryCount + 1, normalizedScheduledNoteId);
            });
            return;
        }
        const currentBoundNoteId = editorSession && editorSession.editorBoundNoteId !== undefined && editorSession.editorBoundNoteId !== null
                ? String(editorSession.editorBoundNoteId).trim()
                : "";
        const currentSelectedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null
                ? ""
                : String(contentsView.selectedNoteId).trim();
        if (normalizedScheduledNoteId.length > 0
                && (currentBoundNoteId !== normalizedScheduledNoteId
                    || currentSelectedNoteId !== normalizedScheduledNoteId)) {
            return;
        }
        editorTypingController.handleEditorTextEdited();
        editorSession.flushPendingEditorText();
    }
    function focusEditorForPendingNote() {
        const selectedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null ? "" : String(contentsView.selectedNoteId).trim();
        const pendingNoteId = selectionSyncCoordinator.takePendingEditorFocusNoteId(selectedNoteId);
        if (pendingNoteId.length === 0 || !contentsView.hasSelectedNote)
            return;

        Qt.callLater(function () {
            const activeNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null ? "" : String(contentsView.selectedNoteId).trim();
            if (activeNoteId !== pendingNoteId)
                return;

            const cursorPosition = Math.max(0, contentsView.logicalTextLength());
            contentEditor.forceActiveFocus();
            if (contentEditor.editorItem && contentEditor.editorItem.forceActiveFocus !== undefined)
                contentEditor.editorItem.forceActiveFocus();
            if (contentEditor.setCursorPositionPreservingInputMethod !== undefined)
                contentEditor.setCursorPositionPreservingInputMethod(cursorPosition);
            else if (contentEditor.cursorPosition !== undefined)
                contentEditor.cursorPosition = cursorPosition;
        });
    }
    function eventRequestsPasteShortcut(event) {
        if (!event)
            return false;
        if (event.matches !== undefined && event.matches(StandardKey.Paste))
            return true;

        const modifiers = event.modifiers;
        const metaPressed = !!(modifiers & Qt.MetaModifier);
        const controlPressed = !!(modifiers & Qt.ControlModifier);
        const altPressed = !!(modifiers & Qt.AltModifier);
        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        const normalizedText = event.text === undefined || event.text === null ? "" : String(event.text).toUpperCase();
        if (!altPressed
                && !shiftPressed
                && (metaPressed || controlPressed)
                && (event.key === Qt.Key_V || normalizedText === "V")) {
            return true;
        }
        if (!metaPressed
                && !controlPressed
                && !altPressed
                && shiftPressed
                && event.key === Qt.Key_Insert) {
            return true;
        }
        return false;
    }
    function clipboardImageAvailableForPaste() {
        if (!contentsView.resourcesImportViewModel)
            return false;
        if (contentsView.resourcesImportViewModel.refreshClipboardImageAvailabilitySnapshot !== undefined)
            return !!contentsView.resourcesImportViewModel.refreshClipboardImageAvailabilitySnapshot();
        if (contentsView.resourcesImportViewModel.clipboardImageAvailable === undefined)
            return false;
        return !!contentsView.resourcesImportViewModel.clipboardImageAvailable;
    }
    function handleClipboardImagePasteShortcut(event) {
        if (!contentsView.eventRequestsPasteShortcut(event))
            return false;
        if (!contentsView.resourcesImportViewModel
                || (contentsView.resourcesImportViewModel.busy !== undefined
                    && contentsView.resourcesImportViewModel.busy)
                || !contentsView.clipboardImageAvailableForPaste()) {
            return false;
        }
        const pasted = contentsView.pasteClipboardImageAsResource();
        if (pasted && event)
            event.accepted = true;
        return pasted;
    }
    function handleInlineFormatShortcutKeyPress(event) {
        if (contentsView.handleClipboardImagePasteShortcut(event))
            return true;
        if (editorTypingController && editorTypingController.handlePlainEnterKeyPress !== undefined) {
            const enterHandled = editorTypingController.handlePlainEnterKeyPress(event);
            if (enterHandled) {
                if (event)
                    event.accepted = true;
                return true;
            }
        }
        if (editorTypingController && editorTypingController.handleTagAwareDeleteKeyPress !== undefined) {
            const deleteHandled = editorTypingController.handleTagAwareDeleteKeyPress(event);
            if (deleteHandled) {
                if (event)
                    event.accepted = true;
                return true;
            }
        }
        return editorSelectionController.handleInlineFormatShortcutKeyPress(event);
    }
    function handleSelectionContextMenuEvent(eventName) {
        if (contentsView.handleStructuredSelectionContextMenuEvent(eventName))
            return;
        editorSelectionController.handleSelectionContextMenuEvent(eventName);
    }
    function commitDocumentPresentationRefresh() {
        EditorTrace.trace(
                    "displayView",
                    "commitDocumentPresentationRefresh",
                    "projectionEnabled=" + contentsView.documentPresentationProjectionEnabled
                    + " structured=" + contentsView.showStructuredDocumentFlow
                    + " " + EditorTrace.describeText(contentsView.documentPresentationSourceText),
                    contentsView)
        const needsHtmlProjection = contentsView.documentPresentationProjectionEnabled;
        if (!needsHtmlProjection) {
            if (contentsView.renderedEditorHtml !== "")
                contentsView.renderedEditorHtml = "";
            contentsView.scheduleMinimapSnapshotRefresh(false);
            if (minimapLayer && contentsView.minimapRefreshEnabled)
                minimapLayer.requestRepaint();
            editorTypingController.synchronizeLiveEditingStateFromPresentation();
            return;
        }
        const nextRenderedText = editorProjection && editorProjection.editorSurfaceHtml !== undefined && editorProjection.editorSurfaceHtml !== null
                ? String(editorProjection.editorSurfaceHtml)
                : "";
        const editorRenderedText = contentsView.resourceBlocksRenderedInlineByHtmlProjection
                ? contentsView.renderEditorSurfaceHtmlWithInlineResources(nextRenderedText)
                : nextRenderedText;
        if (contentsView.renderedEditorHtml !== editorRenderedText)
            contentsView.renderedEditorHtml = editorRenderedText;
        contentsView.scheduleMinimapSnapshotRefresh(false);
        if (minimapLayer && contentsView.minimapRefreshEnabled)
            minimapLayer.requestRepaint();
        editorTypingController.synchronizeLiveEditingStateFromPresentation();
    }
    function documentPresentationRenderDirty() {
        const needsHtmlProjection = contentsView.documentPresentationProjectionEnabled;
        if (!needsHtmlProjection)
            return contentsView.renderedEditorHtml !== "";
        const rendererRenderedText = editorProjection && editorProjection.editorSurfaceHtml !== undefined && editorProjection.editorSurfaceHtml !== null
                ? String(editorProjection.editorSurfaceHtml)
                : "";
        const expectedRenderedText = contentsView.resourceBlocksRenderedInlineByHtmlProjection
                ? contentsView.renderEditorSurfaceHtmlWithInlineResources(rendererRenderedText)
                : rendererRenderedText;
        return contentsView.renderedEditorHtml !== expectedRenderedText;
    }
    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition) {
        return editorSelectionController.inferSelectionRangeFromSelectedText(selectedText, cursorPosition);
    }
    function inlineStyleWrapTags(styleTag) {
        return editorSelectionController.inlineStyleWrapTags(styleTag);
    }
    function sourceContainsCanonicalResourceTag(sourceText) {
        return resourceImportController.sourceContainsCanonicalResourceTag(sourceText);
    }
    function canonicalResourceTagCount(sourceText) {
        return resourceImportController.canonicalResourceTagCount(sourceText);
    }
    function resourceTagLossDetected(previousSourceText, nextSourceText) {
        return resourceImportController.resourceTagLossDetected(previousSourceText, nextSourceText);
    }
    function renderEditorSurfaceHtmlWithInlineResources(editorHtml) {
        return resourceImportController.renderEditorSurfaceHtmlWithInlineResources(editorHtml);
    }
    function refreshInlineResourcePresentation() {
        contentsView.commitDocumentPresentationRefresh();
    }
    function restoreEditorSurfaceFromPresentation() {
        resourceImportController.restoreEditorSurfaceFromPresentation();
    }
    function releaseResourceDropEditorSurfaceGuard(restoreSurface) {
        resourceImportController.releaseResourceDropEditorSurfaceGuard(restoreSurface);
    }
    function insertImportedResourceTags(importedEntries) {
        return resourceImportController.insertImportedResourceTags(importedEntries);
    }
    function pasteClipboardImageAsResource() {
        return resourceImportController.pasteClipboardImageAsResource();
    }
    function isMinimapScrollable() {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return false;
        return contentsView.minimapContentHeight() > (Number(flickable.height) || 0);
    }
    function lineDocumentY(lineNumber) {
        if (contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, contentsView.effectiveStructuredLogicalLineEntries().length);
            const safeLineNumber = Math.max(1, Math.min(structuredLineCount, Number(lineNumber) || 1));
            const structuredEntry = contentsView.structuredLogicalLineEntryAt(safeLineNumber);
            if (structuredEntry && structuredEntry.contentY !== undefined)
                return Math.max(0, Number(structuredEntry.contentY) || 0);
            return Math.max(0, (safeLineNumber - 1) * contentsView.editorLineHeight);
        }
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        if (contentsView.incrementalLineGeometryAvailable()) {
            const lineGroup = contentsView.minimapLineGroups[safeLineNumber - 1];
            if (lineGroup && lineGroup.contentY !== undefined)
                return Math.max(0, (Number(lineGroup.contentY) || 0) - contentsView.editorDocumentStartY);
        }
        contentsView.ensureLogicalLineDocumentYCache();
        const cacheIndex = safeLineNumber - 1;
        if (Array.isArray(contentsView.logicalLineDocumentYCache) && cacheIndex >= 0 && cacheIndex < contentsView.logicalLineDocumentYCache.length) {
            return Number(contentsView.logicalLineDocumentYCache[cacheIndex]) || 0;
        }
        return Math.max(0, (safeLineNumber - 1) * contentsView.editorLineHeight);
    }
    function gutterLineDocumentY(lineNumber) {
        if (contentsView.structuredHostGeometryActive) {
            const structuredEntry = contentsView.structuredLogicalLineEntryAt(lineNumber);
            if (structuredEntry && structuredEntry.gutterContentY !== undefined)
                return Math.max(0, Number(structuredEntry.gutterContentY) || 0);
            if (structuredEntry && structuredEntry.contentY !== undefined)
                return Math.max(0, Number(structuredEntry.contentY) || 0);
        }
        contentsView.ensureLogicalLineGutterDocumentYCache();
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        const cacheIndex = safeLineNumber - 1;
        if (Array.isArray(contentsView.logicalLineGutterDocumentYCache)
                && cacheIndex >= 0
                && cacheIndex < contentsView.logicalLineGutterDocumentYCache.length) {
            return Math.max(0, Number(contentsView.logicalLineGutterDocumentYCache[cacheIndex]) || 0);
        }
        return contentsView.lineDocumentY(safeLineNumber);
    }
    function gutterDocumentOccupiedBottomY() {
        if (contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, contentsView.effectiveStructuredLogicalLineEntries().length);
            const lastLineNumber = structuredLineCount;
            return Math.max(
                        contentsView.editorLineHeight,
                        contentsView.gutterLineDocumentY(lastLineNumber)
                        + contentsView.singleLineGutterHeight(lastLineNumber));
        }
        if (contentsView.logicalLineCount <= 0)
            return contentsView.editorLineHeight;
        contentsView.ensureLogicalLineGutterDocumentYCache();
        const lastLineNumber = contentsView.logicalLineCount;
        return Math.max(
                    contentsView.editorLineHeight,
                    contentsView.gutterLineDocumentY(lastLineNumber)
                    + contentsView.singleLineGutterHeight(lastLineNumber));
    }
    function lineVisualHeight(startLine, lineSpan) {
        const safeLineSpan = Math.max(1, Number(lineSpan) || 1);
        if (contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, contentsView.effectiveStructuredLogicalLineEntries().length);
            const safeStartLine = Math.max(1, Math.min(structuredLineCount, Number(startLine) || 1));
            const startEntry = contentsView.structuredLogicalLineEntryAt(safeStartLine);
            if (safeLineSpan === 1) {
                return Math.max(
                            1,
                            Number(startEntry && startEntry.contentHeight !== undefined
                                   ? startEntry.contentHeight
                                   : 0) || contentsView.editorLineHeight);
            }
            const safeEndLine = Math.max(
                        safeStartLine,
                        Math.min(
                            structuredLineCount,
                            safeStartLine + safeLineSpan - 1));
            const endEntry = contentsView.structuredLogicalLineEntryAt(safeEndLine);
            const startDocumentY = Math.max(0, Number(startEntry && startEntry.contentY !== undefined ? startEntry.contentY : 0) || 0);
            const endDocumentY = Math.max(
                        startDocumentY + contentsView.editorLineHeight,
                        Math.max(0, Number(endEntry && endEntry.contentY !== undefined ? endEntry.contentY : startDocumentY) || startDocumentY)
                        + Math.max(
                            1,
                            Number(endEntry && endEntry.contentHeight !== undefined
                                   ? endEntry.contentHeight
                                   : 0) || contentsView.editorLineHeight));
            return Math.max(contentsView.editorLineHeight, endDocumentY - startDocumentY);
        }
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLine) || 1));
        if (safeLineSpan === 1 && contentsView.incrementalLineGeometryAvailable()) {
            const lineGroup = contentsView.minimapLineGroups[safeStartLine - 1];
            if (lineGroup && lineGroup.contentHeight !== undefined)
                return Math.max(1, Number(lineGroup.contentHeight) || contentsView.editorLineHeight);
        }
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
    function gutterLineVisualHeight(startLine, lineSpan) {
        const safeLineSpan = Math.max(1, Number(lineSpan) || 1);
        if (contentsView.structuredHostGeometryActive) {
            const structuredLineCount = Math.max(1, contentsView.effectiveStructuredLogicalLineEntries().length);
            const safeStartLine = Math.max(1, Math.min(structuredLineCount, Number(startLine) || 1));
            if (safeLineSpan === 1)
                return contentsView.singleLineGutterHeight(safeStartLine);
            const startDocumentY = contentsView.gutterLineDocumentY(safeStartLine);
            const nextLineNumber = safeStartLine + safeLineSpan;
            let endDocumentY = 0;
            if (nextLineNumber <= structuredLineCount)
                endDocumentY = contentsView.gutterLineDocumentY(nextLineNumber);
            else
                endDocumentY = contentsView.gutterDocumentOccupiedBottomY();
            return Math.max(contentsView.editorLineHeight, endDocumentY - startDocumentY);
        }
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLine) || 1));
        if (safeLineSpan === 1)
            return contentsView.singleLineGutterHeight(safeStartLine);
        const startDocumentY = contentsView.gutterLineDocumentY(safeStartLine);
        const nextLineNumber = safeStartLine + safeLineSpan;
        let endDocumentY = 0;
        if (nextLineNumber <= contentsView.logicalLineCount)
            endDocumentY = contentsView.gutterLineDocumentY(nextLineNumber);
        else
            endDocumentY = contentsView.gutterDocumentOccupiedBottomY();
        return Math.max(contentsView.editorLineHeight, endDocumentY - startDocumentY);
    }
    function lineY(lineNumber) {
        return contentsView.editorViewportYForDocumentY(contentsView.lineDocumentY(lineNumber));
    }
    function gutterLineY(lineNumber) {
        return contentsView.editorViewportYForDocumentY(contentsView.gutterLineDocumentY(lineNumber));
    }
    function logicalLineNumberForDocumentY(documentY) {
        if (contentsView.structuredHostGeometryActive) {
            const lineEntries = contentsView.effectiveStructuredLogicalLineEntries();
            if (lineEntries.length === 0)
                return 1;
            const safeDocumentY = Math.max(0, Number(documentY) || 0);
            let bestLineNumber = 1;
            for (let lineIndex = 0; lineIndex < lineEntries.length; ++lineIndex) {
                const entry = lineEntries[lineIndex] && typeof lineEntries[lineIndex] === "object"
                        ? lineEntries[lineIndex]
                        : ({});
                const lineTop = Math.max(0, Number(entry.contentY) || 0);
                const lineBottom = lineTop + Math.max(1, Number(entry.contentHeight) || contentsView.editorLineHeight);
                if (safeDocumentY < lineTop)
                    break;
                bestLineNumber = lineIndex + 1;
                if (safeDocumentY < lineBottom)
                    break;
            }
            return bestLineNumber;
        }
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
    function logicalLineCharacterCountAt(lineIndex) {
        const safeIndex = Math.max(0, Math.min(contentsView.logicalLineCount - 1, Number(lineIndex) || 0));
        const startOffset = contentsView.logicalLineStartOffsetAt(safeIndex);
        const hasNextLine = safeIndex + 1 < contentsView.logicalLineCount;
        const nextOffset = hasNextLine ? contentsView.logicalLineStartOffsetAt(safeIndex + 1) : contentsView.logicalTextLength();
        return Math.max(0, nextOffset - startOffset - (hasNextLine ? 1 : 0));
    }
    function logicalLineNumberForOffset(offset) {
        if (contentsView.logicalLineCount <= 0)
            return 1;
        const safeOffset = Math.max(0, Math.min(contentsView.logicalTextLength(), Number(offset) || 0));
        let low = 0;
        let high = contentsView.logicalLineCount - 1;
        let best = 0;
        while (low <= high) {
            const middle = Math.floor((low + high) / 2);
            const middleOffset = contentsView.logicalLineStartOffsetAt(middle);
            if (middleOffset <= safeOffset) {
                best = middle;
                low = middle + 1;
            } else {
                high = middle - 1;
            }
        }
        return best + 1;
    }
    function logicalLineStartOffsetAt(lineIndex) {
        const safeIndex = Math.max(0, Math.min(contentsView.logicalLineCount - 1, Number(lineIndex) || 0));
        if (Array.isArray(contentsView.logicalLineStartOffsets) && safeIndex < contentsView.logicalLineStartOffsets.length)
            return Math.max(0, Number(contentsView.logicalLineStartOffsets[safeIndex]) || 0);
        return 0;
    }
    function logicalTextLength() {
        return Math.max(0, Number(contentsView.liveLogicalTextLength) || 0);
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
            return contentsView.currentCursorGutterLineHeight();
        if (!markerSpec)
            return contentsView.editorLineHeight;
        return Math.max(1, contentsView.gutterLineVisualHeight(markerSpec.startLine, markerSpec.lineSpan));
    }
    function markerY(markerSpec) {
        const markerType = markerSpec && markerSpec.type !== undefined ? String(markerSpec.type).toLowerCase() : "";
        if (markerType === "current")
            return contentsView.currentCursorGutterLineY();
        if (!markerSpec)
            return contentsView.editorDocumentStartY;
        const startLine = Math.max(1, Number(markerSpec.startLine) || 1);
        return contentsView.gutterLineY(startLine);
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
    function normalizeInlineStyleTag(tagName) {
        return editorSelectionController.normalizeInlineStyleTag(tagName);
    }
    function encodeXmlAttributeValue(value) {
        let encodedValue = value === undefined || value === null ? "" : String(value);
        encodedValue = encodedValue.replace(/&/g, "&amp;");
        encodedValue = encodedValue.replace(/"/g, "&quot;");
        encodedValue = encodedValue.replace(/'/g, "&apos;");
        encodedValue = encodedValue.replace(/</g, "&lt;");
        encodedValue = encodedValue.replace(/>/g, "&gt;");
        return encodedValue;
    }
    function resetStructuredSelectionContextMenuSnapshot() {
        contentsView.structuredContextMenuBlockIndex = -1;
        contentsView.structuredContextMenuSelectionSnapshot = ({});
    }
    function normalizedStructuredSelectionContextMenuSnapshot(snapshot) {
        const safeSnapshot = snapshot && typeof snapshot === "object" ? snapshot : ({});
        return {
            "cursorPosition": Number(safeSnapshot.cursorPosition),
            "selectedText": safeSnapshot.selectedText === undefined || safeSnapshot.selectedText === null
                            ? ""
                            : String(safeSnapshot.selectedText),
            "selectionEnd": Number(safeSnapshot.selectionEnd),
            "selectionStart": Number(safeSnapshot.selectionStart)
        };
    }
    function structuredContextMenuSelectionValid() {
        const snapshot = contentsView.structuredContextMenuSelectionSnapshot;
        const selectionStart = Number(snapshot && snapshot.selectionStart !== undefined ? snapshot.selectionStart : NaN);
        const selectionEnd = Number(snapshot && snapshot.selectionEnd !== undefined ? snapshot.selectionEnd : NaN);
        return contentsView.structuredContextMenuBlockIndex >= 0
                && isFinite(selectionStart)
                && isFinite(selectionEnd)
                && selectionEnd > selectionStart;
    }
    function primeStructuredSelectionContextMenuSnapshot() {
        if (!contentsView.showStructuredDocumentFlow
                || !structuredDocumentFlow
                || structuredDocumentFlow.inlineFormatTargetState === undefined) {
            contentsView.resetStructuredSelectionContextMenuSnapshot();
            return false;
        }
        const targetState = structuredDocumentFlow.inlineFormatTargetState();
        if (!targetState || !targetState.valid) {
            contentsView.resetStructuredSelectionContextMenuSnapshot();
            return false;
        }
        const blockIndex = Number(targetState.blockIndex);
        if (!isFinite(blockIndex) || blockIndex < 0) {
            contentsView.resetStructuredSelectionContextMenuSnapshot();
            return false;
        }
        const selectionSnapshot = contentsView.normalizedStructuredSelectionContextMenuSnapshot(targetState.selectionSnapshot);
        const selectionStart = Number(selectionSnapshot.selectionStart);
        const selectionEnd = Number(selectionSnapshot.selectionEnd);
        if (!isFinite(selectionStart) || !isFinite(selectionEnd) || selectionEnd <= selectionStart) {
            contentsView.resetStructuredSelectionContextMenuSnapshot();
            return false;
        }
        contentsView.structuredContextMenuBlockIndex = Math.floor(blockIndex);
        contentsView.structuredContextMenuSelectionSnapshot = selectionSnapshot;
        return true;
    }
    function structuredContextMenuInlineStyleTag(eventName) {
        const normalizedEventName = eventName === undefined || eventName === null ? "" : String(eventName).trim();
        if (normalizedEventName === "editor.format.plain")
            return "plain";
        if (normalizedEventName === "editor.format.bold")
            return "bold";
        if (normalizedEventName === "editor.format.italic")
            return "italic";
        if (normalizedEventName === "editor.format.underline")
            return "underline";
        if (normalizedEventName === "editor.format.strikethrough")
            return "strikethrough";
        if (normalizedEventName === "editor.format.highlight")
            return "highlight";
        return "";
    }
    function handleStructuredSelectionContextMenuEvent(eventName) {
        const inlineStyleTag = contentsView.structuredContextMenuInlineStyleTag(eventName);
        const plan = contextMenuCoordinator.handleStructuredSelectionEventPlan(
                    inlineStyleTag,
                    contentsView.structuredContextMenuSelectionValid(),
                    !!(structuredDocumentFlow && structuredDocumentFlow.applyInlineFormatToBlockSelection !== undefined));
        if (!plan.applyStructuredInlineFormat) {
            if (plan.requireStructuredSelectionPrime
                    && contentsView.primeStructuredSelectionContextMenuSnapshot()) {
                return contentsView.handleStructuredSelectionContextMenuEvent(eventName);
            }
            return false;
        }
        const handled = !!structuredDocumentFlow.applyInlineFormatToBlockSelection(
                    Number(plan.blockIndex) || 0,
                    String(plan.inlineStyleTag || ""),
                    plan.selectionSnapshot && typeof plan.selectionSnapshot === "object"
                    ? plan.selectionSnapshot
                    : ({}));
        contentsView.resetStructuredSelectionContextMenuSnapshot();
        return handled;
    }
    function openEditorSelectionContextMenu(localX, localY) {
        const plan = contextMenuCoordinator.openSelectionContextMenuPlan(
                    contentsView.structuredContextMenuSelectionValid(),
                    !!editorSelectionContextMenu,
                    Number(localX) || 0,
                    Number(localY) || 0);
        if (plan.delegateToEditorSelectionController)
            return editorSelectionController.openEditorSelectionContextMenu(localX, localY);
        if (plan.requireStructuredSelectionPrime
                && !contentsView.primeStructuredSelectionContextMenuSnapshot())
            return false;
        if (!editorSelectionContextMenu)
            return false;
        if (plan.closeBeforeOpen && editorSelectionContextMenu.opened)
            editorSelectionContextMenu.close();
        editorSelectionContextMenu.openFor(
                    editorViewport,
                    Number(plan.openX) || 0,
                    Number(plan.openY) || 0);
        return true;
    }
    function primeEditorSelectionContextMenuSnapshot() {
        if (contentsView.showStructuredDocumentFlow)
            return contentsView.primeStructuredSelectionContextMenuSnapshot();
        return editorSelectionController.primeContextMenuSelectionSnapshot();
    }
    function persistEditorTextImmediately(nextText) {
        return editorSelectionController.persistEditorTextImmediately(nextText);
    }
    function scheduleEditorEntrySnapshotReconcile() {
        selectionSyncCoordinator.scheduleSnapshotReconcile();
    }
    function pollSelectedNoteSnapshot() {
        const pollPlan = selectionSyncCoordinator.snapshotPollPlan();
        const normalizedNoteId = pollPlan.noteId === undefined || pollPlan.noteId === null
                ? ""
                : String(pollPlan.noteId).trim();
        if (pollPlan.attemptReconcile
                && selectionBridge
                && selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote !== undefined
                && normalizedNoteId.length > 0) {
            const sessionText = editorSession.editorText === undefined || editorSession.editorText === null
                    ? ""
                    : String(editorSession.editorText);
            const preferViewSessionOnMismatch = editorSession
                    && editorSession.localEditorAuthority !== undefined
                    && !!editorSession.localEditorAuthority;
            if (selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote(
                        normalizedNoteId,
                        sessionText,
                        preferViewSessionOnMismatch)) {
                selectionSyncCoordinator.markSnapshotReconcileStarted(normalizedNoteId);
                return;
            }
        }
        if (!pollPlan.allowSnapshotRefresh
                || !selectionBridge
                || selectionBridge.refreshSelectedNoteSnapshot === undefined)
            return;
        selectionBridge.refreshSelectedNoteSnapshot();
        contentsView.scheduleGutterRefresh(2);
    }
    function reconcileEditorEntrySnapshotOnce() {
        const reconcilePlan = selectionSyncCoordinator.snapshotReconcilePlan();
        const normalizedNoteId = reconcilePlan.noteId === undefined || reconcilePlan.noteId === null
                ? ""
                : String(reconcilePlan.noteId).trim();
        if (!reconcilePlan.attemptReconcile)
            return false;
        if (!selectionBridge
                || selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote === undefined) {
            return false;
        }
        const sessionText = editorSession.editorText === undefined || editorSession.editorText === null
                ? ""
                : String(editorSession.editorText);
        const preferViewSessionOnMismatch = editorSession
                && editorSession.localEditorAuthority !== undefined
                && !!editorSession.localEditorAuthority;
        if (!selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote(
                    normalizedNoteId,
                    sessionText,
                    preferViewSessionOnMismatch))
            return false;
        selectionSyncCoordinator.markSnapshotReconcileStarted(normalizedNoteId);
        return true;
    }
    function queueStructuredInlineFormatWrap(tagName) {
        if (!contentsView.showStructuredDocumentFlow
                || !structuredDocumentFlow)
            return false;
        if (structuredDocumentFlow.inlineFormatTargetState === undefined
                || structuredDocumentFlow.applyInlineFormatToBlockSelection === undefined) {
            if (structuredDocumentFlow.applyInlineFormatToActiveSelection !== undefined)
                return structuredDocumentFlow.applyInlineFormatToActiveSelection(tagName);
            return false;
        }
        const targetState = structuredDocumentFlow.inlineFormatTargetState();
        if (!targetState || !targetState.valid)
            return false;
        const normalizedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null
                ? ""
                : String(contentsView.selectedNoteId).trim();
        if (normalizedNoteId.length === 0)
            return false;
        const blockIndex = Math.max(0, Math.floor(Number(targetState.blockIndex) || 0));
        const selectionSnapshot = targetState.selectionSnapshot && typeof targetState.selectionSnapshot === "object"
                ? ({
                       "cursorPosition": Number(targetState.selectionSnapshot.cursorPosition),
                       "selectedText": targetState.selectionSnapshot.selectedText === undefined
                                       || targetState.selectionSnapshot.selectedText === null
                                       ? ""
                                       : String(targetState.selectionSnapshot.selectedText),
                       "selectionEnd": Number(targetState.selectionSnapshot.selectionEnd),
                       "selectionStart": Number(targetState.selectionSnapshot.selectionStart)
                   })
                : ({});
        if (normalizedNoteId !== contentsView.selectedNoteId)
            return false;
        return structuredDocumentFlow.applyInlineFormatToBlockSelection(
                    blockIndex,
                    tagName,
                    selectionSnapshot);
    }
    function queueInlineFormatWrap(tagName) {
        if (contentsView.queueStructuredInlineFormatWrap(tagName))
            return true;
        return editorSelectionController.queueInlineFormatWrap(tagName);
    }
    function queueMarkdownListMutation(listKind) {
        return editorSelectionController.queueMarkdownListMutation(listKind);
    }
    function queueAgendaShortcutInsertion() {
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            if (structuredDocumentFlow.insertStructuredShortcutAtActivePosition("agenda"))
                return true;
        }
        return editorTypingController.queueAgendaShortcutInsertion();
    }
    function queueCalloutShortcutInsertion() {
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            if (structuredDocumentFlow.insertStructuredShortcutAtActivePosition("callout"))
                return true;
        }
        return editorTypingController.queueCalloutShortcutInsertion();
    }
    function queueBreakShortcutInsertion() {
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            if (structuredDocumentFlow.insertStructuredShortcutAtActivePosition("break"))
                return true;
        }
        return editorTypingController.queueBreakShortcutInsertion();
    }
    function requestStructuredDocumentEndEdit() {
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.requestDocumentEndEdit !== undefined) {
            return structuredDocumentFlow.requestDocumentEndEdit();
        }
        const logicalOffset = Math.max(0, contentsView.logicalTextLength());
        contentEditor.forceActiveFocus();
        if (contentEditor.setCursorPositionPreservingInputMethod !== undefined) {
            contentEditor.setCursorPositionPreservingInputMethod(logicalOffset);
            return true;
        }
        if (contentEditor.cursorPosition !== undefined) {
            contentEditor.cursorPosition = logicalOffset;
            return true;
        }
        return false;
    }
    function focusStructuredBlockSourceOffset(sourceOffset) {
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.requestFocus !== undefined) {
            structuredDocumentFlow.requestFocus({
                                                   "sourceOffset": Math.max(0, Math.floor(Number(sourceOffset) || 0))
                                               });
            return;
        }
        const logicalOffset = editorTypingController.logicalOffsetForSourceOffset(
                    Math.max(0, Math.floor(Number(sourceOffset) || 0)));
        contentEditor.forceActiveFocus();
        if (contentEditor.setCursorPositionPreservingInputMethod !== undefined) {
            contentEditor.setCursorPositionPreservingInputMethod(logicalOffset);
            return;
        }
        if (contentEditor.cursorPosition !== undefined)
            contentEditor.cursorPosition = logicalOffset;
    }
    function applyDocumentSourceMutation(nextSourceText, focusRequest) {
        const mutation = sessionCoordinator.normalizedDocumentSourceMutation(nextSourceText);
        const normalizedNextSourceText = String(mutation.nextSourceText || "");
        const currentSourceText = String(mutation.currentSourceText || "");
        EditorTrace.trace(
                    "displayView",
                    "applyDocumentSourceMutation",
                    "selectedNoteId=" + contentsView.selectedNoteId
                    + " focusRequest={" + EditorTrace.describeFocusRequest(focusRequest) + "} "
                    + EditorTrace.describeText(normalizedNextSourceText),
                    contentsView)
        if (!mutation.changed)
            return false;
        if (contentsView.resourceTagLossDetected(currentSourceText, normalizedNextSourceText)) {
            contentsView.restoreEditorSurfaceFromPresentation();
            return false;
        }
        if (contentsView.editorText !== normalizedNextSourceText)
            contentsView.editorText = normalizedNextSourceText;
        presentationRefreshController.clearPendingWhileFocused();
        if (!contentsView.showStructuredDocumentFlow
                && contentsView.commitDocumentPresentationRefresh !== undefined) {
            contentsView.commitDocumentPresentationRefresh();
        } else {
            if (documentPresentationRefreshTimer.running)
                documentPresentationRefreshTimer.stop();
        }
        if (editorSession && editorSession.markLocalEditorAuthority !== undefined)
            editorSession.markLocalEditorAuthority();
        if (focusRequest && structuredDocumentFlow && structuredDocumentFlow.requestFocus !== undefined) {
            const requestedFocus = focusRequest && typeof focusRequest === "object" ? focusRequest : ({});
            Qt.callLater(function () {
                structuredDocumentFlow.requestFocus(requestedFocus);
            });
        }
        if (editorSession && editorSession.scheduleEditorPersistence !== undefined)
            editorSession.scheduleEditorPersistence();
        contentsView.editorTextEdited(normalizedNextSourceText);
        return true;
    }
    function setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked) {
        const currentSourceText = contentsView.editorText === undefined || contentsView.editorText === null
                ? ""
                : String(contentsView.editorText);
        if (!contentsAgendaBackend
                || contentsAgendaBackend.rewriteTaskDoneAttribute === undefined) {
            return false;
        }
        const nextSourceText = String(contentsAgendaBackend.rewriteTaskDoneAttribute(
                                          currentSourceText,
                                          Math.floor(Number(taskOpenTagStart) || 0),
                                          Math.floor(Number(taskOpenTagEnd) || 0),
                                          !!checked));
        return contentsView.applyDocumentSourceMutation(
                    nextSourceText,
                    {
                        "taskOpenTagStart": Math.floor(Number(taskOpenTagStart) || -1)
                    });
    }
    function refreshMinimapSnapshot() {
        if (!contentsView.lineGeometryRefreshEnabled)
            return;
        const currentSourceText = contentsView.currentMinimapSourceText();
        const currentNoteId = contentsView.activeLineGeometryNoteId();
        const currentLineCount = contentsView.structuredHostGeometryActive
                ? Math.max(1, contentsView.effectiveStructuredLogicalLineEntries().length)
                : contentsView.logicalLineCount;
        if (!contentsView.minimapSnapshotForceFullRefresh
                && !contentsView.hasPendingNoteEntryGutterRefresh(currentNoteId)
                && currentNoteId === contentsView.minimapLineGroupsNoteId
                && currentSourceText === contentsView.minimapSnapshotSourceText
                && Array.isArray(contentsView.minimapLineGroups)
                && contentsView.minimapLineGroups.length === currentLineCount) {
            contentsView.refreshMinimapViewportTracking();
            contentsView.refreshMinimapCursorTracking();
            return;
        }

        const nextLineGroups = contentsView.nextMinimapLineGroupsForCurrentState(currentSourceText);
        const nextRows = MinimapSnapshotSupport.flattenLineGroups(nextLineGroups, contentsView.editorLineHeight);
        const nextSilhouetteHeight = contentsView.minimapSilhouetteHeight(nextRows);
        const nextTrackHeight = Math.min(contentsView.minimapAvailableTrackHeight, nextSilhouetteHeight);

        contentsView.minimapLineGroups = nextLineGroups;
        contentsView.minimapLineGroupsNoteId = currentNoteId;
        contentsView.minimapSnapshotForceFullRefresh = false;
        contentsView.minimapSnapshotSourceText = currentSourceText;
        contentsView.minimapVisualRows = nextRows;
        contentsView.minimapResolvedSilhouetteHeight = nextSilhouetteHeight;
        contentsView.minimapResolvedTrackHeight = nextTrackHeight;
        contentsView.refreshMinimapViewportTracking(nextTrackHeight);
        contentsView.refreshMinimapCursorTracking(nextRows);
    }
    function refreshMinimapCursorTracking(rowsOverride) {
        if (!contentsView.minimapRefreshEnabled)
            return;
        const nextCurrentVisualRow = contentsView.minimapCurrentVisualRow(rowsOverride);
        contentsView.minimapResolvedCurrentLineHeight = contentsView.minimapVisualRowPaintHeight(nextCurrentVisualRow);
        contentsView.minimapResolvedCurrentLineWidth = contentsView.minimapBarWidth(nextCurrentVisualRow.charCount);
        contentsView.minimapResolvedCurrentLineY = contentsView.minimapVisualRowPaintY(nextCurrentVisualRow);
    }
    function refreshMinimapViewportTracking(trackHeightOverride) {
        if (!contentsView.minimapRefreshEnabled)
            return;
        const resolvedTrackHeight = Math.max(1, Number(trackHeightOverride) || contentsView.minimapResolvedTrackHeight);
        const nextScrollable = contentsView.isMinimapScrollable();
        const nextViewportHeight = contentsView.minimapViewportHeight(resolvedTrackHeight);
        const nextViewportY = contentsView.minimapViewportY(resolvedTrackHeight, nextViewportHeight);
        contentsView.minimapScrollable = nextScrollable;
        contentsView.minimapResolvedViewportHeight = nextViewportHeight;
        contentsView.minimapResolvedViewportY = nextViewportY;
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        EditorTrace.trace("displayView", "requestViewHook", "reason=" + hookReason, contentsView)
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function resetNoteEntryLineGeometryState() {
        contentsView.resetGutterRefreshState();
        contentsView.minimapLineGroups = [];
        contentsView.minimapLineGroupsNoteId = "";
        contentsView.minimapVisualRows = [];
        contentsView.minimapScrollable = false;
        contentsView.minimapResolvedCurrentLineHeight = 1;
        contentsView.minimapResolvedCurrentLineWidth = 0;
        contentsView.minimapResolvedCurrentLineY = 0;
        contentsView.minimapResolvedSilhouetteHeight = 1;
        contentsView.minimapResolvedTrackHeight = 1;
        contentsView.minimapResolvedViewportHeight = 0;
        contentsView.minimapResolvedViewportY = 0;
        contentsView.minimapSnapshotSourceText = "";
        contentsView.minimapSnapshotForceFullRefresh = true;
    }
    function resetGutterRefreshState() {
        contentsView.visibleGutterLineEntries = [
                    {
                        "lineNumber": 1,
                        "y": 0
                    }
                ];
        contentsView.logicalLineDocumentYCache = [];
        contentsView.logicalLineDocumentYCacheRevision = -1;
        contentsView.logicalLineDocumentYCacheLineCount = 0;
        contentsView.logicalLineGutterDocumentYCache = [];
        contentsView.logicalLineGutterDocumentYCacheRevision = -1;
        contentsView.logicalLineGutterDocumentYCacheLineCount = 0;
        contentsView.structuredGutterGeometrySignature = "";
    }
    function resetEditorSelectionCache() {
        editorSelectionController.resetEditorSelectionCache();
    }
    function resolveEditorFlickable() {
        if (contentsView.showStructuredDocumentFlow) {
            if (contentsView.showPrintEditorLayout)
                return printDocumentViewport;
            return structuredDocumentViewport;
        }
        if (contentEditor && contentEditor.resolvedFlickable !== undefined && contentEditor.resolvedFlickable)
            return contentEditor.resolvedFlickable;
        if (!contentEditor.editorItem || !contentEditor.editorItem.parent)
            return null;
        const candidate = contentEditor.editorItem.parent.parent;
        if (!candidate || candidate.contentY === undefined || candidate.contentHeight === undefined || candidate.height === undefined)
            return null;
        return candidate;
    }
    function scheduleEditorFocusForNote(noteId) {
        selectionSyncCoordinator.scheduleEditorFocusForNote(noteId);
    }
    function applyPresentationRefreshPlan(plan) {
        const refreshPlan = plan && typeof plan === "object" ? plan : ({});
        EditorTrace.trace(
                    "displayView",
                    "applyPresentationRefreshPlan",
                    "reason=" + String(refreshPlan.reason || "")
                    + " clear=" + !!refreshPlan.clearPresentation
                    + " commit=" + !!refreshPlan.commitRefresh
                    + " startTimer=" + !!refreshPlan.startTimer
                    + " stopTimer=" + !!refreshPlan.stopTimer,
                    contentsView)
        if (refreshPlan.stopTimer && documentPresentationRefreshTimer.running)
            documentPresentationRefreshTimer.stop();
        if (refreshPlan.startTimer) {
            if (documentPresentationRefreshTimer.running)
                documentPresentationRefreshTimer.stop();
            documentPresentationRefreshTimer.start();
        }
        if (refreshPlan.clearPresentation && contentsView.renderedEditorHtml !== "")
            contentsView.renderedEditorHtml = "";
        if (refreshPlan.commitRefresh)
            contentsView.commitDocumentPresentationRefresh();
        if (refreshPlan.requestMinimapRefresh)
            contentsView.scheduleMinimapSnapshotRefresh(false);
        if (refreshPlan.requestMinimapRepaint && minimapLayer && contentsView.minimapRefreshEnabled)
            minimapLayer.requestRepaint();
    }
    function executeRefreshPlan(plan) {
        const refreshPlan = plan && typeof plan === "object" ? plan : ({});
        if (refreshPlan.resetNoteEntryLineGeometry)
            contentsView.resetNoteEntryLineGeometryState();
        if (refreshPlan.requestStructuredLayoutRefresh
                && structuredDocumentFlow
                && structuredDocumentFlow.scheduleLayoutCacheRefresh !== undefined)
            structuredDocumentFlow.scheduleLayoutCacheRefresh();
        if (refreshPlan.scheduleViewportGutterRefresh)
            contentsView.scheduleViewportGutterRefresh();
        if (refreshPlan.gutterPassCount !== undefined)
            contentsView.scheduleGutterRefresh(
                        Number(refreshPlan.gutterPassCount) || 0,
                        String(refreshPlan.gutterReason || ""));
    }
    function scheduleDeferredDocumentPresentationRefresh() {
        contentsView.applyPresentationRefreshPlan(presentationRefreshController.planDeferredRequest());
    }
    function scheduleDocumentPresentationRefresh(forceImmediate) {
        const immediate = !!forceImmediate;
        EditorTrace.trace(
                    "displayView",
                    "scheduleDocumentPresentationRefresh",
                    "immediate=" + immediate
                    + " focused=" + contentsView.editorInputFocused
                    + " typingProtected=" + contentsView.typingSessionSyncProtected
                    + " structured=" + contentsView.showStructuredDocumentFlow,
                    contentsView)
        contentsView.applyPresentationRefreshPlan(
                    presentationRefreshController.planRefreshRequest(immediate));
    }
    function refreshLiveLogicalLineMetrics() {
        const metrics = contentsView.structuredHostGeometryActive
                ? viewportCoordinator.buildLogicalLineMetricsFromStructuredEntries(
                      contentsView.effectiveStructuredLogicalLineEntries())
                : viewportCoordinator.buildLogicalLineMetricsFromText(
                      contentsView.activeLogicalTextSnapshot());
        return contentsView.applyLiveLogicalLineMetrics(
                    Number(metrics.logicalTextLength) || 0,
                    metrics.lineStartOffsets || [0],
                    Number(metrics.lineCount) || 1);
    }
    function activeLogicalLineCountSnapshot() {
        const metrics = contentsView.structuredHostGeometryActive
                ? viewportCoordinator.buildLogicalLineMetricsFromStructuredEntries(
                      contentsView.effectiveStructuredLogicalLineEntries())
                : viewportCoordinator.buildLogicalLineMetricsFromText(
                      contentsView.activeLogicalTextSnapshot());
        return Math.max(1, Number(metrics.lineCount) || 1);
    }
    function shouldScheduleGutterRefreshForReason(reason) {
        return refreshCoordinator.shouldScheduleGutterRefreshForReason(
                    reason === undefined || reason === null ? "" : String(reason),
                    contentsView.activeLogicalLineCountSnapshot());
    }
    function scheduleGutterRefresh(passCount, reason) {
        const plan = refreshCoordinator.scheduleGutterRefresh(
                    Number(passCount) || 0,
                    reason === undefined || reason === null ? "" : String(reason),
                    contentsView.activeLogicalLineCountSnapshot());
        if (plan.startTimer && !gutterRefreshTimer.running)
            gutterRefreshTimer.start();
    }
    function scheduleNoteEntryGutterRefresh(noteId) {
        contentsView.executeRefreshPlan(refreshCoordinator.scheduleNoteEntryGutterRefresh(
                                            noteId === undefined || noteId === null ? "" : String(noteId)));
    }
    function scheduleCursorDrivenUiRefresh() {
        const plan = refreshCoordinator.scheduleCursorDrivenUiRefresh();
        if (!plan.queueCallLater)
            return;
        Qt.callLater(function () {
            refreshCoordinator.clearCursorDrivenUiRefreshQueued();
            contentsView.refreshMinimapCursorTracking();
            if (minimapLayer && contentsView.minimapRefreshEnabled)
                minimapLayer.requestRepaint();
        });
    }
    function scheduleViewportGutterRefresh() {
        const plan = refreshCoordinator.scheduleViewportGutterRefresh();
        if (!plan.queueCallLater)
            return;
        Qt.callLater(function () {
            refreshCoordinator.clearViewportGutterRefreshQueued();
            contentsView.refreshVisibleGutterEntries();
        });
    }
    function scheduleMinimapSnapshotRefresh(forceFull) {
        const plan = refreshCoordinator.scheduleMinimapSnapshotRefresh(!!forceFull);
        if (!plan.queueCallLater)
            return;
        Qt.callLater(function () {
            refreshCoordinator.clearMinimapSnapshotRefreshQueued();
            contentsView.refreshMinimapSnapshot();
        });
    }
    function scheduleSelectionModelSync(options) {
        selectionSyncCoordinator.scheduleSelectionSync(options && typeof options === "object" ? options : ({}));
    }
    function scrollEditorViewportToMinimapPosition(localY) {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return;
        const plan = viewportCoordinator.minimapScrollPlan(
                    Number(localY) || 0,
                    Math.max(1, contentsView.editorOccupiedContentHeight()));
        if (plan.apply)
            flickable.contentY = Number(plan.contentY) || 0;
    }
    function correctTypingViewport(forceAnchor) {
        const flickable = contentsView.editorFlickable;
        if (!flickable || flickable.contentY === undefined)
            return;
        const plan = viewportCoordinator.typingViewportCorrectionPlan(
                    !!forceAnchor,
                    Number(flickable.height) || contentsView.editorViewportHeight,
                    Math.max(Number(flickable.contentHeight) || 0,
                             contentsView.editorOccupiedContentHeight()),
                    Number(flickable.contentY) || 0,
                    contentsView.currentCursorVisualRowRect());
        if (plan.apply)
            flickable.contentY = Number(plan.contentY) || 0;
    }
    function scheduleTypingViewportCorrection(forceAnchor) {
        const plan = refreshCoordinator.scheduleTypingViewportCorrection(!!forceAnchor);
        if (!plan.queueCallLater)
            return;
        Qt.callLater(function () {
            const forceCorrection = refreshCoordinator.takeTypingViewportForceCorrectionRequested();
            refreshCoordinator.clearTypingViewportCorrectionQueued();
            contentsView.correctTypingViewport(forceCorrection);
        });
    }
    function selectedEditorRange() {
        return editorSelectionController.selectedEditorRange();
    }
    function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {
        return editorSelectionController.wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange);
    }

    clip: true

    Component.onCompleted: {
        EditorTrace.trace("displayView", "mount", "visible=" + contentsView.visible, contentsView)
        contentsView.scheduleSelectionModelSync({
                                                   "resetSnapshot": true,
                                                   "scheduleReconcile": true,
                                                   "fallbackRefresh": true
                                               });
    }
    Component.onDestruction: {
        EditorTrace.trace(
                    "displayView",
                    "unmount",
                    "selectedNoteId=" + contentsView.selectedNoteId
                    + " pendingBodySave=" + contentsView.pendingBodySave,
                    contentsView)
        editorTypingController.handleEditorTextEdited();
        editorSession.flushPendingEditorText();
    }
    onEditorFlickableChanged: {
        contentsView.scheduleMinimapSnapshotRefresh(true);
        contentsView.scheduleGutterRefresh(2);
    }
    onEditorTextChanged: {
        EditorTrace.trace(
                    "displayView",
                    "editorTextChanged",
                    "selectedNoteId=" + contentsView.selectedNoteId
                    + " structured=" + contentsView.showStructuredDocumentFlow
                    + " " + EditorTrace.describeText(contentsView.editorText),
                    contentsView)
        contentsView.scheduleMinimapSnapshotRefresh(false);
        if (!contentsView.showStructuredDocumentFlow)
            contentsView.scheduleDocumentPresentationRefresh(false);
    }
    onShowStructuredDocumentFlowChanged: {
        EditorTrace.trace(
                    "displayView",
                    "showStructuredDocumentFlowChanged",
                    "showStructuredDocumentFlow=" + contentsView.showStructuredDocumentFlow,
                    contentsView)
        if (contentsView.showStructuredDocumentFlow) {
            presentationRefreshController.clearPendingWhileFocused();
            if (documentPresentationRefreshTimer.running)
                documentPresentationRefreshTimer.stop();
            if (contentsView.renderedEditorHtml !== "")
                contentsView.renderedEditorHtml = "";
            return;
        }
        contentsView.scheduleDocumentPresentationRefresh(true);
    }
    onHeightChanged: {
        contentsView.scheduleMinimapSnapshotRefresh(true);
        contentsView.scheduleGutterRefresh(2);
    }
    onMinimapVisibleChanged: {
        contentsView.minimapSnapshotForceFullRefresh = true;
        contentsView.scheduleDocumentPresentationRefresh(true);
    }
    onSelectedNoteBodyTextChanged: {
        EditorTrace.trace(
                    "displayView",
                    "selectedNoteBodyTextChanged",
                    "selectedNoteId=" + contentsView.selectedNoteId
                    + " bodyNoteId=" + contentsView.selectedNoteBodyNoteId
                    + " loading=" + contentsView.selectedNoteBodyLoading
                    + " " + EditorTrace.describeText(contentsView.selectedNoteBodyText),
                    contentsView)
        if (contentsView.selectedNoteBodyLoading)
            return;
        if (contentsView.selectedNoteBodyNoteId !== contentsView.selectedNoteId)
            return;
        if (contentsView.hasPendingNoteEntryGutterRefresh(contentsView.selectedNoteId)
                && structuredDocumentFlow
                && structuredDocumentFlow.scheduleLayoutCacheRefresh !== undefined) {
            structuredDocumentFlow.scheduleLayoutCacheRefresh();
        }
        contentsView.scheduleSelectionModelSync({
                                                   "scheduleReconcile": true
                                               });
    }
    onSelectedNoteBodyLoadingChanged: {
        EditorTrace.trace(
                    "displayView",
                    "selectedNoteBodyLoadingChanged",
                    "selectedNoteId=" + contentsView.selectedNoteId
                    + " bodyNoteId=" + contentsView.selectedNoteBodyNoteId
                    + " loading=" + contentsView.selectedNoteBodyLoading,
                    contentsView)
        if (!contentsView.selectedNoteBodyLoading
                && contentsView.selectedNoteBodyNoteId === contentsView.selectedNoteId
                && contentsView.selectedNoteBodyText.length === 0) {
            if (contentsView.hasPendingNoteEntryGutterRefresh(contentsView.selectedNoteId)
                    && structuredDocumentFlow
                    && structuredDocumentFlow.scheduleLayoutCacheRefresh !== undefined) {
                structuredDocumentFlow.scheduleLayoutCacheRefresh();
            }
            contentsView.scheduleSelectionModelSync({
                                                       "scheduleReconcile": true,
                                                       "fallbackRefresh": true
                                                   });
        }
    }
    onSelectedNoteIdChanged: {
        EditorTrace.trace(
                    "displayView",
                    "selectedNoteIdChanged",
                    "selectedNoteId=" + contentsView.selectedNoteId,
                    contentsView)
        contentsView.scheduleNoteEntryGutterRefresh(contentsView.selectedNoteId);
        contentsView.scheduleSelectionModelSync({
                                                   "resetSnapshot": true,
                                                   "focusEditor": true
                                               });
    }
    onPendingBodySaveChanged: {
        EditorTrace.trace(
                    "displayView",
                    "pendingBodySaveChanged",
                    "pendingBodySave=" + contentsView.pendingBodySave,
                    contentsView)
        if (!contentsView.pendingBodySave) {
            selectionSyncCoordinator.invalidateComparedSnapshot();
            contentsView.scheduleEditorEntrySnapshotReconcile();
        }
    }
    onTypingSessionSyncProtectedChanged: {
        EditorTrace.trace(
                    "displayView",
                    "typingSessionSyncProtectedChanged",
                    "typingSessionSyncProtected=" + contentsView.typingSessionSyncProtected,
                    contentsView)
        if (!contentsView.typingSessionSyncProtected) {
            selectionSyncCoordinator.invalidateComparedSnapshot();
            contentsView.scheduleEditorEntrySnapshotReconcile();
        }
    }
    onVisibleChanged: {
        EditorTrace.trace("displayView", "visibleChanged", "visible=" + visible, contentsView)
        if (visible) {
            contentsView.scheduleSelectionModelSync({
                                                       "scheduleReconcile": true,
                                                       "forceVisualRefresh": true
                                                   });
        }
    }
    onWidthChanged: {
        contentsView.scheduleMinimapSnapshotRefresh(true);
        contentsView.scheduleGutterRefresh(2);
    }

    ContentsDisplayHostModePolicy {
        id: modePolicy

        editorProjection: editorProjection
        minimapVisible: contentsView.minimapVisible
        mobileHost: contentsView.mobileHost
        preferNativeInputHandling: contentsView.preferNativeInputHandling
        showDedicatedResourceViewer: contentsView.showDedicatedResourceViewer
        showFormattedTextRenderer: contentsView.showFormattedTextRenderer
        showPrintEditorLayout: contentsView.showPrintEditorLayout
        showStructuredDocumentFlow: contentsView.showStructuredDocumentFlow
    }
    ContentsPagePrintLayoutRenderer {
        id: pagePrintLayoutRenderer

        activeEditorViewMode: contentsView.activeEditorViewModeValue
        dedicatedResourceViewerVisible: contentsView.showDedicatedResourceViewer
        editorContentHeight: contentsView.showStructuredDocumentFlow
                             ? (structuredDocumentFlow ? Number(structuredDocumentFlow.implicitHeight) || 0 : 0)
                             : (contentEditor && contentEditor.inputContentHeight !== undefined ? Number(contentEditor.inputContentHeight) || 0 : 0)
        editorViewportHeight: contentsView.editorViewportHeight
        editorViewportWidth: editorViewport ? Number(editorViewport.width) || 0 : 0
        guideHorizontalInset: LV.Theme.gap12 * 2
        guideVerticalInset: LV.Theme.gap12 * 2
        hasSelectedNote: contentsView.hasSelectedNote
        paperHorizontalMargin: LV.Theme.gap12
        paperMaxWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(880)))
        paperSeparatorThickness: Math.max(1, Math.round(LV.Theme.strokeThin))
        paperShadowOffsetX: Math.max(1, Math.round(LV.Theme.scaleMetric(1)))
        paperShadowOffsetY: Math.max(1, Math.round(LV.Theme.scaleMetric(2)))
        paperVerticalMargin: LV.Theme.gap4
    }
    ContentsEditorSelectionController {
        id: editorSelectionController

        contentEditor: contentsView.contentEditor
        editorSession: editorSession
        editorViewport: editorViewport
        selectionBridge: selectionBridge
        selectionContextMenu: editorSelectionContextMenu
        textFormatRenderer: editorProjection
        textMetricsBridge: editorProjection
        view: contentsView
    }
    ContentsEditorSelectionBridge {
        id: selectionBridge

        contentViewModel: contentsView.contentViewModel
        noteListModel: contentsView.noteListModel
    }
    ContentsDisplaySelectionSyncCoordinator {
        id: selectionSyncCoordinator

        editorBoundNoteId: contentsView.editorBoundNoteId
        editorInputFocused: contentsView.editorInputFocused
        editorSessionBoundToSelectedNote: contentsView.editorSessionBoundToSelectedNote
        pendingBodySave: contentsView.pendingBodySave
        selectedNoteBodyLoading: contentsView.selectedNoteBodyLoading
        selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId
        selectedNoteBodyText: contentsView.selectedNoteBodyText
        selectedNoteId: contentsView.selectedNoteId
        typingSessionSyncProtected: contentsView.typingSessionSyncProtected
        visible: contentsView.visible
    }
    ContentsDisplayPresentationRefreshController {
        id: presentationRefreshController

        editorInputFocused: contentsView.editorInputFocused
        projectionEnabled: contentsView.documentPresentationProjectionEnabled
        renderDirty: contentsView.documentPresentationRenderDirty()
        typingSessionSyncProtected: contentsView.typingSessionSyncProtected
    }
    ContentsDisplayRefreshCoordinator {
        id: refreshCoordinator

        editorInputFocused: contentsView.editorInputFocused
        lineGeometryRefreshEnabled: contentsView.lineGeometryRefreshEnabled
        liveLogicalLineCount: Math.max(1, Number(contentsView.liveLogicalLineCount) || 1)
        minimapRefreshEnabled: contentsView.minimapRefreshEnabled
        preferNativeInputHandling: contentsView.preferNativeInputHandling
        showEditorGutter: contentsView.showEditorGutter
        showPrintEditorLayout: contentsView.showPrintEditorLayout
        structuredHostGeometryActive: contentsView.structuredHostGeometryActive
    }
    ContentsDisplaySessionCoordinator {
        id: sessionCoordinator

        editorSessionBoundToSelectedNote: contentsView.editorSessionBoundToSelectedNote
        editorText: contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText)
        selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId === undefined || contentsView.selectedNoteBodyNoteId === null ? "" : String(contentsView.selectedNoteBodyNoteId)
        selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? "" : String(contentsView.selectedNoteBodyText)
        selectedNoteId: contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null ? "" : String(contentsView.selectedNoteId)
        structuredFlowSourceText: contentsView.structuredFlowSourceText === undefined || contentsView.structuredFlowSourceText === null ? "" : String(contentsView.structuredFlowSourceText)
    }
    ContentsDisplayContextMenuCoordinator {
        id: contextMenuCoordinator

        structuredDocumentFlowVisible: contentsView.showStructuredDocumentFlow
        structuredContextMenuBlockIndex: Math.max(0, Math.floor(Number(contentsView.structuredContextMenuBlockIndex) || 0))
        structuredContextMenuSelectionSnapshot: contentsView.structuredContextMenuSelectionSnapshot && typeof contentsView.structuredContextMenuSelectionSnapshot === "object"
                                             ? contentsView.structuredContextMenuSelectionSnapshot
                                             : ({})
    }
    ContentsDisplayViewportCoordinator {
        id: viewportCoordinator

        currentCursorLineNumber: Math.max(1, Number(contentsView.currentCursorLineNumber) || 1)
        currentCursorOffset: Math.max(0, Number(contentEditor.cursorPosition) || 0)
        editorContentOffsetY: Number(contentsView.editorContentOffsetY) || 0
        editorDocumentStartY: Number(contentsView.editorDocumentStartY) || 0
        editorInputFocused: contentsView.editorInputFocused
        editorLineHeight: Number(contentsView.editorLineHeight) || 0
        editorSurfaceHeight: Number(contentsView.editorSurfaceHeight) || 0
        editorViewportHeight: Number(contentsView.editorViewportHeight) || 0
        logicalLineCount: Math.max(1, Number(contentsView.logicalLineCount) || 1)
        logicalTextLength: Math.max(0, Number(contentsView.logicalTextLength()) || 0)
        minimapResolvedTrackHeight: Number(contentsView.minimapResolvedTrackHeight) || 1
        showPrintEditorLayout: contentsView.showPrintEditorLayout
        structuredHostGeometryActive: contentsView.structuredHostGeometryActive
    }
    ContentsDisplayGutterCoordinator {
        id: gutterCoordinator

        gutterViewportHeight: Number(contentsView.gutterViewportHeight) || 0
        logicalLineCount: Math.max(1, Number(contentsView.logicalLineCount) || 1)
        structuredHostGeometryActive: contentsView.structuredHostGeometryActive
    }
    ContentsDisplayMinimapCoordinator {
        id: minimapCoordinator

        editorDocumentStartY: Number(contentsView.editorDocumentStartY) || 0
        editorLineHeight: Number(contentsView.editorLineHeight) || 0
        logicalLineCount: Math.max(1, Number(contentsView.logicalLineCount) || 1)
        structuredHostGeometryActive: contentsView.structuredHostGeometryActive
    }
    ContentsDisplayStructuredFlowCoordinator {
        id: structuredFlowCoordinator

        editorContentOffsetY: Number(contentsView.editorContentOffsetY) || 0
        editorDocumentStartY: Number(contentsView.editorDocumentStartY) || 0
        editorLineHeight: Number(contentsView.editorLineHeight) || 0
        gutterViewportHeight: Number(contentsView.gutterViewportHeight) || 0
        logicalLineCount: Math.max(1, Number(contentsView.logicalLineCount) || 1)
        structuredGutterGeometrySignature: contentsView.structuredGutterGeometrySignature
        structuredHostGeometryActive: contentsView.structuredHostGeometryActive
    }
    ContentsEditorTypingController {
        id: editorTypingController

        agendaBackend: contentsAgendaBackend
        calloutBackend: contentsCalloutBackend
        contentEditor: contentsView.contentEditor
        editorSession: editorSession
        textFormatRenderer: editorProjection
        textMetricsBridge: editorProjection
        view: contentsView
    }
    ContentsResourceImportController {
        id: resourceImportController

        bodyResourceRenderer: bodyResourceRenderer
        clearResourceDropActiveHandler: function () {
            contentsView.resourceDropActive = false;
        }
        clipboardImageAvailableHandler: function () {
            return contentsView.clipboardImageAvailableForPaste();
        }
        contentEditor: contentsView.contentEditor
        currentEditorCursorPositionHandler: function () {
            return contentsView.currentEditorCursorPosition();
        }
        documentSourceMutationHandler: function (nextSourceText, focusRequest) {
            return contentsView.applyDocumentSourceMutation(nextSourceText, focusRequest);
        }
        documentPresentationSourceText: contentsView.documentPresentationSourceText
        editorHorizontalInset: contentsView.editorHorizontalInset
        editorProjection: editorProjection
        editorText: contentsView.editorText
        editorSession: editorSession
        editorTypingController: editorTypingController
        editorViewport: editorViewport
        encodeXmlAttributeValueHandler: function (value) {
            return contentsView.encodeXmlAttributeValue(value);
        }
        hasSelectedNote: contentsView.hasSelectedNote
        printPaperTextWidth: contentsView.printPaperTextWidth
        resourceEditorPlaceholderLineCount: contentsView.resourceEditorPlaceholderLineCount
        resourceImportConflictPolicyAbort: contentsView.resourceImportConflictPolicyAbort
        resourceImportModeClipboard: contentsView.resourceImportModeClipboard
        resourceImportModeNone: contentsView.resourceImportModeNone
        resourceImportModeUrls: contentsView.resourceImportModeUrls
        resourcesImportViewModel: contentsView.resourcesImportViewModel
        inlineHtmlImageRenderingEnabled: contentsView.inlineHtmlImageRenderingEnabled
        selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId
        selectedNoteBodyText: contentsView.selectedNoteBodyText
        selectedNoteId: contentsView.selectedNoteId
        showDedicatedResourceViewer: contentsView.showDedicatedResourceViewer
        showFormattedTextRenderer: contentsView.showFormattedTextRenderer
        showPrintEditorLayout: contentsView.showPrintEditorLayout
        showStructuredDocumentFlow: contentsView.showStructuredDocumentFlow
        structuredDocumentFlow: structuredDocumentFlow
    }
    ContentsBodyResourceRenderer {
        id: bodyResourceRenderer

        contentViewModel: contentsView.contentViewModel
        documentBlocks: structuredBlockRenderer.renderedDocumentBlocks
        fallbackContentViewModel: contentsView.libraryHierarchyViewModel
        maxRenderCount: contentsView.resourceRenderDisplayLimit
        noteId: contentsView.selectedNoteId
        noteDirectoryPath: selectionBridge.selectedNoteDirectoryPath
    }
    Connections {
        target: bodyResourceRenderer

        function onRenderedResourcesChanged() {
            if (!contentsView.resourceBlocksRenderedInlineByHtmlProjection)
                return;
            contentsView.refreshInlineResourcePresentation();
            contentsView.scheduleGutterRefresh(2, "resource-line-geometry");
        }

        ignoreUnknownSignals: true
    }
    ContentsAgendaBackend {
        id: contentsAgendaBackend
    }
    ContentsCalloutBackend {
        id: contentsCalloutBackend
    }
    ContentsStructuredBlockRenderer {
        id: structuredBlockRenderer

        backgroundRefreshEnabled: false
        sourceText: contentsView.documentPresentationSourceText
    }
    ContentsEditorPresentationProjection {
        id: editorProjection

        paperPaletteEnabled: contentsView.showPrintEditorLayout
        previewEnabled: contentsView.showFormattedTextRenderer
        sourceText: contentsView.documentPresentationSourceText
    }
    Connections {
        function onLogicalTextChanged() {
            if (contentsView.structuredHostGeometryActive)
                return;
            const logicalText = editorProjection.logicalText;
            contentsView.liveLogicalTextLength = logicalText !== undefined && logicalText !== null
                    ? String(logicalText).length
                    : 0;
        }
        function onLogicalLineCountChanged() {
            if (contentsView.structuredHostGeometryActive)
                return;
            if (contentsView.refreshLiveLogicalLineMetrics())
                contentsView.scheduleGutterRefresh(1, "projection-line-metrics");
        }
        function onLogicalLineStartOffsetsChanged() {
            if (contentsView.structuredHostGeometryActive)
                return;
            if (contentsView.refreshLiveLogicalLineMetrics())
                contentsView.scheduleGutterRefresh(1, "projection-line-metrics");
        }

        target: editorProjection
    }
    ContentsGutterMarkerBridge {
        id: gutterMarkerBridge

        gutterMarkers: contentsView.gutterMarkers
    }
    ContentsEditorSessionController {
        id: editorSession

        agendaBackend: contentsAgendaBackend
        selectionBridge: selectionBridge
        typingIdleThresholdMs: contentsView.editorIdleSyncThresholdMs
    }
    Timer {
        id: documentPresentationRefreshTimer

        interval: contentsView.documentPresentationRefreshIntervalMs
        repeat: false

        onTriggered: contentsView.applyPresentationRefreshPlan(
                         presentationRefreshController.planDeferredTrigger())
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
        function onViewSessionSnapshotReconciled(noteId, refreshed, success, _errorMessage) {
            selectionSyncCoordinator.handleSnapshotReconcileFinished(noteId, success);
        }

        target: selectionBridge
    }
    Connections {
        function onSelectionSyncFlushRequested(plan) {
            const syncPlan = plan && typeof plan === "object" ? plan : ({});
            if (syncPlan.resetSelectionCache)
                contentsView.resetEditorSelectionCache();
            if (syncPlan.flushPendingEditorText
                    && editorSession
                    && editorSession.flushPendingEditorText !== undefined) {
                editorSession.flushPendingEditorText();
            }
            let selectionSynced = false;
            if (syncPlan.attemptSelectionSync
                    && editorSession
                    && editorSession.requestSyncEditorTextFromSelection !== undefined) {
                selectionSynced = editorSession.requestSyncEditorTextFromSelection(
                            String(syncPlan.selectedNoteId || ""),
                            String(syncPlan.selectedNoteBodyText || ""),
                            String(syncPlan.selectedNoteBodyNoteId || ""));
            }
            if (syncPlan.scheduleSnapshotReconcile)
                contentsView.scheduleEditorEntrySnapshotReconcile();
            if (syncPlan.forceVisualRefresh
                    || (!selectionSynced && syncPlan.fallbackRefreshIfSyncSkipped)) {
                contentsView.scheduleMinimapSnapshotRefresh(true);
                contentsView.scheduleDocumentPresentationRefresh(true);
                contentsView.scheduleGutterRefresh(4);
            }
            if (syncPlan.focusEditorForSelectedNote)
                contentsView.focusEditorForPendingNote();
        }
        function onSnapshotReconcileRequested() {
            contentsView.reconcileEditorEntrySnapshotOnce();
        }
        function onEditorFocusRequested() {
            contentsView.focusEditorForPendingNote();
        }

        target: selectionSyncCoordinator
    }
    Connections {
        function onRenderedBlocksChanged() {
            if (contentsView.structuredHostGeometryActive) {
                if (contentsView.hasPendingNoteEntryGutterRefresh(contentsView.selectedNoteId)
                        && structuredDocumentFlow
                        && structuredDocumentFlow.scheduleLayoutCacheRefresh !== undefined) {
                    structuredDocumentFlow.scheduleLayoutCacheRefresh();
                }
                contentsView.scheduleMinimapSnapshotRefresh(true);
                contentsView.scheduleGutterRefresh(1, "structured-line-geometry");
            }
        }

        target: structuredBlockRenderer
    }
    Connections {
        function onCachedLogicalLineEntriesChanged() {
            if (!contentsView.structuredHostGeometryActive)
                return;
            if (contentsView.finalizePendingNoteEntryGutterRefresh(
                        contentsView.selectedNoteId,
                        "structured-layout-cache")) {
                return;
            }
            const metricsChanged = contentsView.refreshLiveLogicalLineMetrics();
            const geometryChanged = contentsView.consumeStructuredGutterGeometryChange();
            if (!metricsChanged && !geometryChanged)
                return;
            contentsView.scheduleGutterRefresh(
                        geometryChanged ? 2 : 1,
                        geometryChanged ? "structured-line-geometry" : "structured-line-metrics");
        }
        function onCurrentLogicalLineNumberChanged() {
            if (!contentsView.structuredHostGeometryActive)
                return;
            contentsView.scheduleCursorDrivenUiRefresh();
        }
        function onImplicitHeightChanged() {
            if (!contentsView.structuredHostGeometryActive)
                return;
            contentsView.scheduleMinimapSnapshotRefresh(true);
            contentsView.scheduleGutterRefresh(2, "structured-line-geometry");
        }

        ignoreUnknownSignals: true
        target: structuredDocumentFlow
    }
    Connections {
        function onEditorTextSynchronized() {
            if (contentsView.parsedStructuredFlowRequested) {
                if (!contentsView.structuredHostGeometryActive) {
                    contentsView.finalizePendingNoteEntryGutterRefresh(
                                editorSession.editorBoundNoteId,
                                "editor-text-synchronized");
                }
                if (contentsView.renderedEditorHtml !== "")
                    contentsView.renderedEditorHtml = "";
                return;
            }
            if (contentsView.finalizePendingNoteEntryGutterRefresh(
                        editorSession.editorBoundNoteId,
                        "editor-text-synchronized")) {
                return;
            }
            contentsView.scheduleMinimapSnapshotRefresh(true);
            contentsView.scheduleDocumentPresentationRefresh(true);
            contentsView.scheduleGutterRefresh(4);
        }

        target: editorSession
    }
    Connections {
        function onContentHeightChanged() {
            if (contentsView.editorInputFocused)
                contentsView.scheduleDeferredDocumentPresentationRefresh();
            else
                contentsView.scheduleMinimapSnapshotRefresh(false);
            contentsView.scheduleGutterRefresh(2, "editor-line-geometry");
            contentsView.scheduleTypingViewportCorrection(true);
        }
        function onCursorPositionChanged() {
            contentsView.scheduleCursorDrivenUiRefresh();
            contentsView.scheduleTypingViewportCorrection(false);
        }
        function onLineCountChanged() {
            if (contentsView.editorInputFocused)
                contentsView.scheduleDeferredDocumentPresentationRefresh();
            else
                contentsView.scheduleMinimapSnapshotRefresh(false);
            contentsView.scheduleGutterRefresh(2, "editor-line-geometry");
            contentsView.scheduleTypingViewportCorrection(true);
        }

        ignoreUnknownSignals: true
        target: contentsView.legacyInlineEditorActive ? contentEditor : null
    }
    Connections {
        function onContentYChanged() {
            contentsView.refreshMinimapViewportTracking();
            if (minimapLayer && contentsView.minimapRefreshEnabled)
                minimapLayer.requestRepaint();
        }

        ignoreUnknownSignals: true
        target: contentsView.editorFlickable
    }
    Connections {
        function onCursorPositionChanged() {
            contentsView.scheduleCursorDrivenUiRefresh();
            contentsView.scheduleTypingViewportCorrection(false);
        }

        ignoreUnknownSignals: true
        target: contentsView.legacyInlineEditorActive && contentEditor && contentEditor.editorItem ? contentEditor.editorItem : null
    }
    Connections {
        function onCursorPositionChanged() {
            contentsView.scheduleCursorDrivenUiRefresh();
            contentsView.scheduleTypingViewportCorrection(false);
        }

        ignoreUnknownSignals: true
        target: contentsView.legacyInlineEditorActive && contentEditor && contentEditor.editorItem && contentEditor.editorItem.inputItem ? contentEditor.editorItem.inputItem : null
    }
    Connections {
        function onActiveBlockCursorRevisionChanged() {
            contentsView.scheduleCursorDrivenUiRefresh();
            contentsView.scheduleTypingViewportCorrection(false);
        }
        function onImplicitHeightChanged() {
            contentsView.scheduleTypingViewportCorrection(true);
        }
        function onFocusedChanged() {
            if (structuredDocumentFlow && structuredDocumentFlow.focused)
                contentsView.scheduleTypingViewportCorrection(true);
        }

        ignoreUnknownSignals: true
        target: contentsView.showStructuredDocumentFlow ? structuredDocumentFlow : null
    }
    Connections {
        function onContentYChanged() {
            contentsView.scheduleViewportGutterRefresh();
        }
        function onHeightChanged() {
            contentsView.scheduleViewportGutterRefresh();
            contentsView.scheduleTypingViewportCorrection(true);
        }
        function onWidthChanged() {
            contentsView.scheduleGutterRefresh(2);
        }

        target: contentsView.editorFlickable
    }
    Rectangle {
        id: contentsDisplayView

        anchors.fill: parent
        color: contentsView.displayColor

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: contentsView.effectiveFrameHorizontalInset
            anchors.rightMargin: (contentsView.effectiveFrameHorizontalInset
                                  + (contentsView.showMinimapRail ? contentsView.minimapOuterWidth : 0))
            LayoutMirroring.childrenInherit: false
            LayoutMirroring.enabled: false
            layoutDirection: Qt.LeftToRight
            spacing: 0
            visible: contentsView.hasSelectedNote

            ContentsGutterLayer {
                id: gutterLayer

                Layout.fillHeight: true
                Layout.maximumWidth: contentsView.effectiveGutterWidth
                Layout.minimumWidth: contentsView.effectiveGutterWidth
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
                    return contentsView.gutterLineY(lineNumber);
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
                enabled: !contentsView.selectedNoteBodyLoading
                clip: true

                Flickable {
                    id: printDocumentViewport

                    anchors.fill: parent
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    contentHeight: Math.max(height, printDocumentSurface.height)
                    contentWidth: width
                    flickableDirection: Flickable.VerticalFlick
                    interactive: contentsView.showPrintEditorLayout && contentHeight > height
                    visible: contentsView.showPrintEditorLayout
                    z: 0

                    Item {
                        id: printDocumentSurface

                        height: contentsView.printDocumentSurfaceHeight
                        width: printDocumentViewport.width

                            Rectangle {
                                id: printEditorCanvas

                                anchors.fill: parent
                                color: contentsView.printCanvasColor
                            }
                            Item {
                                id: printPaperColumn

                                height: contentsView.printPaperDocumentHeight
                                width: contentsView.printPaperResolvedWidth
                                x: Math.max(0, (Number(parent ? parent.width : 0) - width) / 2)
                                y: contentsView.printPaperVerticalMargin
                            }
                            Repeater {
                                model: contentsView.printDocumentPageCount

                                delegate: Item {
                                    required property int index

                                    x: printPaperColumn.x
                                    y: printPaperColumn.y + index * contentsView.printPaperTextHeight
                                    width: printPaperColumn.width
                                    height: contentsView.printPaperResolvedHeight

                                    Rectangle {
                                        x: contentsView.printPaperShadowOffsetX
                                        y: contentsView.printPaperShadowOffsetY
                                        width: parent.width
                                        height: parent.height
                                        color: contentsView.printPaperShadowColor
                                        radius: LV.Theme.radiusSm
                                        z: -2
                                    }
                                    Rectangle {
                                        id: printPaperSheet

                                        anchors.fill: parent
                                        border.color: contentsView.printPaperBorderColor
                                        border.width: contentsView.printPaperSeparatorThickness
                                        color: contentsView.printPaperColor
                                        gradient: Gradient {
                                            GradientStop {
                                                position: 0.0
                                                color: contentsView.printPaperHighlightColor
                                            }
                                            GradientStop {
                                                position: 0.72
                                                color: contentsView.printPaperColor
                                            }
                                            GradientStop {
                                                position: 1.0
                                                color: contentsView.printPaperShadeColor
                                            }
                                        }
                                        radius: LV.Theme.radiusSm
                                    }
                                    Rectangle {
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        color: contentsView.printPaperSeparatorColor
                                        height: contentsView.printPaperSeparatorThickness
                                        visible: index > 0
                                    }
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
                        }
                    }
                    Flickable {
                        id: structuredDocumentViewport

                        anchors.fill: parent
                        boundsBehavior: Flickable.StopAtBounds
                        clip: true
                        contentHeight: Math.max(
                                           height,
                                           (Number(structuredDocumentFlow.y) || 0)
                                           + (Number(structuredDocumentFlow.implicitHeight) || 0)
                                           + contentsView.editorBottomInset)
                        contentWidth: width
                        flickableDirection: Flickable.VerticalFlick
                        interactive: !contentsView.showPrintEditorLayout && contentHeight > height
                        visible: contentsView.showStructuredDocumentFlow && !contentsView.showPrintEditorLayout
                        z: 1

                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            enabled: structuredDocumentViewport.visible

                            onTapped: function (eventPoint) {
                                const viewportTapX = eventPoint && eventPoint.position ? Number(eventPoint.position.x) || 0 : 0;
                                const viewportTapY = eventPoint && eventPoint.position ? Number(eventPoint.position.y) || 0 : 0;
                                const contentTapX = viewportTapX + (Number(structuredDocumentViewport.contentX) || 0);
                                const contentTapY = viewportTapY + (Number(structuredDocumentViewport.contentY) || 0);
                                const flowTapX = contentTapX - (Number(structuredDocumentFlow.x) || 0);
                                const flowTapY = contentTapY - (Number(structuredDocumentFlow.y) || 0);
                                if (structuredDocumentFlow
                                        && structuredDocumentFlow.visible
                                        && structuredDocumentFlow.hasBlockAtPoint !== undefined
                                        && structuredDocumentFlow.hasBlockAtPoint(flowTapX, flowTapY)) {
                                    return;
                                }
                                Qt.callLater(function () {
                                    contentsView.requestStructuredDocumentEndEdit();
                                });
                            }
                        }
                    }
                    ContentsStructuredDocumentFlow {
                        id: structuredDocumentFlow

                        agendaBackend: contentsAgendaBackend
                        calloutBackend: contentsCalloutBackend
                        documentBlocks: structuredBlockRenderer.renderedDocumentBlocks
                        lineHeightHint: contentsView.editorLineHeight
                        paperPaletteEnabled: contentsView.showPrintEditorLayout
                        parent: contentsView.showPrintEditorLayout ? printDocumentSurface : structuredDocumentViewport.contentItem
                        renderedResources: bodyResourceRenderer.renderedResources
                        shortcutKeyPressHandler: function (event) {
                            return contentsView.handleClipboardImagePasteShortcut(event);
                        }
                        sourceText: contentsView.structuredFlowSourceText
                        viewportContentY: contentsView.showPrintEditorLayout ? 0 : (Number(structuredDocumentViewport.contentY) || 0)
                        viewportHeight: contentsView.showPrintEditorLayout ? 0 : (Number(structuredDocumentViewport.height) || 0)
                        width: contentsView.showPrintEditorLayout
                               ? contentsView.printPaperTextWidth
                               : Math.max(0, structuredDocumentViewport.width - contentsView.editorHorizontalInset * 2)
                        x: contentsView.showPrintEditorLayout ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset : contentsView.editorHorizontalInset
                        y: contentsView.showPrintEditorLayout ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset : contentsView.editorDocumentStartY
                        visible: contentsView.showStructuredDocumentFlow
                        z: contentsView.showPrintEditorLayout ? 2 : 1

                        onSourceMutationRequested: function (nextSourceText, focusRequest) {
                            contentsView.applyDocumentSourceMutation(nextSourceText, focusRequest);
                        }
                    }
                    ContentsAgendaLayer {
                        id: agendaBackgroundLayer

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.leftMargin: contentsView.showPrintEditorLayout
                                            ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset
                                            : contentsView.editorHorizontalInset
                        anchors.rightMargin: contentsView.showPrintEditorLayout
                                             ? Math.max(
                                                   0,
                                                   (parent ? Number(parent.width) || 0 : 0)
                                                   - ((Number(printPaperColumn.x) || 0)
                                                      + contentsView.printGuideHorizontalInset
                                                      + contentsView.printPaperTextWidth))
                                             : contentsView.editorHorizontalInset
                        anchors.topMargin: (contentsView.showPrintEditorLayout
                                           ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset
                                           : contentsView.editorDocumentStartY)
                        enableCardFocus: false
                        enableTaskToggle: false
                        paperPaletteEnabled: contentsView.showPrintEditorLayout
                        renderedAgendas: contentsView.showStructuredDocumentFlow ? [] : structuredBlockRenderer.renderedAgendas
                        showTaskCheckbox: false
                        showTaskText: false
                        blockFocusHandler: function (sourceOffset) {
                            contentsView.focusStructuredBlockSourceOffset(sourceOffset);
                        }
                        sourceOffsetYResolver: function (sourceOffset) {
                            const logicalOffset = editorTypingController.logicalOffsetForSourceOffset(
                                        Math.max(0, Math.floor(Number(sourceOffset) || 0)));
                            const documentY = Math.max(0, Number(contentsView.documentYForOffset(logicalOffset)) || 0);
                            return documentY;
                        }
                        taskToggleHandler: function (taskOpenTagStart, taskOpenTagEnd, checked) {
                            contentsView.setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked);
                        }
                        visible: contentsView.hasSelectedNote
                                 && !contentsView.showStructuredDocumentFlow
                                 && !contentsView.showDedicatedResourceViewer
                                 && !contentsView.showFormattedTextRenderer
                                 && agendaBackgroundLayer.agendaCount > 0
                        enabled: visible
                        z: 0
                    }
                    ContentsCalloutLayer {
                        id: calloutBackgroundLayer

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.leftMargin: contentsView.showPrintEditorLayout
                                            ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset
                                            : contentsView.editorHorizontalInset
                        anchors.rightMargin: contentsView.showPrintEditorLayout
                                             ? Math.max(
                                                   0,
                                                   (parent ? Number(parent.width) || 0 : 0)
                                                   - ((Number(printPaperColumn.x) || 0)
                                                      + contentsView.printGuideHorizontalInset
                                                      + contentsView.printPaperTextWidth))
                                             : contentsView.editorHorizontalInset
                        anchors.topMargin: contentsView.showPrintEditorLayout
                                           ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset
                                           : contentsView.editorDocumentStartY
                        enableCardFocus: false
                        paperPaletteEnabled: contentsView.showPrintEditorLayout
                        renderedCallouts: contentsView.showStructuredDocumentFlow ? [] : structuredBlockRenderer.renderedCallouts
                        showText: false
                        blockFocusHandler: function (sourceOffset) {
                            contentsView.focusStructuredBlockSourceOffset(sourceOffset);
                        }
                        sourceOffsetYResolver: function (sourceOffset) {
                            const logicalOffset = editorTypingController.logicalOffsetForSourceOffset(
                                        Math.max(0, Math.floor(Number(sourceOffset) || 0)));
                            const documentY = Math.max(0, Number(contentsView.documentYForOffset(logicalOffset)) || 0);
                            return documentY;
                        }
                        visible: contentsView.hasSelectedNote
                                 && !contentsView.showStructuredDocumentFlow
                                 && !contentsView.showDedicatedResourceViewer
                                 && !contentsView.showFormattedTextRenderer
                                 && calloutBackgroundLayer.calloutCount > 0
                        enabled: visible
                        z: 0
                    }
                    QtObject {
                        id: contentEditorProxy

                        property int cursorPosition: 0
                        property real contentOffsetY: 0
                        property real contentHeight: 0
                        property var editorItem: null
                        property bool focused: false
                        property bool activeFocus: false
                        property real inputContentHeight: 0
                        property bool inputMethodComposing: false
                        property int length: 0
                        property string preeditText: ""
                        property var resolvedFlickable: null
                        property real width: 0

                        function forceActiveFocus() {
                        }

                        function setCursorPositionPreservingInputMethod(_cursorPosition) {
                        }
                    }
                    Component {
                        id: contentEditorComponent

                        ContentsInlineFormatEditor {
                            autoFocusOnPress: modePolicy.inlineEditorAutoFocusOnPress
                            anchors.fill: parent
                            backgroundColor: "transparent"
                            backgroundColorDisabled: "transparent"
                            backgroundColorFocused: "transparent"
                            backgroundColorHover: "transparent"
                            backgroundColorPressed: "transparent"
                            centeredTextHeight: contentsView.editorTextLineBoxHeight
                            cornerRadius: 0
                            editorHeight: contentsView.showPrintEditorLayout ? contentsView.printDocumentPageCount * contentsView.printPaperTextHeight : contentsView.editorSurfaceHeight
                            enforceModeDefaults: false
                            externalScroll: contentsView.showPrintEditorLayout
                            externalScrollViewport: contentsView.showPrintEditorLayout ? printDocumentViewport : null
                            fieldMinHeight: contentsView.showPrintEditorLayout ? contentsView.printDocumentPageCount * contentsView.printPaperTextHeight : Math.max(contentsView.minEditorHeight, contentsView.editorSurfaceHeight)
                            fontFamily: LV.Theme.fontBody
                            fontLetterSpacing: 0
                            fontPixelSize: contentsView.effectiveEditorFontPixelSize
                            fontWeight: contentsView.editorFontWeight
                            insetHorizontal: contentsView.showPrintEditorLayout ? 0 : contentsView.editorHorizontalInset
                            insetVertical: contentsView.showPrintEditorLayout ? 0 : contentsView.editorBottomInset
                            placeholderText: ""
                            preferNativeInputHandling: contentsView.preferNativeInputHandling
                            selectByMouse: true
                            selectedTextColor: LV.Theme.textPrimary
                            selectionColor: LV.Theme.accent
                            shapeStyle: 0
                            shortcutKeyPressHandler: function (event) {
                                return contentsView.handleInlineFormatShortcutKeyPress(event);
                            }
                            blockExternalDropMutation: contentsView.resourceDropActive
                                                       || contentsView.resourceDropEditorSurfaceGuardActive
                            renderedText: contentsView.renderedEditorHtml
                            showRenderedOutput: !contentsView.preferNativeInputHandling
                                                && contentsView.documentPresentationProjectionEnabled
                            showScrollBar: false
                            suppressCommittedTextEditedDispatch: contentsView.resourceDropEditorSurfaceGuardActive
                            text: contentsView.nativeInputDisplayText
                            textColor: contentsView.showPrintEditorLayout ? contentsView.printPaperTextColor : LV.Theme.bodyColor
                            wrapMode: TextEdit.Wrap

                            onFocusedChanged: {
                                if (focused) {
                                    if (documentPresentationRefreshTimer.running)
                                        documentPresentationRefreshTimer.stop();
                                    return;
                                }
                                const blurredNoteId = editorSession && editorSession.editorBoundNoteId !== undefined
                                        ? editorSession.editorBoundNoteId
                                        : "";
                                contentsView.flushEditorStateAfterInputSettles(0, blurredNoteId);
                                contentsView.scheduleDocumentPresentationRefresh(true);
                            }
                            onTextEdited: function () {
                                editorTypingController.handleEditorTextEdited();
                            }
                        }
                    }
                    Loader {
                        id: contentEditorLoader

                        active: contentsView.legacyInlineEditorActive
                        sourceComponent: contentEditorComponent
                        x: contentsView.showPrintEditorLayout ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset : 0
                        y: contentsView.showPrintEditorLayout ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset : contentsView.editorDocumentStartY
                        width: contentsView.showPrintEditorLayout ? contentsView.printPaperTextWidth : (parent ? parent.width : 0)
                        height: contentsView.showPrintEditorLayout ? contentsView.printDocumentPageCount * contentsView.printPaperTextHeight : (parent ? Math.max(0, parent.height - contentsView.editorDocumentStartY) : 0)
                        z: contentsView.showPrintEditorLayout ? 2 : 1
                        parent: contentsView.showPrintEditorLayout ? printDocumentSurface : editorViewport
                    }
                    ContentsAgendaLayer {
                        id: agendaRenderLayer

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.leftMargin: contentsView.showPrintEditorLayout
                                            ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset
                                            : contentsView.editorHorizontalInset
                        anchors.rightMargin: contentsView.showPrintEditorLayout
                                             ? Math.max(
                                                   0,
                                                   (parent ? Number(parent.width) || 0 : 0)
                                                   - ((Number(printPaperColumn.x) || 0)
                                                      + contentsView.printGuideHorizontalInset
                                                      + contentsView.printPaperTextWidth))
                                             : contentsView.editorHorizontalInset
                        anchors.topMargin: (contentsView.showPrintEditorLayout
                                           ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset
                                           : contentsView.editorDocumentStartY)
                        enableCardFocus: false
                        paperPaletteEnabled: contentsView.showPrintEditorLayout
                        renderedAgendas: contentsView.showStructuredDocumentFlow ? [] : structuredBlockRenderer.renderedAgendas
                        showFrame: false
                        showHeader: false
                        showTaskText: false
                        blockFocusHandler: function (sourceOffset) {
                            contentsView.focusStructuredBlockSourceOffset(sourceOffset);
                        }
                        sourceOffsetYResolver: function (sourceOffset) {
                            const logicalOffset = editorTypingController.logicalOffsetForSourceOffset(
                                        Math.max(0, Math.floor(Number(sourceOffset) || 0)));
                            const documentY = Math.max(0, Number(contentsView.documentYForOffset(logicalOffset)) || 0);
                            return documentY;
                        }
                        taskToggleHandler: function (taskOpenTagStart, taskOpenTagEnd, checked) {
                            contentsView.setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked);
                        }
                        visible: contentsView.hasSelectedNote
                                 && !contentsView.showStructuredDocumentFlow
                                 && !contentsView.showDedicatedResourceViewer
                                 && !contentsView.showFormattedTextRenderer
                                 && agendaRenderLayer.agendaCount > 0
                        enabled: visible
                        z: 3
                    }
                    Flickable {
                        id: formattedPreviewViewport

                        readonly property real bottomInset: contentsView.showPrintEditorLayout ? contentsView.printGuideVerticalInset : contentsView.editorBottomInset
                        readonly property real horizontalInset: contentsView.showPrintEditorLayout ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset : contentsView.editorHorizontalInset
                        readonly property real textWidth: {
                            if (contentsView.showPrintEditorLayout) {
                                const pageWidth = Number(printPaperColumn.width) || 0;
                                return Math.max(0, pageWidth - contentsView.printGuideHorizontalInset * 2);
                            }
                            return Math.max(0, formattedPreviewViewport.width - contentsView.editorHorizontalInset * 2);
                        }
                        readonly property real topInset: contentsView.showPrintEditorLayout ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset : contentsView.editorDocumentStartY

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
                            font.weight: contentsView.editorFontWeight
                            onLinkActivated: function (link) {
                                Qt.openUrlExternally(link);
                            }
                            text: editorProjection.renderedHtml
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
                        z: 5

                        onDropped: function (drop) {
                            const dropUrls = contentsView.extractResourceDropUrls(drop);
                            if (!contentsView.canAcceptResourceDropUrls(dropUrls)) {
                                if (drop)
                                    drop.accepted = false;
                                contentsView.releaseResourceDropEditorSurfaceGuard(false);
                                contentsView.resourceDropActive = false;
                                return;
                            }
                            if (drop && drop.acceptProposedAction !== undefined)
                                drop.acceptProposedAction();
                            const inserted = contentsView.importUrlsAsResourcesWithPrompt(dropUrls);
                            if (drop)
                                drop.accepted = inserted;
                        }
                        onEntered: function (drag) {
                            const dropUrls = contentsView.extractResourceDropUrls(drag);
                            const accepted = contentsView.canAcceptResourceDropUrls(dropUrls);
                            if (drag && accepted && drag.acceptProposedAction !== undefined)
                                drag.acceptProposedAction();
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
                            if (drag && accepted && drag.acceptProposedAction !== undefined)
                                drag.acceptProposedAction();
                            if (drag)
                                drag.accepted = accepted;
                            contentsView.resourceDropActive = accepted;
    }
    LV.Alert {
        id: resourceImportConflictAlert

        parent: contentsView
        buttonCount: 3
        dismissOnBackground: false
        message: contentsView.resourceImportConflictAlertMessage()
        open: contentsView.resourceImportConflictAlertOpen
        primaryText: "Overwrite"
        secondaryText: "Keep Both"
        tertiaryText: "Cancel Import"
        title: "Duplicate Resource Import"

        onPrimaryClicked: {
            contentsView.executePendingResourceImportWithPolicy(
                        contentsView.resourceImportConflictPolicyOverwrite);
        }
        onSecondaryClicked: {
            contentsView.executePendingResourceImportWithPolicy(
                        contentsView.resourceImportConflictPolicyKeepBoth);
        }
        onTertiaryClicked: {
            contentsView.cancelPendingResourceImportConflict();
        }
        onDismissed: {
            contentsView.cancelPendingResourceImportConflict();
        }
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
                            contentsView.primeEditorSelectionContextMenuSnapshot();
                        }
                    }
                    Shortcut {
                        autoRepeat: false
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote
                                 && !contentsView.showDedicatedResourceViewer
                                 && !contentsView.showFormattedTextRenderer
                                 && contentsView.resourcesImportViewModel
                                 && (!contentsView.resourcesImportViewModel.busy)
                                 && contentsView.resourcesImportViewModel.clipboardImageAvailable
                        sequence: StandardKey.Paste

                        onActivated: contentsView.pasteClipboardImageAsResource()
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
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+Shift+7"

                        onActivated: contentsView.queueMarkdownListMutation("ordered")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Alt+Shift+7"

                        onActivated: contentsView.queueMarkdownListMutation("ordered")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+Shift+8"

                        onActivated: contentsView.queueMarkdownListMutation("unordered")
                    }
                    Shortcut {
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Alt+Shift+8"

                        onActivated: contentsView.queueMarkdownListMutation("unordered")
                    }
                    Shortcut {
                        autoRepeat: false
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+Alt+T"

                        onActivated: contentsView.queueAgendaShortcutInsertion()
                    }
                    Shortcut {
                        autoRepeat: false
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Ctrl+Alt+T"

                        onActivated: contentsView.queueAgendaShortcutInsertion()
                    }
                    Shortcut {
                        autoRepeat: false
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+Alt+C"

                        onActivated: contentsView.queueCalloutShortcutInsertion()
                    }
                    Shortcut {
                        autoRepeat: false
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Ctrl+Alt+C"

                        onActivated: contentsView.queueCalloutShortcutInsertion()
                    }
                    Shortcut {
                        autoRepeat: false
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Meta+Shift+H"

                        onActivated: contentsView.queueBreakShortcutInsertion()
                    }
                    Shortcut {
                        autoRepeat: false
                        context: Qt.WindowShortcut
                        enabled: contentsView.hasSelectedNote && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
                        sequence: "Ctrl+Shift+H"

                        onActivated: contentsView.queueBreakShortcutInsertion()
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
                    Rectangle {
                        anchors.fill: parent
                        color: Qt.rgba(0.10, 0.11, 0.13, 0.78)
                        visible: contentsView.hasSelectedNote && contentsView.selectedNoteBodyLoading
                        z: 4

                        LV.Label {
                            anchors.centerIn: parent
                            color: LV.Theme.descriptionColor
                            style: caption
                            text: "Loading note..."
                        }
                    }
                }
            }
            ContentsMinimapLayer {
                id: minimapSpacer

                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                Layout.fillHeight: true
                Layout.maximumWidth: 0
                Layout.minimumWidth: 0
                Layout.preferredWidth: 0
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
                visible: false
            }
        }
        ContentsMinimapLayer {
            id: minimapLayer

            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: contentsView.effectiveFrameHorizontalInset
            anchors.top: parent.top
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
            visible: contentsView.showMinimapRail
            width: visible ? contentsView.minimapOuterWidth : 0
        }
    }
