pragma ComponentBehavior: Bound
import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "../../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace
import "../../../../models/editor/display" as EditorDisplayModel
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
    property alias minimapSnapshotForceFullRefresh: geometryState.minimapSnapshotForceFullRefresh
    property alias cursorDrivenUiRefreshQueued: geometryState.cursorDrivenUiRefreshQueued
    property alias typingViewportCorrectionQueued: geometryState.typingViewportCorrectionQueued
    property alias typingViewportForceCorrectionRequested: geometryState.typingViewportForceCorrectionRequested
    property alias viewportGutterRefreshQueued: geometryState.viewportGutterRefreshQueued
    property alias minimapSnapshotRefreshQueued: geometryState.minimapSnapshotRefreshQueued
    property alias minimapSnapshotEntries: geometryState.minimapSnapshotEntries
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

    EditorDisplayModel.ContentsDisplayGeometryState {
        id: geometryState
    }
    EditorDisplayModel.ContentsDisplayPresentationState {
        id: presentationState
    }
    EditorDisplayModel.ContentsDisplayResourceUiState {
        id: resourceUiState
    }
    EditorDisplayModel.ContentsDisplayMountState {
        id: mountState

        editorSession: editorSession
        noteBodyMountCoordinator: noteBodyMountCoordinator
        selectionBridge: selectionBridge
    }
    EditorDisplayModel.ContentsDisplayInputState {
        id: inputState

        editorInputPolicyAdapter: editorInputPolicyAdapter
        surfacePolicy: surfacePolicy
    }
    EditorDisplayModel.ContentsDisplayGeometrySnapshotModel {
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
    EditorDisplayModel.ContentsDisplayViewportModel {
        id: viewportModel

        contentsView: contentsView
        structuredDocumentFlow: structuredDocumentFlow
        viewportCoordinator: viewportCoordinator
    }
    EditorDisplayModel.ContentsDisplayInputOrchestrationModel {
        id: inputOrchestrationModel

        contentsView: contentsView
        contextMenuCoordinator: contextMenuCoordinator
        editorSelectionController: editorSelectionController
        editorViewport: editorViewport
        resourceImportController: resourceImportController
        structuredDocumentFlow: structuredDocumentFlow
    }
    EditorDisplayModel.ContentsDisplaySelectionOrchestrationModel {
        id: selectionOrchestrationModel

        contentsView: contentsView
        editorInputPolicyAdapter: editorInputPolicyAdapter
        selectionMountViewModel: selectionMountViewModel
        structuredDocumentFlow: structuredDocumentFlow
    }
    EditorDisplayModel.ContentsDisplayPresentationOrchestrationModel {
        id: presentationOrchestrationModel

        contentsView: contentsView
        presentationViewModel: presentationViewModel
        structuredDocumentFlow: structuredDocumentFlow
    }

    function activeLogicalTextSnapshot() { return geometrySnapshotModel.activeLogicalTextSnapshot(); }
    function normalizedSnapshotEntries(rawEntries) { return geometrySnapshotModel.normalizedSnapshotEntries(rawEntries); }
    function minimapSnapshotToken(entry, fallbackIndex) { return geometrySnapshotModel.minimapSnapshotToken(entry, fallbackIndex); }
    function normalizedMinimapSnapshotText(rawText) { return geometrySnapshotModel.normalizedMinimapSnapshotText(rawText); }
    function plainMinimapSnapshotEntries(rawText) { return geometrySnapshotModel.plainMinimapSnapshotEntries(rawText); }
    function hasStructuredLogicalLineGeometry() { return geometrySnapshotModel.hasStructuredLogicalLineGeometry(); }
    function effectiveStructuredMinimapEntries() { return geometrySnapshotModel.effectiveStructuredMinimapEntries(); }
    function hasStructuredMinimapEntries() { return geometrySnapshotModel.hasStructuredMinimapEntries(); }
    function currentMinimapSnapshotEntries() { return geometrySnapshotModel.currentMinimapSnapshotEntries(); }
    function minimapSnapshotEntriesEqual(previousEntries, nextEntries) { return geometrySnapshotModel.minimapSnapshotEntriesEqual(previousEntries, nextEntries); }
    function logEditorCreationState(reason) { presentationOrchestrationModel.logEditorCreationState(reason); }
    function normalizedStructuredLogicalLineEntries() { return geometrySnapshotModel.normalizedStructuredLogicalLineEntries(); }
    function structuredLogicalLineEntryAt(lineNumber) { return geometrySnapshotModel.structuredLogicalLineEntryAt(lineNumber); }
    function effectiveStructuredLogicalLineEntries() { return geometrySnapshotModel.effectiveStructuredLogicalLineEntries(); }
    function currentStructuredGutterGeometrySignature() { return geometrySnapshotModel.currentStructuredGutterGeometrySignature(); }
    function consumeStructuredGutterGeometryChange() { return geometrySnapshotModel.consumeStructuredGutterGeometryChange(); }
    function buildStructuredMinimapLineGroupsForRange(startLineNumber, endLineNumber) { return geometrySnapshotModel.buildStructuredMinimapLineGroupsForRange(startLineNumber, endLineNumber); }
    function buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber) { return geometrySnapshotModel.buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber); }
    function buildMinimapLineGroupsForRange(startLineNumber, endLineNumber) { return geometrySnapshotModel.buildMinimapLineGroupsForRange(startLineNumber, endLineNumber); }
    function activeLineGeometryNoteId() { return geometrySnapshotModel.activeLineGeometryNoteId(); }
    function nextMinimapLineGroupsForCurrentState(currentSnapshotEntries) { return geometrySnapshotModel.nextMinimapLineGroupsForCurrentState(currentSnapshotEntries); }
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
    function shouldFlushBlurredEditorState(scheduledNoteId) { return selectionOrchestrationModel.shouldFlushBlurredEditorState(scheduledNoteId); }
    function nativeEditorCompositionActive() { return selectionOrchestrationModel.nativeEditorCompositionActive(); }
    function nativeTextInputSessionOwnsKeyboard() { return selectionOrchestrationModel.nativeTextInputSessionOwnsKeyboard(); }
    function flushEditorStateAfterInputSettles(scheduledNoteId) { selectionOrchestrationModel.flushEditorStateAfterInputSettles(scheduledNoteId); }
    function focusEditorForSelectedNoteId(noteId) { selectionOrchestrationModel.focusEditorForSelectedNoteId(noteId); }
    function focusEditorForPendingNote() { selectionOrchestrationModel.focusEditorForPendingNote(); }
    function eventRequestsPasteShortcut(event) { return inputOrchestrationModel.eventRequestsPasteShortcut(event); }
    function inlineFormatShortcutTag(event) { return inputOrchestrationModel.inlineFormatShortcutTag(event); }
    function handleInlineFormatTagShortcut(event) { return inputOrchestrationModel.handleInlineFormatTagShortcut(event); }
    function clipboardImageAvailableForPaste() { return inputOrchestrationModel.clipboardImageAvailableForPaste(); }
    function handleClipboardImagePasteShortcut(event) { return inputOrchestrationModel.handleClipboardImagePasteShortcut(event); }
    function handleTagManagementShortcutKeyPress(event) { return inputOrchestrationModel.handleTagManagementShortcutKeyPress(event); }
    function handleSelectionContextMenuEvent(eventName) { return inputOrchestrationModel.handleSelectionContextMenuEvent(eventName); }
    function commitDocumentPresentationRefresh() { presentationOrchestrationModel.commitDocumentPresentationRefresh(); }
    function documentPresentationRenderDirty() { return presentationOrchestrationModel.documentPresentationRenderDirty(); }
    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition) {
        return editorSelectionController.inferSelectionRangeFromSelectedText(selectedText, cursorPosition);
    }
    function inlineStyleWrapTags(styleTag) {
        return editorSelectionController.inlineStyleWrapTags(styleTag);
    }
    function refreshInlineResourcePresentation() { presentationOrchestrationModel.refreshInlineResourcePresentation(); }
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
    function encodeXmlAttributeValue(value) { return inputOrchestrationModel.encodeXmlAttributeValue(value); }
    function resetStructuredSelectionContextMenuSnapshot() { return inputOrchestrationModel.resetStructuredSelectionContextMenuSnapshot(); }
    function primeStructuredSelectionContextMenuSnapshot() { return inputOrchestrationModel.primeStructuredSelectionContextMenuSnapshot(); }
    function handleStructuredSelectionContextMenuEvent(eventName) { return inputOrchestrationModel.handleStructuredSelectionContextMenuEvent(eventName); }
    function openEditorSelectionContextMenu(localX, localY) { return inputOrchestrationModel.openEditorSelectionContextMenu(localX, localY); }
    function editorContextMenuPointerTriggerAccepted(triggerKind) { return inputOrchestrationModel.editorContextMenuPointerTriggerAccepted(triggerKind); }
    function requestEditorSelectionContextMenuFromPointer(localX, localY, triggerKind) { return inputOrchestrationModel.requestEditorSelectionContextMenuFromPointer(localX, localY, triggerKind); }
    function editorSelectionContextMenuSnapshotValid() { return inputOrchestrationModel.editorSelectionContextMenuSnapshotValid(); }
    function ensureEditorSelectionContextMenuSnapshot() { return inputOrchestrationModel.ensureEditorSelectionContextMenuSnapshot(); }
    function primeEditorSelectionContextMenuSnapshot() { return inputOrchestrationModel.primeEditorSelectionContextMenuSnapshot(); }
    function persistEditorTextImmediately(nextText) {
        return mutationViewModel.persistEditorTextImmediately(nextText);
    }
    function scheduleEditorEntrySnapshotReconcile() { selectionOrchestrationModel.scheduleEditorEntrySnapshotReconcile(); }
    function pollSelectedNoteSnapshot() { selectionOrchestrationModel.pollSelectedNoteSnapshot(); }
    function reconcileEditorEntrySnapshotOnce() { return selectionOrchestrationModel.reconcileEditorEntrySnapshotOnce(); }
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
    function requestViewHook(reason) { presentationOrchestrationModel.requestViewHook(reason); }
    function resetNoteEntryLineGeometryState() {
        geometryViewModel.resetNoteEntryLineGeometryState();
    }
    function resetGutterRefreshState() {
        geometryViewModel.resetGutterRefreshState();
    }
    function resetEditorSelectionCache() { selectionOrchestrationModel.resetEditorSelectionCache(); }
    function resolveEditorFlickable() {
        if (contentsView.showStructuredDocumentFlow) {
            if (contentsView.showPrintEditorLayout)
                return printDocumentViewport;
            return structuredDocumentViewport;
        }
        return null;
    }
    function scheduleEditorFocusForNote(noteId) { selectionOrchestrationModel.scheduleEditorFocusForNote(noteId); }
    function applyPresentationRefreshPlan(plan) { presentationOrchestrationModel.applyPresentationRefreshPlan(plan); }
    function executeRefreshPlan(plan) { presentationOrchestrationModel.executeRefreshPlan(plan); }
    function scheduleStructuredDocumentOpenLayoutRefresh(reason) { presentationOrchestrationModel.scheduleStructuredDocumentOpenLayoutRefresh(reason); }
    function scheduleDeferredDocumentPresentationRefresh() { presentationOrchestrationModel.scheduleDeferredDocumentPresentationRefresh(); }
    function scheduleDocumentPresentationRefresh(forceImmediate) { presentationOrchestrationModel.scheduleDocumentPresentationRefresh(forceImmediate); }
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
    function scheduleSelectionModelSync(options) { selectionOrchestrationModel.scheduleSelectionModelSync(options); }
    function executeSelectionDeliveryPlan(plan, fallbackKey) { return selectionOrchestrationModel.executeSelectionDeliveryPlan(plan, fallbackKey); }
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
    EditorInputModel.ContentsEditorTypingController {
        id: editorTypingController

        agendaBackend: contentsAgendaBackend
        calloutBackend: contentsCalloutBackend
        contentEditor: contentsView.contentEditor
        editorSession: editorSession
        plainTextSourceMutator: plainTextSourceMutator
        textMetricsBridge: editorProjection
        view: contentsView
    }
    ContentsPlainTextSourceMutator {
        id: plainTextSourceMutator
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
    EditorDisplayModel.ContentsDisplaySelectionMountController {
        id: selectionMountController

        contentsView: contentsView
        editorSelectionController: editorSelectionController
        editorSession: editorSession
        editorTypingController: editorTypingController
        noteBodyMountCoordinator: noteBodyMountCoordinator
        selectionBridge: selectionBridge
        selectionSyncCoordinator: selectionSyncCoordinator
        structuredDocumentFlow: structuredDocumentFlow
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
