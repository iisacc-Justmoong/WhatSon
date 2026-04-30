pragma ComponentBehavior: Bound
import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "../../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace
import "../../../../models/editor/display/ContentsMinimapSnapshotSupport.js" as MinimapSnapshotSupport
import "../../../../models/editor/format/ContentsRawInlineStyleMutationSupport.js" as RawInlineStyleMutationSupport
import "../../../../models/editor/tags/ContentsRawBodyTagMutationSupport.js" as RawTagMutationSupport

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
    readonly property bool contentPersistenceContractAvailable: mountState.contentPersistenceContractAvailable
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
    property alias gutterMarkers: geometryState.gutterMarkers
    property alias gutterRefreshPassesRemaining: geometryState.gutterRefreshPassesRemaining
    property alias gutterRefreshRevision: geometryState.gutterRefreshRevision
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
    property alias liveLogicalLineCount: geometryState.liveLogicalLineCount
    readonly property int logicalLineCount: Math.max(1, Number(geometryState.liveLogicalLineCount) || 1)
    property alias logicalLineDocumentYCache: geometryState.logicalLineDocumentYCache
    property alias logicalLineDocumentYCacheLineCount: geometryState.logicalLineDocumentYCacheLineCount
    property alias logicalLineDocumentYCacheRevision: geometryState.logicalLineDocumentYCacheRevision
    property alias logicalLineGutterDocumentYCache: geometryState.logicalLineGutterDocumentYCache
    property alias logicalLineGutterDocumentYCacheLineCount: geometryState.logicalLineGutterDocumentYCacheLineCount
    property alias logicalLineGutterDocumentYCacheRevision: geometryState.logicalLineGutterDocumentYCacheRevision
    property alias structuredGutterGeometrySignature: geometryState.structuredGutterGeometrySignature
    property alias liveLogicalLineStartOffsets: geometryState.liveLogicalLineStartOffsets
    readonly property var logicalLineStartOffsets: geometryState.liveLogicalLineStartOffsets
    property alias liveLogicalTextLength: geometryState.liveLogicalTextLength
    readonly property int resolvedLogicalTextLength: Math.max(0, Number(viewportCoordinator.logicalTextLength) || 0)
    readonly property bool lineGeometryRefreshEnabled: modePolicy.lineGeometryRefreshEnabled
    readonly property int minEditorHeight: LV.Theme.gap20 * 12
    readonly property real minimapAvailableTrackHeight: Math.max(1, contentsView.editorViewportHeight - contentsView.minimapTrackInset * 2)
    readonly property color minimapCurrentLineColor: contentsView.activeLineNumberColor
    readonly property color minimapLineColor: contentsView.lineNumberColor
    readonly property int minimapOuterWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(56)))
    property alias minimapResolvedCurrentLineHeight: geometryState.minimapResolvedCurrentLineHeight
    property alias minimapResolvedCurrentLineWidth: geometryState.minimapResolvedCurrentLineWidth
    property alias minimapResolvedCurrentLineY: geometryState.minimapResolvedCurrentLineY
    property alias minimapResolvedSilhouetteHeight: geometryState.minimapResolvedSilhouetteHeight
    property alias minimapResolvedTrackHeight: geometryState.minimapResolvedTrackHeight
    readonly property real minimapResolvedTrackWidth: contentsView.minimapTrackWidth
    property alias minimapResolvedViewportHeight: geometryState.minimapResolvedViewportHeight
    property alias minimapResolvedViewportY: geometryState.minimapResolvedViewportY
    readonly property int minimapRowGap: Math.max(1, Math.round(LV.Theme.scaleMetric(1)))
    property alias minimapLineGroups: geometryState.minimapLineGroups
    property alias minimapLineGroupsNoteId: geometryState.minimapLineGroupsNoteId
    property alias minimapScrollable: geometryState.minimapScrollable
    property alias cursorDrivenUiRefreshQueued: geometryState.cursorDrivenUiRefreshQueued
    property alias typingViewportCorrectionQueued: geometryState.typingViewportCorrectionQueued
    property alias typingViewportForceCorrectionRequested: geometryState.typingViewportForceCorrectionRequested
    property alias viewportGutterRefreshQueued: geometryState.viewportGutterRefreshQueued
    property alias minimapSnapshotRefreshQueued: geometryState.minimapSnapshotRefreshQueued
    readonly property int minimapTrackInset: LV.Theme.gap8
    readonly property int minimapTrackWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(36)))
    readonly property color minimapViewportFillColor: LV.Theme.accentTransparent
    readonly property int minimapViewportMinHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(28)))
    property alias minimapVisible: geometryState.minimapVisible
    readonly property bool showMinimapRail: modePolicy.showMinimapRail
    readonly property bool minimapRefreshEnabled: modePolicy.minimapRefreshEnabled
    property alias minimapVisualRows: geometryState.minimapVisualRows
    readonly property var normalizedExternalGutterMarkers: gutterMarkerBridge.normalizedExternalGutterMarkers
    readonly property bool noteCountContractAvailable: mountState.noteCountContractAvailable
    property var noteListModel: null
    readonly property bool noteSelectionContractAvailable: mountState.noteSelectionContractAvailable
    readonly property bool typingSessionSyncProtected: editorSession && editorSession.isTypingSessionActive !== undefined && editorSession.isTypingSessionActive()
    readonly property bool noteSnapshotRefreshEnabled: contentsView.visible && contentsView.hasSelectedNote && !contentsView.selectedNoteBodyLoading && (contentsView.editorSessionBoundToSelectedNote || contentsView.selectedNoteBodyResolved) && !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer && contentsView.noteSelectionContractAvailable && !contentsView.editorInputFocused && !contentsView.typingSessionSyncProtected && !contentsView.pendingBodySave
    readonly property int noteSnapshotRefreshIntervalMs: 1200
    readonly property int pageEditorViewModeValue: pagePrintLayoutRenderer.pageViewModeValue
    property var panelViewModel: null
    property alias pendingBodySave: editorSession.pendingBodySave
    readonly property string editorEntrySnapshotComparedNoteId: selectionSyncCoordinator.comparedSnapshotNoteId
    readonly property string editorEntrySnapshotPendingNoteId: selectionSyncCoordinator.pendingSnapshotNoteId
    property alias pendingNoteEntryGutterRefreshNoteId: geometryState.pendingNoteEntryGutterRefreshNoteId
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
    readonly property int documentPresentationRefreshIntervalMs: presentationState.documentPresentationRefreshIntervalMs
    readonly property string documentPresentationSourceText: documentSourceResolver.documentPresentationSourceText
    readonly property bool documentPresentationRefreshPendingWhileFocused: presentationRefreshController.pendingWhileFocused
    property alias renderedEditorHtml: presentationState.renderedEditorHtml
    readonly property var resolvedEditorViewModeViewModel: {
        if (contentsView.editorViewModeViewModel)
            return contentsView.editorViewModeViewModel;
        if (LV.ViewModels && LV.ViewModels.get !== undefined)
            return LV.ViewModels.get("editorViewModeViewModel");
        return null;
    }
    property alias resourceDropActive: resourceUiState.resourceDropActive
    readonly property bool resourceDropEditorSurfaceGuardActive: resourceImportController.resourceDropEditorSurfaceGuardActive
    readonly property int resourceImportConflictPolicyAbort: resourceUiState.resourceImportConflictPolicyAbort
    readonly property int resourceImportConflictPolicyOverwrite: resourceUiState.resourceImportConflictPolicyOverwrite
    readonly property int resourceImportConflictPolicyKeepBoth: resourceUiState.resourceImportConflictPolicyKeepBoth
    readonly property int resourceImportModeNone: resourceUiState.resourceImportModeNone
    readonly property int resourceImportModeUrls: resourceUiState.resourceImportModeUrls
    readonly property int resourceImportModeClipboard: resourceUiState.resourceImportModeClipboard
    readonly property bool resourceImportConflictAlertOpen: resourceImportController.resourceImportConflictAlertOpen
    readonly property color resourceRenderBorderColor: resourceUiState.resourceRenderBorderColor
    readonly property color resourceRenderCardColor: resourceUiState.resourceRenderCardColor
    readonly property int resourceRenderDisplayLimit: resourceUiState.resourceRenderDisplayLimit
    property var resourcesImportViewModel: null
    property var sidebarHierarchyViewModel: null
    readonly property bool preferNativeInputHandling: inputState.preferNativeInputHandling
    readonly property bool documentPresentationProjectionEnabled: inputState.documentPresentationProjectionEnabled
    readonly property bool inlineHtmlImageRenderingEnabled: inputState.inlineHtmlImageRenderingEnabled
    readonly property int resourceEditorPlaceholderLineCount: resourceUiState.resourceEditorPlaceholderLineCount
    readonly property int editorIdleSyncThresholdMs: 1000
    readonly property string selectedNoteBodyText: mountState.selectedNoteBodyText
    readonly property string selectedNoteBodyNoteId: mountState.selectedNoteBodyNoteId
    readonly property bool selectedNoteBodyResolved: mountState.selectedNoteBodyResolved
    readonly property bool selectedNoteBodyLoading: mountState.selectedNoteBodyLoading
    readonly property string selectedNoteId: mountState.selectedNoteId
    readonly property string selectedNoteDirectoryPath: mountState.selectedNoteDirectoryPath
    readonly property bool editorSessionBoundToSelectedNote: mountState.editorSessionBoundToSelectedNote
    readonly property bool noteDocumentMountDecisionClean: mountState.noteDocumentMountDecisionClean
    readonly property bool noteDocumentMountPending: mountState.noteDocumentMountPending
    readonly property bool noteDocumentParseMounted: mountState.noteDocumentParseMounted
    readonly property bool noteDocumentSourceMounted: mountState.noteDocumentSourceMounted
    readonly property bool noteDocumentMounted: mountState.noteDocumentMounted
    readonly property bool noteDocumentMountFailureVisible: mountState.noteDocumentMountFailureVisible
    readonly property string noteDocumentMountFailureReason: mountState.noteDocumentMountFailureReason
    readonly property string noteDocumentMountFailureMessage: mountState.noteDocumentMountFailureMessage
    readonly property string noteDocumentExceptionReason: mountState.noteDocumentExceptionReason
    readonly property string noteDocumentExceptionTitle: mountState.noteDocumentExceptionTitle
    readonly property string noteDocumentExceptionMessage: mountState.noteDocumentExceptionMessage
    readonly property bool noteDocumentExceptionVisible: mountState.noteDocumentExceptionVisible
    readonly property bool nativeTextInputPriority: inputState.nativeTextInputPriority
    readonly property bool editorCustomTextInputEnabled: inputState.editorCustomTextInputEnabled
    readonly property bool editorTagManagementInputEnabled: inputState.editorTagManagementInputEnabled
    readonly property bool contextMenuLongPressEnabled: inputState.contextMenuLongPressEnabled
    readonly property bool noteDocumentShortcutSurfaceEnabled: inputState.noteDocumentShortcutSurfaceEnabled
    readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: inputState.noteDocumentTagManagementShortcutSurfaceEnabled
    readonly property bool noteDocumentContextMenuSurfaceEnabled: inputState.noteDocumentContextMenuSurfaceEnabled
    readonly property string structuredFlowSourceText: contentsView.documentPresentationSourceText
    readonly property bool liveResourceStructuredFlowRequested: resourceImportController.sourceContainsCanonicalResourceTag(
                                                                    contentsView.documentPresentationSourceText)
    readonly property string activeSurfaceKind: inputState.activeSurfaceKind
    readonly property bool structuredDocumentFlowRequested: inputState.structuredDocumentFlowRequested
    readonly property bool resourceResolverNeedsLiveEditorSource: contentsView.showStructuredDocumentFlow
                                                                  || contentsView.liveResourceStructuredFlowRequested
    readonly property bool resourceBlocksRenderedInlineByHtmlProjection: inputState.resourceBlocksRenderedInlineByHtmlProjection
    readonly property bool programmaticEditorSurfaceSyncActive: resourceImportController.programmaticEditorSurfaceSyncActive
    readonly property bool showDedicatedResourceViewer: inputState.showDedicatedResourceViewer
    readonly property bool showEditorGutter: modePolicy.showEditorGutter
    readonly property bool showFormattedTextRenderer: inputState.showFormattedTextRenderer
    readonly property bool showStructuredDocumentFlow: inputState.showStructuredDocumentFlow
    readonly property bool structuredBlockBackgroundRefreshEnabled: contentsView.showStructuredDocumentFlow
                                                                 || contentsView.resourceBlocksRenderedInlineByHtmlProjection
    readonly property bool structuredHostGeometryActive: modePolicy.structuredHostGeometryActive
    readonly property bool showPageEditorLayout: pagePrintLayoutRenderer.showPageEditorLayout
    readonly property bool showPrintEditorLayout: pagePrintLayoutRenderer.showPrintEditorLayout
    readonly property bool showPrintMarginGuides: pagePrintLayoutRenderer.showPrintMarginGuides
    readonly property bool showPrintModeActive: pagePrintLayoutRenderer.showPrintModeActive
    property alias syncingEditorTextFromModel: editorSession.syncingEditorTextFromModel
    readonly property real textOriginY: {
        return contentsView.editorDocumentStartY + contentsView.editorContentOffsetY;
    }
    property alias visibleGutterLineEntries: geometryState.visibleGutterLineEntries
    readonly property int visibleNoteCount: mountState.visibleNoteCount

    signal editorTextEdited(string text)
    signal viewHookRequested

    ContentsDisplayGeometryState {
        id: geometryState
    }
    ContentsDisplayPresentationState {
        id: presentationState
    }
    ContentsDisplayResourceUiState {
        id: resourceUiState
    }
    ContentsDisplayMountState {
        id: mountState

        editorSession: editorSession
        noteBodyMountCoordinator: noteBodyMountCoordinator
        selectionBridge: selectionBridge
    }
    ContentsDisplayInputState {
        id: inputState

        editorInputPolicyAdapter: editorInputPolicyAdapter
        surfacePolicy: surfacePolicy
    }
    ContentsDisplayGeometrySnapshotModel {
        id: geometrySnapshotModel

        contentsView: contentsView
        editorProjection: editorProjection
        editorTypingController: editorTypingController
        gutterCoordinator: gutterCoordinator
        minimapCoordinator: minimapCoordinator
        structuredDocumentFlow: structuredDocumentFlow
        structuredFlowCoordinator: structuredFlowCoordinator
        viewportCoordinator: viewportCoordinator
    }
    ContentsDisplayViewportModel {
        id: viewportModel

        contentsView: contentsView
        structuredDocumentFlow: structuredDocumentFlow
        viewportCoordinator: viewportCoordinator
    }
    function activeLogicalTextSnapshot() { return geometrySnapshotModel.activeLogicalTextSnapshot(); }
    function normalizedSnapshotEntries(rawEntries) { return geometrySnapshotModel.normalizedSnapshotEntries(rawEntries); }
    function hasStructuredLogicalLineGeometry() { return geometrySnapshotModel.hasStructuredLogicalLineGeometry(); }
    function effectiveStructuredMinimapEntries() { return geometrySnapshotModel.effectiveStructuredMinimapEntries(); }
    function hasStructuredMinimapEntries() { return geometrySnapshotModel.hasStructuredMinimapEntries(); }
    function logEditorCreationState(reason) { interactionViewModel.logEditorCreationState(reason); }
    function normalizedStructuredLogicalLineEntries() { return geometrySnapshotModel.normalizedStructuredLogicalLineEntries(); }
    function structuredLogicalLineEntryAt(lineNumber) { return geometrySnapshotModel.structuredLogicalLineEntryAt(lineNumber); }
    function effectiveStructuredLogicalLineEntries() { return geometrySnapshotModel.effectiveStructuredLogicalLineEntries(); }
    function currentStructuredGutterGeometrySignature() { return geometrySnapshotModel.currentStructuredGutterGeometrySignature(); }
    function consumeStructuredGutterGeometryChange() { return geometrySnapshotModel.consumeStructuredGutterGeometryChange(); }
    function buildStructuredMinimapLineGroupsForRange(startLineNumber, endLineNumber) { return geometrySnapshotModel.buildStructuredMinimapLineGroupsForRange(startLineNumber, endLineNumber); }
    function buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber) { return geometrySnapshotModel.buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber); }
    function buildMinimapLineGroupsForRange(startLineNumber, endLineNumber) { return geometrySnapshotModel.buildMinimapLineGroupsForRange(startLineNumber, endLineNumber); }
    function activeLineGeometryNoteId() { return geometrySnapshotModel.activeLineGeometryNoteId(); }
    function buildVisibleGutterLineEntries() { return geometrySnapshotModel.buildVisibleGutterLineEntries(); }
    function clampUnit(value) { return geometrySnapshotModel.clampUnit(value); }
    function logicalLineOffsetsEqual(previousOffsets, nextOffsets) { return geometrySnapshotModel.logicalLineOffsetsEqual(previousOffsets, nextOffsets); }
    function applyLiveLogicalLineMetrics(logicalTextLength, lineStartOffsets, lineCount) { return geometrySnapshotModel.applyLiveLogicalLineMetrics(logicalTextLength, lineStartOffsets, lineCount); }
    function hasPendingNoteEntryGutterRefresh(noteId) { return geometrySnapshotModel.hasPendingNoteEntryGutterRefresh(noteId); }
    function finalizePendingNoteEntryGutterRefresh(noteId, reason, refreshStructuredLayout) { return geometrySnapshotModel.finalizePendingNoteEntryGutterRefresh(noteId, reason, refreshStructuredLayout); }
    function commitGutterRefresh() { geometrySnapshotModel.commitGutterRefresh(); }
    function refreshVisibleGutterEntries() { geometrySnapshotModel.refreshVisibleGutterEntries(); }
    function traceVisibleGutterSnapshot(reason) { geometrySnapshotModel.traceVisibleGutterSnapshot(reason); }
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
        if (contentsView.structuredHostGeometryActive) {
            return !contentsView.hasPendingNoteEntryGutterRefresh()
                    && contentsView.effectiveStructuredLogicalLineEntries().length === contentsView.logicalLineCount
                    && contentsView.effectiveStructuredLogicalLineEntries().length > 0;
        }
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
    function shouldFlushBlurredEditorState(scheduledNoteId) { return interactionViewModel.shouldFlushBlurredEditorState(scheduledNoteId); }
    function nativeEditorCompositionActive() { return interactionViewModel.nativeEditorCompositionActive(); }
    function nativeTextInputSessionOwnsKeyboard() { return interactionViewModel.nativeTextInputSessionOwnsKeyboard(); }
    function flushEditorStateAfterInputSettles(scheduledNoteId) { interactionViewModel.flushEditorStateAfterInputSettles(scheduledNoteId); }
    function focusEditorForSelectedNoteId(noteId) { interactionViewModel.focusEditorForSelectedNoteId(noteId); }
    function focusEditorForPendingNote() { interactionViewModel.focusEditorForPendingNote(); }
    function eventRequestsPasteShortcut(event) { return interactionViewModel.eventRequestsPasteShortcut(event); }
    function inlineFormatShortcutTag(event) { return interactionViewModel.inlineFormatShortcutTag(event); }
    function handleInlineFormatTagShortcut(event) { return interactionViewModel.handleInlineFormatTagShortcut(event); }
    function clipboardImageAvailableForPaste() { return interactionViewModel.clipboardImageAvailableForPaste(); }
    function handleClipboardImagePasteShortcut(event) { return interactionViewModel.handleClipboardImagePasteShortcut(event); }
    function handleTagManagementShortcutKeyPress(event) { return interactionViewModel.handleTagManagementShortcutKeyPress(event); }
    function handleSelectionContextMenuEvent(eventName) { return interactionViewModel.handleSelectionContextMenuEvent(eventName); }
    function commitDocumentPresentationRefresh() { interactionViewModel.commitDocumentPresentationRefresh(); }
    function documentPresentationRenderDirty() { return interactionViewModel.documentPresentationRenderDirty(); }
    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition) {
        return editorSelectionController.inferSelectionRangeFromSelectedText(selectedText, cursorPosition);
    }
    function inlineStyleWrapTags(styleTag) {
        return editorSelectionController.inlineStyleWrapTags(styleTag);
    }
    function refreshInlineResourcePresentation() { interactionViewModel.refreshInlineResourcePresentation(); }
    function isMinimapScrollable() { return viewportModel.isMinimapScrollable(); }
    function lineDocumentY(lineNumber) { return viewportModel.lineDocumentY(lineNumber); }
    function gutterLineDocumentY(lineNumber) { return viewportModel.gutterLineDocumentY(lineNumber); }
    function gutterDocumentOccupiedBottomY() { return viewportModel.gutterDocumentOccupiedBottomY(); }
    function lineVisualHeight(startLine, lineSpan) { return viewportModel.lineVisualHeight(startLine, lineSpan); }
    function gutterLineVisualHeight(startLine, lineSpan) { return viewportModel.gutterLineVisualHeight(startLine, lineSpan); }
    function lineY(lineNumber) { return viewportModel.lineY(lineNumber); }
    function gutterLineY(lineNumber) { return viewportModel.gutterLineY(lineNumber); }
    function logicalLineNumberForDocumentY(documentY) { return viewportModel.logicalLineNumberForDocumentY(documentY); }
    function markerColorForType(markerType) { return viewportModel.markerColorForType(markerType); }
    function markerHeight(markerSpec) { return viewportModel.markerHeight(markerSpec); }
    function markerY(markerSpec) { return viewportModel.markerY(markerSpec); }
    function flattenMinimapLineGroups(lineGroups) { return MinimapSnapshotSupport.flattenLineGroups(lineGroups, contentsView.editorLineHeight); }
    function minimapContentHeight() { return viewportModel.minimapContentHeight(); }
    function minimapContentYForLine(lineNumber) { return viewportModel.minimapContentYForLine(lineNumber); }
    function minimapCurrentVisualRow(rowsOverride) { return viewportModel.minimapCurrentVisualRow(rowsOverride); }
    function minimapLineY(lineNumber) { return viewportModel.minimapLineY(lineNumber); }
    function minimapSilhouetteHeight(rowsOverride) { return viewportModel.minimapSilhouetteHeight(rowsOverride); }
    function minimapVisualRowPaintHeight(rowSpec) { return viewportModel.minimapVisualRowPaintHeight(rowSpec); }
    function minimapVisualRowPaintY(rowSpec) { return viewportModel.minimapVisualRowPaintY(rowSpec); }
    function normalizeInlineStyleTag(tagName) {
        return editorSelectionController.normalizeInlineStyleTag(tagName);
    }
    function encodeXmlAttributeValue(value) { return interactionViewModel.encodeXmlAttributeValue(value); }
    function resetStructuredSelectionContextMenuSnapshot() { interactionViewModel.resetStructuredSelectionContextMenuSnapshot(); }
    function primeStructuredSelectionContextMenuSnapshot() { return interactionViewModel.primeStructuredSelectionContextMenuSnapshot(); }
    function handleStructuredSelectionContextMenuEvent(eventName) { return interactionViewModel.handleStructuredSelectionContextMenuEvent(eventName); }
    function openEditorSelectionContextMenu(localX, localY) { return interactionViewModel.openEditorSelectionContextMenu(localX, localY); }
    function editorContextMenuPointerTriggerAccepted(triggerKind) { return interactionViewModel.editorContextMenuPointerTriggerAccepted(triggerKind); }
    function requestEditorSelectionContextMenuFromPointer(localX, localY, triggerKind) { return interactionViewModel.requestEditorSelectionContextMenuFromPointer(localX, localY, triggerKind); }
    function editorSelectionContextMenuSnapshotValid() { return interactionViewModel.editorSelectionContextMenuSnapshotValid(); }
    function ensureEditorSelectionContextMenuSnapshot() { return interactionViewModel.ensureEditorSelectionContextMenuSnapshot(); }
    function primeEditorSelectionContextMenuSnapshot() { return interactionViewModel.primeEditorSelectionContextMenuSnapshot(); }
    function persistEditorTextImmediately(nextText) { return interactionViewModel.persistEditorTextImmediately(nextText); }
    function scheduleEditorEntrySnapshotReconcile() { interactionViewModel.scheduleEditorEntrySnapshotReconcile(); }
    function pollSelectedNoteSnapshot() { interactionViewModel.pollSelectedNoteSnapshot(); }
    function reconcileEditorEntrySnapshotOnce() { return interactionViewModel.reconcileEditorEntrySnapshotOnce(); }
    function queueStructuredInlineFormatWrap(tagName) { return interactionViewModel.queueStructuredInlineFormatWrap(tagName); }
    function queueInlineFormatWrap(tagName) { return interactionViewModel.queueInlineFormatWrap(tagName); }
    function queueAgendaShortcutInsertion() { return interactionViewModel.queueAgendaShortcutInsertion(); }
    function queueCalloutShortcutInsertion() { return interactionViewModel.queueCalloutShortcutInsertion(); }
    function queueBreakShortcutInsertion() { return interactionViewModel.queueBreakShortcutInsertion(); }
    function focusStructuredBlockSourceOffset(sourceOffset) { interactionViewModel.focusStructuredBlockSourceOffset(sourceOffset); }
    function applyDocumentSourceMutation(nextSourceText, focusRequest) { return interactionViewModel.applyDocumentSourceMutation(nextSourceText, focusRequest); }
    function setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked) { return interactionViewModel.setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked); }
    function refreshMinimapSnapshot() {
        geometryViewModel.refreshMinimapSnapshot();
    }
    function refreshMinimapCursorTracking(rowsOverride) {
        geometryViewModel.refreshMinimapCursorTracking(rowsOverride);
    }
    function refreshMinimapViewportTracking(trackHeightOverride) {
        geometryViewModel.refreshMinimapViewportTracking(trackHeightOverride);
    }
    function requestViewHook(reason) { interactionViewModel.requestViewHook(reason); }
    function resetNoteEntryLineGeometryState() {
        geometryViewModel.resetNoteEntryLineGeometryState();
    }
    function resetGutterRefreshState() {
        geometryViewModel.resetGutterRefreshState();
    }
    function resetEditorSelectionCache() { interactionViewModel.resetEditorSelectionCache(); }
    function resolveEditorFlickable() {
        if (contentsView.showStructuredDocumentFlow) {
            if (contentsView.showPrintEditorLayout)
                return printDocumentViewport;
            return structuredDocumentViewport;
        }
        return null;
    }
    function scheduleEditorFocusForNote(noteId) { interactionViewModel.scheduleEditorFocusForNote(noteId); }
    function applyPresentationRefreshPlan(plan) { interactionViewModel.applyPresentationRefreshPlan(plan); }
    function executeRefreshPlan(plan) { interactionViewModel.executeRefreshPlan(plan); }
    function scheduleStructuredDocumentOpenLayoutRefresh(reason) { interactionViewModel.scheduleStructuredDocumentOpenLayoutRefresh(reason); }
    function scheduleDeferredDocumentPresentationRefresh() { interactionViewModel.scheduleDeferredDocumentPresentationRefresh(); }
    function scheduleDocumentPresentationRefresh(forceImmediate) { interactionViewModel.scheduleDocumentPresentationRefresh(forceImmediate); }
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
    function scheduleSelectionModelSync(options) { interactionViewModel.scheduleSelectionModelSync(options); }
    function executeSelectionDeliveryPlan(plan, fallbackKey) { return interactionViewModel.executeSelectionDeliveryPlan(plan, fallbackKey); }
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
        return interactionViewModel.selectedEditorRange();
    }
    function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {
        return interactionViewModel.wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange);
    }

    clip: true

    Component.onCompleted: {
        EditorTrace.trace("displayView", "mount", "visible=" + contentsView.visible, contentsView)
        contentsView.logEditorCreationState("componentCompleted");
        contentsView.scheduleSelectionModelSync({
                                                   "resetSnapshot": true,
                                                   "scheduleReconcile": true,
                                                   "fallbackRefresh": true,
                                                   "focusEditor": contentsView.mobileHost && contentsView.hasSelectedNote
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
        contentsView.scheduleDocumentPresentationRefresh(true);
    }
    onDocumentPresentationSourceTextChanged: {
        contentsView.refreshLiveLogicalLineMetrics();
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
                && structuredDocumentFlow) {
            contentsView.scheduleStructuredDocumentOpenLayoutRefresh("selected-body-text");
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
                && structuredDocumentFlow) {
            contentsView.scheduleStructuredDocumentOpenLayoutRefresh("selected-body-resolved");
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
                && structuredDocumentFlow) {
            contentsView.scheduleStructuredDocumentOpenLayoutRefresh("selected-body-loading");
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
        if (contentsView.noteDocumentMounted)
            contentsView.scheduleStructuredDocumentOpenLayoutRefresh("note-mounted");
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
    ContentsDisplayHostModePolicy {
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


QtObject {
    id: controller

    property var contentEditor: null
    readonly property var contextMenuItems: [
        {
            "label": "Plain",
            "keyVisible": false,
            "eventName": "editor.format.plain"
        },
        {
            "label": "Bold",
            "keyVisible": false,
            "eventName": "editor.format.bold"
        },
        {
            "label": "Italic",
            "keyVisible": false,
            "eventName": "editor.format.italic"
        },
        {
            "label": "Underline",
            "keyVisible": false,
            "eventName": "editor.format.underline"
        },
        {
            "label": "Strikethrough",
            "keyVisible": false,
            "eventName": "editor.format.strikethrough"
        },
        {
            "label": "Highlight",
            "keyVisible": false,
            "eventName": "editor.format.highlight"
        }
    ]
    property int contextMenuSelectionEnd: -1
    property real contextMenuSelectionCursorPosition: NaN
    property int contextMenuSelectionStart: -1
    property string contextMenuSelectionText: ""
    property var editorSession: null
    property var editorViewport: null
    property var selectionBridge: null
    property var selectionContextMenu: null
    property var textMetricsBridge: null
    property var view: null

    function preferNativeInputHandling() {
        return !!(controller.view
                  && controller.view.preferNativeInputHandling !== undefined
                  && controller.view.preferNativeInputHandling);
    }
    function contextMenuEditorSelectionRange() {
        if (controller.contextMenuSelectionEnd <= controller.contextMenuSelectionStart)
            return ({
                    "start": -1,
                    "end": -1
                });
        return ({
                "start": controller.contextMenuSelectionStart,
                "end": controller.contextMenuSelectionEnd
            });
    }
    function contextMenuEditorSelectionSnapshot() {
        const selectionRange = controller.contextMenuEditorSelectionRange();
        return {
            "start": selectionRange.start,
            "end": selectionRange.end,
            "selectedText": controller.contextMenuSelectionText,
            "cursorPosition": isFinite(controller.contextMenuSelectionCursorPosition)
                              ? Number(controller.contextMenuSelectionCursorPosition)
                              : controller.currentEditorCursorPosition()
        };
    }
    function currentInlineFormatSelectionSnapshot() {
        if (!controller.contentEditor)
            return {
                "cursorPosition": NaN,
                "selectedText": "",
                "selectionEnd": NaN,
                "selectionStart": NaN
            };
        if (controller.contentEditor.inlineFormatSelectionSnapshot !== undefined) {
            const inlineSnapshot = controller.contentEditor.inlineFormatSelectionSnapshot();
            if (inlineSnapshot)
                return inlineSnapshot;
        }
        if (controller.contentEditor.selectionSnapshot !== undefined) {
            const snapshot = controller.contentEditor.selectionSnapshot();
            if (snapshot)
                return snapshot;
        }
        return {
            "cursorPosition": controller.contentEditor.cursorPosition,
            "selectedText": controller.contentEditor.selectedText,
            "selectionEnd": controller.contentEditor.selectionEnd,
            "selectionStart": controller.contentEditor.selectionStart
        };
    }
    function currentEditorCursorPosition() {
        if (!controller.contentEditor)
            return NaN;
        const selectionSnapshot = controller.currentInlineFormatSelectionSnapshot();
        if (selectionSnapshot && selectionSnapshot.cursorPosition !== undefined) {
            const selectionCursor = Number(selectionSnapshot.cursorPosition);
            if (isFinite(selectionCursor))
                return selectionCursor;
        }
        if (controller.contentEditor.cursorPosition !== undefined) {
            const cursor = Number(controller.contentEditor.cursorPosition);
            if (isFinite(cursor))
                return cursor;
        }
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.cursorPosition !== undefined) {
            const cursor = Number(controller.contentEditor.editorItem.cursorPosition);
            if (isFinite(cursor))
                return cursor;
        }
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.inputItem && controller.contentEditor.editorItem.inputItem.cursorPosition !== undefined) {
            const cursor = Number(controller.contentEditor.editorItem.inputItem.cursorPosition);
            if (isFinite(cursor))
                return cursor;
        }

    }
    function currentEditorInputItem() {
        if (!controller.contentEditor)
            return null;
        if (controller.contentEditor.getFormattedText !== undefined && controller.contentEditor.getText !== undefined)
            return controller.contentEditor;
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.inputItem)
            return controller.contentEditor.editorItem.inputItem;
        if (controller.contentEditor.editorItem)
            return controller.contentEditor.editorItem;

    }
    function currentEditorPlainText() {
        const surfaceLength = controller.currentEditorSurfaceLength();
        if (controller.contentEditor && controller.contentEditor.currentPlainText !== undefined)
            return controller.normalizeSelectionTextValue(controller.contentEditor.currentPlainText());
        if (controller.contentEditor && controller.contentEditor.getText !== undefined)
            return controller.normalizeSelectionTextValue(controller.contentEditor.getText(0, surfaceLength));
        const inputItem = controller.currentEditorInputItem();
        if (inputItem && inputItem.getText !== undefined)
            return controller.normalizeSelectionTextValue(inputItem.getText(0, surfaceLength));
        return controller.normalizeSelectionTextValue(controller.currentLogicalText());
    }
    function currentEditorSelectionSnapshot() {
        if (!controller.contentEditor)
            return {
                "cursorPosition": NaN,
                "selectedText": "",
                "selectionEnd": NaN,
                "selectionStart": NaN
            };
        return controller.currentInlineFormatSelectionSnapshot();
    }
    function currentRawEditorSelectionRange() {
        if (!controller.contentEditor)
            return ({
                    "start": -1,
                    "end": -1
                });
        const selectionSnapshot = controller.currentEditorSelectionSnapshot();
        let start = selectionSnapshot.selectionStart !== undefined ? Number(selectionSnapshot.selectionStart) : NaN;
        let end = selectionSnapshot.selectionEnd !== undefined ? Number(selectionSnapshot.selectionEnd) : NaN;
        if (!isFinite(start) || !isFinite(end)) {
            if (controller.contentEditor.editorItem) {
                if (!isFinite(start) && controller.contentEditor.editorItem.selectionStart !== undefined)
                    start = Number(controller.contentEditor.editorItem.selectionStart);
                if (!isFinite(end) && controller.contentEditor.editorItem.selectionEnd !== undefined)
                    end = Number(controller.contentEditor.editorItem.selectionEnd);
                if (controller.contentEditor.editorItem.inputItem) {
                    if (!isFinite(start) && controller.contentEditor.editorItem.inputItem.selectionStart !== undefined)
                        start = Number(controller.contentEditor.editorItem.inputItem.selectionStart);
                    if (!isFinite(end) && controller.contentEditor.editorItem.inputItem.selectionEnd !== undefined)
                        end = Number(controller.contentEditor.editorItem.inputItem.selectionEnd);
                }
            }
        }
        const surfaceLength = controller.currentEditorSurfaceLength();
        const boundedStart = isFinite(start) ? Math.max(0, Math.min(surfaceLength, Math.floor(Math.min(start, end)))) : NaN;
        const boundedEnd = isFinite(end) ? Math.max(0, Math.min(surfaceLength, Math.floor(Math.max(start, end)))) : NaN;
        if (!isFinite(boundedStart) || !isFinite(boundedEnd) || boundedEnd <= boundedStart) {
            return ({
                    "start": -1,
                    "end": -1
                });
        }
        return ({
                "start": boundedStart,
                "end": boundedEnd
            });
    }
    function currentEditorSurfaceLength() {
        if (controller.contentEditor && controller.contentEditor.length !== undefined) {
            const lengthValue = Number(controller.contentEditor.length);
            if (isFinite(lengthValue))
                return Math.max(0, Math.floor(lengthValue));
        }
        const inputItem = controller.currentEditorInputItem();
        if (inputItem && inputItem.length !== undefined) {
            const lengthValue = Number(inputItem.length);
            if (isFinite(lengthValue))
                return Math.max(0, Math.floor(lengthValue));
        }

    }
    function currentLogicalText() {
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalText !== undefined)
            return String(controller.textMetricsBridge.logicalText === undefined || controller.textMetricsBridge.logicalText === null ? "" : controller.textMetricsBridge.logicalText);
        if (!controller.view || controller.view.editorText === undefined || controller.view.editorText === null)
            return "";
        return String(controller.view.editorText);
    }
    function currentSelectedEditorText() {
        if (!controller.contentEditor)
            return "";
        const selectionSnapshot = controller.currentInlineFormatSelectionSnapshot();
        if (selectionSnapshot && selectionSnapshot.selectedText !== undefined) {
            return String(
                        selectionSnapshot.selectedText === undefined || selectionSnapshot.selectedText === null
                        ? ""
                        : selectionSnapshot.selectedText);
        }
        if (controller.contentEditor.selectedText !== undefined)
            return String(controller.contentEditor.selectedText === undefined || controller.contentEditor.selectedText === null ? "" : controller.contentEditor.selectedText);
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.selectedText !== undefined)
            return String(controller.contentEditor.editorItem.selectedText === undefined || controller.contentEditor.editorItem.selectedText === null ? "" : controller.contentEditor.editorItem.selectedText);
        if (controller.contentEditor.editorItem && controller.contentEditor.editorItem.inputItem && controller.contentEditor.editorItem.inputItem.selectedText !== undefined)
            return String(controller.contentEditor.editorItem.inputItem.selectedText === undefined || controller.contentEditor.editorItem.inputItem.selectedText === null ? "" : controller.contentEditor.editorItem.inputItem.selectedText);
        return "";
    }
    function currentSelectedNoteId() {
        if (!controller.view || controller.view.selectedNoteId === undefined || controller.view.selectedNoteId === null)
            return "";
        return String(controller.view.selectedNoteId).trim();
    }
    function currentSourceText() {
        if (!controller.view || controller.view.editorText === undefined || controller.view.editorText === null)
            return "";
        return String(controller.view.editorText);
    }
    function resourceTagLossDetectedForMutation(currentSourceText, nextSourceText) {
        if (!controller.view || controller.view.resourceTagLossDetected === undefined)
            return false;
        return !!controller.view.resourceTagLossDetected(currentSourceText, nextSourceText);
    }
    function restoreEditorSurfaceFromSourcePresentation() {
        if (controller.view && controller.view.restoreEditorSurfaceFromPresentation !== undefined) {
            controller.view.restoreEditorSurfaceFromPresentation();
            return;
        }
        if (controller.view && controller.view.commitDocumentPresentationRefresh !== undefined)
            controller.view.commitDocumentPresentationRefresh();
    }
    function clampLogicalPosition(position, maximumLength) {
        const numericPosition = Number(position);
        if (!isFinite(numericPosition))
            return 0;
        return Math.max(0, Math.min(Math.floor(numericPosition), Math.max(0, Number(maximumLength) || 0)));
    }
    function sourceOffsetForLogicalOffset(logicalOffset) {
        const safeOffset = Math.max(0, Math.floor(Number(logicalOffset) || 0));
        if (controller.textMetricsBridge && controller.textMetricsBridge.sourceOffsetForLogicalOffset !== undefined) {
            const mappedOffset = Number(controller.textMetricsBridge.sourceOffsetForLogicalOffset(safeOffset));
            if (isFinite(mappedOffset))
                return Math.max(0, Math.floor(mappedOffset));
        }
        const currentSourceText = controller.currentSourceText();
        return Math.max(0, Math.min(currentSourceText.length, safeOffset));
    }
    function selectionCursorUsesStartEdge(cursorPosition, selectionStart, selectionEnd) {
        const boundedStart = Math.max(0, Math.floor(Number(selectionStart) || 0));
        const boundedEnd = Math.max(boundedStart, Math.floor(Number(selectionEnd) || 0));
        const numericCursor = Number(cursorPosition);
        if (!isFinite(numericCursor))
            return false;
        const boundedCursor = controller.clampLogicalPosition(numericCursor, boundedEnd);
        if (boundedCursor <= boundedStart)
            return true;
        if (boundedCursor >= boundedEnd)
            return false;
        return Math.abs(boundedCursor - boundedStart) <= Math.abs(boundedCursor - boundedEnd);
    }
    function restoreEditorSelection(inputItem, selectionStart, selectionEnd, cursorPosition) {
        if (!inputItem)
            return false;
        const boundedStart = Math.max(0, Math.floor(Number(selectionStart) || 0));
        const boundedEnd = Math.max(boundedStart, Math.floor(Number(selectionEnd) || 0));
        if (boundedEnd <= boundedStart)
            return false;
        const activeCursor = isFinite(Number(cursorPosition)) ? Number(cursorPosition) : controller.currentEditorCursorPosition();
        const activeEdgeIsStart = controller.selectionCursorUsesStartEdge(activeCursor, boundedStart, boundedEnd);
        if (inputItem.moveCursorSelection !== undefined) {
            const anchorPosition = activeEdgeIsStart ? boundedEnd : boundedStart;
            const activePosition = activeEdgeIsStart ? boundedStart : boundedEnd;
            inputItem.cursorPosition = anchorPosition;
            inputItem.moveCursorSelection(activePosition, TextEdit.SelectCharacters);
            return true;
        }
        if (inputItem.select !== undefined) {
            inputItem.select(boundedStart, boundedEnd);
            return true;
        }
        return false;
    }
    function scheduleEditorSelection(selectionStart, selectionEnd, cursorPosition) {
        const requestedStart = Number(selectionStart);
        const requestedEnd = Number(selectionEnd);
        const requestedCursor = Number(cursorPosition);
        Qt.callLater(function () {
            if (!controller.contentEditor)
                return;
            const logicalLength = controller.currentLogicalText().length;
            const boundedStart = controller.clampLogicalPosition(requestedStart, logicalLength);
            const boundedEnd = controller.clampLogicalPosition(requestedEnd, logicalLength);
            const fallbackCursor = isFinite(requestedCursor) ? requestedCursor : controller.currentEditorCursorPosition();
            const boundedCursor = controller.clampLogicalPosition(fallbackCursor, logicalLength);
            const editorItem = controller.contentEditor.editorItem ? controller.contentEditor.editorItem : null;
            const inputItem = editorItem && editorItem.inputItem ? editorItem.inputItem : null;
            if (isFinite(requestedStart) && isFinite(requestedEnd) && boundedEnd > boundedStart) {
                controller.restoreEditorSelection(inputItem, boundedStart, boundedEnd, boundedCursor);
                return;
            }
            if (inputItem && inputItem.deselect !== undefined)
                inputItem.deselect();
            if (controller.contentEditor.cursorPosition !== undefined)
                controller.contentEditor.cursorPosition = boundedCursor;
            if (editorItem && editorItem.cursorPosition !== undefined)
                editorItem.cursorPosition = boundedCursor;
            if (inputItem && inputItem.cursorPosition !== undefined)
                inputItem.cursorPosition = boundedCursor;
        });
    }
    function editorPlainTextSlice(start, end) {
        const surfaceLength = controller.currentEditorSurfaceLength();
        const boundedStart = Math.max(0, Math.min(surfaceLength, Math.floor(Number(start) || 0)));
        const boundedEnd = Math.max(0, Math.min(surfaceLength, Math.floor(Number(end) || 0)));
        if (boundedEnd <= boundedStart)
            return "";
        if (controller.contentEditor && controller.contentEditor.getText !== undefined)
            return controller.normalizeSelectionTextValue(controller.contentEditor.getText(boundedStart, boundedEnd));
        const inputItem = controller.currentEditorInputItem();
        if (inputItem && inputItem.getText !== undefined)
            return controller.normalizeSelectionTextValue(inputItem.getText(boundedStart, boundedEnd));
        return controller.currentEditorPlainText().slice(boundedStart, boundedEnd);
    }
    function handleSelectionContextMenuEvent(eventName) {
        const contextSelectionRange = controller.contextMenuEditorSelectionSnapshot();
        if (eventName === "editor.format.plain")
            controller.wrapSelectedEditorTextWithTag("plain", contextSelectionRange);
        else if (eventName === "editor.format.bold")
            controller.wrapSelectedEditorTextWithTag("bold", contextSelectionRange);
        else if (eventName === "editor.format.italic")
            controller.wrapSelectedEditorTextWithTag("italic", contextSelectionRange);
        else if (eventName === "editor.format.underline")
            controller.wrapSelectedEditorTextWithTag("underline", contextSelectionRange);
        else if (eventName === "editor.format.strikethrough")
            controller.wrapSelectedEditorTextWithTag("strikethrough", contextSelectionRange);
        else if (eventName === "editor.format.highlight")
            controller.wrapSelectedEditorTextWithTag("highlight", contextSelectionRange);
    }
    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition, preferredRangeStart, preferredRangeEnd) {
        if (!controller.view)
            return ({
                    "start": -1,
                    "end": -1
                });
        const plainText = controller.currentEditorPlainText();
        const normalizedSelectedText = controller.normalizeSelectionTextValue(selectedText);
        if (plainText.length === 0 || normalizedSelectedText.length === 0)
            return ({
                    "start": -1,
                    "end": -1
                });
        const occurrenceRanges = [];
        let searchOffset = 0;
        while (searchOffset <= plainText.length) {
            const occurrenceStart = plainText.indexOf(normalizedSelectedText, searchOffset);
            if (occurrenceStart < 0)
                break;
            occurrenceRanges.push({
                "start": occurrenceStart,
                "end": occurrenceStart + normalizedSelectedText.length
            });
            searchOffset = occurrenceStart + 1;
        }
        if (occurrenceRanges.length === 0)
            return ({
                    "start": -1,
                    "end": -1
                });
        let anchor = NaN;
        const numericPreferredStart = Number(preferredRangeStart);
        const numericPreferredEnd = Number(preferredRangeEnd);
        if (isFinite(numericPreferredStart) && isFinite(numericPreferredEnd) && numericPreferredEnd > numericPreferredStart)
            anchor = Math.max(0, Math.min(plainText.length, Math.floor((numericPreferredStart + numericPreferredEnd) / 2)));
        else if (isFinite(cursorPosition))
            anchor = Math.max(0, Math.min(plainText.length, Math.floor(cursorPosition)));
        let bestOccurrence = occurrenceRanges[0];
        let bestScore = Number.POSITIVE_INFINITY;
        for (let index = 0; index < occurrenceRanges.length; ++index) {
            const occurrence = occurrenceRanges[index];
            const occurrenceMidpoint = (occurrence.start + occurrence.end) / 2;
            const occurrenceScore = isFinite(anchor) ? Math.abs(occurrenceMidpoint - anchor) : Math.abs(occurrence.end - plainText.length);
            if (occurrenceScore < bestScore) {
                bestScore = occurrenceScore;
                bestOccurrence = occurrence;
            }
        }
        return ({
                "start": bestOccurrence.start,
                "end": bestOccurrence.end
            });
    }
    function inlineStyleWrapTags(styleTag) {
        if (!controller.view)
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        switch (styleTag) {
        case "plain":
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        case "bold":
            return ({
                    "openTag": "<bold>",
                    "closeTag": "</bold>"
                });
        case "italic":
            return ({
                    "openTag": "<italic>",
                    "closeTag": "</italic>"
                });
        case "underline":
            return ({
                    "openTag": "<underline>",
                    "closeTag": "</underline>"
                });
        case "strikethrough":
            return ({
                    "openTag": "<strikethrough>",
                    "closeTag": "</strikethrough>"
                });
        case "highlight":
            return ({
                    "openTag": "<highlight>",
                    "closeTag": "</highlight>"
                });
        default:
            return ({
                    "openTag": "",
                    "closeTag": ""
                });
        }
    }
    function normalizeInlineStyleTag(tagName) {
        return RawInlineStyleMutationSupport.normalizedInlineStyleTag(tagName);
    }
    function normalizeSelectionTextValue(text) {
        let normalizedText = text === undefined || text === null ? "" : String(text);
        normalizedText = normalizedText.replace(/\r\n/g, "\n");
        normalizedText = normalizedText.replace(/\r/g, "\n");
        normalizedText = normalizedText.replace(/\u2028/g, "\n");
        normalizedText = normalizedText.replace(/\u2029/g, "\n");
        normalizedText = normalizedText.replace(/\uFFFC/g, "");
        normalizedText = normalizedText.replace(/\u00a0/g, " ");
        return normalizedText;
    }
    function openEditorSelectionContextMenu(localX, localY) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        let selectionRange = controller.resolveSelectionSnapshot(controller.contextMenuEditorSelectionSnapshot());
        if (selectionRange.end <= selectionRange.start) {
            selectionRange = controller.selectedEditorRange();
        }
        if (selectionRange.end <= selectionRange.start) {
            const inferredRange = controller.inferSelectionRangeFromSelectedText(controller.currentSelectedEditorText(), controller.currentEditorCursorPosition());
            if (inferredRange.end > inferredRange.start)
                selectionRange = inferredRange;
        }
        if (selectionRange.end <= selectionRange.start || !controller.selectionContextMenu)
            return false;
        if (controller.selectionContextMenu.opened)
            controller.selectionContextMenu.close();
        controller.contextMenuSelectionStart = selectionRange.start;
        controller.contextMenuSelectionEnd = selectionRange.end;
        controller.contextMenuSelectionCursorPosition = controller.currentEditorCursorPosition();
        controller.contextMenuSelectionText = controller.editorPlainTextSlice(selectionRange.start, selectionRange.end);
        controller.selectionContextMenu.openFor(controller.editorViewport, Number(localX) || 0, Number(localY) || 0);
        return true;
    }
    function primeContextMenuSelectionSnapshot() {
        const rawSelectionRange = controller.currentRawEditorSelectionRange();
        let selectionRange = rawSelectionRange;
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.selectedEditorRange();
        if (selectionRange.end <= selectionRange.start) {
            controller.resetEditorSelectionCache();
            return false;
        }
        controller.contextMenuSelectionStart = selectionRange.start;
        controller.contextMenuSelectionEnd = selectionRange.end;
        controller.contextMenuSelectionCursorPosition = controller.currentEditorCursorPosition();
        controller.contextMenuSelectionText = controller.editorPlainTextSlice(selectionRange.start, selectionRange.end);
        return true;
    }
    function persistEditorTextImmediately(nextText) {
        if (!controller.view
                || !controller.editorSession
                || (controller.editorSession.persistEditorTextImmediatelyWithText === undefined
                    && controller.editorSession.persistEditorTextImmediately === undefined))
            return false;
        if (controller.editorSession.persistEditorTextImmediatelyWithText !== undefined)
            return !!controller.editorSession.persistEditorTextImmediatelyWithText(nextText);
        return !!controller.editorSession.persistEditorTextImmediately();
    }
    function queueInlineFormatWrap(tagName) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        const normalizedTagName = controller.normalizeInlineStyleTag(tagName);
        if (normalizedTagName.length === 0)
            return false;
        if (controller.view.queueStructuredInlineFormatWrap !== undefined
                && controller.view.queueStructuredInlineFormatWrap(normalizedTagName)) {
            return true;
        }
        let selectionRange = controller.selectedEditorRange();
        const selectedText = controller.currentSelectedEditorText();
        const selectionSnapshot = {
            "start": selectionRange.start,
            "end": selectionRange.end,
            "selectedText": selectedText,
            "cursorPosition": controller.currentEditorCursorPosition()
        };
        if (selectionRange.end <= selectionRange.start)
            return false;
        return controller.wrapSelectedEditorTextWithTag(normalizedTagName, selectionSnapshot);
    }
    function resetEditorSelectionCache() {
        controller.contextMenuSelectionStart = -1;
        controller.contextMenuSelectionEnd = -1;
        controller.contextMenuSelectionCursorPosition = NaN;
        controller.contextMenuSelectionText = "";
    }
    function resolveSelectionSnapshot(selectionSnapshot) {
        const normalizedSelectedText = controller.normalizeSelectionTextValue(selectionSnapshot && selectionSnapshot.selectedText !== undefined ? selectionSnapshot.selectedText : controller.currentSelectedEditorText());
        const cursorPosition = selectionSnapshot && selectionSnapshot.cursorPosition !== undefined ? Number(selectionSnapshot.cursorPosition) : controller.currentEditorCursorPosition();
        const start = selectionSnapshot && selectionSnapshot.start !== undefined ? Number(selectionSnapshot.start) : NaN;
        const end = selectionSnapshot && selectionSnapshot.end !== undefined ? Number(selectionSnapshot.end) : NaN;
        if (isFinite(start) && isFinite(end) && end > start) {
            const candidateRange = {
                "start": Math.floor(start),
                "end": Math.floor(end)
            };
            if (normalizedSelectedText.length === 0 || controller.selectionRangeMatchesSelectedText(candidateRange, normalizedSelectedText)) {
                return candidateRange;
            }
        }
        if (normalizedSelectedText.length > 0)
            return controller.inferSelectionRangeFromSelectedText(normalizedSelectedText, cursorPosition);
        return {
            "start": -1,
            "end": -1
        };
    }
    function refreshPresentationStateAfterProgrammaticChange() {
        if (!controller.view || controller.view.commitDocumentPresentationRefresh === undefined)
            return;
        controller.view.commitDocumentPresentationRefresh();
    }
    function selectedEditorRange() {
        if (!controller.contentEditor)
            return ({
                    "start": -1,
                    "end": -1
                });
        const selectionSnapshot = controller.currentEditorSelectionSnapshot();
        const selectedText = selectionSnapshot.selectedText !== undefined ? selectionSnapshot.selectedText : controller.currentSelectedEditorText();
        const numericSelectionRange = controller.currentRawEditorSelectionRange();
        const numericRange = {
            "start": numericSelectionRange.start,
            "end": numericSelectionRange.end,
            "selectedText": selectedText,
            "cursorPosition": selectionSnapshot.cursorPosition !== undefined ? Number(selectionSnapshot.cursorPosition) : controller.currentEditorCursorPosition()
        };
        const resolvedRange = controller.resolveSelectionSnapshot(numericRange);
        if (resolvedRange.end > resolvedRange.start)
            return resolvedRange;
        return ({
                "start": -1,
                "end": -1
            });
    }
    function selectionRangeMatchesSelectedText(selectionRange, selectedText) {
        if (!selectionRange)
            return false;
        const start = Number(selectionRange.start);
        const end = Number(selectionRange.end);
        if (!isFinite(start) || !isFinite(end) || end <= start)
            return false;
        const normalizedSelectedText = controller.normalizeSelectionTextValue(selectedText);
        if (normalizedSelectedText.length === 0)
            return false;
        return controller.editorPlainTextSlice(start, end) === normalizedSelectedText;
    }
    function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {
        if (!controller.view || !controller.view.hasSelectedNote || controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        const normalizedTagName = controller.normalizeInlineStyleTag(tagName);
        if (normalizedTagName.length === 0)
            return false;
        let selectionRange = controller.resolveSelectionSnapshot(explicitSelectionRange || null);
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.selectedEditorRange();
        if (selectionRange.end <= selectionRange.start)
            selectionRange = controller.resolveSelectionSnapshot(controller.contextMenuEditorSelectionSnapshot());
        if (selectionRange.end <= selectionRange.start)
            return false;
        const currentText = controller.view.editorText === undefined || controller.view.editorText === null ? "" : String(controller.view.editorText);
        const logicalLength = controller.currentLogicalText().length;
        const boundedStart = Math.max(0, Math.min(logicalLength, Math.floor(selectionRange.start)));
        const boundedEnd = Math.max(0, Math.min(logicalLength, Math.floor(selectionRange.end)));
        if (boundedEnd <= boundedStart)
            return false;
        const mutationPayload = RawInlineStyleMutationSupport.buildInlineStyleSelectionPayload(
                    currentText,
                    boundedStart,
                    boundedEnd,
                    normalizedTagName);
        if (!mutationPayload || !mutationPayload.applied)
            return false;
        const nextText = String(mutationPayload.nextSourceText);
        if (nextText.length === 0 && currentText.length > 0)
            return false;
        if (controller.resourceTagLossDetectedForMutation(currentText, nextText)) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
        }
        if (!controller.commitRawEditorTextMutation(nextText))
            return false;
        const committedText = controller.committedEditorText(nextText);
        controller.refreshPresentationStateAfterProgrammaticChange();
        controller.resetEditorSelectionCache();
        controller.view.editorTextEdited(committedText);
        return true;
    }

    function commitRawEditorTextMutation(nextSourceText) {
        if (!controller.editorSession
                || controller.editorSession.commitRawEditorTextMutation === undefined) {
            return false;
        }
        return !!controller.editorSession.commitRawEditorTextMutation(nextSourceText);
    }

    function committedEditorText(fallbackText) {
        if (controller.editorSession && controller.editorSession.editorText !== undefined
                && controller.editorSession.editorText !== null) {
            return String(controller.editorSession.editorText);
        }
        return fallbackText === undefined || fallbackText === null ? "" : String(fallbackText);
    }
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

    ContentsEditorInputPolicyAdapter {
        id: editorInputPolicyAdapter
        objectName: "contentsDisplayEditorInputPolicyAdapter"

        editorCompositionActive: contentsView.nativeEditorCompositionActive()
        editorInputFocused: contentsView.editorInputFocused
        editorTagManagementInputEnabled: contentsView.editorTagManagementInputEnabled
        nativeTextInputPriority: contentsView.nativeTextInputPriority
        noteDocumentParseMounted: contentsView.noteDocumentParseMounted
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
        pendingBodySave: contentsView.pendingBodySave
        selectedNoteBodyLoading: contentsView.selectedNoteBodyLoading
        selectedNoteBodyNoteId: contentsView.selectedNoteBodyNoteId === undefined || contentsView.selectedNoteBodyNoteId === null ? "" : String(contentsView.selectedNoteBodyNoteId)
        selectedNoteBodyText: contentsView.selectedNoteBodyText === undefined || contentsView.selectedNoteBodyText === null ? "" : String(contentsView.selectedNoteBodyText)
        selectedNoteBodyResolved: contentsView.selectedNoteBodyResolved
        selectedNoteId: contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null ? "" : String(contentsView.selectedNoteId)
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
        structuredHostGeometryActive: contentsView.hasStructuredMinimapEntries()
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


QtObject {
    id: controller
    objectName: "contentsEditorTypingController"

    property var view: null
    property var contentEditor: null
    property var editorSession: null
    property var plainTextSourceMutator: null
    property var textMetricsBridge: null
    property var agendaBackend: null
    property var calloutBackend: null
    property string liveAuthoritativePlainText: ""
    property var liveLogicalLineStartOffsets: [0]
    property var liveLogicalToSourceOffsets: [0]
    property string liveSnapshotSourceText: ""
    property bool pendingCursorPositionRequest: false
    property int pendingCursorPosition: 0

    function normalizePlainText(text) {
        let normalizedText = text === undefined || text === null ? "" : String(text);
        normalizedText = normalizedText.replace(/\r\n/g, "\n");
        normalizedText = normalizedText.replace(/\r/g, "\n");
        normalizedText = normalizedText.replace(/\u2028/g, "\n");
        normalizedText = normalizedText.replace(/\u2029/g, "\n");
        normalizedText = normalizedText.replace(/\uFFFC/g, "");
        normalizedText = normalizedText.replace(/\u00a0/g, " ");
        return normalizedText;
    }

    function spliceSourceText(sourceText, sourceStart, sourceEnd, replacementSourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        const safeLength = normalizedSourceText.length;
        const boundedStart = Math.max(0, Math.min(safeLength, Math.floor(Number(sourceStart) || 0)));
        const boundedEnd = Math.max(boundedStart, Math.min(safeLength, Math.floor(Number(sourceEnd) || 0)));
        const replacementText = controller.normalizePlainText(replacementSourceText);
        return normalizedSourceText.slice(0, boundedStart)
                + replacementText
                + normalizedSourceText.slice(boundedEnd);
    }

    function presentationSourceText() {
        if (controller.view && controller.view.documentPresentationSourceText !== undefined)
            return controller.view.documentPresentationSourceText === undefined || controller.view.documentPresentationSourceText === null
                    ? ""
                    : String(controller.view.documentPresentationSourceText);
        if (controller.view && controller.view.editorText !== undefined)
            return controller.view.editorText === undefined || controller.view.editorText === null
                    ? ""
                    : String(controller.view.editorText);
        return "";
    }

    function currentSourceTextSnapshot() {
        if (controller.view && controller.view.editorText !== undefined)
            return controller.view.editorText === undefined || controller.view.editorText === null
                    ? ""
                    : String(controller.view.editorText);
        return controller.presentationSourceText();
    }

    function presentationSnapshotStale() {
        return controller.currentSourceTextSnapshot() !== controller.presentationSourceText();
    }

    function identityOffsetArray(sourceLength) {
        const safeLength = Math.max(0, Math.floor(Number(sourceLength) || 0));
        const offsets = new Array(safeLength + 1);
        for (let index = 0; index <= safeLength; ++index)
            offsets[index] = index;
        return offsets;
    }

    function computeLineStartOffsets(text) {
        const normalizedText = controller.normalizePlainText(text);
        const offsets = [0];
        for (let index = 0; index < normalizedText.length; ++index) {
            if (normalizedText.charAt(index) === "\n")
                offsets.push(index + 1);
        }
        return offsets;
    }

    function normalizeLineStartOffsets(rawOffsets, logicalText) {
        const normalizedText = controller.normalizePlainText(logicalText);
        const rawLength = rawOffsets && rawOffsets.length !== undefined
                ? Math.max(0, Math.floor(Number(rawOffsets.length) || 0))
                : 0;
        if (rawLength > 0) {
            const offsets = new Array(rawLength);
            for (let index = 0; index < rawLength; ++index) {
                const numericOffset = Number(rawOffsets[index]);
                offsets[index] = isFinite(numericOffset) ? Math.max(0, Math.floor(numericOffset)) : 0;
            }
            return offsets;
        }
        return controller.computeLineStartOffsets(normalizedText);
    }

    function normalizeOffsetArray(rawOffsets, logicalLength, fallbackSourceText) {
        const safeLogicalLength = Math.max(0, Math.floor(Number(logicalLength) || 0));
        const rawLength = rawOffsets && rawOffsets.length !== undefined
                ? Math.max(0, Math.floor(Number(rawOffsets.length) || 0))
                : 0;
        if (rawLength === safeLogicalLength + 1) {
            const offsets = new Array(rawLength);
            for (let index = 0; index < rawLength; ++index) {
                const numericOffset = Number(rawOffsets[index]);
                offsets[index] = isFinite(numericOffset) ? Math.max(0, Math.floor(numericOffset)) : 0;
            }
            return offsets;
        }
        if (controller.textMetricsBridge && controller.textMetricsBridge.sourceOffsetForLogicalOffset !== undefined) {
            const offsets = new Array(safeLogicalLength + 1);
            for (let index = 0; index <= safeLogicalLength; ++index) {
                const numericOffset = Number(controller.textMetricsBridge.sourceOffsetForLogicalOffset(index));
                offsets[index] = isFinite(numericOffset) ? Math.max(0, Math.floor(numericOffset)) : 0;
            }
            return offsets;
        }
        return controller.identityOffsetArray(safeLogicalLength);
    }

    function lastLineIndexForOffset(lineStartOffsets, offset) {
        const offsets = Array.isArray(lineStartOffsets) && lineStartOffsets.length > 0 ? lineStartOffsets : [0];
        const safeOffset = Math.max(0, Math.floor(Number(offset) || 0));
        let bestIndex = 0;
        for (let index = 0; index < offsets.length; ++index) {
            const lineOffset = Math.max(0, Number(offsets[index]) || 0);
            if (lineOffset > safeOffset)
                break;
            bestIndex = index;
        }
        return bestIndex;
    }

    function firstLineIndexAtOrAfterOffset(lineStartOffsets, offset, minimumIndex) {
        const offsets = Array.isArray(lineStartOffsets) && lineStartOffsets.length > 0 ? lineStartOffsets : [0];
        const safeOffset = Math.max(0, Math.floor(Number(offset) || 0));
        const safeMinimumIndex = Math.max(0, Math.floor(Number(minimumIndex) || 0));
        for (let index = safeMinimumIndex; index < offsets.length; ++index) {
            const lineOffset = Math.max(0, Number(offsets[index]) || 0);
            if (lineOffset >= safeOffset)
                return index;
        }
        return offsets.length;
    }

    function escapedSourceLengthForCharacter(ch) {
        const safeChar = ch === undefined || ch === null ? "" : String(ch);
        if (safeChar === "&")
            return 5;
        if (safeChar === "<" || safeChar === ">")
            return 4;
        if (safeChar === "\"")
            return 6;
        if (safeChar === "'")
            return 5;
        return 1;
    }

    function buildReplacementSourceOffsets(text) {
        const normalizedText = controller.normalizePlainText(text);
        const offsets = [0];
        let sourceOffset = 0;
        for (let index = 0; index < normalizedText.length; ++index) {
            sourceOffset += controller.escapedSourceLengthForCharacter(normalizedText.charAt(index));
            offsets.push(sourceOffset);
        }
        return offsets;
    }

    function buildReplacementLineStartOffsets(currentLineStart, text) {
        const baseStart = Math.max(0, Math.floor(Number(currentLineStart) || 0));
        const normalizedText = controller.normalizePlainText(text);
        const offsets = [baseStart];
        for (let index = 0; index < normalizedText.length; ++index) {
            if (normalizedText.charAt(index) === "\n")
                offsets.push(baseStart + index + 1);
        }
        return offsets;
    }

    function adoptLiveStateIntoBridge(sourceText) {
        controller.liveSnapshotSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        if (!controller.textMetricsBridge
                || controller.textMetricsBridge.adoptIncrementalState === undefined) {
            return;
        }
        controller.textMetricsBridge.adoptIncrementalState(
                    sourceText,
                    controller.liveAuthoritativePlainText,
                    controller.liveLogicalLineStartOffsets,
                    controller.liveLogicalToSourceOffsets);
    }

    function synchronizeLiveEditingStateFromPresentation() {
        const snapshotSourceText = controller.presentationSourceText();
        const presentationLogicalText = controller.textMetricsBridge && controller.textMetricsBridge.logicalText !== undefined
                ? controller.normalizePlainText(controller.textMetricsBridge.logicalText)
                : controller.normalizePlainText(snapshotSourceText);
        let lineStartOffsets = [];
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalLineStartOffsets !== undefined)
            lineStartOffsets = controller.textMetricsBridge.logicalLineStartOffsets;
        let sourceOffsets = [];
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalToSourceOffsets !== undefined)
            sourceOffsets = controller.textMetricsBridge.logicalToSourceOffsets();
        EditorTrace.trace(
                    "typingController",
                    "synchronizeLiveEditingStateFromPresentation",
                    "source=" + EditorTrace.describeText(snapshotSourceText)
                    + " logical=" + EditorTrace.describeText(presentationLogicalText),
                    controller)
        controller.liveSnapshotSourceText = snapshotSourceText;
        controller.liveAuthoritativePlainText = presentationLogicalText;
        controller.liveLogicalLineStartOffsets = controller.normalizeLineStartOffsets(
                    lineStartOffsets,
                    presentationLogicalText);
        controller.liveLogicalToSourceOffsets = controller.normalizeOffsetArray(
                    sourceOffsets,
                    presentationLogicalText.length,
                    snapshotSourceText);
    }

    function ensureLiveEditingStateReady() {
        const snapshotSourceText = controller.presentationSourceText();
        const currentOffsets = Array.isArray(controller.liveLogicalToSourceOffsets)
                ? controller.liveLogicalToSourceOffsets
                : [];
        const currentLineStartOffsets = Array.isArray(controller.liveLogicalLineStartOffsets)
                ? controller.liveLogicalLineStartOffsets
                : [];
        const liveStateValid = currentOffsets.length === controller.liveAuthoritativePlainText.length + 1
                && currentLineStartOffsets.length > 0;
        EditorTrace.trace(
                    "typingController",
                    "ensureLiveEditingStateReady",
                    "snapshotChanged=" + (controller.liveSnapshotSourceText !== snapshotSourceText)
                    + " liveStateValid=" + liveStateValid
                    + " localAuthority=" + (controller.editorSession && controller.editorSession.localEditorAuthority !== undefined
                                            ? controller.editorSession.localEditorAuthority
                                            : false),
                    controller)
        if (controller.presentationSnapshotStale()
                && liveStateValid
                && controller.editorSession
                && controller.editorSession.localEditorAuthority !== undefined
                && controller.editorSession.localEditorAuthority) {
            return;
        }
        if (controller.liveSnapshotSourceText !== snapshotSourceText
                || !liveStateValid) {
            controller.synchronizeLiveEditingStateFromPresentation();
        }
    }

    function authoritativeSourcePlainText() {
        controller.ensureLiveEditingStateReady();
        if (controller.liveAuthoritativePlainText.length > 0
                || controller.liveSnapshotSourceText.length > 0) {
            return controller.liveAuthoritativePlainText;
        }
        if (controller.textMetricsBridge && controller.textMetricsBridge.logicalText !== undefined)
            return controller.normalizePlainText(controller.textMetricsBridge.logicalText);
        if (controller.view
                && controller.view.selectedNoteBodyText !== undefined
                && controller.view.selectedNoteBodyNoteId !== undefined
                && controller.view.selectedNoteId !== undefined
                && String(controller.view.selectedNoteBodyNoteId) === String(controller.view.selectedNoteId))
            return controller.normalizePlainText(controller.view.selectedNoteBodyText);
        return "";
    }

    function currentEditorPlainText() {
        if (!controller.contentEditor)
            return controller.authoritativeSourcePlainText();
        if (controller.contentEditor.currentPlainText !== undefined)
            return controller.normalizePlainText(controller.contentEditor.currentPlainText());
        if (controller.contentEditor.getText === undefined)
            return controller.authoritativeSourcePlainText();
        const editorLength = controller.contentEditor.length !== undefined
                ? Math.max(0, Number(controller.contentEditor.length) || 0)
                : 0;
        return controller.normalizePlainText(controller.contentEditor.getText(0, editorLength));
    }

    function nativeCompositionActive() {
        if (!controller.contentEditor)
            return false;
        const activePreeditText = controller.contentEditor.preeditText !== undefined
                ? String(controller.contentEditor.preeditText === undefined || controller.contentEditor.preeditText === null
                             ? ""
                             : controller.contentEditor.preeditText)
                : "";
        return (controller.contentEditor.inputMethodComposing !== undefined
                && controller.contentEditor.inputMethodComposing)
                || activePreeditText.length > 0;
    }

    function applyEditorCursorPosition(position) {
        if (!controller.contentEditor || controller.nativeCompositionActive())
            return false;
        const targetPosition = Math.max(0, Math.floor(Number(position) || 0));
        if (controller.contentEditor.setCursorPositionPreservingNativeInput !== undefined)
            return !!controller.contentEditor.setCursorPositionPreservingNativeInput(targetPosition);
        if (controller.contentEditor.cursorPosition !== undefined) {
            controller.contentEditor.cursorPosition = targetPosition;
            return true;
        }
        return false;
    }

    function applyPendingCursorPositionIfInputSettled() {
        if (!controller.pendingCursorPositionRequest || controller.nativeCompositionActive())
            return false;
        const targetPosition = Math.max(0, Math.floor(Number(controller.pendingCursorPosition) || 0));
        controller.pendingCursorPositionRequest = false;
        return controller.applyEditorCursorPosition(targetPosition);
    }

    function scheduleCursorPosition(position) {
        const targetPosition = Math.max(0, Math.floor(Number(position) || 0));
        Qt.callLater(function () {
            if (!controller.contentEditor)
                return;
            if (controller.nativeCompositionActive()) {
                controller.pendingCursorPosition = targetPosition;
                controller.pendingCursorPositionRequest = true;
                return;
            }
            controller.pendingCursorPositionRequest = false;
            controller.applyEditorCursorPosition(targetPosition);
        });
    }

    function computePlainTextReplacementDelta(previousText, nextText) {
        const previous = previousText === undefined || previousText === null ? "" : String(previousText);
        const next = nextText === undefined || nextText === null ? "" : String(nextText);
        if (previous === next) {
            return {
                "insertedText": "",
                "previousEnd": 0,
                "start": 0,
                "valid": false
            };
        }

        let prefixLength = 0;
        const prefixLimit = Math.min(previous.length, next.length);
        while (prefixLength < prefixLimit && previous.charAt(prefixLength) === next.charAt(prefixLength))
            ++prefixLength;

        let suffixLength = 0;
        const suffixLimit = Math.min(previous.length - prefixLength, next.length - prefixLength);
        while (suffixLength < suffixLimit
                && previous.charAt(previous.length - 1 - suffixLength) === next.charAt(next.length - 1 - suffixLength)) {
            ++suffixLength;
        }

        const previousEnd = previous.length - suffixLength;
        const nextEnd = next.length - suffixLength;
        return {
            "insertedText": next.slice(prefixLength, nextEnd),
            "previousEnd": previousEnd,
            "start": prefixLength,
            "valid": true
        };
    }

    function breakShortcutInsertion(previousPlainText, replacementStart, replacementEnd, insertedText) {
        const previousText = controller.normalizePlainText(previousPlainText);
        const safeStart = Math.max(0, Math.min(previousText.length, Math.floor(Number(replacementStart) || 0)));
        const safeEnd = Math.max(safeStart, Math.min(previousText.length, Math.floor(Number(replacementEnd) || 0)));
        const normalizedInsertedText = controller.normalizePlainText(insertedText);
        if (normalizedInsertedText.indexOf("\n") >= 0)
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });

        const candidateText = previousText.slice(0, safeStart)
                + normalizedInsertedText
                + previousText.slice(safeEnd);
        const candidateCursor = safeStart + normalizedInsertedText.length;
        const candidateLineAnchor = Math.max(0, candidateCursor - 1);
        const candidateLineStart = candidateLineAnchor > 0
                ? candidateText.lastIndexOf("\n", candidateLineAnchor) + 1
                : 0;
        const candidateLineEndIndex = candidateText.indexOf("\n", candidateLineStart);
        const candidateLineEnd = candidateLineEndIndex >= 0 ? candidateLineEndIndex : candidateText.length;
        const candidateLineText = candidateText.slice(candidateLineStart, candidateLineEnd);
        if (candidateLineText !== "---")
            return ({ "applied": false, "cursorPosition": -1, "insertedText": "", "replacementEnd": -1, "replacementStart": -1 });

        const previousLineAnchor = Math.max(0, safeStart - 1);
        const previousLineStart = previousLineAnchor > 0
                ? previousText.lastIndexOf("\n", previousLineAnchor) + 1
                : 0;
        const previousLineEndIndex = previousText.indexOf("\n", previousLineStart);
        const previousLineEnd = previousLineEndIndex >= 0 ? previousLineEndIndex : previousText.length;
        return ({
                "applied": true,
                "cursorPosition": previousLineStart + 1,
                "insertedText": "</break>",
                "replacementEnd": previousLineEnd,
                "replacementStart": previousLineStart
            });
    }

    function currentEditorSelectionSnapshot() {
        if (!controller.contentEditor) {
            return ({
                    "cursorPosition": NaN,
                    "selectionEnd": NaN,
                    "selectionStart": NaN
                });
        }
        if (controller.contentEditor.selectionSnapshot !== undefined) {
            const snapshot = controller.contentEditor.selectionSnapshot();
            if (snapshot)
                return snapshot;
        }
        return ({
                "cursorPosition": controller.contentEditor.cursorPosition,
                "selectionEnd": controller.contentEditor.selectionEnd,
                "selectionStart": controller.contentEditor.selectionStart
            });
    }

    function currentRawEditorSelectionRange() {
        const snapshot = controller.currentEditorSelectionSnapshot();
        const cursorPosition = Number(snapshot && snapshot.cursorPosition !== undefined ? snapshot.cursorPosition : NaN);
        const selectionStart = Number(snapshot && snapshot.selectionStart !== undefined ? snapshot.selectionStart : NaN);
        const selectionEnd = Number(snapshot && snapshot.selectionEnd !== undefined ? snapshot.selectionEnd : NaN);
        const logicalLength = controller.authoritativeSourcePlainText().length;
        if (isFinite(selectionStart) && isFinite(selectionEnd)) {
            return ({
                    "start": Math.max(0, Math.min(logicalLength, Math.floor(Math.min(selectionStart, selectionEnd)))),
                    "end": Math.max(0, Math.min(logicalLength, Math.floor(Math.max(selectionStart, selectionEnd))))
                });
        }
        const boundedCursor = isFinite(cursorPosition)
                ? Math.max(0, Math.min(logicalLength, Math.floor(cursorPosition)))
                : logicalLength;
        return ({
                "start": boundedCursor,
                "end": boundedCursor
            });
    }

    function sourceTagRanges(sourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        const ranges = [];
        const tagPattern = /<!--[\s\S]*?-->|<\s*\/?\s*[A-Za-z_][A-Za-z0-9_.:-]*\b[^>]*?>/g;
        let match = tagPattern.exec(normalizedSourceText);
        while (match) {
            const token = String(match[0] || "");
            const start = Math.max(0, Number(match.index) || 0);
            const end = start + token.length;
            if (end > start) {
                ranges.push({
                        "end": end,
                        "start": start,
                        "token": token
                    });
            }
            if (tagPattern.lastIndex === match.index)
                tagPattern.lastIndex = match.index + Math.max(1, token.length);
            match = tagPattern.exec(normalizedSourceText);
        }
        return ranges;
    }

    function normalizedSourceTagName(tagToken) {
        const normalizedToken = tagToken === undefined || tagToken === null ? "" : String(tagToken);
        if (normalizedToken.length === 0)
            return "";
        const match = normalizedToken.match(/^<\s*\/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b/i);
        if (!match || match.length < 2)
            return "";
        return String(match[1] || "").trim().toLowerCase();
    }
    function isClosingSourceTagToken(tagToken) {
        const normalizedToken = tagToken === undefined || tagToken === null ? "" : String(tagToken);
        return /^<\s*\//.test(normalizedToken);
    }
    function isInlineStyleTagName(tagName) {
        const normalizedTagName = tagName === undefined || tagName === null ? "" : String(tagName).trim().toLowerCase();
        return normalizedTagName === "bold"
                || normalizedTagName === "b"
                || normalizedTagName === "strong"
                || normalizedTagName === "italic"
                || normalizedTagName === "i"
                || normalizedTagName === "em"
                || normalizedTagName === "underline"
                || normalizedTagName === "u"
                || normalizedTagName === "strikethrough"
                || normalizedTagName === "strike"
                || normalizedTagName === "s"
                || normalizedTagName === "del"
                || normalizedTagName === "highlight"
                || normalizedTagName === "mark";
    }
    function sourceTagRangeStartingAt(sourceText, sourceOffset) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        const boundedOffset = Math.max(
                    0,
                    Math.min(
                        normalizedSourceText.length,
                        Math.floor(Number(sourceOffset) || 0)));
        if (boundedOffset >= normalizedSourceText.length || normalizedSourceText.charAt(boundedOffset) !== "<")
            return null;
        const tagEnd = normalizedSourceText.indexOf(">", boundedOffset + 1);
        if (tagEnd <= boundedOffset)
            return null;
        return {
            "end": tagEnd + 1,
            "start": boundedOffset,
            "token": normalizedSourceText.slice(boundedOffset, tagEnd + 1)
        };
    }
    function advanceSourceOffsetPastClosingInlineStyleTags(sourceText, sourceOffset) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        let nextOffset = Math.max(
                    0,
                    Math.min(
                        normalizedSourceText.length,
                        Math.floor(Number(sourceOffset) || 0)));
        while (nextOffset < normalizedSourceText.length) {
            const tagRange = controller.sourceTagRangeStartingAt(normalizedSourceText, nextOffset);
            if (!tagRange)
                break;
            const normalizedTagName = controller.normalizedSourceTagName(tagRange.token);
            if (!controller.isClosingSourceTagToken(tagRange.token)
                    || !controller.isInlineStyleTagName(normalizedTagName)) {
                break;
            }
            nextOffset = Math.max(nextOffset, Number(tagRange.end) || nextOffset);
        }
        return nextOffset;
    }
    function sourceOffsetForCollapsedLogicalInsertion(sourceText, logicalOffset) {
        const normalizedSourceText = sourceText === undefined || sourceText === null ? "" : String(sourceText);
        const baseSourceOffset = controller.sourceOffsetForLogicalOffset(logicalOffset);
        return controller.advanceSourceOffsetPastClosingInlineStyleTags(
                    normalizedSourceText,
                    baseSourceOffset);
    }

    function sourceRangeTouchesNamedTag(sourceTagRanges, sourceStart, sourceEnd, normalizedTagName) {
        const ranges = Array.isArray(sourceTagRanges) ? sourceTagRanges : [];
        const targetTagName = normalizedTagName === undefined || normalizedTagName === null
                ? ""
                : String(normalizedTagName).trim().toLowerCase();
        if (targetTagName.length === 0)
            return false;
        const safeStart = Math.max(0, Math.floor(Number(sourceStart) || 0));
        const safeEnd = Math.max(safeStart, Math.floor(Number(sourceEnd) || 0));
        for (let index = 0; index < ranges.length; ++index) {
            const range = ranges[index];
            if (controller.normalizedSourceTagName(range.token) !== targetTagName)
                continue;
            const rangeStart = Math.max(0, Math.floor(Number(range.start) || 0));
            const rangeEnd = Math.max(rangeStart, Math.floor(Number(range.end) || 0));
            if (safeEnd > safeStart) {
                if (rangeEnd > safeStart && rangeStart < safeEnd)
                    return true;
                continue;
            }
            if (safeStart > rangeStart && safeStart < rangeEnd)
                return true;
        }
        return false;
    }

    function logicalRangeCollapsesToSingleSourceOffset(logicalStart, logicalEnd) {
        const safeLogicalStart = Math.max(0, Math.floor(Number(logicalStart) || 0));
        const safeLogicalEnd = Math.max(safeLogicalStart, Math.floor(Number(logicalEnd) || 0));
        if (safeLogicalEnd <= safeLogicalStart)
            return false;
        return controller.sourceOffsetForLogicalOffset(safeLogicalStart)
                === controller.sourceOffsetForLogicalOffset(safeLogicalEnd);
    }

    function restoreEditorSurfaceFromSourcePresentation() {
        if (controller.view && controller.view.restoreEditorSurfaceFromPresentation !== undefined) {
            controller.view.restoreEditorSurfaceFromPresentation();
        } else if (controller.view && controller.view.commitDocumentPresentationRefresh !== undefined) {
            controller.view.commitDocumentPresentationRefresh();
        } else {
            controller.synchronizeLiveEditingStateFromPresentation();
            return;
        }
        controller.synchronizeLiveEditingStateFromPresentation();
    }
    function resourceTagLossDetectedForMutation(currentSourceText, nextSourceText) {
        if (!controller.view || controller.view.resourceTagLossDetected === undefined)
            return false;
        return !!controller.view.resourceTagLossDetected(currentSourceText, nextSourceText);
    }

    function collapsedDeletionTagRange(sourceTagRanges, sourceOffset, deleteForward) {
        const ranges = Array.isArray(sourceTagRanges) ? sourceTagRanges : [];
        const safeOffset = Math.max(0, Math.floor(Number(sourceOffset) || 0));
        if (deleteForward) {
            for (let index = 0; index < ranges.length; ++index) {
                const range = ranges[index];
                const rangeStart = Math.max(0, Math.floor(Number(range.start) || 0));
                const rangeEnd = Math.max(rangeStart, Math.floor(Number(range.end) || 0));
                if (safeOffset >= rangeStart && safeOffset < rangeEnd)
                    return range;
            }
            return null;
        }
        for (let index = ranges.length - 1; index >= 0; --index) {
            const range = ranges[index];
            const rangeStart = Math.max(0, Math.floor(Number(range.start) || 0));
            const rangeEnd = Math.max(rangeStart, Math.floor(Number(range.end) || 0));
            if (safeOffset > rangeStart && safeOffset <= rangeEnd)
                return range;
        }
        return null;
    }

    function expandedDeletionSourceRange(sourceTagRanges, sourceStart, sourceEnd) {
        const ranges = Array.isArray(sourceTagRanges) ? sourceTagRanges : [];
        let expandedStart = Math.max(0, Math.floor(Number(sourceStart) || 0));
        let expandedEnd = Math.max(expandedStart, Math.floor(Number(sourceEnd) || 0));
        if (expandedEnd <= expandedStart) {
            return ({
                    "start": expandedStart,
                    "end": expandedEnd
                });
        }

        let changed = true;
        while (changed) {
            changed = false;
            for (let index = 0; index < ranges.length; ++index) {
                const range = ranges[index];
                const rangeStart = Math.max(0, Math.floor(Number(range.start) || 0));
                const rangeEnd = Math.max(rangeStart, Math.floor(Number(range.end) || 0));
                const intersects = rangeEnd > expandedStart && rangeStart < expandedEnd;
                if (!intersects)
                    continue;
                if (rangeStart < expandedStart) {
                    expandedStart = rangeStart;
                    changed = true;
                }
                if (rangeEnd > expandedEnd) {
                    expandedEnd = rangeEnd;
                    changed = true;
                }
            }
        }

        return ({
                "start": expandedStart,
                "end": expandedEnd
            });
    }

    function queueAgendaShortcutInsertion() {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        return controller.insertStructuredShortcutSourceAtCursor("agenda");
    }

    function queueCalloutShortcutInsertion() {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        return controller.insertStructuredShortcutSourceAtCursor("callout");
    }

    function queueBreakShortcutInsertion() {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        return controller.insertStructuredShortcutSourceAtCursor("break");
    }

    function insertStructuredShortcutSourceAtCursor(shortcutKind) {
        controller.ensureLiveEditingStateReady();
        const currentSourceText = controller.view && controller.view.editorText !== undefined && controller.view.editorText !== null
                ? String(controller.view.editorText)
                : "";
        const normalizedShortcutKind = String(shortcutKind || "").trim().toLowerCase();
        if (normalizedShortcutKind === "callout") {
            const selectionRange = controller.currentRawEditorSelectionRange();
            const logicalSelectionStart = Math.max(0, Math.floor(Number(selectionRange.start) || 0));
            const logicalSelectionEnd = Math.max(logicalSelectionStart, Math.floor(Number(selectionRange.end) || 0));
            if (logicalSelectionEnd > logicalSelectionStart) {
                const sourceSelectionStart = Math.max(
                            0,
                            Math.min(
                                currentSourceText.length,
                                Math.floor(Number(controller.sourceOffsetForLogicalOffset(logicalSelectionStart)) || 0)));
                const sourceSelectionEnd = Math.max(
                            sourceSelectionStart,
                            Math.min(
                                currentSourceText.length,
                                Math.floor(Number(controller.sourceOffsetForLogicalOffset(logicalSelectionEnd)) || 0)));
                const wrapPayload = RawTagMutationSupport.buildCalloutRangeWrappingPayload(
                            currentSourceText,
                            sourceSelectionStart,
                            sourceSelectionEnd);
                return controller.commitRawSourceInsertionPayload(wrapPayload, currentSourceText);
            }
        }
        const logicalCursor = controller.currentLogicalCursorOffsetForShortcutInsertion();
        const rawSourceCursorOffset = Math.max(
                    0,
                    Math.min(
                        currentSourceText.length,
                        Math.floor(Number(controller.sourceOffsetForCollapsedLogicalInsertion(
                                             currentSourceText,
                                             logicalCursor)) || 0)));
        const insertionPayload = RawTagMutationSupport.buildStructuredShortcutInsertionPayload(
                    currentSourceText,
                    rawSourceCursorOffset,
                    normalizedShortcutKind);
        return controller.commitRawSourceInsertionPayload(insertionPayload, currentSourceText);
    }

    function currentLogicalCursorOffsetForShortcutInsertion() {
        const plainTextLength = controller.authoritativeSourcePlainText().length;
        if (!controller.contentEditor || controller.contentEditor.cursorPosition === undefined)
            return plainTextLength;
        const numericCursor = Number(controller.contentEditor.cursorPosition);
        if (!isFinite(numericCursor))
            return plainTextLength;
        return Math.max(0, Math.min(plainTextLength, Math.floor(numericCursor)));
    }

    function commitRawSourceInsertionPayload(insertionPayload, currentSourceText) {
        const safePayload = insertionPayload && typeof insertionPayload === "object" ? insertionPayload : ({});
        if (!safePayload.applied)
            return false;
        const nextSourceText = safePayload.nextSourceText !== undefined && safePayload.nextSourceText !== null
                ? String(safePayload.nextSourceText)
                : currentSourceText;
        if (nextSourceText === currentSourceText)
            return false;
        const cursorSourceOffset = Math.max(0, Math.floor(Number(safePayload.sourceOffset) || 0));

        if (!controller.commitRawEditorTextMutation(nextSourceText))
            return false;
        const committedText = controller.committedEditorText(nextSourceText);
        if (controller.view.commitDocumentPresentationRefresh !== undefined)
            controller.view.commitDocumentPresentationRefresh();
        else
            controller.synchronizeLiveEditingStateFromPresentation();

        controller.scheduleCursorPosition(controller.logicalOffsetForSourceOffset(cursorSourceOffset));
        controller.view.editorTextEdited(committedText);
        return true;
    }

    function insertRawSourceTextAtCursor(rawSourceText, cursorSourceOffsetFromInsertionStart) {
        if (!controller.view)
            return false;
        const normalizedRawSourceText = controller.normalizePlainText(rawSourceText);
        if (normalizedRawSourceText.length === 0)
            return false;
        controller.ensureLiveEditingStateReady();
        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        const logicalCursor = controller.currentLogicalCursorOffsetForShortcutInsertion();
        const rawSourceCursorOffset = Math.max(
                    0,
                    Math.min(
                        currentSourceText.length,
                        Math.floor(Number(controller.sourceOffsetForCollapsedLogicalInsertion(
                                             currentSourceText,
                                             logicalCursor)) || 0)));
        const insertionPayload = RawTagMutationSupport.buildRawSourceInsertionPayload(
                    currentSourceText,
                    rawSourceCursorOffset,
                    normalizedRawSourceText,
                    Math.max(0, Math.floor(Number(cursorSourceOffsetFromInsertionStart) || 0)));
        return controller.commitRawSourceInsertionPayload(insertionPayload, currentSourceText);
    }

    function agendaTodoShortcutInsertion(previousPlainText, replacementStart, replacementEnd, insertedText) {
        if (!controller.agendaBackend
                || controller.agendaBackend.detectTodoShortcutReplacement === undefined) {
            return ({ "applied": false });
        }
        return controller.agendaBackend.detectTodoShortcutReplacement(
                    previousPlainText,
                    replacementStart,
                    replacementEnd,
                    insertedText);
    }

    function agendaTaskEnterInsertion(currentSourceText, sourceStart, sourceEnd, insertedText) {
        if (!controller.agendaBackend
                || controller.agendaBackend.detectAgendaTaskEnterReplacement === undefined) {
            return ({ "applied": false });
        }
        return controller.agendaBackend.detectAgendaTaskEnterReplacement(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    insertedText);
    }

    function calloutEnterInsertion(currentSourceText, sourceStart, sourceEnd, insertedText) {
        if (!controller.calloutBackend
                || controller.calloutBackend.detectCalloutEnterReplacement === undefined) {
            return ({ "applied": false });
        }
        return controller.calloutBackend.detectCalloutEnterReplacement(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    insertedText);
    }

    function commitExplicitSourceMutation(nextSourceText) {
        EditorTrace.trace(
                    "typingController",
                    "commitExplicitSourceMutation",
                    EditorTrace.describeText(nextSourceText),
                    controller)
        if (!controller.view)
            return false;
        if (controller.view.applyDocumentSourceMutation !== undefined)
            return controller.view.applyDocumentSourceMutation(nextSourceText);

        const normalizedNextSourceText = nextSourceText === undefined || nextSourceText === null
                ? ""
                : String(nextSourceText);
        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        if (normalizedNextSourceText === currentSourceText)
            return false;
        if (controller.resourceTagLossDetectedForMutation(currentSourceText, normalizedNextSourceText)) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
        }

        if (!controller.commitRawEditorTextMutation(normalizedNextSourceText))
            return false;
        const committedText = controller.committedEditorText(normalizedNextSourceText);
        if (controller.view.commitDocumentPresentationRefresh !== undefined)
            controller.view.commitDocumentPresentationRefresh();
        else
            controller.synchronizeLiveEditingStateFromPresentation();

        controller.view.editorTextEdited(committedText);
        return true;
    }

    function applyExplicitPlainTextLogicalReplacement(logicalReplacementStart, logicalReplacementEnd, insertedText) {
        EditorTrace.trace(
                    "typingController",
                    "applyExplicitPlainTextLogicalReplacement",
                    "logicalStart=" + logicalReplacementStart
                    + " logicalEnd=" + logicalReplacementEnd
                    + " inserted=" + EditorTrace.describeText(insertedText),
                    controller)
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        if (!controller.contentEditor
                || !controller.plainTextSourceMutator
                || controller.plainTextSourceMutator.applyPlainTextReplacementToSource === undefined) {
            return false;
        }

        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        const previousPlainText = controller.authoritativeSourcePlainText();
        const plainTextLength = previousPlainText.length;
        const boundedLogicalStart = Math.max(
                    0,
                    Math.min(
                        plainTextLength,
                        Math.floor(Number(logicalReplacementStart) || 0)));
        const boundedLogicalEnd = Math.max(
                    boundedLogicalStart,
                    Math.min(
                        plainTextLength,
                        Math.floor(Number(logicalReplacementEnd) || 0)));
        const normalizedRequestedText = controller.normalizePlainText(insertedText);
        const syntheticNextPlainText = previousPlainText.slice(0, boundedLogicalStart)
                + normalizedRequestedText
                + previousPlainText.slice(boundedLogicalEnd);
        let normalizedInsertedText = normalizedRequestedText;
        let resolvedLogicalReplacementStart = boundedLogicalStart;
        let resolvedLogicalReplacementEnd = boundedLogicalEnd;

        let rawSourceReplacementText = "";
        let rawReplacementEnabled = false;
        let cursorLogicalOverride = Math.max(
                    0,
                    Math.min(
                        syntheticNextPlainText.length,
                        resolvedLogicalReplacementStart + normalizedInsertedText.length));
        let cursorSourceOffsetOverride = NaN;

        const breakShortcut = controller.breakShortcutInsertion(
                    previousPlainText,
                    resolvedLogicalReplacementStart,
                    resolvedLogicalReplacementEnd,
                    normalizedInsertedText);
        if (!rawReplacementEnabled && breakShortcut.applied) {
            normalizedInsertedText = breakShortcut.insertedText;
            resolvedLogicalReplacementStart = Math.max(0, Math.floor(Number(breakShortcut.replacementStart) || 0));
            resolvedLogicalReplacementEnd = Math.max(
                        resolvedLogicalReplacementStart,
                        Math.floor(Number(breakShortcut.replacementEnd) || 0));
            rawSourceReplacementText = normalizedInsertedText;
            rawReplacementEnabled = true;
            cursorLogicalOverride = Math.max(0, Math.floor(Number(breakShortcut.cursorPosition) || 0));
        }

        const collapsedLogicalInsertion = resolvedLogicalReplacementEnd === resolvedLogicalReplacementStart;
        const sourceStart = collapsedLogicalInsertion
                ? controller.sourceOffsetForCollapsedLogicalInsertion(
                    currentSourceText,
                    resolvedLogicalReplacementStart)
                : controller.sourceOffsetForLogicalOffset(resolvedLogicalReplacementStart);
        const sourceEnd = collapsedLogicalInsertion
                ? sourceStart
                : controller.sourceOffsetForLogicalOffset(resolvedLogicalReplacementEnd);
        const currentSourceTagRanges = controller.sourceTagRanges(currentSourceText);
        const targetsVirtualSourceOnly = controller.logicalRangeCollapsesToSingleSourceOffset(
                    resolvedLogicalReplacementStart,
                    resolvedLogicalReplacementEnd);

        const agendaTaskEnter = controller.agendaTaskEnterInsertion(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    normalizedInsertedText);
        let replacementSourceStart = sourceStart;
        let replacementSourceEnd = sourceEnd;
        if (agendaTaskEnter.applied) {
            replacementSourceStart = Math.max(0, Math.floor(Number(agendaTaskEnter.replacementSourceStart) || 0));
            replacementSourceEnd = Math.max(
                        replacementSourceStart,
                        Math.floor(Number(agendaTaskEnter.replacementSourceEnd) || 0));
            rawSourceReplacementText = String(agendaTaskEnter.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(
                        0,
                        Math.floor(Number(agendaTaskEnter.cursorSourceOffsetFromReplacementStart) || 0));
            cursorLogicalOverride = NaN;
        }

        const calloutEnter = agendaTaskEnter.applied
                ? ({ "applied": false })
                : controller.calloutEnterInsertion(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    normalizedInsertedText);
        if (calloutEnter.applied) {
            replacementSourceStart = Math.max(0, Math.floor(Number(calloutEnter.replacementSourceStart) || 0));
            replacementSourceEnd = Math.max(
                        replacementSourceStart,
                        Math.floor(Number(calloutEnter.replacementSourceEnd) || 0));
            rawSourceReplacementText = String(calloutEnter.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(
                        0,
                        Math.floor(Number(calloutEnter.cursorSourceOffsetFromReplacementStart) || 0));
            cursorLogicalOverride = NaN;
        }

        const touchesResourceTag = controller.sourceRangeTouchesNamedTag(
                    currentSourceTagRanges,
                    replacementSourceStart,
                    replacementSourceEnd,
                    "resource");
        if (touchesResourceTag || targetsVirtualSourceOnly) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
        }

        let nextSourceText = "";
        if (rawReplacementEnabled) {
            nextSourceText = controller.spliceSourceText(
                        currentSourceText,
                        replacementSourceStart,
                        replacementSourceEnd,
                        rawSourceReplacementText);
        } else {
            nextSourceText = String(controller.plainTextSourceMutator.applyPlainTextReplacementToSource(
                                        currentSourceText,
                                        sourceStart,
                                        sourceEnd,
                                        normalizedInsertedText));
        }
        if (nextSourceText === currentSourceText)
            return false;

        const committed = controller.commitExplicitSourceMutation(nextSourceText);
        if (!committed)
            return false;

        if (rawReplacementEnabled && isFinite(cursorSourceOffsetOverride))
            controller.scheduleCursorPosition(
                        controller.logicalOffsetForSourceOffset(replacementSourceStart + cursorSourceOffsetOverride));
        else if (isFinite(cursorLogicalOverride))
            controller.scheduleCursorPosition(cursorLogicalOverride);
        return true;
    }

    function sourceOffsetForLogicalOffset(logicalOffset) {
        controller.ensureLiveEditingStateReady();
        const safeOffset = Math.max(0, Math.floor(Number(logicalOffset) || 0));
        const offsets = Array.isArray(controller.liveLogicalToSourceOffsets)
                ? controller.liveLogicalToSourceOffsets
                : [];
        if (offsets.length > 0) {
            const boundedOffset = Math.max(0, Math.min(offsets.length - 1, safeOffset));
            const mappedOffset = Number(offsets[boundedOffset]);
            if (isFinite(mappedOffset))
                return Math.max(0, Math.floor(mappedOffset));
        }
        if (controller.textMetricsBridge && controller.textMetricsBridge.sourceOffsetForLogicalOffset !== undefined) {
            const mappedOffset = Number(controller.textMetricsBridge.sourceOffsetForLogicalOffset(safeOffset));
            if (isFinite(mappedOffset))
                return Math.max(0, Math.floor(mappedOffset));
        }
        const currentSourceText = controller.view && controller.view.editorText !== undefined && controller.view.editorText !== null
                ? String(controller.view.editorText)
                : "";
        return Math.max(0, Math.min(currentSourceText.length, safeOffset));
    }

    function logicalOffsetForSourceOffset(sourceOffset) {
        controller.ensureLiveEditingStateReady();
        const safeSourceOffset = Math.max(0, Math.floor(Number(sourceOffset) || 0));
        const offsets = Array.isArray(controller.liveLogicalToSourceOffsets)
                ? controller.liveLogicalToSourceOffsets
                : [];
        if (offsets.length === 0)
            return safeSourceOffset;
        for (let logicalIndex = 0; logicalIndex < offsets.length; ++logicalIndex) {
            const mappedSourceOffset = Math.max(0, Math.floor(Number(offsets[logicalIndex]) || 0));
            if (mappedSourceOffset >= safeSourceOffset)
                return logicalIndex;
        }
        return Math.max(0, offsets.length - 1);
    }

    function applyLiveEditingStateReplacement(logicalStart, logicalEnd, replacementText, sourceStart, sourceEnd) {
        controller.ensureLiveEditingStateReady();
        const previousLogicalText = controller.liveAuthoritativePlainText;
        const boundedLogicalStart = Math.max(0, Math.min(previousLogicalText.length, Math.floor(Number(logicalStart) || 0)));
        const boundedLogicalEnd = Math.max(boundedLogicalStart, Math.min(previousLogicalText.length, Math.floor(Number(logicalEnd) || 0)));
        const boundedSourceStart = Math.max(0, Math.floor(Number(sourceStart) || 0));
        const boundedSourceEnd = Math.max(boundedSourceStart, Math.floor(Number(sourceEnd) || 0));
        const insertedText = controller.normalizePlainText(replacementText);
        const insertedSourceOffsets = controller.buildReplacementSourceOffsets(insertedText);
        const previousOffsets = Array.isArray(controller.liveLogicalToSourceOffsets)
                ? controller.liveLogicalToSourceOffsets
                : controller.identityOffsetArray(previousLogicalText.length);
        const previousLogicalLength = previousLogicalText.length;
        const logicalInsertedLength = insertedText.length;
        const sourceRemovedLength = boundedSourceEnd - boundedSourceStart;
        const sourceInsertedLength = Math.max(0, Number(insertedSourceOffsets[insertedSourceOffsets.length - 1]) || 0);
        const sourceDelta = sourceInsertedLength - sourceRemovedLength;
        const nextLogicalText = previousLogicalText.slice(0, boundedLogicalStart)
                + insertedText
                + previousLogicalText.slice(boundedLogicalEnd);
        const nextLineStartOffsets = controller.computeLineStartOffsets(nextLogicalText);
        const nextOffsets = new Array(nextLogicalText.length + 1);

        for (let index = 0; index < boundedLogicalStart; ++index)
            nextOffsets[index] = Math.max(0, Number(previousOffsets[index]) || 0);
        nextOffsets[boundedLogicalStart] = boundedSourceStart;
        for (let index = 1; index <= logicalInsertedLength; ++index)
            nextOffsets[boundedLogicalStart + index] = boundedSourceStart + Math.max(0, Number(insertedSourceOffsets[index]) || 0);
        for (let previousIndex = boundedLogicalEnd; previousIndex <= previousLogicalLength; ++previousIndex) {
            const nextIndex = boundedLogicalStart + logicalInsertedLength + (previousIndex - boundedLogicalEnd);
            nextOffsets[nextIndex] = Math.max(0, Number(previousOffsets[previousIndex]) || 0) + sourceDelta;
        }

        controller.liveAuthoritativePlainText = nextLogicalText;
        controller.liveLogicalLineStartOffsets = nextLineStartOffsets;
        controller.liveLogicalToSourceOffsets = nextOffsets;
    }

    function handleEditorTextEdited() {
        EditorTrace.trace(
                    "typingController",
                    "handleEditorTextEdited",
                    "selectedNoteId=" + (controller.view ? String(controller.view.selectedNoteId || "") : "")
                    + " structured=" + (controller.view && controller.view.showStructuredDocumentFlow !== undefined
                                        ? controller.view.showStructuredDocumentFlow
                                        : false),
                    controller)
        if (!controller.view
                || !controller.view.hasSelectedNote
                || (controller.view.showStructuredDocumentFlow !== undefined
                    && controller.view.showStructuredDocumentFlow)
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        if (controller.view.resourceDropEditorSurfaceGuardActive !== undefined
                && controller.view.resourceDropEditorSurfaceGuardActive) {
            return false;
        }
        if ((controller.view.programmaticEditorSurfaceSyncActive !== undefined
             && controller.view.programmaticEditorSurfaceSyncActive)
                || (controller.editorSession
                    && controller.editorSession.syncingEditorTextFromModel !== undefined
                    && controller.editorSession.syncingEditorTextFromModel)) {
            return false;
        }
        const boundNoteId = controller.editorSession
                && controller.editorSession.editorBoundNoteId !== undefined
                && controller.editorSession.editorBoundNoteId !== null
                ? String(controller.editorSession.editorBoundNoteId).trim()
                : "";
        const selectedNoteId = controller.view.selectedNoteId !== undefined
                && controller.view.selectedNoteId !== null
                ? String(controller.view.selectedNoteId).trim()
                : "";
        if (boundNoteId.length > 0
                && selectedNoteId.length > 0
                && boundNoteId !== selectedNoteId) {
            return false;
        }
        const hasReadableEditorSurface = controller.contentEditor
                && (controller.contentEditor.currentPlainText !== undefined
                    || controller.contentEditor.getText !== undefined);
        if (!hasReadableEditorSurface) {
            return false;
        }
        if (!controller.contentEditor
                || !controller.plainTextSourceMutator
                || controller.plainTextSourceMutator.applyPlainTextReplacementToSource === undefined) {
            return false;
        }

        const currentSourceText = controller.view.editorText === undefined || controller.view.editorText === null
                ? ""
                : String(controller.view.editorText);
        const previousPlainText = controller.authoritativeSourcePlainText();
        const nextPlainText = controller.currentEditorPlainText();
        if (previousPlainText === nextPlainText) {
            return false;
        }

        const replacementDelta = controller.computePlainTextReplacementDelta(previousPlainText, nextPlainText);
        if (!replacementDelta.valid) {
            return false;
        }
        let normalizedInsertedText = replacementDelta.insertedText;
        let logicalReplacementStart = replacementDelta.start;
        let logicalReplacementEnd = replacementDelta.previousEnd;

        let rawSourceReplacementText = "";
        let rawReplacementEnabled = false;
        let cursorLogicalOverride = NaN;
        let cursorSourceOffsetOverride = NaN;

        const agendaTodoShortcut = controller.agendaTodoShortcutInsertion(
                    previousPlainText,
                    logicalReplacementStart,
                    logicalReplacementEnd,
                    normalizedInsertedText);
        if (agendaTodoShortcut.applied) {
            logicalReplacementStart = Math.max(0, Math.floor(Number(agendaTodoShortcut.replacementStart) || 0));
            logicalReplacementEnd = Math.max(logicalReplacementStart, Math.floor(Number(agendaTodoShortcut.replacementEnd) || 0));
            rawSourceReplacementText = String(agendaTodoShortcut.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(0, Math.floor(Number(agendaTodoShortcut.cursorSourceOffsetFromReplacementStart) || 0));
        }

        const breakShortcut = controller.breakShortcutInsertion(
                    previousPlainText,
                    logicalReplacementStart,
                    logicalReplacementEnd,
                    normalizedInsertedText);
        if (!rawReplacementEnabled && breakShortcut.applied) {
            normalizedInsertedText = breakShortcut.insertedText;
            logicalReplacementStart = Math.max(0, Math.floor(Number(breakShortcut.replacementStart) || 0));
            logicalReplacementEnd = Math.max(logicalReplacementStart, Math.floor(Number(breakShortcut.replacementEnd) || 0));
            rawSourceReplacementText = normalizedInsertedText;
            rawReplacementEnabled = true;
            cursorLogicalOverride = Math.max(0, Math.floor(Number(breakShortcut.cursorPosition) || 0));
        }
        const collapsedLogicalInsertion = logicalReplacementEnd === logicalReplacementStart;
        const sourceStart = collapsedLogicalInsertion
                ? controller.sourceOffsetForCollapsedLogicalInsertion(
                    currentSourceText,
                    logicalReplacementStart)
                : controller.sourceOffsetForLogicalOffset(logicalReplacementStart);
        const sourceEnd = collapsedLogicalInsertion
                ? sourceStart
                : controller.sourceOffsetForLogicalOffset(logicalReplacementEnd);
        const currentSourceTagRanges = controller.sourceTagRanges(currentSourceText);
        const targetsVirtualSourceOnly = controller.logicalRangeCollapsesToSingleSourceOffset(
                    logicalReplacementStart,
                    logicalReplacementEnd);

        const agendaTaskEnter = controller.agendaTaskEnterInsertion(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    normalizedInsertedText);
        let replacementSourceStart = sourceStart;
        let replacementSourceEnd = sourceEnd;
        if (agendaTaskEnter.applied) {
            replacementSourceStart = Math.max(0, Math.floor(Number(agendaTaskEnter.replacementSourceStart) || 0));
            replacementSourceEnd = Math.max(replacementSourceStart, Math.floor(Number(agendaTaskEnter.replacementSourceEnd) || 0));
            rawSourceReplacementText = String(agendaTaskEnter.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(0, Math.floor(Number(agendaTaskEnter.cursorSourceOffsetFromReplacementStart) || 0));
            cursorLogicalOverride = NaN;
        }
        const calloutEnter = agendaTaskEnter.applied
                ? ({ "applied": false })
                : controller.calloutEnterInsertion(
                    currentSourceText,
                    sourceStart,
                    sourceEnd,
                    normalizedInsertedText);
        if (calloutEnter.applied) {
            replacementSourceStart = Math.max(0, Math.floor(Number(calloutEnter.replacementSourceStart) || 0));
            replacementSourceEnd = Math.max(replacementSourceStart, Math.floor(Number(calloutEnter.replacementSourceEnd) || 0));
            rawSourceReplacementText = String(calloutEnter.replacementSourceText || "");
            rawReplacementEnabled = true;
            cursorSourceOffsetOverride = Math.max(0, Math.floor(Number(calloutEnter.cursorSourceOffsetFromReplacementStart) || 0));
            cursorLogicalOverride = NaN;
        }

        const touchesResourceTag = controller.sourceRangeTouchesNamedTag(
                    currentSourceTagRanges,
                    replacementSourceStart,
                    replacementSourceEnd,
                    "resource");
        if (touchesResourceTag || targetsVirtualSourceOnly) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
        }

        let nextSourceText = "";
        if (rawReplacementEnabled) {
            nextSourceText = controller.spliceSourceText(
                        currentSourceText,
                        replacementSourceStart,
                        replacementSourceEnd,
                        rawSourceReplacementText);
        } else {
            nextSourceText = String(controller.plainTextSourceMutator.applyPlainTextReplacementToSource(
                                        currentSourceText,
                                        sourceStart,
                                        sourceEnd,
                                        normalizedInsertedText));
            controller.applyLiveEditingStateReplacement(
                        logicalReplacementStart,
                        logicalReplacementEnd,
                        normalizedInsertedText,
                        sourceStart,
                        sourceEnd);
        }

        if (nextSourceText === currentSourceText)
            return false;
        if (controller.resourceTagLossDetectedForMutation(currentSourceText, nextSourceText)) {
            controller.restoreEditorSurfaceFromSourcePresentation();
            return false;
        }

        if (!controller.commitRawEditorTextMutation(nextSourceText))
            return false;
        const committedText = controller.committedEditorText(nextSourceText);
        if (rawReplacementEnabled) {
            if (controller.view.commitDocumentPresentationRefresh !== undefined)
                controller.view.commitDocumentPresentationRefresh();
            else
                controller.synchronizeLiveEditingStateFromPresentation();
        } else {
            controller.adoptLiveStateIntoBridge(committedText);
        }

        if (rawReplacementEnabled && isFinite(cursorSourceOffsetOverride))
            controller.scheduleCursorPosition(controller.logicalOffsetForSourceOffset(replacementSourceStart + cursorSourceOffsetOverride));
        else if (isFinite(cursorLogicalOverride))
            controller.scheduleCursorPosition(cursorLogicalOverride);

        controller.view.editorTextEdited(committedText);
        return true;
    }

    function commitRawEditorTextMutation(nextSourceText) {
        if (!controller.editorSession
                || controller.editorSession.commitRawEditorTextMutation === undefined) {
            return false;
        }
        return !!controller.editorSession.commitRawEditorTextMutation(nextSourceText);
    }

    function committedEditorText(fallbackText) {
        if (controller.editorSession && controller.editorSession.editorText !== undefined
                && controller.editorSession.editorText !== null) {
            return String(controller.editorSession.editorText);
        }
        return fallbackText === undefined || fallbackText === null ? "" : String(fallbackText);
    }

    Component.onCompleted: {
        EditorTrace.trace("typingController", "mount", "", controller)
    }

    Component.onDestruction: {
        EditorTrace.trace("typingController", "unmount", "", controller)
    }
}
    ContentsPlainTextSourceMutator {
        id: plainTextSourceMutator
    }
    ContentsResourceImportController {
        id: resourceImportController

        bodyResourceRenderer: bodyResourceRenderer
        contentEditor: contentsView.contentEditor
        documentPresentationSourceText: contentsView.documentPresentationSourceText
        editorHorizontalInset: contentsView.editorHorizontalInset
        editorProjection: editorProjection
        editorText: contentsView.editorText
        editorSession: editorSession
        editorTypingController: editorTypingController
        editorViewport: editorViewport
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
        view: contentsView
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
    ContentsStructuredBlockRenderer {
        id: structuredBlockRenderer

        backgroundRefreshEnabled: contentsView.structuredBlockBackgroundRefreshEnabled
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
    ContentsDisplayEventPump {
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
    ContentsDisplayInteractionViewModel {
        id: interactionViewModel

        contentsAgendaBackend: contentsAgendaBackend
        contentsView: contentsView
        contextMenuCoordinator: contextMenuCoordinator
        editOperationCoordinator: editOperationCoordinator
        editorInputPolicyAdapter: editorInputPolicyAdapter
        editorProjection: editorProjection
        editorSelectionController: editorSelectionController
        editorSession: editorSession
        editorTypingController: editorTypingController
        editorViewport: editorViewport
        eventPump: eventPump
        minimapLayer: minimapLayer
        panelViewModel: contentsView.panelViewModel
        presentationRefreshController: presentationRefreshController
        resourceImportController: resourceImportController
        selectionMountViewModel: selectionMountInteraction
        structuredDocumentFlow: structuredDocumentFlow
    }
    ContentsDisplayPresentationViewModel {
        id: presentationViewModel

        controller: interactionViewModel
    }
    ContentsDisplayMutationViewModel {
        id: mutationViewModel

        controller: interactionViewModel
    }
    ContentsDisplaySelectionMountInteraction {
        id: selectionMountInteraction

        contentsView: contentsView
        editorSelectionController: editorSelectionController
        editorSession: editorSession
        editorTypingController: editorTypingController
        noteBodyMountCoordinator: noteBodyMountCoordinator
        selectionBridge: selectionBridge
        selectionSyncCoordinator: selectionSyncCoordinator
        structuredDocumentFlow: structuredDocumentFlow
    }
    ContentsDisplayGeometryInteraction {
        id: geometryInteraction

        contentsView: contentsView
        eventPump: eventPump
        minimapLayer: minimapLayer
        refreshCoordinator: refreshCoordinator
        viewportCoordinator: viewportCoordinator
    }
    ContentsDisplayGeometryViewModel {
        id: geometryViewModel

        controller: geometryInteraction
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
