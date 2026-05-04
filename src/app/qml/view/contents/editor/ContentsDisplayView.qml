pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import ".." as ContentsChrome

Item {
    id: contentsDisplayView

    property var contentController: null
    property color displayColor: "transparent"
    property var editorViewModeController: null
    readonly property bool editorCustomTextInputEnabled: false
    readonly property bool editorTagManagementInputEnabled: true
    property int editorTopInsetOverride: -1
    property int frameHorizontalInsetOverride: -1
    property int gutterWidthOverride: -1
    property var htmlTokens: editorPresentationProjection.htmlTokens
    property var libraryHierarchyController: null
    property int lineNumberColumnLeftOverride: -1
    property int lineNumberColumnTextWidthOverride: -1
    property bool minimapVisible: true
    property bool mobileHost: false
    property var normalizedHtmlBlocks: editorPresentationProjection.normalizedHtmlBlocks
    property var noteActiveState: null
    property var noteListModel: null
    property var panelController: null
    property bool paperPaletteEnabled: false
    property var resourcesImportController: null
    property var sidebarHierarchyController: null
    readonly property var currentNoteEntry: contentsDisplayView.noteEntryFromModel()
    readonly property string currentNoteId: contentsDisplayView.activeStateAvailable()
                                            ? contentsDisplayView.activeStateStringProperty("activeNoteId")
                                            : contentsDisplayView.stringValue(
                                                contentsDisplayView.currentNoteEntry,
                                                "noteId",
                                                contentsDisplayView.modelStringProperty("currentNoteId"))
    readonly property string currentNoteDirectoryPath: contentsDisplayView.activeStateAvailable()
                                                           ? contentsDisplayView.activeStateStringProperty("activeNoteDirectoryPath")
                                                           : contentsDisplayView.stringValue(
                                                           contentsDisplayView.currentNoteEntry,
                                                           "noteDirectoryPath",
                                                           contentsDisplayView.modelStringProperty("currentNoteDirectoryPath"))
    readonly property string currentRawBodyText: contentsDisplayView.activeStateAvailable()
                                                    ? contentsDisplayView.activeStateStringProperty("activeNoteBodyText")
                                                    : contentsDisplayView.stringValue(
                                                    contentsDisplayView.currentNoteEntry,
                                                    "bodyText",
                                                    contentsDisplayView.modelStringProperty("currentBodyText"))
    readonly property bool noteDocumentParseMounted: contentsDisplayView.currentNoteId.length > 0
    readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: contentsDisplayView.noteDocumentParseMounted
    readonly property bool structuredBlockBackgroundRefreshEnabled: contentsDisplayView.noteDocumentParseMounted

    signal editorTextEdited(string text)
    signal viewHookRequested

    function commitEditedSourceText(text) {
        const nextText = text === undefined || text === null ? "" : String(text);
        const changed = editorSession.commitRawEditorTextMutation(nextText);
        if (changed) {
            if (contentsDisplayView.contentController
                    && contentsDisplayView.contentController.saveCurrentBodyText !== undefined) {
                contentsDisplayView.contentController.saveCurrentBodyText(nextText);
            }
            contentsDisplayView.editorTextEdited(nextText);
        }
    }

    function applyDocumentSourceMutation(payload) {
        if (payload === null || payload === undefined || typeof payload !== "object")
            return false;
        if (payload.nextSourceText === undefined || payload.nextSourceText === null)
            return false;
        contentsDisplayView.commitEditedSourceText(String(payload.nextSourceText));
        return true;
    }

    function commitDocumentPresentationRefresh() {
        editorPresentationProjection.sourceText = editorSession.editorText;
    }

    function encodeXmlAttributeValue(value) {
        return (value === undefined || value === null ? "" : String(value))
                .replace(/&/g, "&amp;")
                .replace(/"/g, "&quot;")
                .replace(/'/g, "&apos;")
                .replace(/</g, "&lt;")
                .replace(/>/g, "&gt;");
    }

    function modelStringProperty(propertyName) {
        if (!contentsDisplayView.noteListModel
                || contentsDisplayView.noteListModel[propertyName] === undefined
                || contentsDisplayView.noteListModel[propertyName] === null) {
            return "";
        }
        return String(contentsDisplayView.noteListModel[propertyName]);
    }

    function activeStateAvailable() {
        return contentsDisplayView.noteActiveState !== null
                && contentsDisplayView.noteActiveState !== undefined;
    }

    function activeStateStringProperty(propertyName) {
        if (!contentsDisplayView.activeStateAvailable()
                || contentsDisplayView.noteActiveState[propertyName] === undefined
                || contentsDisplayView.noteActiveState[propertyName] === null) {
            return "";
        }
        return String(contentsDisplayView.noteActiveState[propertyName]);
    }

    function attachEditorSessionToActiveState() {
        if (!contentsDisplayView.activeStateAvailable()
                || contentsDisplayView.noteActiveState.editorSession === undefined) {
            return false;
        }
        contentsDisplayView.noteActiveState.editorSession = editorSession;
        return true;
    }

    function noteEntryFromModel() {
        if (contentsDisplayView.activeStateAvailable()
                && contentsDisplayView.noteActiveState.activeNoteEntry !== undefined
                && contentsDisplayView.noteActiveState.activeNoteEntry !== null
                && typeof contentsDisplayView.noteActiveState.activeNoteEntry === "object") {
            return contentsDisplayView.noteActiveState.activeNoteEntry;
        }
        if (contentsDisplayView.noteListModel
                && contentsDisplayView.noteListModel.currentNoteEntry !== undefined
                && contentsDisplayView.noteListModel.currentNoteEntry !== null
                && typeof contentsDisplayView.noteListModel.currentNoteEntry === "object") {
            return contentsDisplayView.noteListModel.currentNoteEntry;
        }
        return ({});
    }

    function requestViewHook(reason) {
        if (contentsDisplayView.panelController && contentsDisplayView.panelController.requestControllerHook)
            contentsDisplayView.panelController.requestControllerHook(reason !== undefined ? String(reason) : "manual");
        contentsDisplayView.viewHookRequested();
    }

    function renderInlineResourceEditorSurfaceHtml(editorHtml, renderedResources) {
        return inlineResourcePresentation.renderEditorSurfaceHtmlWithInlineResources(
                    editorHtml === undefined || editorHtml === null ? "" : String(editorHtml),
                    renderedResources === undefined || renderedResources === null ? [] : renderedResources);
    }

    function requestEditorSelectionContextMenuFromPointer(pointerKind) {
        const normalizedKind = pointerKind === undefined || pointerKind === null ? "" : String(pointerKind);
        return normalizedKind === "right-click" || normalizedKind === "long-press";
    }

    function scheduleStructuredDocumentOpenLayoutRefresh() {
        contentsDisplayView.commitDocumentPresentationRefresh();
        contentsDisplayView.refreshGutterLineNumberGeometry();
    }

    function refreshGutterLineNumberGeometry() {
        Qt.callLater(function () {
            gutterLineNumberGeometry.refresh();
            gutterMarkerGeometry.refresh();
        });
    }

    function refreshGutterForDocumentStateChange() {
        contentsDisplayView.refreshGutterLineNumberGeometry();
    }

    function stringValue(payload, key, fallback) {
        if (payload !== null
                && payload !== undefined
                && typeof payload === "object"
                && payload[key] !== undefined
                && payload[key] !== null) {
            return String(payload[key]);
        }
        return fallback === undefined || fallback === null ? "" : String(fallback);
    }

    function terminalBodyClickSourceOffset() {
        return editorSession.editorText.length;
    }

    function syncSessionFromCurrentNote(resetViewport) {
        if (contentsDisplayView.currentNoteId.length === 0) {
            contentsDisplayView.refreshGutterLineNumberGeometry();
            return;
        }
        if (resetViewport === true && editorDocumentViewport)
            editorDocumentViewport.contentY = 0;
        if (contentsDisplayView.activeStateAvailable()
                && contentsDisplayView.noteActiveState.syncEditorSessionFromActiveNote !== undefined) {
            contentsDisplayView.noteActiveState.syncEditorSessionFromActiveNote();
            contentsDisplayView.refreshGutterForDocumentStateChange();
            return;
        }
        editorSession.requestSyncEditorTextFromSelection(
                    contentsDisplayView.currentNoteId,
                    contentsDisplayView.currentRawBodyText,
                    contentsDisplayView.currentNoteId,
                    contentsDisplayView.currentNoteDirectoryPath);
        contentsDisplayView.refreshGutterForDocumentStateChange();
    }

    clip: true

    Component.onCompleted: {
        contentsDisplayView.attachEditorSessionToActiveState();
        contentsDisplayView.syncSessionFromCurrentNote(true);
        contentsDisplayView.refreshGutterLineNumberGeometry();
    }
    Component.onDestruction: {
        if (contentsDisplayView.activeStateAvailable()
                && contentsDisplayView.noteActiveState.editorSession === editorSession)
            contentsDisplayView.noteActiveState.editorSession = null;
    }
    onNoteActiveStateChanged: {
        contentsDisplayView.attachEditorSessionToActiveState();
        contentsDisplayView.syncSessionFromCurrentNote(true);
        contentsDisplayView.refreshGutterForDocumentStateChange();
    }
    onCurrentNoteDirectoryPathChanged: contentsDisplayView.syncSessionFromCurrentNote(true)
    onCurrentNoteIdChanged: contentsDisplayView.syncSessionFromCurrentNote(true)
    onCurrentRawBodyTextChanged: contentsDisplayView.syncSessionFromCurrentNote(false)
    onHeightChanged: contentsDisplayView.refreshGutterLineNumberGeometry()
    onNoteListModelChanged: contentsDisplayView.syncSessionFromCurrentNote(true)
    onWidthChanged: contentsDisplayView.refreshGutterLineNumberGeometry()

    Connections {
        target: contentsDisplayView.noteListModel
        ignoreUnknownSignals: true

        function onCurrentBodyTextChanged() {
            contentsDisplayView.syncSessionFromCurrentNote(false);
        }

        function onCurrentIndexChanged() {
            contentsDisplayView.syncSessionFromCurrentNote(true);
        }

        function onCurrentNoteDirectoryPathChanged() {
            contentsDisplayView.syncSessionFromCurrentNote(true);
        }

        function onCurrentNoteEntryChanged() {
            contentsDisplayView.syncSessionFromCurrentNote(true);
        }

        function onCurrentNoteIdChanged() {
            contentsDisplayView.syncSessionFromCurrentNote(true);
        }
    }

    Connections {
        target: contentsDisplayView.noteActiveState
        ignoreUnknownSignals: true

        function onActiveNoteStateChanged() {
            contentsDisplayView.syncSessionFromCurrentNote(false);
            contentsDisplayView.refreshGutterForDocumentStateChange();
        }

        function onActiveNoteListModelChanged() {
            contentsDisplayView.refreshGutterForDocumentStateChange();
        }

        function onHasActiveNoteChanged() {
            contentsDisplayView.refreshGutterForDocumentStateChange();
        }
    }

    ContentsEditorSessionController {
        id: editorSession

        objectName: "contentsDisplayEditorSession"
    }

    Connections {
        target: editorSession

        function onEditorBoundNoteDirectoryPathChanged() {
            contentsDisplayView.refreshGutterForDocumentStateChange();
        }

        function onEditorBoundNoteIdChanged() {
            contentsDisplayView.refreshGutterForDocumentStateChange();
        }

        function onEditorTextChanged() {
            contentsDisplayView.refreshGutterForDocumentStateChange();
        }

        function onEditorTextSynchronized() {
            contentsDisplayView.refreshGutterForDocumentStateChange();
        }

        function onPendingBodySaveChanged() {
            contentsDisplayView.refreshGutterLineNumberGeometry();
        }
    }

    ContentsEditorPresentationProjection {
        id: editorPresentationProjection

        objectName: "contentsDisplayPresentationProjection"
        paperPaletteEnabled: contentsDisplayView.paperPaletteEnabled
        sourceText: editorSession.editorText
    }

    Connections {
        target: editorPresentationProjection

        function onEditorSurfaceHtmlChanged() {
            contentsDisplayView.refreshGutterLineNumberGeometry();
        }

        function onLogicalLineCountChanged() {
            contentsDisplayView.refreshGutterLineNumberGeometry();
        }

        function onNormalizedHtmlBlocksChanged() {
            contentsDisplayView.refreshGutterLineNumberGeometry();
        }
    }

    ContentsStructuredBlockRenderer {
        id: structuredBlockRenderer

        backgroundRefreshEnabled: contentsDisplayView.structuredBlockBackgroundRefreshEnabled
        objectName: "contentsDisplayStructuredBlockRenderer"
        sourceText: editorSession.editorText
    }

    Connections {
        target: structuredBlockRenderer

        function onRenderedBlocksChanged() {
            contentsDisplayView.refreshGutterLineNumberGeometry();
        }

        function onRenderPendingChanged() {
            contentsDisplayView.refreshGutterLineNumberGeometry();
        }
    }

    ContentsBodyResourceRenderer {
        id: bodyResourceRenderer

        contentController: contentsDisplayView.contentController
        documentBlocks: structuredBlockRenderer.renderedDocumentBlocks
        fallbackContentController: contentsDisplayView.libraryHierarchyController
        maxRenderCount: 0
        noteDirectoryPath: contentsDisplayView.currentNoteDirectoryPath
        noteId: contentsDisplayView.currentNoteId
        objectName: "contentsDisplayBodyResourceRenderer"
    }

    Connections {
        target: bodyResourceRenderer

        function onRenderedResourcesChanged() {
            contentsDisplayView.refreshGutterLineNumberGeometry();
        }
    }

    ContentsResourceTagController {
        id: resourceTagController

        bodyResourceRenderer: bodyResourceRenderer
        editorText: editorSession.editorText
        selectedNoteBodyNoteId: contentsDisplayView.currentNoteId
        selectedNoteBodyText: contentsDisplayView.currentRawBodyText
        selectedNoteId: contentsDisplayView.currentNoteId
        view: contentsDisplayView
    }

    ContentsInlineResourcePresentationController {
        id: inlineResourcePresentation

        bodyResourceRenderer: bodyResourceRenderer
        contentEditor: contentsDisplayView
        editorHorizontalInset: LV.Theme.gapNone
        editorViewport: contentsDisplayView
        inlineHtmlImageRenderingEnabled: true
        resourceEditorPlaceholderLineCount: 1
        resourceTagController: resourceTagController
        view: contentsDisplayView
    }

    ContentsGutterLayoutMetrics {
        id: gutterLayoutMetrics

        controlHeightMd: LV.Theme.controlHeightMd
        controlHeightSm: LV.Theme.controlHeightSm
        dialogMaxWidth: LV.Theme.dialogMaxWidth
        gapNone: LV.Theme.gapNone
        gap2: LV.Theme.gap2
        gap3: LV.Theme.gap3
        gap5: LV.Theme.gap5
        gap7: LV.Theme.gap7
        gap14: LV.Theme.gap14
        gap20: LV.Theme.gap20
        gap24: LV.Theme.gap24
        gutterWidthOverride: contentsDisplayView.gutterWidthOverride
        inputWidthMd: LV.Theme.inputWidthMd
        lineNumberColumnLeftOverride: contentsDisplayView.lineNumberColumnLeftOverride
        lineNumberColumnTextWidthOverride: contentsDisplayView.lineNumberColumnTextWidthOverride
        logicalLineCount: editorPresentationProjection.logicalLineCount
        objectName: "contentsDisplayGutterLayoutMetrics"
        strokeThin: LV.Theme.strokeThin
    }

    ContentsGutterLineNumberGeometry {
        id: gutterLineNumberGeometry

        displayBlocks: editorPresentationProjection.normalizedHtmlBlocks
        documentBlocks: structuredBlockRenderer.renderedDocumentBlocks
        editorContentHeight: structuredDocumentFlow.editorContentHeight
        editorGeometryHost: structuredDocumentFlow
        fallbackLineHeight: LV.Theme.textBodyLineHeight
        fallbackTopInset: LV.Theme.gapNone
        lineNumberBaseOffset: gutterLayoutMetrics.lineNumberBaseOffset
        lineNumberCount: gutterLayoutMetrics.effectiveLineNumberCount
        logicalLineStartOffsets: editorPresentationProjection.logicalLineStartOffsets
        logicalToSourceOffsets: editorPresentationProjection.logicalToSourceOffsets()
        mapTarget: contentsDisplayGutter
        objectName: "contentsDisplayGutterLineNumberGeometry"
        renderedResources: bodyResourceRenderer.renderedResources
        sourceText: editorSession.editorText
    }

    ContentsGutterMarkerGeometry {
        id: gutterMarkerGeometry

        cursorPosition: structuredDocumentFlow.editorCursorPosition
        editorMounted: contentsDisplayView.noteDocumentParseMounted
        lineNumberBaseOffset: gutterLayoutMetrics.lineNumberBaseOffset
        lineNumberEntries: gutterLineNumberGeometry.lineNumberEntries
        markerHeight: LV.Theme.textBodyLineHeight
        objectName: "contentsDisplayGutterMarkerGeometry"
        savedSourceText: contentsDisplayView.currentRawBodyText
        sourceText: editorSession.editorText
    }

    ContentsMinimapLayoutMetrics {
        id: minimapLayoutMetrics

        buttonMinWidth: LV.Theme.buttonMinWidth
        gapNone: LV.Theme.gapNone
        gap8: LV.Theme.gap8
        gap12: LV.Theme.gap12
        gap20: LV.Theme.gap20
        gap24: LV.Theme.gap24
        logicalLineCount: editorPresentationProjection.logicalLineCount
        minimapVisible: contentsDisplayView.minimapVisible
        objectName: "contentsDisplayMinimapLayoutMetrics"
        strokeThin: LV.Theme.strokeThin
    }

    Rectangle {
        anchors.fill: parent
        color: contentsDisplayView.displayColor
    }

    LV.HStack {
        anchors.fill: parent
        objectName: "contentsDisplayEditorChromeHStack"
        spacing: LV.Theme.gapNone

        ContentsChrome.Gutter {
            id: contentsDisplayGutter

            Layout.fillHeight: true
            Layout.preferredWidth: gutterLayoutMetrics.effectiveGutterWidth
            activeLineNumber: gutterMarkerGeometry.cursorLineNumber
            iconRailX: gutterLayoutMetrics.iconRailX
            lineNumberBaseOffset: gutterLayoutMetrics.lineNumberBaseOffset
            lineNumberColumnLeft: gutterLayoutMetrics.lineNumberColumnLeft
            lineNumberColumnTextWidth: gutterLayoutMetrics.lineNumberColumnTextWidth
            lineNumberCount: gutterLayoutMetrics.effectiveLineNumberCount
            lineNumberEntries: gutterLineNumberGeometry.lineNumberEntries
            markerEntries: gutterMarkerGeometry.markerEntries
            objectName: "contentsDisplayGutter"

            onViewHookRequested: function (reason) {
                contentsDisplayView.requestViewHook(reason);
            }
        }

        Item {
            id: editorDocumentSlot

            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            objectName: "contentsDisplayEditorDocumentSlot"

            onHeightChanged: contentsDisplayView.refreshGutterLineNumberGeometry()
            onWidthChanged: contentsDisplayView.refreshGutterLineNumberGeometry()

            Flickable {
                id: editorDocumentViewport

                anchors.fill: parent
                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds
                clip: true
                contentHeight: Math.max(height, structuredDocumentFlow.editorContentHeight)
                contentWidth: Math.max(width, structuredDocumentFlow.width)
                flickableDirection: Flickable.VerticalFlick
                interactive: contentHeight > height
                objectName: "contentsDisplayEditorDocumentViewport"

                onContentYChanged: contentsDisplayView.refreshGutterLineNumberGeometry()
                onHeightChanged: contentsDisplayView.refreshGutterLineNumberGeometry()
                onMovementEnded: contentsDisplayView.refreshGutterLineNumberGeometry()
                onWidthChanged: contentsDisplayView.refreshGutterLineNumberGeometry()

                ContentsStructuredDocumentFlow {
                    id: structuredDocumentFlow

                    editorSurfaceHtml: contentsDisplayView.renderInlineResourceEditorSurfaceHtml(
                                           editorPresentationProjection.editorSurfaceHtml,
                                           bodyResourceRenderer.renderedResources)
                    height: Math.max(editorDocumentViewport.height, editorDocumentViewport.contentHeight)
                    htmlTokens: editorPresentationProjection.htmlTokens
                    logicalCursorPosition: editorPresentationProjection.logicalLengthForSourceText(
                                               editorSession.editorText.slice(
                                                   0,
                                                   Math.max(
                                                       0,
                                                       Math.min(structuredDocumentFlow.editorCursorPosition,
                                                                editorSession.editorText.length))))
                    logicalText: editorPresentationProjection.logicalText
                    logicalToSourceOffsets: editorPresentationProjection.logicalToSourceOffsets()
                    normalizedHtmlBlocks: editorPresentationProjection.normalizedHtmlBlocks
                    objectName: "contentsDisplayStructuredDocumentFlow"
                    paperPaletteEnabled: contentsDisplayView.paperPaletteEnabled
                    sourceText: editorSession.editorText
                    textColor: LV.Theme.bodyColor
                    visible: contentsDisplayView.noteDocumentParseMounted
                    width: editorDocumentViewport.width

                    onEditorContentHeightChanged: contentsDisplayView.refreshGutterLineNumberGeometry()

                    onSourceTextEdited: function (text) {
                        contentsDisplayView.commitEditedSourceText(text);
                    }

                    onViewHookRequested: {
                        contentsDisplayView.viewHookRequested();
                    }
                }
            }

            LV.WheelScrollGuard {
                consumeInside: true
                targetFlickable: editorDocumentViewport
            }

            Text {
                anchors.centerIn: parent
                color: LV.Theme.descriptionColor
                font.family: LV.Theme.fontBody
                font.pixelSize: LV.Theme.textBody
                text: "No document opened"
                visible: !contentsDisplayView.noteDocumentParseMounted
            }
        }

        ContentsChrome.Minimap {
            id: contentsDisplayMinimap

            Layout.fillHeight: true
            Layout.preferredWidth: minimapLayoutMetrics.effectiveMinimapWidth
            lineColor: LV.Theme.captionColor
            objectName: "contentsDisplayMinimap"
            rowCount: minimapLayoutMetrics.effectiveRowCount
            visible: contentsDisplayView.minimapVisible

            onViewHookRequested: function (reason) {
                contentsDisplayView.requestViewHook(reason);
            }
        }
    }
}
