pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "ContentsMinimapSnapshotSupport.js" as MinimapSnapshotSupport

Item {
    id: contentsView

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
    readonly property var contentEditor: contentEditorLoader.item ? contentEditorLoader.item : contentEditorProxy
    property alias contextMenuSelectionEnd: editorSelectionController.contextMenuSelectionEnd
    property alias contextMenuSelectionStart: editorSelectionController.contextMenuSelectionStart
    readonly property int currentCursorLineNumber: contentsView.showStructuredDocumentFlow
                                                   ? 1
                                                   : contentsView.logicalLineNumberForOffset(Number(contentEditor.cursorPosition) || 0)
    readonly property color decorativeMarkerYellow: LV.Theme.warning
    readonly property int desktopEditorFontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    property color displayColor: "transparent"
    readonly property int editorBottomInset: LV.Theme.gap16
    property alias editorBoundNoteId: editorSession.editorBoundNoteId
    readonly property real editorContentOffsetY: {
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
    readonly property int editorHorizontalInset: LV.Theme.gapNone
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
    property int gutterWidthOverride: -1
    readonly property bool hasSelectedNote: contentsView.selectedNoteId.length > 0
    property var libraryHierarchyViewModel: null
    readonly property color lineNumberColor: LV.Theme.descriptionColor
    readonly property int lineNumberColumnLeft: Math.max(0, Math.round(LV.Theme.scaleMetric(14)))
    property int lineNumberColumnLeftOverride: -1
    readonly property int lineNumberColumnTextWidth: contentsView.gutterWidth - contentsView.lineNumberColumnLeft - contentsView.lineNumberRightInset
    property int lineNumberColumnTextWidthOverride: -1
    readonly property int lineNumberColumnWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(26)))
    readonly property int lineNumberRightInset: contentsView.editorHorizontalInset
    property int liveLogicalLineCount: Math.max(1, Number(textMetricsBridge.logicalLineCount) || 1)
    readonly property int logicalLineCount: Math.max(1, Number(contentsView.liveLogicalLineCount) || 1)
    property var logicalLineDocumentYCache: []
    property int logicalLineDocumentYCacheLineCount: 0
    property int logicalLineDocumentYCacheRevision: -1
    property var liveLogicalLineStartOffsets: Array.isArray(textMetricsBridge.logicalLineStartOffsets) && textMetricsBridge.logicalLineStartOffsets.length > 0
                                           ? textMetricsBridge.logicalLineStartOffsets
                                           : [0]
    readonly property var logicalLineStartOffsets: contentsView.liveLogicalLineStartOffsets
    property int liveLogicalTextLength: textMetricsBridge.logicalText !== undefined && textMetricsBridge.logicalText !== null
                                      ? String(textMetricsBridge.logicalText).length
                                      : 0
    readonly property bool lineGeometryRefreshEnabled: !contentsView.showDedicatedResourceViewer
                                                       && !contentsView.showFormattedTextRenderer
                                                       && !contentsView.showStructuredDocumentFlow
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
    property bool minimapScrollable: false
    property bool minimapSnapshotForceFullRefresh: true
    property bool cursorDrivenUiRefreshQueued: false
    property bool minimapSnapshotRefreshQueued: false
    property string minimapSnapshotSourceText: ""
    readonly property int minimapTrackInset: LV.Theme.gap8
    readonly property int minimapTrackWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(36)))
    readonly property color minimapViewportFillColor: LV.Theme.accentTransparent
    readonly property int minimapViewportMinHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(28)))
    property bool minimapVisible: true
    readonly property bool showMinimapRail: contentsView.minimapVisible
                                           && !contentsView.showDedicatedResourceViewer
                                           && !contentsView.showPrintEditorLayout
                                           && !contentsView.showFormattedTextRenderer
                                           && !contentsView.showStructuredDocumentFlow
    readonly property bool minimapRefreshEnabled: contentsView.minimapVisible
                                                  && !contentsView.showDedicatedResourceViewer
                                                  && !contentsView.showPrintEditorLayout
                                                  && !contentsView.showFormattedTextRenderer
                                                  && !contentsView.showStructuredDocumentFlow
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
    property string editorEntrySnapshotComparedNoteId: ""
    property string editorEntrySnapshotPendingNoteId: ""
    property bool editorEntrySnapshotReconcileQueued: false
    property bool selectionModelSyncQueued: false
    property bool selectionModelSyncResetSnapshotPending: false
    property bool selectionModelSyncReconcilePending: false
    property bool selectionModelSyncFocusEditorPending: false
    property bool selectionModelSyncFallbackRefreshPending: false
    property bool selectionModelSyncForceVisualRefreshPending: false
    property string pendingNoteEntryGutterRefreshNoteId: ""
    property string structuredDocumentFlowActivatedNoteId: ""
    property string pendingEditorFocusNoteId: ""
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
    property string documentPresentationSourceText: ""
    property bool documentPresentationRefreshPendingWhileFocused: false
    property string renderedEditorText: ""
    readonly property string mobileEditorDisplayText: textMetricsBridge.logicalText
    readonly property var resolvedEditorViewModeViewModel: {
        if (contentsView.editorViewModeViewModel)
            return contentsView.editorViewModeViewModel;
        if (LV.ViewModels && LV.ViewModels.get !== undefined)
            return LV.ViewModels.get("editorViewModeViewModel");
        return null;
    }
    property bool resourceDropActive: false
    property bool resourceDropEditorSurfaceGuardActive: false
    property int resourceDropEditorSurfaceGuardToken: 0
    readonly property int resourceImportConflictPolicyAbort: 0
    readonly property int resourceImportConflictPolicyOverwrite: 1
    readonly property int resourceImportConflictPolicyKeepBoth: 2
    readonly property int resourceImportModeNone: 0
    readonly property int resourceImportModeUrls: 1
    readonly property int resourceImportModeClipboard: 2
    property var pendingResourceImportConflict: ({})
    property int pendingResourceImportMode: contentsView.resourceImportModeNone
    property var pendingResourceImportUrls: []
    property bool resourceImportConflictAlertOpen: false
    readonly property color resourceRenderBorderColor: "#334E5157"
    readonly property color resourceRenderCardColor: "#E61A1D22"
    readonly property int resourceRenderDisplayLimit: 0
    property var resourcesImportViewModel: null
    property var sidebarHierarchyViewModel: null
    readonly property string richTextHighlightOpenTag: "<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">"
    readonly property bool preferNativeInputHandling: true
    readonly property bool richTextInlineImageRenderingEnabled: false
    readonly property int resourceEditorPlaceholderLineCount: 6
    property int programmaticEditorSurfaceSyncDepth: 0
    readonly property int editorIdleSyncThresholdMs: 1000
    readonly property string selectedNoteBodyText: selectionBridge.selectedNoteBodyText
    readonly property string selectedNoteBodyNoteId: selectionBridge.selectedNoteBodyNoteId
    readonly property bool selectedNoteBodyLoading: selectionBridge.selectedNoteBodyLoading
    readonly property string selectedNoteId: selectionBridge.selectedNoteId
    readonly property bool showCurrentLineMarker: contentsView.hasSelectedNote || contentsView.editorText.length > 0 || contentsView.editorInputFocused
    readonly property bool editorSessionBoundToSelectedNote: editorSession.editorBoundNoteId === contentsView.selectedNoteId
    readonly property bool liveEditorSourceContainsResourceTag: contentsView.editorSessionBoundToSelectedNote
                                                                && contentsView.sourceContainsCanonicalResourceTag(contentsView.editorText)
    readonly property bool presentationSourceContainsResourceTag: contentsView.editorSessionBoundToSelectedNote
                                                                  && contentsView.sourceContainsCanonicalResourceTag(contentsView.documentPresentationSourceText)
    readonly property bool selectionSourceContainsResourceTag: (!editorSession.localEditorAuthority
                                                                || contentsView.resourceDropEditorSurfaceGuardActive)
                                                               && !contentsView.selectedNoteBodyLoading
                                                               && contentsView.selectedNoteBodyNoteId === contentsView.selectedNoteId
                                                               && contentsView.sourceContainsCanonicalResourceTag(contentsView.selectedNoteBodyText)
    readonly property string structuredFlowSourceText: {
        if (contentsView.liveEditorSourceContainsResourceTag)
            return contentsView.editorText;
        if (contentsView.presentationSourceContainsResourceTag)
            return contentsView.documentPresentationSourceText;
        if (contentsView.selectionSourceContainsResourceTag)
            return contentsView.selectedNoteBodyText;
        return contentsView.editorText;
    }
    readonly property bool liveResourceStructuredFlowRequested: contentsView.liveEditorSourceContainsResourceTag
                                                                || contentsView.presentationSourceContainsResourceTag
                                                                || contentsView.selectionSourceContainsResourceTag
    readonly property bool parsedStructuredFlowRequested: contentsView.editorSessionBoundToSelectedNote
                                                          && structuredBlockRenderer.hasRenderedBlocks
    readonly property bool structuredDocumentFlowEnabled: contentsView.parsedStructuredFlowRequested
                                                          || (contentsView.editorSessionBoundToSelectedNote
                                                              && structuredBlockRenderer.renderPending
                                                              && contentsView.structuredDocumentFlowActivatedNoteId === contentsView.selectedNoteId
                                                              && contentsView.selectedNoteId.length > 0)
    readonly property bool resourceResolverNeedsLiveEditorSource: contentsView.showStructuredDocumentFlow
                                                                  || contentsView.liveResourceStructuredFlowRequested
    readonly property bool legacyInlineEditorActive: !contentsView.showStructuredDocumentFlow
                                                     && !contentsView.showDedicatedResourceViewer
                                                     && !contentsView.showFormattedTextRenderer
    readonly property bool resourceBlocksRenderedInlineByRichTextEditor: contentsView.legacyInlineEditorActive
                                                                        && !contentsView.preferNativeInputHandling
    readonly property bool programmaticEditorSurfaceSyncActive: contentsView.programmaticEditorSurfaceSyncDepth > 0
    readonly property bool showDedicatedResourceViewer: false
    readonly property bool showEditorGutter: false
    readonly property bool showFormattedTextRenderer: false
    readonly property bool showStructuredDocumentFlow: contentsView.structuredDocumentFlowEnabled
                                                       && !contentsView.showDedicatedResourceViewer
                                                       && !contentsView.showFormattedTextRenderer
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

    function applyEditorRichTextSurface() {
        editorSelectionController.applyEditorRichTextSurface();
    }
    function activeLogicalTextSnapshot() {
        if (editorTypingController && editorTypingController.currentEditorPlainText !== undefined) {
            const livePlainText = editorTypingController.currentEditorPlainText();
            if (livePlainText !== undefined && livePlainText !== null)
                return String(livePlainText);
        }
        if (textMetricsBridge && textMetricsBridge.logicalText !== undefined && textMetricsBridge.logicalText !== null)
            return String(textMetricsBridge.logicalText);
        return "";
    }
    function buildFallbackMinimapLineGroupsForRange(startLineNumber, endLineNumber) {
        const groups = [];
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLineNumber) || 1));
        const safeEndLine = Math.max(safeStartLine, Math.min(contentsView.logicalLineCount, Number(endLineNumber) || safeStartLine));
        const textStartY = contentsView.editorDocumentStartY;
        for (let lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber) {
            const contentHeight = Math.max(1, contentsView.lineVisualHeight(lineNumber, 1));
            groups.push({
                "charCount": contentsView.logicalLineCharacterCountAt(lineNumber - 1),
                "contentHeight": contentHeight,
                "contentY": textStartY + contentsView.lineDocumentY(lineNumber),
                "lineNumber": lineNumber,
                "rowCount": Math.max(1, Math.round(contentHeight / Math.max(1, contentsView.editorLineHeight)))
            });
        }
        if (groups.length === 0) {
            groups.push({
                "charCount": 0,
                "contentHeight": contentsView.editorLineHeight,
                "contentY": textStartY,
                "lineNumber": 1,
                "rowCount": 1
            });
        }
        return groups;
    }
    function buildMinimapLineGroupsForRange(startLineNumber, endLineNumber) {
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLineNumber) || 1));
        const safeEndLine = Math.max(safeStartLine, Math.min(contentsView.logicalLineCount, Number(endLineNumber) || safeStartLine));
        const editorItem = contentEditor ? contentEditor.editorItem : null;
        const editorWidth = Number(contentEditor ? contentEditor.width : 0) || 0;
        const editorContentHeight = Number(contentEditor ? contentEditor.contentHeight : 0) || 0;
        if (!editorItem
                || editorItem.positionToRectangle === undefined
                || editorWidth <= 0
                || editorContentHeight <= 0) {
            return contentsView.buildFallbackMinimapLineGroupsForRange(safeStartLine, safeEndLine);
        }

        const groups = [];
        const textStartY = contentsView.editorDocumentStartY;
        const logicalLength = contentsView.logicalTextLength();
        let currentStartOffset = contentsView.logicalLineStartOffsetAt(safeStartLine - 1);
        let currentRect = editorItem.positionToRectangle(currentStartOffset);
        for (let lineNumber = safeStartLine; lineNumber <= safeEndLine; ++lineNumber) {
            const safeCurrentRectY = Number(currentRect.y);
            const currentRectY = isFinite(safeCurrentRectY) ? safeCurrentRectY : Math.max(0, contentsView.lineDocumentY(lineNumber));
            const nextStartOffset = lineNumber < contentsView.logicalLineCount
                    ? contentsView.logicalLineStartOffsetAt(lineNumber)
                    : Math.max(0, logicalLength);
            const nextRect = editorItem.positionToRectangle(nextStartOffset);
            const safeNextRectY = Number(nextRect.y);
            const safeNextRectHeight = Number(nextRect.height);
            let contentHeight = contentsView.editorLineHeight;
            if (lineNumber < contentsView.logicalLineCount) {
                const nextRectY = isFinite(safeNextRectY) ? safeNextRectY : currentRectY + contentsView.editorLineHeight;
                contentHeight = Math.max(contentsView.editorLineHeight, nextRectY - currentRectY);
            } else {
                const nextRectY = isFinite(safeNextRectY) ? safeNextRectY : currentRectY;
                const nextRectHeight = isFinite(safeNextRectHeight) ? safeNextRectHeight : contentsView.editorLineHeight;
                contentHeight = Math.max(contentsView.editorLineHeight, nextRectY + Math.max(contentsView.editorLineHeight, nextRectHeight) - currentRectY);
            }

            groups.push({
                "charCount": contentsView.logicalLineCharacterCountAt(lineNumber - 1),
                "contentHeight": contentHeight,
                "contentY": textStartY + currentRectY,
                "lineNumber": lineNumber,
                "rowCount": Math.max(1, Math.round(contentHeight / Math.max(1, contentsView.editorLineHeight)))
            });
            currentStartOffset = nextStartOffset;
            currentRect = nextRect;
        }

        return groups.length > 0 ? groups : contentsView.buildFallbackMinimapLineGroupsForRange(safeStartLine, safeEndLine);
    }
    function currentMinimapSourceText() {
        return contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText);
    }
    function nextMinimapLineGroupsForCurrentState(currentSourceText) {
        if (contentsView.minimapSnapshotForceFullRefresh
                || !Array.isArray(contentsView.minimapLineGroups)
                || contentsView.minimapLineGroups.length !== contentsView.logicalLineCount
                || contentsView.minimapLineGroups.length === 0
                || contentsView.minimapSnapshotSourceText.length === 0) {
            return contentsView.buildMinimapLineGroupsForRange(1, contentsView.logicalLineCount);
        }

        const changeRange = MinimapSnapshotSupport.computeChangedLineRange(contentsView.minimapSnapshotSourceText, currentSourceText);
        if (!changeRange.valid)
            return contentsView.minimapLineGroups;

        const replacementGroups = contentsView.buildMinimapLineGroupsForRange(changeRange.nextStartLine, changeRange.nextEndLine);
        const mergedGroups = MinimapSnapshotSupport.spliceLineGroups(
                    contentsView.minimapLineGroups,
                    replacementGroups,
                    changeRange.previousStartLine,
                    changeRange.previousEndLine);
        if (!Array.isArray(mergedGroups) || mergedGroups.length !== contentsView.logicalLineCount)
            return contentsView.buildMinimapLineGroupsForRange(1, contentsView.logicalLineCount);
        return mergedGroups;
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
    function clearPendingResourceImportConflict() {
        contentsView.pendingResourceImportConflict = ({});
        contentsView.pendingResourceImportMode = contentsView.resourceImportModeNone;
        contentsView.pendingResourceImportUrls = [];
        contentsView.resourceImportConflictAlertOpen = false;
    }
    function normalizedResourceImportConflict(conflict) {
        return conflict && typeof conflict === "object" ? conflict : ({});
    }
    function resourceImportConflictAlertMessage() {
        const conflict = contentsView.normalizedResourceImportConflict(contentsView.pendingResourceImportConflict);
        const fileName = conflict.sourceFileName !== undefined ? String(conflict.sourceFileName).trim() : "";
        const resourcePath = conflict.existingResourcePath !== undefined ? String(conflict.existingResourcePath).trim() : "";
        if (fileName.length === 0)
            return "A resource with the same name already exists. Choose how to continue.";
        if (resourcePath.length === 0)
            return "A resource named \"" + fileName + "\" already exists. Choose whether to overwrite it, keep both copies, or cancel the import.";
        return "A resource named \"" + fileName + "\" already exists at \"" + resourcePath + "\". Choose whether to overwrite it, keep both copies, or cancel the import.";
    }
    function scheduleResourceImportConflictPrompt(importMode, urls, conflict) {
        contentsView.activateResourceDropEditorSurfaceGuard();
        contentsView.pendingResourceImportMode = importMode;
        contentsView.pendingResourceImportUrls = Array.isArray(urls) ? urls.slice(0) : [];
        contentsView.pendingResourceImportConflict = contentsView.normalizedResourceImportConflict(conflict);
        contentsView.resourceDropActive = false;
        contentsView.resourceImportConflictAlertOpen = true;
        return true;
    }
    function finalizeInsertedImportedResources(importedEntries) {
        const importedEntryCount = contentsView.normalizedImportedResourceEntries(importedEntries).length;
        const inserted = contentsView.insertImportedResourceTags(importedEntries);
        if (importedEntryCount > 0
                && contentsView.resourcesImportViewModel
                && contentsView.resourcesImportViewModel.reloadImportedResources !== undefined) {
            contentsView.resourcesImportViewModel.reloadImportedResources();
        }
        contentsView.releaseResourceDropEditorSurfaceGuard(inserted);
        contentsView.resourceDropActive = false;
        contentsView.clearPendingResourceImportConflict();
        return inserted;
    }
    function cancelPendingResourceImportConflict() {
        contentsView.releaseResourceDropEditorSurfaceGuard(false);
        contentsView.resourceDropActive = false;
        contentsView.clearPendingResourceImportConflict();
    }
    function executePendingResourceImportWithPolicy(conflictPolicy) {
        if (!contentsView.resourcesImportViewModel) {
            contentsView.cancelPendingResourceImportConflict();
            return false;
        }

        contentsView.resourceImportConflictAlertOpen = false;
        let importedEntries = [];
        if (contentsView.pendingResourceImportMode === contentsView.resourceImportModeUrls) {
            if (contentsView.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy === undefined) {
                contentsView.cancelPendingResourceImportConflict();
                return false;
            }
            importedEntries = contentsView.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy(
                        contentsView.pendingResourceImportUrls,
                        conflictPolicy);
        } else if (contentsView.pendingResourceImportMode === contentsView.resourceImportModeClipboard) {
            if (contentsView.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy === undefined) {
                contentsView.cancelPendingResourceImportConflict();
                return false;
            }
            importedEntries = contentsView.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy(
                        conflictPolicy);
        } else {
            contentsView.cancelPendingResourceImportConflict();
            return false;
        }

        return contentsView.finalizeInsertedImportedResources(importedEntries);
    }
    function importUrlsAsResourcesWithPrompt(urls) {
        if (!contentsView.resourcesImportViewModel
                || contentsView.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy === undefined) {
            return false;
        }

        const conflict = contentsView.resourcesImportViewModel.inspectImportConflictForUrls !== undefined
                ? contentsView.resourcesImportViewModel.inspectImportConflictForUrls(urls)
                : ({});
        if (conflict && conflict.conflict)
            return contentsView.scheduleResourceImportConflictPrompt(contentsView.resourceImportModeUrls, urls, conflict);

        contentsView.activateResourceDropEditorSurfaceGuard();
        const importedEntries = contentsView.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy(
                    urls,
                    contentsView.resourceImportConflictPolicyAbort);
        return contentsView.finalizeInsertedImportedResources(importedEntries);
    }
    function clampUnit(value) {
        return Math.max(0, Math.min(1, Number(value) || 0));
    }
    function consumePendingNoteEntryGutterRefresh(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length > 0
                && contentsView.pendingNoteEntryGutterRefreshNoteId === normalizedNoteId) {
            contentsView.pendingNoteEntryGutterRefreshNoteId = "";
            contentsView.resetGutterRefreshState();
            return true;
        }
        return false;
    }
    function commitGutterRefresh() {
        contentsView.refreshLiveLogicalLineMetrics();
        contentsView.gutterRefreshRevision += 1;
        contentsView.refreshMinimapSnapshot();
        contentsView.visibleGutterLineEntries = contentsView.buildVisibleGutterLineEntries();
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
        return Array.isArray(contentsView.minimapLineGroups)
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
    function appendResourceDropPayloadLines(rawText, urls) {
        if (!Array.isArray(urls))
            return;
        const normalizedText = rawText === undefined || rawText === null ? "" : String(rawText).trim();
        if (normalizedText.length === 0)
            return;
        const payloadLines = normalizedText.split(/\r?\n|\u0000/g);
        for (let lineIndex = 0; lineIndex < payloadLines.length; ++lineIndex) {
            const line = String(payloadLines[lineIndex] || "").trim();
            if (line.length === 0 || line.charAt(0) === "#")
                continue;
            urls.push(line);
        }
    }
    function appendResourceDropMimePayload(drop, mimeType, urls) {
        if (!drop
                || drop.getDataAsString === undefined
                || !Array.isArray(urls))
            return;
        contentsView.appendResourceDropPayloadLines(drop.getDataAsString(mimeType), urls);
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
        contentsView.appendResourceDropPayloadLines(drop && drop.text !== undefined ? drop.text : "", urls);
        const mimeTypes = [
            "text/uri-list",
            "text/plain",
            "public.file-url",
            "public.url",
            "text/x-moz-url"
        ];
        for (let index = 0; index < mimeTypes.length; ++index)
            contentsView.appendResourceDropMimePayload(drop, mimeTypes[index], urls);
        return urls;
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
            if (contentEditor.setCursorPositionPreservingInputMethod !== undefined)
                contentEditor.setCursorPositionPreservingInputMethod(cursorPosition);
            else if (contentEditor.cursorPosition !== undefined)
                contentEditor.cursorPosition = cursorPosition;
        });
    }
    function handleInlineFormatShortcutKeyPress(event) {
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
        editorSelectionController.handleSelectionContextMenuEvent(eventName);
    }
    function commitDocumentPresentationRefresh() {
        const currentSourceText = contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText);
        if (contentsView.documentPresentationSourceText !== currentSourceText)
            contentsView.documentPresentationSourceText = currentSourceText;
        const presentationSourceText = contentsView.documentPresentationSourceText;
        if (textMetricsBridge && textMetricsBridge.text !== undefined && textMetricsBridge.text !== presentationSourceText)
            textMetricsBridge.text = presentationSourceText;
        const needsRichTextProjection = !contentsView.preferNativeInputHandling || contentsView.showFormattedTextRenderer;
        if (!needsRichTextProjection) {
            if (contentsView.renderedEditorText !== "")
                contentsView.renderedEditorText = "";
            contentsView.scheduleMinimapSnapshotRefresh(false);
            if (minimapLayer && contentsView.minimapRefreshEnabled)
                minimapLayer.requestRepaint();
            editorTypingController.synchronizeLiveEditingStateFromPresentation();
            return;
        }
        if (textFormatRenderer && textFormatRenderer.sourceText !== undefined && textFormatRenderer.sourceText !== presentationSourceText)
            textFormatRenderer.sourceText = presentationSourceText;
        const nextRenderedText = textFormatRenderer && textFormatRenderer.editorSurfaceHtml !== undefined && textFormatRenderer.editorSurfaceHtml !== null
                ? String(textFormatRenderer.editorSurfaceHtml)
                : "";
        const editorRenderedText = contentsView.resourceBlocksRenderedInlineByRichTextEditor
                ? contentsView.renderEditorSurfaceHtmlWithInlineResources(nextRenderedText)
                : nextRenderedText;
        if (contentsView.renderedEditorText !== editorRenderedText) {
            contentsView.markProgrammaticEditorSurfaceSync();
            contentsView.renderedEditorText = editorRenderedText;
        }
        contentsView.scheduleEditorRichTextSurfaceSync();
        contentsView.scheduleMinimapSnapshotRefresh(false);
        if (minimapLayer && contentsView.minimapRefreshEnabled)
            minimapLayer.requestRepaint();
        editorTypingController.synchronizeLiveEditingStateFromPresentation();
    }
    function documentPresentationRenderDirty() {
        const currentSourceText = contentsView.editorText === undefined || contentsView.editorText === null ? "" : String(contentsView.editorText);
        const presentationSourceText = contentsView.documentPresentationSourceText === undefined || contentsView.documentPresentationSourceText === null
                ? ""
                : String(contentsView.documentPresentationSourceText);
        if (presentationSourceText !== currentSourceText)
            return true;
        const needsRichTextProjection = !contentsView.preferNativeInputHandling || contentsView.showFormattedTextRenderer;
        if (!needsRichTextProjection)
            return false;
        const rendererSourceText = textFormatRenderer && textFormatRenderer.sourceText !== undefined && textFormatRenderer.sourceText !== null
                ? String(textFormatRenderer.sourceText)
                : "";
        if (rendererSourceText !== presentationSourceText)
            return true;
        const rendererRenderedText = textFormatRenderer && textFormatRenderer.editorSurfaceHtml !== undefined && textFormatRenderer.editorSurfaceHtml !== null
                ? String(textFormatRenderer.editorSurfaceHtml)
                : "";
        const expectedRenderedText = contentsView.resourceBlocksRenderedInlineByRichTextEditor
                ? contentsView.renderEditorSurfaceHtmlWithInlineResources(rendererRenderedText)
                : rendererRenderedText;
        return contentsView.renderedEditorText !== expectedRenderedText;
    }
    function inferSelectionRangeFromSelectedText(selectedText, cursorPosition) {
        return editorSelectionController.inferSelectionRangeFromSelectedText(selectedText, cursorPosition);
    }
    function inlineStyleWrapTags(styleTag) {
        return editorSelectionController.inlineStyleWrapTags(styleTag);
    }
    function normalizedImportedResourceEntries(importedEntries) {
        if (Array.isArray(importedEntries))
            return importedEntries;
        if (!importedEntries)
            return [];

        const explicitLength = Number(importedEntries.length);
        if (isFinite(explicitLength) && explicitLength >= 0) {
            const normalizedEntries = [];
            for (let index = 0; index < Math.floor(explicitLength); ++index)
                normalizedEntries.push(importedEntries[index]);
            return normalizedEntries;
        }

        const explicitCount = Number(importedEntries.count);
        if (isFinite(explicitCount) && explicitCount >= 0) {
            const normalizedEntries = [];
            for (let index = 0; index < Math.floor(explicitCount); ++index)
                normalizedEntries.push(importedEntries[index]);
            return normalizedEntries;
        }

        const indexedKeys = Object.keys(importedEntries).filter(function (key) {
            return /^\d+$/.test(key);
        }).sort(function (lhs, rhs) {
            return Number(lhs) - Number(rhs);
        });
        if (indexedKeys.length === 0)
            return [];

        const normalizedEntries = [];
        for (let index = 0; index < indexedKeys.length; ++index)
            normalizedEntries.push(importedEntries[indexedKeys[index]]);
        return normalizedEntries;
    }
    function resourceBlockSourceText(tagTexts) {
        if (!Array.isArray(tagTexts) || tagTexts.length === 0)
            return "";
        const blockSourceText = tagTexts.join("\n");
        const currentSourceText = contentsView.editorText !== undefined && contentsView.editorText !== null
                ? String(contentsView.editorText)
                : "";
        const cursorSourceOffset = editorTypingController.sourceOffsetForLogicalOffset(
                    contentsView.currentEditorCursorPosition());
        const boundedCursorOffset = Math.max(0, Math.min(currentSourceText.length, Number(cursorSourceOffset) || 0));
        const previousCharacter = boundedCursorOffset > 0 ? currentSourceText.charAt(boundedCursorOffset - 1) : "";
        const nextCharacter = boundedCursorOffset < currentSourceText.length ? currentSourceText.charAt(boundedCursorOffset) : "";
        const leadingBreak = currentSourceText.length > 0 && previousCharacter !== "\n" ? "\n" : "";
        const trailingBreak = nextCharacter !== "\n" ? "\n" : "";
        return leadingBreak + blockSourceText + trailingBreak;
    }
    function sourceContainsCanonicalResourceTag(sourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null
                ? ""
                : String(sourceText);
        if (normalizedSourceText.length === 0)
            return false;
        return /<resource\b[^>]*\/?>/i.test(normalizedSourceText);
    }
    function canonicalResourceTagCount(sourceText) {
        const normalizedSourceText = sourceText === undefined || sourceText === null
                ? ""
                : String(sourceText);
        if (normalizedSourceText.length === 0)
            return 0;
        const matches = normalizedSourceText.match(/<resource\b[^>]*\/?>/gi);
        return matches ? matches.length : 0;
    }
    function resourceTagLossDetected(previousSourceText, nextSourceText) {
        if (contentsView.showStructuredDocumentFlow)
            return false;
        const selectedBodyCount = (!editorSession.localEditorAuthority
                                   && contentsView.selectedNoteBodyNoteId === contentsView.selectedNoteId)
                ? contentsView.canonicalResourceTagCount(contentsView.selectedNoteBodyText)
                : 0;
        const baselineCount = Math.max(
                    contentsView.canonicalResourceTagCount(previousSourceText),
                    contentsView.canonicalResourceTagCount(contentsView.documentPresentationSourceText),
                    selectedBodyCount);
        return baselineCount > contentsView.canonicalResourceTagCount(nextSourceText);
    }
    function inlineResourcePreviewWidth() {
        if (contentsView.showPrintEditorLayout)
            return Math.max(120, Math.floor(contentsView.printPaperTextWidth));
        const editorWidth = contentEditor && contentEditor.width !== undefined
                ? Number(contentEditor.width) || 0
                : 0;
        const viewportWidth = editorViewport ? Number(editorViewport.width) || 0 : 0;
        const availableWidth = Math.max(editorWidth, viewportWidth) - contentsView.editorHorizontalInset * 2;
        return Math.max(120, Math.min(480, Math.floor(Math.max(120, availableWidth))));
    }
    function resourceEntryOpenTarget(resourceEntry) {
        const safeEntry = resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({});
        const sourceUrl = safeEntry.source !== undefined ? String(safeEntry.source).trim() : "";
        if (sourceUrl.length > 0)
            return sourceUrl;
        const resolvedPath = safeEntry.resolvedPath !== undefined ? String(safeEntry.resolvedPath).trim() : "";
        return resolvedPath;
    }
    function richTextParagraphHtml(innerHtml) {
        const paragraphBody = innerHtml === undefined || innerHtml === null || String(innerHtml).length === 0
                ? "&nbsp;"
                : String(innerHtml);
        return "<p style=\"margin-top:0px;margin-bottom:0px;\">" + paragraphBody + "</p>";
    }
    function inlineResourcePlaceholderHtml(lineCount) {
        const lines = [];
        const placeholderLineCount = Math.max(0, Math.floor(Number(lineCount) || 0));
        for (let index = 0; index < placeholderLineCount; ++index)
            lines.push(contentsView.richTextParagraphHtml("&nbsp;"));
        return lines.join("");
    }
    function resourcePlaceholderBlockHtml() {
        return contentsView.inlineResourcePlaceholderHtml(contentsView.resourceEditorPlaceholderLineCount);
    }
    function inlineResourceBlockHtml(resourceEntry) {
        const safeEntry = resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({});
        const renderMode = safeEntry.renderMode !== undefined ? String(safeEntry.renderMode).trim().toLowerCase() : "";
        const sourceUrl = contentsView.resourceEntryOpenTarget(safeEntry);
        const encodedSourceUrl = contentsView.encodeXmlAttributeValue(sourceUrl);
        const previewWidth = contentsView.inlineResourcePreviewWidth();
        if (renderMode === "image" && encodedSourceUrl.length > 0) {
            return "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" style=\"margin-top:0px;margin-bottom:0px;\">"
                    + "<tr><td align=\"center\">"
                    + "<img src=\"" + encodedSourceUrl + "\" width=\"" + String(previewWidth) + "\" />"
                    + "</td></tr></table>";
        }
        return contentsView.resourcePlaceholderBlockHtml();
    }
    function resourceEntryCanRenderInlineInRichText(resourceEntry) {
        const safeEntry = resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({});
        const renderMode = safeEntry.renderMode !== undefined ? String(safeEntry.renderMode).trim().toLowerCase() : "";
        const sourceUrl = contentsView.resourceEntryOpenTarget(safeEntry);
        return contentsView.richTextInlineImageRenderingEnabled
                && renderMode === "image"
                && sourceUrl.length > 0;
    }
    function renderEditorSurfaceHtmlWithInlineResources(editorHtml) {
        const baseEditorHtml = editorHtml === undefined || editorHtml === null ? "" : String(editorHtml);
        const renderedResources = contentsView.normalizedImportedResourceEntries(bodyResourceRenderer.renderedResources);
        return baseEditorHtml.replace(
                    /<!--whatson-resource-block:(\d+)-->[\s\S]*?<!--\/whatson-resource-block:\1-->/g,
                    function (_match, resourceIndexText) {
                        const resourceIndex = Math.max(0, Math.floor(Number(resourceIndexText) || 0));
                        const entry = resourceIndex < renderedResources.length ? renderedResources[resourceIndex] : null;
                        // Resolved image resources now become real RichText document blocks.
                        // Unresolved or non-image resources still keep a reserved placeholder slot.
                        return contentsView.resourceEntryCanRenderInlineInRichText(entry)
                                ? contentsView.inlineResourceBlockHtml(entry)
                                : contentsView.resourcePlaceholderBlockHtml();
                    });
    }
    function refreshInlineResourcePresentation() {
        if (!contentsView.resourceBlocksRenderedInlineByRichTextEditor)
            return;
        const rendererRenderedText = textFormatRenderer && textFormatRenderer.editorSurfaceHtml !== undefined && textFormatRenderer.editorSurfaceHtml !== null
                ? String(textFormatRenderer.editorSurfaceHtml)
                : "";
        const nextRenderedText = contentsView.renderEditorSurfaceHtmlWithInlineResources(rendererRenderedText);
        if (contentsView.renderedEditorText !== nextRenderedText) {
            contentsView.markProgrammaticEditorSurfaceSync();
            contentsView.renderedEditorText = nextRenderedText;
        }
        contentsView.scheduleEditorRichTextSurfaceSync();
    }
    function activateResourceDropEditorSurfaceGuard() {
        contentsView.resourceDropEditorSurfaceGuardToken += 1;
        if (!contentsView.resourceDropEditorSurfaceGuardActive)
            contentsView.resourceDropEditorSurfaceGuardActive = true;
    }
    function markProgrammaticEditorSurfaceSync() {
        contentsView.programmaticEditorSurfaceSyncDepth += 1;
        Qt.callLater(function () {
            Qt.callLater(function () {
                contentsView.programmaticEditorSurfaceSyncDepth = Math.max(
                            0,
                            (Number(contentsView.programmaticEditorSurfaceSyncDepth) || 0) - 1);
            });
        });
    }
    function restoreEditorSurfaceFromPresentation() {
        if (!contentEditor)
            return;
        const nextSurfaceText = contentsView.preferNativeInputHandling
                ? String(textMetricsBridge.logicalText === undefined || textMetricsBridge.logicalText === null
                             ? ""
                             : textMetricsBridge.logicalText)
                : contentsView.renderedEditorText;
        contentsView.markProgrammaticEditorSurfaceSync();
        if (contentEditor.setProgrammaticText !== undefined) {
            contentEditor.setProgrammaticText(nextSurfaceText);
            return;
        }
        if (contentEditor.text !== undefined && contentEditor.text !== nextSurfaceText)
            contentEditor.text = nextSurfaceText;
    }
    function releaseResourceDropEditorSurfaceGuard(restoreSurface) {
        const scheduledToken = Math.max(0, Number(contentsView.resourceDropEditorSurfaceGuardToken) || 0);
        if (!restoreSurface) {
            contentsView.resourceDropEditorSurfaceGuardActive = false;
            return;
        }
        Qt.callLater(function () {
            Qt.callLater(function () {
                if (contentsView.resourceDropEditorSurfaceGuardToken !== scheduledToken)
                    return;
                contentsView.restoreEditorSurfaceFromPresentation();
                Qt.callLater(function () {
                    if (contentsView.resourceDropEditorSurfaceGuardToken !== scheduledToken)
                        return;
                    contentsView.resourceDropEditorSurfaceGuardActive = false;
                });
            });
        });
    }
    function insertImportedResourceTags(importedEntries) {
        const normalizedImportedEntries = contentsView.normalizedImportedResourceEntries(importedEntries);
        if (normalizedImportedEntries.length === 0)
            return false;
        const tagTexts = [];
        for (let index = 0; index < normalizedImportedEntries.length; ++index) {
            const tagText = contentsView.resourceTagTextForImportedEntry(normalizedImportedEntries[index]);
            if (tagText.length > 0)
                tagTexts.push(tagText);
        }
        if (tagTexts.length === 0)
            return false;
        let inserted = false;
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.insertResourceBlocksAtActivePosition !== undefined) {
            inserted = structuredDocumentFlow.insertResourceBlocksAtActivePosition(tagTexts);
        } else {
            const insertedSourceText = contentsView.resourceBlockSourceText(tagTexts);
            inserted = editorTypingController.insertRawSourceTextAtCursor(
                        insertedSourceText,
                        insertedSourceText.length);
        }
        if (!inserted)
            return false;
        bodyResourceRenderer.requestRenderRefresh();
        return inserted;
    }
    function pasteClipboardImageAsResource() {
        if (!contentsView.hasSelectedNote
                || contentsView.showDedicatedResourceViewer
                || contentsView.showFormattedTextRenderer) {
            return false;
        }
        if (!contentsView.resourcesImportViewModel
                || contentsView.resourcesImportViewModel.importClipboardImageForEditor === undefined) {
            return false;
        }
        if (contentsView.resourcesImportViewModel.busy !== undefined
                && contentsView.resourcesImportViewModel.busy) {
            return false;
        }
        if (contentsView.resourcesImportViewModel.clipboardImageAvailable !== undefined
                && !contentsView.resourcesImportViewModel.clipboardImageAvailable) {
            return false;
        }

        const conflict = contentsView.resourcesImportViewModel.inspectClipboardImageImportConflict !== undefined
                ? contentsView.resourcesImportViewModel.inspectClipboardImageImportConflict()
                : ({});
        if (conflict && conflict.conflict)
            return contentsView.scheduleResourceImportConflictPrompt(contentsView.resourceImportModeClipboard, [], conflict);

        if (contentsView.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy === undefined)
            return false;

        contentsView.activateResourceDropEditorSurfaceGuard();
        const importedEntries = contentsView.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy(
                    contentsView.resourceImportConflictPolicyAbort);
        return contentsView.finalizeInsertedImportedResources(importedEntries);
    }
    function isMinimapScrollable() {
        const flickable = contentsView.editorFlickable;
        if (!flickable)
            return false;
        return contentsView.minimapContentHeight() > (Number(flickable.height) || 0);
    }
    function lineDocumentY(lineNumber) {
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
    function lineVisualHeight(startLine, lineSpan) {
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLine) || 1));
        const safeLineSpan = Math.max(1, Number(lineSpan) || 1);
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
    function encodeXmlAttributeValue(value) {
        let encodedValue = value === undefined || value === null ? "" : String(value);
        encodedValue = encodedValue.replace(/&/g, "&amp;");
        encodedValue = encodedValue.replace(/"/g, "&quot;");
        encodedValue = encodedValue.replace(/'/g, "&apos;");
        encodedValue = encodedValue.replace(/</g, "&lt;");
        encodedValue = encodedValue.replace(/>/g, "&gt;");
        return encodedValue;
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
    function primeEditorSelectionContextMenuSnapshot() {
        return editorSelectionController.primeContextMenuSelectionSnapshot();
    }
    function persistEditorTextImmediately(nextText) {
        return editorSelectionController.persistEditorTextImmediately(nextText);
    }
    function scheduleEditorEntrySnapshotReconcile() {
        if (contentsView.editorEntrySnapshotReconcileQueued)
            return;
        contentsView.editorEntrySnapshotReconcileQueued = true;
        Qt.callLater(function () {
            contentsView.editorEntrySnapshotReconcileQueued = false;
            contentsView.reconcileEditorEntrySnapshotOnce();
        });
    }
    function pollSelectedNoteSnapshot() {
        if (contentsView.typingSessionSyncProtected || contentsView.pendingBodySave)
            return;
        if (!contentsView.editorSessionBoundToSelectedNote)
            return;
        const normalizedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null
                ? ""
                : String(contentsView.selectedNoteId).trim();
        if (normalizedNoteId.length > 0
                && contentsView.selectedNoteBodyNoteId !== normalizedNoteId)
            return;
        if (contentsView.editorEntrySnapshotPendingNoteId === normalizedNoteId)
            return;
        if (selectionBridge
                && selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote !== undefined
                && normalizedNoteId.length > 0) {
            const sessionText = editorSession.editorText === undefined || editorSession.editorText === null
                    ? ""
                    : String(editorSession.editorText);
            if (selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote(
                        normalizedNoteId,
                        sessionText)) {
                contentsView.editorEntrySnapshotPendingNoteId = normalizedNoteId;
                return;
            }
        }
        if (!selectionBridge || selectionBridge.refreshSelectedNoteSnapshot === undefined)
            return;
        selectionBridge.refreshSelectedNoteSnapshot();
        contentsView.scheduleGutterRefresh(2);
    }
    function reconcileEditorEntrySnapshotOnce() {
        if (!contentsView.visible || !contentsView.hasSelectedNote)
            return false;
        if (!contentsView.editorSessionBoundToSelectedNote)
            return false;
        const normalizedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null
                ? ""
                : String(contentsView.selectedNoteId).trim();
        if (normalizedNoteId.length === 0)
            return false;
        if (contentsView.selectedNoteBodyNoteId !== normalizedNoteId)
            return false;
        if (contentsView.editorEntrySnapshotComparedNoteId === normalizedNoteId)
            return false;
        if (contentsView.editorEntrySnapshotPendingNoteId === normalizedNoteId)
            return false;
        if (contentsView.editorInputFocused || contentsView.pendingBodySave || contentsView.typingSessionSyncProtected)
            return false;
        if (!selectionBridge
                || selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote === undefined) {
            return false;
        }
        const sessionText = editorSession.editorText === undefined || editorSession.editorText === null
                ? ""
                : String(editorSession.editorText);
        if (!selectionBridge.reconcileViewSessionAndRefreshSnapshotForNote(
                    normalizedNoteId,
                    sessionText))
            return false;
        contentsView.editorEntrySnapshotPendingNoteId = normalizedNoteId;
        return true;
    }
    function queueInlineFormatWrap(tagName) {
        return editorSelectionController.queueInlineFormatWrap(tagName);
    }
    function queueMarkdownListMutation(listKind) {
        return editorSelectionController.queueMarkdownListMutation(listKind);
    }
    function queueAgendaShortcutInsertion() {
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            return structuredDocumentFlow.insertStructuredShortcutAtActivePosition("agenda");
        }
        return editorTypingController.queueAgendaShortcutInsertion();
    }
    function queueCalloutShortcutInsertion() {
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            return structuredDocumentFlow.insertStructuredShortcutAtActivePosition("callout");
        }
        return editorTypingController.queueCalloutShortcutInsertion();
    }
    function queueBreakShortcutInsertion() {
        if (contentsView.showStructuredDocumentFlow
                && structuredDocumentFlow
                && structuredDocumentFlow.insertStructuredShortcutAtActivePosition !== undefined) {
            return structuredDocumentFlow.insertStructuredShortcutAtActivePosition("break");
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
        const normalizedNextSourceText = nextSourceText === undefined || nextSourceText === null
                ? ""
                : String(nextSourceText);
        const currentSourceText = contentsView.editorText === undefined || contentsView.editorText === null
                ? ""
                : String(contentsView.editorText);
        if (normalizedNextSourceText === currentSourceText)
            return false;
        if (contentsView.resourceTagLossDetected(currentSourceText, normalizedNextSourceText)) {
            contentsView.restoreEditorSurfaceFromPresentation();
            return false;
        }
        if (contentsView.editorText !== normalizedNextSourceText)
            contentsView.editorText = normalizedNextSourceText;
        if (!contentsView.showStructuredDocumentFlow
                && contentsView.commitDocumentPresentationRefresh !== undefined) {
            contentsView.documentPresentationRefreshPendingWhileFocused = false;
            contentsView.commitDocumentPresentationRefresh();
        } else {
            contentsView.documentPresentationRefreshPendingWhileFocused = true;
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
        if (contentsView.persistEditorTextImmediately !== undefined)
            contentsView.persistEditorTextImmediately(normalizedNextSourceText);
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
        if (!contentsView.minimapSnapshotForceFullRefresh
                && currentSourceText === contentsView.minimapSnapshotSourceText
                && Array.isArray(contentsView.minimapLineGroups)
                && contentsView.minimapLineGroups.length === contentsView.logicalLineCount) {
            contentsView.refreshMinimapViewportTracking();
            contentsView.refreshMinimapCursorTracking();
            return;
        }

        const nextLineGroups = contentsView.nextMinimapLineGroupsForCurrentState(currentSourceText);
        const nextRows = MinimapSnapshotSupport.flattenLineGroups(nextLineGroups, contentsView.editorLineHeight);
        const nextSilhouetteHeight = contentsView.minimapSilhouetteHeight(nextRows);
        const nextTrackHeight = Math.min(contentsView.minimapAvailableTrackHeight, nextSilhouetteHeight);

        contentsView.minimapLineGroups = nextLineGroups;
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
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
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
    function resourceTagTextForImportedEntry(entry) {
        const resourceEntry = entry && typeof entry === "object" ? entry : ({});
        const resourcePath = resourceEntry.resourcePath !== undefined ? String(resourceEntry.resourcePath).trim() : "";
        if (resourcePath.length === 0)
            return "";
        const resourceType = contentsView.normalizeResourceType(resourceEntry.type);
        const resourceFormat = contentsView.normalizeResourceFormat(resourceEntry.format);
        const resourceId = resourceEntry.resourceId !== undefined ? String(resourceEntry.resourceId).trim() : "";
        let tagText = "<resource type=\"" + contentsView.encodeXmlAttributeValue(resourceType)
                + "\" format=\"" + contentsView.encodeXmlAttributeValue(resourceFormat)
                + "\" path=\"" + contentsView.encodeXmlAttributeValue(resourcePath) + "\"";
        if (resourceId.length > 0)
            tagText += " id=\"" + contentsView.encodeXmlAttributeValue(resourceId) + "\"";
        tagText += " />";
        return tagText;
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
    function scheduleDeferredDocumentPresentationRefresh() {
        if (contentsView.editorInputFocused || contentsView.typingSessionSyncProtected) {
            contentsView.documentPresentationRefreshPendingWhileFocused = true;
            if (documentPresentationRefreshTimer.running)
                documentPresentationRefreshTimer.stop();
            return;
        }
        if (documentPresentationRefreshTimer.running)
            documentPresentationRefreshTimer.stop();
        documentPresentationRefreshTimer.start();
    }
    function scheduleDocumentPresentationRefresh(forceImmediate) {
        const immediate = !!forceImmediate;
        if (!contentsView.documentPresentationRenderDirty()) {
            contentsView.documentPresentationRefreshPendingWhileFocused = false;
            if (documentPresentationRefreshTimer.running)
                documentPresentationRefreshTimer.stop();
            if (immediate)
                contentsView.scheduleEditorRichTextSurfaceSync();
            contentsView.scheduleMinimapSnapshotRefresh(false);
            if (minimapLayer && contentsView.minimapRefreshEnabled)
                minimapLayer.requestRepaint();
            return;
        }
        if (contentsView.typingSessionSyncProtected) {
            contentsView.documentPresentationRefreshPendingWhileFocused = true;
            contentsView.scheduleDeferredDocumentPresentationRefresh();
            return;
        }
        if (immediate || !contentsView.editorInputFocused) {
            contentsView.documentPresentationRefreshPendingWhileFocused = false;
            if (documentPresentationRefreshTimer.running)
                documentPresentationRefreshTimer.stop();
            contentsView.commitDocumentPresentationRefresh();
            return;
        }
        contentsView.documentPresentationRefreshPendingWhileFocused = true;
        contentsView.scheduleDeferredDocumentPresentationRefresh();
    }
    function refreshLiveLogicalLineMetrics() {
        const normalizedLogicalText = contentsView.activeLogicalTextSnapshot().replace(/\r\n/g, "\n").replace(/\r/g, "\n");
        const nextLineStartOffsets = [0];
        for (let characterIndex = 0; characterIndex < normalizedLogicalText.length; ++characterIndex) {
            if (normalizedLogicalText.charAt(characterIndex) === "\n")
                nextLineStartOffsets.push(characterIndex + 1);
        }
        contentsView.liveLogicalTextLength = normalizedLogicalText.length;
        contentsView.liveLogicalLineStartOffsets = nextLineStartOffsets;
        contentsView.liveLogicalLineCount = Math.max(1, nextLineStartOffsets.length);
    }
    function scheduleGutterRefresh(passCount) {
        const requestedPassCount = Math.max(1, Number(passCount) || 1);
        contentsView.gutterRefreshPassesRemaining = Math.max(contentsView.gutterRefreshPassesRemaining, requestedPassCount);
        if (!gutterRefreshTimer.running)
            gutterRefreshTimer.start();
    }
    function scheduleNoteEntryGutterRefresh(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        contentsView.pendingNoteEntryGutterRefreshNoteId = normalizedNoteId;
        contentsView.resetGutterRefreshState();
        if (normalizedNoteId.length === 0)
            return;
        contentsView.scheduleGutterRefresh(4);
    }
    function scheduleCursorDrivenUiRefresh() {
        if (!contentsView.minimapRefreshEnabled && contentsView.preferNativeInputHandling)
            return;
        if (contentsView.cursorDrivenUiRefreshQueued)
            return;
        contentsView.cursorDrivenUiRefreshQueued = true;
        Qt.callLater(function () {
            contentsView.cursorDrivenUiRefreshQueued = false;
            contentsView.refreshMinimapCursorTracking();
            if (minimapLayer && contentsView.minimapRefreshEnabled)
                minimapLayer.requestRepaint();
            if (!contentsView.preferNativeInputHandling)
                contentsView.scheduleEditorRichTextSurfaceSync();
        });
    }
    function scheduleMinimapSnapshotRefresh(forceFull) {
        if (!contentsView.lineGeometryRefreshEnabled)
            return;
        if (forceFull)
            contentsView.minimapSnapshotForceFullRefresh = true;
        if (contentsView.minimapSnapshotRefreshQueued)
            return;
        contentsView.minimapSnapshotRefreshQueued = true;
        Qt.callLater(function () {
            contentsView.minimapSnapshotRefreshQueued = false;
            contentsView.refreshMinimapSnapshot();
        });
    }
    function flushSelectionModelSync() {
        const shouldResetSnapshot = contentsView.selectionModelSyncResetSnapshotPending;
        const shouldScheduleReconcile = contentsView.selectionModelSyncReconcilePending;
        const shouldFocusEditor = contentsView.selectionModelSyncFocusEditorPending;
        const shouldFallbackRefresh = contentsView.selectionModelSyncFallbackRefreshPending;
        const shouldForceVisualRefresh = contentsView.selectionModelSyncForceVisualRefreshPending;
        contentsView.selectionModelSyncQueued = false;
        contentsView.selectionModelSyncResetSnapshotPending = false;
        contentsView.selectionModelSyncReconcilePending = false;
        contentsView.selectionModelSyncFocusEditorPending = false;
        contentsView.selectionModelSyncFallbackRefreshPending = false;
        contentsView.selectionModelSyncForceVisualRefreshPending = false;

        if (shouldResetSnapshot) {
            contentsView.editorEntrySnapshotComparedNoteId = "";
            contentsView.editorEntrySnapshotPendingNoteId = "";
            contentsView.resetEditorSelectionCache();
        }

        if (shouldFocusEditor)
            contentsView.pendingEditorFocusNoteId = contentsView.selectedNoteId;

        if (contentsView.selectedNoteBodyLoading && contentsView.selectedNoteId.length > 0) {
            if (editorSession.editorBoundNoteId !== contentsView.selectedNoteId
                    && editorSession.pendingBodySave
                    && editorSession.flushPendingEditorText !== undefined) {
                editorSession.flushPendingEditorText();
            }
            return;
        }
        if (contentsView.selectedNoteId.length > 0
                && contentsView.selectedNoteBodyNoteId !== contentsView.selectedNoteId)
            return;

        const selectionSynced = editorSession.requestSyncEditorTextFromSelection(
                    contentsView.selectedNoteId,
                    contentsView.selectedNoteBodyText,
                    contentsView.selectedNoteBodyNoteId);
        if (shouldScheduleReconcile)
            contentsView.scheduleEditorEntrySnapshotReconcile();
        if (shouldForceVisualRefresh || (!selectionSynced && shouldFallbackRefresh)) {
            contentsView.scheduleMinimapSnapshotRefresh(true);
            contentsView.scheduleDocumentPresentationRefresh(true);
            contentsView.scheduleGutterRefresh(4);
        }
        if (contentsView.pendingEditorFocusNoteId === contentsView.selectedNoteId)
            contentsView.focusEditorForPendingNote();
    }
    function scheduleSelectionModelSync(options) {
        const syncOptions = options && typeof options === "object" ? options : ({});
        if (syncOptions.resetSnapshot)
            contentsView.selectionModelSyncResetSnapshotPending = true;
        if (syncOptions.scheduleReconcile)
            contentsView.selectionModelSyncReconcilePending = true;
        if (syncOptions.focusEditor)
            contentsView.selectionModelSyncFocusEditorPending = true;
        if (syncOptions.fallbackRefresh)
            contentsView.selectionModelSyncFallbackRefreshPending = true;
        if (syncOptions.forceVisualRefresh)
            contentsView.selectionModelSyncForceVisualRefreshPending = true;
        if (contentsView.selectionModelSyncQueued)
            return;
        contentsView.selectionModelSyncQueued = true;
        Qt.callLater(function () {
            contentsView.flushSelectionModelSync();
        });
    }
    function refreshStructuredDocumentFlowActivation() {
        const normalizedSelectedNoteId = contentsView.selectedNoteId === undefined || contentsView.selectedNoteId === null
                ? ""
                : String(contentsView.selectedNoteId).trim();
        if (normalizedSelectedNoteId.length === 0) {
            if (contentsView.structuredDocumentFlowActivatedNoteId !== "")
                contentsView.structuredDocumentFlowActivatedNoteId = "";
            return;
        }
        if (!contentsView.editorSessionBoundToSelectedNote) {
            if (!structuredBlockRenderer || !structuredBlockRenderer.renderPending) {
                if (contentsView.structuredDocumentFlowActivatedNoteId !== "")
                    contentsView.structuredDocumentFlowActivatedNoteId = "";
            }
            return;
        }
        if (contentsView.parsedStructuredFlowRequested) {
            if (contentsView.structuredDocumentFlowActivatedNoteId !== normalizedSelectedNoteId)
                contentsView.structuredDocumentFlowActivatedNoteId = normalizedSelectedNoteId;
            return;
        }
        if (structuredBlockRenderer && structuredBlockRenderer.renderPending)
            return;
        if (contentsView.structuredDocumentFlowActivatedNoteId === normalizedSelectedNoteId)
            contentsView.structuredDocumentFlowActivatedNoteId = "";
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
        contentsView.scheduleSelectionModelSync({
                                                   "resetSnapshot": true,
                                                   "scheduleReconcile": true,
                                                   "fallbackRefresh": true
                                               });
    }
    Component.onDestruction: {
        editorTypingController.handleEditorTextEdited();
        editorSession.flushPendingEditorText();
    }
    onEditorFlickableChanged: {
        contentsView.scheduleMinimapSnapshotRefresh(true);
        contentsView.scheduleGutterRefresh(2);
    }
    onEditorTextChanged: {
        contentsView.refreshStructuredDocumentFlowActivation();
        contentsView.scheduleMinimapSnapshotRefresh(false);
        if (!contentsView.showStructuredDocumentFlow)
            contentsView.scheduleDocumentPresentationRefresh(false);
    }
    onShowStructuredDocumentFlowChanged: {
        if (contentsView.showStructuredDocumentFlow) {
            contentsView.documentPresentationRefreshPendingWhileFocused = true;
            if (documentPresentationRefreshTimer.running)
                documentPresentationRefreshTimer.stop();
            if (contentsView.renderedEditorText !== "")
                contentsView.renderedEditorText = "";
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
        contentsView.scheduleSelectionModelSync({});
    }
    onSelectedNoteBodyLoadingChanged: {
        // Empty-body notes keep the same text payload across load completion, so loading state must also re-arm sync.
        if (!contentsView.selectedNoteBodyLoading) {
            contentsView.scheduleSelectionModelSync({
                                                       "fallbackRefresh": true
                                                   });
        }
    }
    onSelectedNoteIdChanged: {
        contentsView.structuredDocumentFlowActivatedNoteId = "";
        contentsView.scheduleNoteEntryGutterRefresh(contentsView.selectedNoteId);
        contentsView.scheduleSelectionModelSync({
                                                   "resetSnapshot": true,
                                                   "scheduleReconcile": true,
                                                   "fallbackRefresh": true,
                                                   "focusEditor": true
                                               });
    }
    onPendingBodySaveChanged: {
        if (!contentsView.pendingBodySave) {
            contentsView.editorEntrySnapshotComparedNoteId = "";
            contentsView.scheduleEditorEntrySnapshotReconcile();
        }
    }
    onTypingSessionSyncProtectedChanged: {
        if (!contentsView.typingSessionSyncProtected) {
            contentsView.editorEntrySnapshotComparedNoteId = "";
            contentsView.scheduleEditorEntrySnapshotReconcile();
        }
    }
    onVisibleChanged: {
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

        agendaBackend: contentsAgendaBackend
        calloutBackend: contentsCalloutBackend
        contentEditor: contentsView.contentEditor
        editorSession: editorSession
        textFormatRenderer: textFormatRenderer
        textMetricsBridge: textMetricsBridge
        view: contentsView
    }
    ContentsBodyResourceRenderer {
        id: bodyResourceRenderer

        bodySourceText: contentsView.editorSessionBoundToSelectedNote
                        ? contentsView.structuredFlowSourceText
                        : ""
        contentViewModel: contentsView.contentViewModel
        fallbackContentViewModel: contentsView.libraryHierarchyViewModel
        maxRenderCount: contentsView.resourceRenderDisplayLimit
        noteId: contentsView.selectedNoteId
        noteDirectoryPath: selectionBridge.selectedNoteDirectoryPath
    }
    Connections {
        target: bodyResourceRenderer

        function onRenderedResourcesChanged() {
            if (!contentsView.resourceBlocksRenderedInlineByRichTextEditor)
                return;
            contentsView.refreshInlineResourcePresentation();
            contentsView.scheduleGutterRefresh(2);
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

        backgroundRefreshEnabled: (!contentsView.editorInputFocused
                                   || contentsView.syncingEditorTextFromModel)
                                  && !contentsView.typingSessionSyncProtected
        sourceText: contentsView.structuredFlowSourceText
    }
    ContentsTextFormatRenderer {
        id: textFormatRenderer

        previewEnabled: contentsView.showFormattedTextRenderer
    }
    ContentsLogicalTextBridge {
        id: textMetricsBridge
    }
    ContentsGutterMarkerBridge {
        id: gutterMarkerBridge

        gutterMarkers: contentsView.gutterMarkers
    }
    ContentsEditorSession {
        id: editorSession

        agendaBackend: contentsAgendaBackend
        selectionBridge: selectionBridge
        typingIdleThresholdMs: contentsView.editorIdleSyncThresholdMs
    }
    Timer {
        id: documentPresentationRefreshTimer

        interval: contentsView.documentPresentationRefreshIntervalMs
        repeat: false

        onTriggered: {
            if (contentsView.editorInputFocused || contentsView.typingSessionSyncProtected) {
                contentsView.documentPresentationRefreshPendingWhileFocused = true;
                stop();
                return;
            }
            contentsView.documentPresentationRefreshPendingWhileFocused = false;
            contentsView.commitDocumentPresentationRefresh();
        }
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
            const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
            if (contentsView.editorEntrySnapshotPendingNoteId === normalizedNoteId)
                contentsView.editorEntrySnapshotPendingNoteId = "";
            if (normalizedNoteId.length === 0 || normalizedNoteId !== contentsView.selectedNoteId)
                return;
            if (success)
                contentsView.editorEntrySnapshotComparedNoteId = normalizedNoteId;
        }

        target: selectionBridge
    }
    Connections {
        function onRenderPendingChanged() {
            contentsView.refreshStructuredDocumentFlowActivation();
        }
        function onRenderedBlocksChanged() {
            contentsView.refreshStructuredDocumentFlowActivation();
        }

        target: structuredBlockRenderer
    }
    Connections {
        function onEditorTextSynchronized() {
            contentsView.scheduleMinimapSnapshotRefresh(true);
            contentsView.scheduleDocumentPresentationRefresh(true);
            contentsView.consumePendingNoteEntryGutterRefresh(editorSession.editorBoundNoteId);
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
            contentsView.scheduleGutterRefresh(2);
        }
        function onCursorPositionChanged() {
            contentsView.scheduleCursorDrivenUiRefresh();
        }
        function onLineCountChanged() {
            if (contentsView.editorInputFocused)
                contentsView.scheduleDeferredDocumentPresentationRefresh();
            else
                contentsView.scheduleMinimapSnapshotRefresh(false);
            contentsView.scheduleGutterRefresh(2);
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
        }

        ignoreUnknownSignals: true
        target: contentsView.legacyInlineEditorActive && contentEditor && contentEditor.editorItem ? contentEditor.editorItem : null
    }
    Connections {
        function onCursorPositionChanged() {
            contentsView.scheduleCursorDrivenUiRefresh();
        }

        ignoreUnknownSignals: true
        target: contentsView.legacyInlineEditorActive && contentEditor && contentEditor.editorItem && contentEditor.editorItem.inputItem ? contentEditor.editorItem.inputItem : null
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

                            onTapped: {
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
                        parent: contentsView.showPrintEditorLayout ? printDocumentSurface : structuredDocumentViewport.contentItem
                        renderedResources: bodyResourceRenderer.renderedResources
                        sourceText: contentsView.structuredFlowSourceText
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
                            autoFocusOnPress: !contentsView.preferNativeInputHandling
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
                            fontWeight: Font.Medium
                            preferNativeInputHandling: contentsView.preferNativeInputHandling
                            insetHorizontal: contentsView.showPrintEditorLayout ? 0 : contentsView.editorHorizontalInset
                            insetVertical: contentsView.showPrintEditorLayout ? 0 : contentsView.editorBottomInset
                            placeholderText: ""
                            selectByMouse: true
                            selectedTextColor: LV.Theme.textPrimary
                            selectionColor: LV.Theme.accent
                            shapeStyle: 0
                            shortcutKeyPressHandler: function (event) {
                                return contentsView.handleInlineFormatShortcutKeyPress(event);
                            }
                            blockExternalDropMutation: contentsView.resourceDropActive
                                                       || contentsView.resourceDropEditorSurfaceGuardActive
                            showRenderedOutput: !contentsView.preferNativeInputHandling
                            showScrollBar: false
                            suppressCommittedTextEditedDispatch: contentsView.resourceDropEditorSurfaceGuardActive
                            text: contentsView.preferNativeInputHandling ? contentsView.mobileEditorDisplayText : contentsView.renderedEditorText
                            textColor: contentsView.showPrintEditorLayout ? contentsView.printPaperTextColor : LV.Theme.bodyColor
                            textFormat: contentsView.preferNativeInputHandling ? TextEdit.PlainText : TextEdit.RichText
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
