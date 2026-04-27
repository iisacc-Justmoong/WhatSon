pragma ComponentBehavior: Bound
import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "../../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace
import "../../../../models/editor/display" as EditorDisplayModel
import "../../../../models/editor/display/ContentsMinimapSnapshotSupport.js" as MinimapSnapshotSupport
import "../../../../models/editor/input" as EditorInputModel
import "../../../../models/editor/resource" as EditorResourceModel

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
    readonly property var editorViewport: auxiliaryRailHost ? auxiliaryRailHost.editorViewport : null
    readonly property var inputCommandSurface: auxiliaryRailHost ? auxiliaryRailHost.inputCommandSurface : null
    readonly property var minimapLayer: auxiliaryRailHost ? auxiliaryRailHost.minimapLayer : null
    readonly property var printDocumentViewport: auxiliaryRailHost ? auxiliaryRailHost.printDocumentViewport : null
    readonly property var structuredDocumentFlow: auxiliaryRailHost ? auxiliaryRailHost.structuredDocumentFlow : null
    readonly property var structuredDocumentViewport: auxiliaryRailHost ? auxiliaryRailHost.structuredDocumentViewport : null
    readonly property var contentEditor: contentsView.structuredDocumentFlow
    property alias contextMenuSelectionEnd: editorSelectionController.contextMenuSelectionEnd
    property alias contextMenuSelectionStart: editorSelectionController.contextMenuSelectionStart
    property int structuredContextMenuBlockIndex: -1
    property var structuredContextMenuSelectionSnapshot: ({})
    readonly property int currentCursorLineNumber: contentsView.structuredHostGeometryActive
                                                   ? (structuredDocumentFlow && structuredDocumentFlow.currentLogicalLineNumber !== undefined
                                                      ? Math.max(1, Number(structuredDocumentFlow.currentLogicalLineNumber) || 1)
                                                      : 1)
                                                   : 1
    readonly property color decorativeMarkerYellow: LV.Theme.warning
    readonly property int desktopEditorFontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property int editorFontWeight: modePolicy.editorFontWeight
    property color displayColor: "transparent"
    readonly property int editorBottomInset: Math.max(
                                                LV.Theme.gap16,
                                                Math.round(contentsView.editorLineHeight * 6),
                                                Math.round(contentsView.editorSurfaceHeight * 0.5))
    property alias editorBoundNoteId: editorSession.editorBoundNoteId
    property alias editorBoundNoteDirectoryPath: editorSession.editorBoundNoteDirectoryPath
    readonly property var documentSourcePlan: documentSourceResolver.documentSourcePlan
    readonly property real editorContentOffsetY: {
        const flickable = contentsView.editorFlickable;
        if (flickable && flickable.contentY !== undefined)
            return -(Number(flickable.contentY) || 0);
        return 0;
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
    readonly property int resolvedLogicalTextLength: Math.max(0, Number(viewportCoordinator.logicalTextLength) || 0)
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
    property var minimapSnapshotEntries: []
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
    readonly property bool noteSnapshotRefreshEnabled: contentsView.visible && contentsView.hasSelectedNote && !contentsView.selectedNoteBodyLoading && (contentsView.editorSessionBoundToSelectedNote || contentsView.selectedNoteBodyResolved) && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer && contentsView.noteSelectionContractAvailable && !contentsView.editorInputFocused && !contentsView.typingSessionSyncProtected && !contentsView.pendingBodySave
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
    readonly property string documentPresentationSourceText: documentSourceResolver.documentPresentationSourceText
    readonly property bool documentPresentationRefreshPendingWhileFocused: presentationRefreshController.pendingWhileFocused
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
    readonly property bool preferNativeInputHandling: surfacePolicy.nativeInputPriority
    readonly property bool documentPresentationProjectionEnabled: surfacePolicy.documentPresentationProjectionEnabled
    readonly property bool inlineHtmlImageRenderingEnabled: surfacePolicy.inlineHtmlImageRenderingEnabled
    readonly property int resourceEditorPlaceholderLineCount: 1
    readonly property int editorIdleSyncThresholdMs: 1000
    readonly property string selectedNoteBodyText: selectionBridge.selectedNoteBodyText
    readonly property string selectedNoteBodyNoteId: selectionBridge.selectedNoteBodyNoteId
    readonly property bool selectedNoteBodyResolved: selectionBridge.selectedNoteBodyResolved
    readonly property bool selectedNoteBodyLoading: selectionBridge.selectedNoteBodyLoading
    readonly property string selectedNoteId: selectionBridge.selectedNoteId
    readonly property string selectedNoteDirectoryPath: selectionBridge.selectedNoteDirectoryPath
    readonly property bool showCurrentLineMarker: contentsView.hasSelectedNote || contentsView.editorText.length > 0 || contentsView.editorInputFocused
    readonly property bool editorSessionBoundToSelectedNote: {
        if (editorSession.editorBoundNoteId !== contentsView.selectedNoteId)
            return false;
        if (contentsView.selectedNoteDirectoryPath === "")
            return true;
        return editorSession.editorBoundNoteDirectoryPath === contentsView.selectedNoteDirectoryPath;
    }
    readonly property bool noteDocumentMountDecisionClean: noteBodyMountCoordinator.mountDecisionClean
    readonly property bool noteDocumentMountPending: noteBodyMountCoordinator.mountPending
                                                     && !contentsView.noteDocumentMountDecisionClean
    readonly property bool noteDocumentParseMounted: noteBodyMountCoordinator.parseMounted
    readonly property bool noteDocumentSourceMounted: noteBodyMountCoordinator.sourceMounted
    readonly property bool noteDocumentMounted: noteBodyMountCoordinator.noteMounted
    readonly property bool noteDocumentMountFailureVisible: noteBodyMountCoordinator.mountFailed
    readonly property string noteDocumentMountFailureReason: noteBodyMountCoordinator.mountFailureReason
    readonly property string noteDocumentMountFailureMessage: noteBodyMountCoordinator.mountFailureMessage
    readonly property bool noteDocumentSurfaceVisible: noteBodyMountCoordinator.surfaceVisible
    readonly property bool noteDocumentSurfaceInteractive: noteBodyMountCoordinator.surfaceInteractive
    readonly property string noteDocumentExceptionReason: noteBodyMountCoordinator.exceptionReason
    readonly property string noteDocumentExceptionTitle: noteBodyMountCoordinator.exceptionTitle
    readonly property string noteDocumentExceptionMessage: noteBodyMountCoordinator.exceptionMessage
    readonly property bool noteDocumentExceptionVisible: noteBodyMountCoordinator.exceptionVisible
    readonly property bool noteDocumentCommandSurfaceEnabled: noteBodyMountCoordinator.commandSurfaceEnabled
                                                             && !contentsView.showDedicatedResourceViewer
                                                             && !contentsView.showFormattedTextRenderer
    readonly property bool nativeTextInputPriority: surfacePolicy.nativeInputPriority
    readonly property bool editorCustomTextInputEnabled: false
    readonly property bool editorTagManagementInputEnabled: true
    readonly property bool contextMenuLongPressEnabled: editorInputPolicyAdapter.contextMenuLongPressEnabled
    readonly property bool noteDocumentShortcutSurfaceEnabled: editorInputPolicyAdapter.shortcutSurfaceEnabled
    readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: editorInputPolicyAdapter.tagManagementShortcutSurfaceEnabled
    readonly property bool noteDocumentContextMenuSurfaceEnabled: editorInputPolicyAdapter.contextMenuSurfaceEnabled
    readonly property string structuredFlowSourceText: contentsView.documentPresentationSourceText
    readonly property bool liveResourceStructuredFlowRequested: resourceImportController.sourceContainsCanonicalResourceTag(
                                                                    contentsView.documentPresentationSourceText)
    readonly property string activeSurfaceKind: surfacePolicy.activeSurfaceKind
    readonly property bool structuredDocumentFlowRequested: surfacePolicy.structuredDocumentSurfaceRequested
    readonly property bool resourceResolverNeedsLiveEditorSource: contentsView.showStructuredDocumentFlow
                                                                  || contentsView.liveResourceStructuredFlowRequested
    readonly property bool resourceBlocksRenderedInlineByHtmlProjection: surfacePolicy.resourceBlocksRenderedInlineByHtmlProjection
    readonly property bool programmaticEditorSurfaceSyncActive: resourceImportController.programmaticEditorSurfaceSyncActive
    readonly property bool showDedicatedResourceViewer: surfacePolicy.dedicatedResourceViewerVisible
    readonly property bool showEditorGutter: modePolicy.showEditorGutter
    readonly property bool showFormattedTextRenderer: surfacePolicy.formattedTextRendererVisible
    readonly property bool showStructuredDocumentFlow: surfacePolicy.structuredDocumentFlowVisible
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

    function activeLogicalTextSnapshot() {
        if (editorTypingController && editorTypingController.currentEditorPlainText !== undefined) {
            const livePlainText = editorTypingController.currentEditorPlainText();
            if (livePlainText !== undefined && livePlainText !== null)
                return String(livePlainText);
        }
        if (editorProjection && editorProjection.logicalText !== undefined && editorProjection.logicalText !== null)
            return String(editorProjection.logicalText);
        if (contentsView.documentPresentationSourceText !== undefined
                && contentsView.documentPresentationSourceText !== null)
            return String(contentsView.documentPresentationSourceText);
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
        const normalizedText = contentsView.normalizedMinimapSnapshotText(rawText);
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
        return contentsView.structuredHostGeometryActive
                && contentsView.effectiveStructuredLogicalLineEntries().length > 0;
    }
    function currentMinimapSnapshotEntries() {
        if (contentsView.hasStructuredLogicalLineGeometry())
            return contentsView.normalizedSnapshotEntries(contentsView.effectiveStructuredLogicalLineEntries());
        return contentsView.plainMinimapSnapshotEntries(contentsView.activeLogicalTextSnapshot());
    }
    function minimapSnapshotEntriesEqual(previousEntries, nextEntries) {
        const normalizedPrevious = contentsView.normalizedSnapshotEntries(previousEntries);
        const normalizedNext = contentsView.normalizedSnapshotEntries(nextEntries);
        if (normalizedPrevious.length !== normalizedNext.length)
            return false;
        for (let index = 0; index < normalizedPrevious.length; ++index) {
            if (contentsView.minimapSnapshotToken(normalizedPrevious[index], index)
                    !== contentsView.minimapSnapshotToken(normalizedNext[index], index)) {
                return false;
            }
        }
        return true;
    }
            function logEditorCreationState(reason) {
        presentationViewModel.logEditorCreationState(reason);
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
            lineCharacterCounts.push(viewportCoordinator.logicalLineCharacterCountAt(
                                         lineNumber - 1,
                                         contentsView.logicalLineStartOffsets));
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
        if (contentsView.hasStructuredLogicalLineGeometry())
            return contentsView.buildStructuredMinimapLineGroupsForRange(safeStartLine, safeEndLine);
        const editorWidth = 0;
        const editorContentHeight = 0;
        const lineCharacterCounts = [];
        const lineStartOffsets = [];
        const fallbackLineDocumentYs = [];
        const fallbackLineVisualHeights = [];
        const editorRects = [];
        const logicalLength = contentsView.resolvedLogicalTextLength;
        for (let lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber) {
            const lineIndex = lineNumber - 1;
            const startOffset = viewportCoordinator.logicalLineStartOffsetAt(
                        lineIndex,
                        contentsView.logicalLineStartOffsets);
            lineCharacterCounts.push(viewportCoordinator.logicalLineCharacterCountAt(
                                         lineIndex,
                                         contentsView.logicalLineStartOffsets));
            lineStartOffsets.push(startOffset);
            fallbackLineDocumentYs.push(contentsView.lineDocumentY(lineNumber));
            fallbackLineVisualHeights.push(contentsView.lineVisualHeight(lineNumber, 1));
        }
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
    function activeLineGeometryNoteId() {
        return viewportCoordinator.normalizedNoteId(contentsView.selectedNoteId);
    }
    function nextMinimapLineGroupsForCurrentState(currentSnapshotEntries) {
        const currentNoteId = contentsView.activeLineGeometryNoteId();
        const useStructuredGeometry = contentsView.hasStructuredLogicalLineGeometry();
        const structuredLineCount = useStructuredGeometry
                ? contentsView.effectiveStructuredLogicalLineEntries().length
                : 0;
        const snapshotPlan = minimapCoordinator.buildNextMinimapSnapshotPlan(
                    contentsView.minimapLineGroups,
                    contentsView.minimapLineGroupsNoteId,
                    currentNoteId,
                    contentsView.minimapSnapshotEntries,
                    currentSnapshotEntries,
                    contentsView.minimapSnapshotForceFullRefresh,
                    contentsView.hasPendingNoteEntryGutterRefresh(currentNoteId),
                    structuredLineCount,
                    contentsView.logicalLineCount);
        if (snapshotPlan.reuseExisting)
            return contentsView.minimapLineGroups;
        if (snapshotPlan.requiresFullRebuild) {
            return useStructuredGeometry
                    ? contentsView.buildStructuredMinimapLineGroupsForRange(1, Math.max(1, structuredLineCount))
                    : contentsView.buildMinimapLineGroupsForRange(1, contentsView.logicalLineCount);
        }

        const replacementGroups = useStructuredGeometry
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
        const expectedLineCount = useStructuredGeometry
                ? Math.max(1, structuredLineCount)
                : contentsView.logicalLineCount;
        if (!Array.isArray(mergedGroups) || mergedGroups.length !== expectedLineCount) {
            return useStructuredGeometry
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
        const safeOffset = 0;
        const rect = ({ });
        return minimapCoordinator.currentCursorVisualRowRectFromTextRect(
                    rect,
                    safeOffset,
                    contentsView.documentYForOffset(safeOffset));
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
        return cachedBottom;
    }
    function documentYForOffset(offset) {
        const refreshRevision = contentsView.gutterRefreshRevision;
        const safeOffset = Math.max(0, Number(offset) || 0);
        if (contentsView.structuredHostGeometryActive)
            return contentsView.lineDocumentY(viewportCoordinator.logicalLineNumberForOffset(
                                                  safeOffset,
                                                  contentsView.logicalLineStartOffsets));
        const fallbackLineNumber = viewportCoordinator.logicalLineNumberForOffset(
                    safeOffset,
                    contentsView.logicalLineStartOffsets);
        return (fallbackLineNumber - 1) * contentsView.editorLineHeight;
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
            const startOffset = viewportCoordinator.logicalLineStartOffsetAt(
                        lineIndex,
                        contentsView.logicalLineStartOffsets);
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
        return selectionMountViewModel.shouldFlushBlurredEditorState(scheduledNoteId);
    }
    function nativeEditorCompositionActive() {
        return !!(structuredDocumentFlow
                  && structuredDocumentFlow.nativeCompositionActive !== undefined
                  && structuredDocumentFlow.nativeCompositionActive());
    }
    function nativeTextInputSessionOwnsKeyboard() {
        return editorInputPolicyAdapter.nativeTextInputSessionActive;
    }
    function flushEditorStateAfterInputSettles(scheduledNoteId) {
        selectionMountViewModel.flushEditorStateAfterInputSettles(scheduledNoteId);
    }
    function focusEditorForSelectedNoteId(noteId) {
        selectionMountViewModel.focusEditorForSelectedNoteId(noteId);
    }
    function focusEditorForPendingNote() {
        selectionMountViewModel.focusEditorForPendingNote();
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
    function inlineFormatShortcutTag(event) {
        if (!event)
            return "";

        const modifiers = Number(event.modifiers) || 0;
        const metaPressed = !!(modifiers & Qt.MetaModifier);
        const controlPressed = !!(modifiers & Qt.ControlModifier);
        const altPressed = !!(modifiers & Qt.AltModifier);
        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        if (altPressed || (metaPressed && controlPressed) || (!metaPressed && !controlPressed))
            return "";

        const normalizedText = event.text === undefined || event.text === null ? "" : String(event.text).toUpperCase();
        const key = Number(event.key);
        if (!shiftPressed) {
            if (key === Qt.Key_B || normalizedText === "B")
                return "bold";
            if (key === Qt.Key_I || normalizedText === "I")
                return "italic";
            if (key === Qt.Key_U || normalizedText === "U")
                return "underline";
            return "";
        }
        if (key === Qt.Key_X || normalizedText === "X")
            return "strikethrough";
        if (key === Qt.Key_E || normalizedText === "E")
            return "highlight";
        return "";
    }
    function handleInlineFormatTagShortcut(event) {
        const tagName = contentsView.inlineFormatShortcutTag(event);
        if (tagName.length === 0)
            return false;
        const handled = contentsView.queueInlineFormatWrap(tagName);
        if (handled && event)
            event.accepted = true;
        return handled;
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
        const pasted = resourceImportController.pasteClipboardImageAsResource();
        if (pasted && event)
            event.accepted = true;
        return pasted;
    }
    function handleTagManagementShortcutKeyPress(event) {
        if (contentsView.handleClipboardImagePasteShortcut(event))
            return true;
        if (contentsView.handleInlineFormatTagShortcut(event))
            return true;
        return false;
    }
    function handleSelectionContextMenuEvent(eventName) {
        if (contentsView.handleStructuredSelectionContextMenuEvent(eventName))
            return;
        editorSelectionController.handleSelectionContextMenuEvent(eventName);
    }
    function commitDocumentPresentationRefresh() {
        presentationViewModel.commitDocumentPresentationRefresh();
    }
    function documentPresentationRenderDirty() {
        return presentationViewModel.documentPresentationRenderDirty();
    }
    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition) {
        return editorSelectionController.inferSelectionRangeFromSelectedText(selectedText, cursorPosition);
    }
    function inlineStyleWrapTags(styleTag) {
        return editorSelectionController.inlineStyleWrapTags(styleTag);
    }
                    function refreshInlineResourcePresentation() {
        presentationViewModel.refreshInlineResourcePresentation();
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
                "contentAvailableWidth": contentsView.minimapResolvedTrackWidth,
                "contentHeight": contentsView.editorLineHeight,
                "contentWidth": 0,
                "contentY": textStartY,
                "lineNumber": 1,
                "visualIndex": 0
            });
    }
    function minimapLineY(lineNumber) {
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        return viewportCoordinator.minimapTrackYForContentY(
                    contentsView.minimapContentYForLine(safeLineNumber),
                    contentsView.minimapContentHeight());
    }
    function minimapSilhouetteHeight(rowsOverride) {
        const rows = Array.isArray(rowsOverride) ? rowsOverride : (Array.isArray(contentsView.minimapVisualRows) ? contentsView.minimapVisualRows : []);
        if (rows.length === 0)
            return 1;
        const safeEditorLineHeight = Math.max(1, Number(contentsView.editorLineHeight) || 1);
        return Math.max(1, Math.ceil(contentsView.minimapContentHeight() / safeEditorLineHeight));
    }
    function minimapVisualRowPaintHeight(rowSpec) {
        const safeContentHeight = contentsView.minimapContentHeight();
        const safeRowContentHeight = Math.max(
                    1,
                    Number(rowSpec && rowSpec.contentHeight !== undefined ? rowSpec.contentHeight : 0)
                    || contentsView.editorLineHeight);
        return Math.max(
                    1,
                    viewportCoordinator.minimapTrackHeightForContentHeight(
                        safeRowContentHeight,
                        safeContentHeight));
    }
    function minimapVisualRowPaintY(rowSpec) {
        const safeContentHeight = contentsView.minimapContentHeight();
        const safeRowContentY = Math.max(
                    0,
                    Number(rowSpec && rowSpec.contentY !== undefined ? rowSpec.contentY : 0) || 0);
        return viewportCoordinator.minimapTrackYForContentY(
                    safeRowContentY,
                    safeContentHeight);
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
        contextMenuCoordinator.structuredContextMenuBlockIndex = -1;
        contextMenuCoordinator.structuredContextMenuSelectionSnapshot = ({})
    }
    function primeStructuredSelectionContextMenuSnapshot() {
        if (!contentsView.showStructuredDocumentFlow
                || !structuredDocumentFlow
                || structuredDocumentFlow.inlineFormatTargetState === undefined) {
            contentsView.resetStructuredSelectionContextMenuSnapshot();
            return false;
        }
        const targetState = structuredDocumentFlow.inlineFormatTargetState();
        const plan = contextMenuCoordinator.primeStructuredSelectionSnapshotPlan(
                    targetState && typeof targetState === "object" ? targetState : ({}));
        if (!plan.accepted) {
            contentsView.resetStructuredSelectionContextMenuSnapshot();
            return false;
        }
        contentsView.structuredContextMenuBlockIndex = Number(plan.blockIndex) || 0;
        contentsView.structuredContextMenuSelectionSnapshot = plan.selectionSnapshot && typeof plan.selectionSnapshot === "object"
                ? plan.selectionSnapshot
                : ({});
        contextMenuCoordinator.structuredContextMenuBlockIndex = contentsView.structuredContextMenuBlockIndex;
        contextMenuCoordinator.structuredContextMenuSelectionSnapshot = contentsView.structuredContextMenuSelectionSnapshot;
        return true;
    }
    function handleStructuredSelectionContextMenuEvent(eventName) {
        const inlineStyleTag = contextMenuCoordinator.inlineStyleTagForEvent(eventName === undefined || eventName === null ? "" : String(eventName));
        const plan = contextMenuCoordinator.handleStructuredSelectionEventPlan(
                    inlineStyleTag,
                    contextMenuCoordinator.structuredSelectionValid(),
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
                    contextMenuCoordinator.structuredSelectionValid(),
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
    function editorContextMenuPointerTriggerAccepted(triggerKind) {
        const normalizedTrigger = triggerKind === undefined || triggerKind === null ? "" : String(triggerKind).trim().toLowerCase();
        if (normalizedTrigger === "rightclick"
                || normalizedTrigger === "right-click"
                || normalizedTrigger === "contextmenu"
                || normalizedTrigger === "context-menu") {
            return true;
        }
        if (normalizedTrigger === "longpress"
                || normalizedTrigger === "long-press"
                || normalizedTrigger === "pressandhold"
                || normalizedTrigger === "press-and-hold") {
            return contentsView.contextMenuLongPressEnabled;
        }
        return false;
    }
    function requestEditorSelectionContextMenuFromPointer(localX, localY, triggerKind) {
        if (!contentsView.editorContextMenuPointerTriggerAccepted(triggerKind))
            return false;
        contentsView.primeEditorSelectionContextMenuSnapshot();
        Qt.callLater(function () {
            contentsView.openEditorSelectionContextMenu(localX, localY);
        });
        return true;
    }
    function primeEditorSelectionContextMenuSnapshot() {
        if (contentsView.showStructuredDocumentFlow)
            return contentsView.primeStructuredSelectionContextMenuSnapshot();
        return editorSelectionController.primeContextMenuSelectionSnapshot();
    }
    function persistEditorTextImmediately(nextText) {
        return mutationViewModel.persistEditorTextImmediately(nextText);
    }
    function scheduleEditorEntrySnapshotReconcile() {
        selectionMountViewModel.scheduleEditorEntrySnapshotReconcile();
    }
    function pollSelectedNoteSnapshot() {
        selectionMountViewModel.pollSelectedNoteSnapshot();
    }
    function reconcileEditorEntrySnapshotOnce() {
        return selectionMountViewModel.reconcileEditorEntrySnapshotOnce();
    }
    function queueStructuredInlineFormatWrap(tagName) {
        return mutationViewModel.queueStructuredInlineFormatWrap(tagName);
    }
    function queueInlineFormatWrap(tagName) {
        return mutationViewModel.queueInlineFormatWrap(tagName);
    }
    function queueAgendaShortcutInsertion() {
        return mutationViewModel.queueAgendaShortcutInsertion();
    }
    function queueCalloutShortcutInsertion() {
        return mutationViewModel.queueCalloutShortcutInsertion();
    }
    function queueBreakShortcutInsertion() {
        return mutationViewModel.queueBreakShortcutInsertion();
    }
    function requestStructuredDocumentEndEdit() {
        return mutationViewModel.requestStructuredDocumentEndEdit();
    }
    function focusStructuredBlockSourceOffset(sourceOffset) {
        mutationViewModel.focusStructuredBlockSourceOffset(sourceOffset);
    }
    function applyDocumentSourceMutation(nextSourceText, focusRequest) {
        return mutationViewModel.applyDocumentSourceMutation(nextSourceText, focusRequest);
    }
    function setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked) {
        return mutationViewModel.setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked);
    }
    function refreshMinimapSnapshot() {
        geometryViewModel.refreshMinimapSnapshot();
    }
    function refreshMinimapCursorTracking(rowsOverride) {
        geometryViewModel.refreshMinimapCursorTracking(rowsOverride);
    }
    function refreshMinimapViewportTracking(trackHeightOverride) {
        geometryViewModel.refreshMinimapViewportTracking(trackHeightOverride);
    }
    function requestViewHook(reason) {
        presentationViewModel.requestViewHook(reason);
    }
    function resetNoteEntryLineGeometryState() {
        geometryViewModel.resetNoteEntryLineGeometryState();
    }
    function resetGutterRefreshState() {
        geometryViewModel.resetGutterRefreshState();
    }
    function resetEditorSelectionCache() {
        selectionMountViewModel.resetEditorSelectionCache();
    }
    function resolveEditorFlickable() {
        if (contentsView.showStructuredDocumentFlow) {
            if (contentsView.showPrintEditorLayout)
                return printDocumentViewport;
            return structuredDocumentViewport;
        }
        return null;
    }
    function scheduleEditorFocusForNote(noteId) {
        selectionMountViewModel.scheduleEditorFocusForNote(noteId);
    }
    function applyPresentationRefreshPlan(plan) {
        presentationViewModel.applyPresentationRefreshPlan(plan);
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
        presentationViewModel.scheduleDeferredDocumentPresentationRefresh();
    }
    function scheduleDocumentPresentationRefresh(forceImmediate) {
        presentationViewModel.scheduleDocumentPresentationRefresh(forceImmediate);
    }
    function refreshLiveLogicalLineMetrics() {
        return geometryViewModel.refreshLiveLogicalLineMetrics();
    }
    function activeLogicalLineCountSnapshot() {
        return geometryViewModel.activeLogicalLineCountSnapshot();
    }
    function shouldScheduleGutterRefreshForReason(reason) {
        return refreshCoordinator.shouldScheduleGutterRefreshForReason(
                    reason === undefined || reason === null ? "" : String(reason),
                    contentsView.activeLogicalLineCountSnapshot());
    }
    function scheduleGutterRefresh(passCount, reason) {
        geometryViewModel.scheduleGutterRefresh(passCount, reason);
    }
    function scheduleNoteEntryGutterRefresh(noteId) {
        geometryViewModel.scheduleNoteEntryGutterRefresh(noteId);
    }
    function scheduleCursorDrivenUiRefresh() {
        geometryViewModel.scheduleCursorDrivenUiRefresh();
    }
    function scheduleViewportGutterRefresh() {
        geometryViewModel.scheduleViewportGutterRefresh();
    }
    function scheduleMinimapSnapshotRefresh(forceFull) {
        geometryViewModel.scheduleMinimapSnapshotRefresh(forceFull);
    }
    function scheduleSelectionModelSync(options) {
        selectionMountViewModel.scheduleSelectionModelSync(options);
    }
    function executeSelectionDeliveryPlan(plan, fallbackKey) {
        return selectionMountViewModel.executeSelectionDeliveryPlan(plan, fallbackKey);
    }
    function scrollEditorViewportToMinimapPosition(localY) {
        geometryViewModel.scrollEditorViewportToMinimapPosition(localY);
    }
    function correctTypingViewport(forceAnchor) {
        geometryViewModel.correctTypingViewport(forceAnchor);
    }
    function scheduleTypingViewportCorrection(forceAnchor) {
        geometryViewModel.scheduleTypingViewportCorrection(forceAnchor);
    }
    function selectedEditorRange() {
        return mutationViewModel.selectedEditorRange();
    }
    function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {
        return mutationViewModel.wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange);
    }

    clip: true

    Component.onCompleted: {
        EditorTrace.trace("displayView", "mount", "visible=" + contentsView.visible, contentsView)
        contentsView.logEditorCreationState("componentCompleted");
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
        contentsView.logEditorCreationState("showStructuredDocumentFlowChanged");
        if (contentsView.showStructuredDocumentFlow) {
            presentationRefreshController.clearPendingWhileFocused();
            eventPump.stopDocumentPresentationRefreshTimer();
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
    onDocumentPresentationSourceTextChanged: {
        contentsView.refreshLiveLogicalLineMetrics();
        contentsView.minimapSnapshotForceFullRefresh = true;
        contentsView.scheduleMinimapSnapshotRefresh(true);
    }
    onSelectedNoteBodyTextChanged: {
        EditorTrace.trace(
                    "displayView",
                    "selectedNoteBodyTextChanged",
                    "selectedNoteId=" + contentsView.selectedNoteId
                    + " bodyNoteId=" + contentsView.selectedNoteBodyNoteId
                    + " resolved=" + contentsView.selectedNoteBodyResolved
                    + " loading=" + contentsView.selectedNoteBodyLoading
                    + " " + EditorTrace.describeText(contentsView.selectedNoteBodyText),
                    contentsView)
        if (contentsView.selectedNoteBodyLoading)
            return;
        if ((contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null || String(contentsView.selectedNoteBodyText).length === 0)
                && !contentsView.selectedNoteBodyResolved)
            return;
        if (contentsView.selectedNoteBodyNoteId !== "" && contentsView.selectedNoteBodyNoteId !== contentsView.selectedNoteId)
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
    onSelectedNoteBodyResolvedChanged: {
        EditorTrace.trace(
                    "displayView",
                    "selectedNoteBodyResolvedChanged",
                    "selectedNoteId=" + contentsView.selectedNoteId
                    + " bodyNoteId=" + contentsView.selectedNoteBodyNoteId
                    + " resolved=" + contentsView.selectedNoteBodyResolved
                    + " loading=" + contentsView.selectedNoteBodyLoading,
                    contentsView)
        if (contentsView.selectedNoteBodyLoading)
            return;
        if ((contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null || String(contentsView.selectedNoteBodyText).length === 0)
                && !contentsView.selectedNoteBodyResolved)
            return;
        if (contentsView.selectedNoteBodyNoteId !== "" && contentsView.selectedNoteBodyNoteId !== contentsView.selectedNoteId)
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
                    + " resolved=" + contentsView.selectedNoteBodyResolved
                    + " loading=" + contentsView.selectedNoteBodyLoading,
                    contentsView)
        if (contentsView.selectedNoteBodyLoading)
            return;
        if (contentsView.selectedNoteId.length === 0)
            return;
        if (contentsView.selectedNoteBodyNoteId !== "" && contentsView.selectedNoteBodyNoteId !== contentsView.selectedNoteId)
            return;
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
    onSelectedNoteIdChanged: {
        EditorTrace.trace(
                    "displayView",
                    "selectedNoteIdChanged",
                    "selectedNoteId=" + contentsView.selectedNoteId,
                    contentsView)
        contentsView.logEditorCreationState("selectedNoteIdChanged");
        contentsView.scheduleNoteEntryGutterRefresh(contentsView.selectedNoteId);
        contentsView.scheduleSelectionModelSync({
                                                   "resetSnapshot": true,
                                                   "focusEditor": true
                                               });
    }
    onSelectedNoteBodyNoteIdChanged: {
        EditorTrace.trace(
                    "displayView",
                    "selectedNoteBodyNoteIdChanged",
                    "selectedNoteId=" + contentsView.selectedNoteId
                    + " bodyNoteId=" + contentsView.selectedNoteBodyNoteId,
                    contentsView)
        contentsView.logEditorCreationState("selectedNoteBodyNoteIdChanged");
    }
    onEditorBoundNoteIdChanged: {
        EditorTrace.trace(
                    "displayView",
                    "editorBoundNoteIdChanged",
                    "editorBoundNoteId=" + contentsView.editorBoundNoteId
                    + " selectedNoteId=" + contentsView.selectedNoteId,
                    contentsView)
        contentsView.logEditorCreationState("editorBoundNoteIdChanged");
    }
    onEditorSessionBoundToSelectedNoteChanged: {
        EditorTrace.trace(
                    "displayView",
                    "editorSessionBoundToSelectedNoteChanged",
                    "editorSessionBoundToSelectedNote=" + contentsView.editorSessionBoundToSelectedNote
                    + " editorBoundNoteId=" + contentsView.editorBoundNoteId
                    + " selectedNoteId=" + contentsView.selectedNoteId,
                    contentsView)
        contentsView.logEditorCreationState("editorSessionBoundToSelectedNoteChanged");
    }
    onStructuredDocumentFlowRequestedChanged: {
        EditorTrace.trace(
                    "displayView",
                    "structuredDocumentFlowRequestedChanged",
                    "structuredDocumentFlowRequested=" + contentsView.structuredDocumentFlowRequested,
                    contentsView)
        contentsView.logEditorCreationState("structuredDocumentFlowRequestedChanged");
    }
    onNoteDocumentMountPendingChanged: {
        EditorTrace.trace(
                    "displayView",
                    "noteDocumentMountPendingChanged",
                    "noteDocumentMountPending=" + contentsView.noteDocumentMountPending,
                    contentsView)
        contentsView.logEditorCreationState("noteDocumentMountPendingChanged");
    }
    onNoteDocumentMountedChanged: {
        EditorTrace.trace(
                    "displayView",
                    "noteDocumentMountedChanged",
                    "noteDocumentMounted=" + contentsView.noteDocumentMounted,
                    contentsView)
        contentsView.logEditorCreationState("noteDocumentMountedChanged");
    }
    onNoteDocumentMountFailureReasonChanged: {
        EditorTrace.trace(
                    "displayView",
                    "noteDocumentMountFailureReasonChanged",
                    "noteDocumentMountFailureReason=" + contentsView.noteDocumentMountFailureReason,
                    contentsView)
        contentsView.logEditorCreationState("noteDocumentMountFailureReasonChanged");
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

    ContentsDisplaySurfacePolicy {
        id: surfacePolicy

        hasSelectedNote: contentsView.hasSelectedNote
    }
    EditorDisplayModel.ContentsDisplayHostModePolicy {
        id: modePolicy

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
        editorContentHeight: structuredDocumentFlow ? Number(structuredDocumentFlow.implicitHeight) || 0 : 0
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
    EditorInputModel.ContentsEditorSelectionController {
        id: editorSelectionController

        contentEditor: contentsView.contentEditor
        editorSession: editorSession
        editorViewport: editorViewport
        selectionBridge: selectionBridge
        selectionContextMenu: contentsView.inputCommandSurface ? contentsView.inputCommandSurface.selectionContextMenu : null
        textFormatRenderer: editorProjection
        textMetricsBridge: editorProjection
        view: contentsView
    }
    ContentsEditorSelectionBridge {
        id: selectionBridge
        objectName: "contentsDisplaySelectionBridge"

        contentViewModel: contentsView.contentViewModel
        noteListModel: contentsView.noteListModel

        Component.onCompleted: {
            EditorTrace.trace(
                        "displayView",
                        "selectionBridgeCreated",
                        EditorTrace.describeObject(selectionBridge, [
                                                      "selectedNoteId",
                                                      "selectedNoteBodyNoteId",
                                                      "selectedNoteBodyResolved",
                                                      "selectedNoteBodyLoading"
                                                  ]),
                        selectionBridge)
        }
        Component.onDestruction: {
            EditorTrace.trace(
                        "displayView",
                        "selectionBridgeDestroyed",
                        "selectedNoteId=" + selectionBridge.selectedNoteId,
                        selectionBridge)
        }
    }
    ContentsDisplayTraceFormatter {
        id: traceFormatter
    }

    EditorInputModel.ContentsEditorInputPolicyAdapter {
        id: editorInputPolicyAdapter
        objectName: "contentsDisplayEditorInputPolicyAdapter"

        editorCompositionActive: contentsView.nativeEditorCompositionActive()
        editorInputFocused: contentsView.editorInputFocused
        editorTagManagementInputEnabled: contentsView.editorTagManagementInputEnabled
        nativeTextInputPriority: contentsView.nativeTextInputPriority
        noteDocumentCommandSurfaceEnabled: contentsView.noteDocumentCommandSurfaceEnabled
        structuredCompositionActive: structuredDocumentFlow
                                     && structuredDocumentFlow.nativeCompositionActive !== undefined
                                     && structuredDocumentFlow.nativeCompositionActive()
    }

    ContentsDisplayEditOperationCoordinator {
        id: editOperationCoordinator
    }
    ContentsDisplayDocumentSourceResolver {
        id: documentSourceResolver

        editorBoundNoteId: contentsView.editorBoundNoteId === undefined || contentsView.editorBoundNoteId === null ? "" : String(contentsView.editorBoundNoteId)
        editorBoundNoteDirectoryPath: contentsView.editorBoundNoteDirectoryPath === undefined || contentsView.editorBoundNoteDirectoryPath === null ? "" : String(contentsView.editorBoundNoteDirectoryPath)
        editorText: contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText)
        pendingBodySave: contentsView.pendingBodySave
        selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId === undefined || contentsView.selectedNoteBodyNoteId === null ? "" : String(contentsView.selectedNoteBodyNoteId)
        selectedNoteBodyResolved: contentsView.selectedNoteBodyResolved
        selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? "" : String(contentsView.selectedNoteBodyText)
        selectedNoteDirectoryPath: contentsView.selectedNoteDirectoryPath === undefined || contentsView.selectedNoteDirectoryPath === null ? "" : String(contentsView.selectedNoteDirectoryPath)
        selectedNoteId: contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null ? "" : String(contentsView.selectedNoteId)
        structuredFlowSourceText: contentsView.structuredFlowSourceText === undefined || contentsView.structuredFlowSourceText === null ? "" : String(contentsView.structuredFlowSourceText)
    }
    ContentsDisplayNoteBodyMountCoordinator {
        id: noteBodyMountCoordinator
        objectName: "contentsDisplayNoteBodyMountCoordinator"

        editorBoundNoteId: contentsView.editorBoundNoteId === undefined || contentsView.editorBoundNoteId === null ? "" : String(contentsView.editorBoundNoteId)
        editorSessionBoundToSelectedNote: contentsView.editorSessionBoundToSelectedNote
        editorText: contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText)
        inlineDocumentSurfaceLoading: surfacePolicy.inlineDocumentSurfaceLoading
        inlineDocumentSurfaceReady: surfacePolicy.inlineDocumentSurfaceReady
        inlineDocumentSurfaceRequested: surfacePolicy.inlineDocumentSurfaceRequested
        pendingBodySave: contentsView.pendingBodySave
        selectedNoteBodyLoading: contentsView.selectedNoteBodyLoading
        selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId === undefined || contentsView.selectedNoteBodyNoteId === null ? "" : String(contentsView.selectedNoteBodyNoteId)
        selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? "" : String(contentsView.selectedNoteBodyText)
        selectedNoteBodyResolved: contentsView.selectedNoteBodyResolved
        selectedNoteId: contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null ? "" : String(contentsView.selectedNoteId)
        structuredDocumentSurfaceReady: contentsView.structuredDocumentFlowRequested
                                        && structuredDocumentFlow
                                        && structuredDocumentFlow.visible
        structuredDocumentSurfaceRequested: contentsView.structuredDocumentFlowRequested
        visible: contentsView.visible

        Component.onCompleted: {
            EditorTrace.trace(
                        "displayView",
                        "noteBodyMountCoordinatorCreated",
                        EditorTrace.describeObject(noteBodyMountCoordinator, [
                                                      "mountPending",
                                                      "mountDecisionClean",
                                                      "noteMounted",
                                                      "mountFailed",
                                                      "mountFailureReason"
                                                  ]),
                        noteBodyMountCoordinator)
        }
        Component.onDestruction: {
            EditorTrace.trace(
                        "displayView",
                        "noteBodyMountCoordinatorDestroyed",
                        "mountFailureReason=" + noteBodyMountCoordinator.mountFailureReason,
                        noteBodyMountCoordinator)
        }
    }
    ContentsDisplaySelectionSyncCoordinator {
        id: selectionSyncCoordinator

        editorBoundNoteId: contentsView.editorBoundNoteId
        editorInputFocused: contentsView.editorInputFocused
        editorSessionBoundToSelectedNote: contentsView.editorSessionBoundToSelectedNote
        pendingBodySave: contentsView.pendingBodySave
        selectedNoteBodyLoading: contentsView.selectedNoteBodyLoading
        selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId
        selectedNoteBodyResolved: contentsView.selectedNoteBodyResolved
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
        currentCursorOffset: 0
        editorContentOffsetY: Number(contentsView.editorContentOffsetY) || 0
        editorDocumentStartY: Number(contentsView.editorDocumentStartY) || 0
        editorInputFocused: contentsView.editorInputFocused
        editorLineHeight: Number(contentsView.editorLineHeight) || 0
        editorSurfaceHeight: Number(contentsView.editorSurfaceHeight) || 0
        editorViewportHeight: Number(contentsView.editorViewportHeight) || 0
        logicalLineCount: Math.max(1, Number(contentsView.logicalLineCount) || 1)
        logicalTextLength: Number(contentsView.liveLogicalTextLength) || 0
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
        structuredHostGeometryActive: contentsView.hasStructuredLogicalLineGeometry()
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
    EditorInputModel.ContentsEditorTypingController {
        id: editorTypingController

        agendaBackend: contentsAgendaBackend
        bodyTagInsertionPlanner: contentsBodyTagInsertionPlanner
        calloutBackend: contentsCalloutBackend
        contentEditor: contentsView.contentEditor
        editorSession: editorSession
        textFormatRenderer: editorProjection
        textMetricsBridge: editorProjection
        view: contentsView
    }
    EditorResourceModel.ContentsResourceImportController {
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
    ContentsAgendaBackend {
        id: contentsAgendaBackend
    }
    ContentsCalloutBackend {
        id: contentsCalloutBackend
    }
    ContentsEditorBodyTagInsertionPlanner {
        id: contentsBodyTagInsertionPlanner

        agendaBackend: contentsAgendaBackend
        calloutBackend: contentsCalloutBackend
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
    ContentsGutterMarkerBridge {
        id: gutterMarkerBridge

        gutterMarkers: contentsView.gutterMarkers
    }
    ContentsEditorSessionController {
        id: editorSession
        objectName: "contentsDisplayEditorSession"

        agendaBackend: contentsAgendaBackend
        selectionBridge: selectionBridge
        typingIdleThresholdMs: contentsView.editorIdleSyncThresholdMs

        Component.onCompleted: {
            EditorTrace.trace(
                        "displayView",
                        "editorSessionCreated",
                        EditorTrace.describeObject(editorSession, [
                                                      "editorBoundNoteId",
                                                      "pendingBodySave",
                                                      "syncingEditorTextFromModel"
                                                  ]),
                        editorSession)
        }
        Component.onDestruction: {
            EditorTrace.trace(
                        "displayView",
                        "editorSessionDestroyed",
                        "editorBoundNoteId=" + editorSession.editorBoundNoteId
                        + " pendingBodySave=" + editorSession.pendingBodySave,
                        editorSession)
        }
    }
    EditorDisplayModel.ContentsDisplayEventPump {
        id: eventPump

        bodyResourceRenderer: bodyResourceRenderer
        contentsView: contentsView
        editorProjection: editorProjection
        editorSession: editorSession
        editorTypingController: editorTypingController
        minimapLayer: minimapLayer
        noteBodyMountCoordinator: noteBodyMountCoordinator
        presentationRefreshController: presentationRefreshController
        resourceImportController: resourceImportController
        selectionBridge: selectionBridge
        selectionSyncCoordinator: selectionSyncCoordinator
        structuredBlockRenderer: structuredBlockRenderer
        structuredDocumentFlow: structuredDocumentFlow
        traceFormatter: traceFormatter
    }
    EditorDisplayModel.ContentsDisplayPresentationController {
        id: presentationController

        contentsView: contentsView
        documentSourceResolver: documentSourceResolver
        editorProjection: editorProjection
        editorSession: editorSession
        editorTypingController: editorTypingController
        eventPump: eventPump
        minimapLayer: minimapLayer
        noteBodyMountCoordinator: noteBodyMountCoordinator
        panelViewModel: contentsView.panelViewModel
        presentationRefreshController: presentationRefreshController
        resourceImportController: resourceImportController
        selectionBridge: selectionBridge
        structuredDocumentFlow: structuredDocumentFlow
        traceFormatter: traceFormatter
    }
    ContentsDisplayPresentationViewModel {
        id: presentationViewModel

        controller: presentationController
    }
    EditorDisplayModel.ContentsDisplayMutationController {
        id: mutationController

        contentEditor: contentsView.contentEditor
        contentsAgendaBackend: contentsAgendaBackend
        contentsView: contentsView
        editOperationCoordinator: editOperationCoordinator
        editorInputPolicyAdapter: editorInputPolicyAdapter
        editorSelectionController: editorSelectionController
        editorSession: editorSession
        editorTypingController: editorTypingController
        eventPump: eventPump
        presentationRefreshController: presentationRefreshController
        resourceImportController: resourceImportController
        structuredDocumentFlow: structuredDocumentFlow
    }
    ContentsDisplayMutationViewModel {
        id: mutationViewModel

        controller: mutationController
    }
    ContentsActiveEditorSurfaceAdapter {
        id: activeEditorSurfaceAdapter

        contentEditor: null
        inlineSurfaceActive: false
        structuredDocumentFlow: structuredDocumentFlow
        structuredSurfaceActive: contentsView.showStructuredDocumentFlow
    }
    EditorDisplayModel.ContentsDisplaySelectionMountController {
        id: selectionMountController

        activeEditorSurface: activeEditorSurfaceAdapter
        contentsView: contentsView
        editorSelectionController: editorSelectionController
        editorSession: editorSession
        editorTypingController: editorTypingController
        noteBodyMountCoordinator: noteBodyMountCoordinator
        selectionBridge: selectionBridge
        selectionSyncCoordinator: selectionSyncCoordinator
        traceFormatter: traceFormatter
    }
    ContentsDisplaySelectionMountViewModel {
        id: selectionMountViewModel

        controller: selectionMountController
    }
    EditorDisplayModel.ContentsDisplayGeometryController {
        id: geometryController

        contentsView: contentsView
        eventPump: eventPump
        minimapLayer: minimapLayer
        refreshCoordinator: refreshCoordinator
        structuredDocumentFlow: structuredDocumentFlow
        viewportCoordinator: viewportCoordinator
    }
    ContentsDisplayGeometryViewModel {
        id: geometryViewModel

        controller: geometryController
    }
    Rectangle {
        id: contentsDisplayView

        anchors.fill: parent
        color: contentsView.displayColor

        ContentsDisplayAuxiliaryRailHost {
            id: auxiliaryRailHost

            anchors.fill: parent
            bodyResourceRenderer: bodyResourceRenderer
            contentsAgendaBackend: contentsAgendaBackend
            contentsCalloutBackend: contentsCalloutBackend
            contentsView: contentsView
            editorProjection: editorProjection
            editorTypingController: editorTypingController
            resourceImportController: resourceImportController
            structuredBlockRenderer: structuredBlockRenderer
            viewportCoordinator: viewportCoordinator
        }

        ContentsDisplayOverlayHost {
            anchors.fill: parent
            contentsView: contentsView
            resourceImportController: resourceImportController
        }
    }
}
