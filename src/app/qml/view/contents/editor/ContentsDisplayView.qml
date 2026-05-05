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
    property var htmlTokens: editorPresentationProjection.htmlTokens
    property var libraryHierarchyController: null
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

    function renderInlineResourceEditorSurfaceHtml(editorHtml, renderedResources, targetFrameWidth) {
        return inlineResourcePresentation.renderEditorSurfaceHtmlWithInlineResources(
                    editorHtml === undefined || editorHtml === null ? "" : String(editorHtml),
                    renderedResources === undefined || renderedResources === null ? [] : renderedResources,
                    Math.max(120, Math.floor(Number(targetFrameWidth) || 0)));
    }

    function editorViewportScrollRange() {
        if (!editorDocumentViewport)
            return 0;
        return Math.max(0, editorDocumentViewport.contentHeight - editorDocumentViewport.height);
    }

    function editorViewportScrollRatio() {
        const scrollRange = contentsDisplayView.editorViewportScrollRange();
        if (scrollRange <= 0)
            return 0;
        return Math.max(0, Math.min(1, editorDocumentViewport.contentY / scrollRange));
    }

    function editorViewportVisibleRatio() {
        if (!editorDocumentViewport || editorDocumentViewport.contentHeight <= 0)
            return 1;
        return Math.max(0, Math.min(1, editorDocumentViewport.height / editorDocumentViewport.contentHeight));
    }

    function scrollEditorViewportToRatio(ratio) {
        const scrollRange = contentsDisplayView.editorViewportScrollRange();
        const normalizedRatio = Math.max(0, Math.min(1, Number(ratio) || 0));
        editorDocumentViewport.contentY = scrollRange * normalizedRatio;
        return true;
    }

    function requestEditorSelectionContextMenuFromPointer(pointerKind) {
        const normalizedKind = pointerKind === undefined || pointerKind === null ? "" : String(pointerKind);
        return normalizedKind === "right-click" || normalizedKind === "long-press";
    }

    function scheduleStructuredDocumentOpenLayoutRefresh() {
        contentsDisplayView.commitDocumentPresentationRefresh();
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
            return;
        }
        if (resetViewport === true && editorDocumentViewport)
            editorDocumentViewport.contentY = 0;
        if (contentsDisplayView.activeStateAvailable()
                && contentsDisplayView.noteActiveState.syncEditorSessionFromActiveNote !== undefined) {
            contentsDisplayView.noteActiveState.syncEditorSessionFromActiveNote();
            return;
        }
        editorSession.requestSyncEditorTextFromSelection(
                    contentsDisplayView.currentNoteId,
                    contentsDisplayView.currentRawBodyText,
                    contentsDisplayView.currentNoteId,
                    contentsDisplayView.currentNoteDirectoryPath);
    }

    clip: true

    Component.onCompleted: {
        contentsDisplayView.attachEditorSessionToActiveState();
        contentsDisplayView.syncSessionFromCurrentNote(true);
    }
    Component.onDestruction: {
        if (contentsDisplayView.activeStateAvailable()
                && contentsDisplayView.noteActiveState.editorSession === editorSession)
            contentsDisplayView.noteActiveState.editorSession = null;
    }
    onNoteActiveStateChanged: {
        contentsDisplayView.attachEditorSessionToActiveState();
        contentsDisplayView.syncSessionFromCurrentNote(true);
    }
    onCurrentNoteDirectoryPathChanged: contentsDisplayView.syncSessionFromCurrentNote(true)
    onCurrentNoteIdChanged: contentsDisplayView.syncSessionFromCurrentNote(true)
    onCurrentRawBodyTextChanged: contentsDisplayView.syncSessionFromCurrentNote(false)
    onNoteListModelChanged: contentsDisplayView.syncSessionFromCurrentNote(true)

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
        }
    }

    ContentsEditorSessionController {
        id: editorSession

        objectName: "contentsDisplayEditorSession"
    }

    ContentsEditorPresentationProjection {
        id: editorPresentationProjection

        objectName: "contentsDisplayPresentationProjection"
        paperPaletteEnabled: contentsDisplayView.paperPaletteEnabled
        sourceText: editorSession.editorText
    }

    ContentsStructuredBlockRenderer {
        id: structuredBlockRenderer

        backgroundRefreshEnabled: contentsDisplayView.structuredBlockBackgroundRefreshEnabled
        objectName: "contentsDisplayStructuredBlockRenderer"
        sourceText: editorSession.editorText
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

    ContentsMinimapLayoutMetrics {
        id: minimapLayoutMetrics

        buttonMinWidth: LV.Theme.buttonMinWidth
        gapNone: LV.Theme.gapNone
        gap8: LV.Theme.gap8
        gap12: LV.Theme.gap12
        gap20: LV.Theme.gap20
        gap24: LV.Theme.gap24
        minimapVisible: contentsDisplayView.minimapVisible
        objectName: "contentsDisplayMinimapLayoutMetrics"
        strokeThin: LV.Theme.strokeThin
        visualLineCount: structuredDocumentFlow.editorVisualLineCount
    }

    Rectangle {
        anchors.fill: parent
        color: contentsDisplayView.displayColor
    }

    LV.HStack {
        anchors.fill: parent
        objectName: "contentsDisplayEditorChromeHStack"
        spacing: LV.Theme.gapNone

        Item {
            id: editorDocumentSlot

            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            objectName: "contentsDisplayEditorDocumentSlot"

            Flickable {
                id: editorDocumentViewport

                anchors.fill: parent
                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds
                clip: true
                contentHeight: editorDocumentContent.height
                contentWidth: Math.max(width, editorDocumentContent.width)
                flickableDirection: Flickable.VerticalFlick
                interactive: contentHeight > height
                objectName: "contentsDisplayEditorDocumentViewport"

                Item {
                    id: editorDocumentContent

                    height: Math.max(editorDocumentViewport.height, structuredDocumentFlow.editorContentHeight)
                    width: editorDocumentViewport.width

                    ContentsLineNumberRail {
                        id: contentsDisplayGutter

                        height: editorDocumentContent.height
                        objectName: "contentsDisplayGutter"
                        rows: structuredDocumentFlow.editorLogicalGutterRows
                        visible: contentsDisplayView.noteDocumentParseMounted
                        width: LV.Theme.buttonMinWidth
                        x: LV.Theme.gapNone
                        y: LV.Theme.gapNone
                    }

                    ContentsStructuredDocumentFlow {
                        id: structuredDocumentFlow

                        editorSurfaceHtml: contentsDisplayView.renderInlineResourceEditorSurfaceHtml(
                                               editorPresentationProjection.editorSurfaceHtml,
                                               bodyResourceRenderer.renderedResources,
                                               structuredDocumentFlow.width - LV.Theme.gap16 * 2)
                        height: editorDocumentContent.height
                        htmlTokens: editorPresentationProjection.htmlTokens
                        logicalCursorPosition: editorPresentationProjection.logicalOffsetForSourceOffset(
                                                   structuredDocumentFlow.editorCursorPosition)
                        logicalText: editorPresentationProjection.logicalText
                        logicalToSourceOffsets: editorPresentationProjection.logicalToSourceOffsets()
                        normalizedHtmlBlocks: editorPresentationProjection.normalizedHtmlBlocks
                        objectName: "contentsDisplayStructuredDocumentFlow"
                        paperPaletteEnabled: contentsDisplayView.paperPaletteEnabled
                        sourceText: editorSession.editorText
                        textColor: LV.Theme.bodyColor
                        visible: contentsDisplayView.noteDocumentParseMounted
                        width: Math.max(0, editorDocumentContent.width - contentsDisplayGutter.width)
                        x: contentsDisplayGutter.width
                        y: LV.Theme.gapNone

                        onSourceTextEdited: function (text) {
                            contentsDisplayView.commitEditedSourceText(text);
                        }

                        onViewHookRequested: {
                            contentsDisplayView.viewHookRequested();
                        }
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
            rowWidthRatios: structuredDocumentFlow.editorVisualLineWidthRatios
            scrollDragEnabled: editorDocumentViewport.contentHeight > editorDocumentViewport.height
            scrollPositionRatio: contentsDisplayView.editorViewportScrollRatio()
            visible: contentsDisplayView.minimapVisible
            viewportRatio: contentsDisplayView.editorViewportVisibleRatio()

            onScrollRatioRequested: function (ratio) {
                contentsDisplayView.scrollEditorViewportToRatio(ratio);
            }

            onViewHookRequested: function (reason) {
                contentsDisplayView.requestViewHook(reason);
            }
        }
    }
}
