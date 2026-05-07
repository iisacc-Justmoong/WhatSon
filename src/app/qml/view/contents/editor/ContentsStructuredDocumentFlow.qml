pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: documentFlow

    property var htmlTokens: []
    property var normalizedHtmlBlocks: []
    property var documentBlocks: []
    property var coordinateMapper: null
    property bool paperPaletteEnabled: false
    property string editorSurfaceHtml: ""
    property string lastReadyEditorSurfaceHtml: ""
    property string lastReadyLogicalText: ""
    property string logicalText: ""
    property string projectionSourceText: documentFlow.sourceText
    property var resourceVisualBlocks: []
    property string sourceText: ""
    property color textColor: LV.Theme.bodyColor
    readonly property real editorContentHeight: editor.displayContentHeight
    readonly property int editorCursorPosition: editor.sourceCursorPosition
    readonly property var editorLogicalGutterRows: editor.logicalGutterRows
    readonly property int editorSelectionEnd: editor.sourceSelectionEnd
    readonly property int editorSelectionStart: editor.sourceSelectionStart
    readonly property bool editorRenderedOverlayVisible: editor.renderedOverlayVisible
    readonly property int editorVisualLineCount: editor.visualLineCount
    readonly property var editorVisualLineWidthRatios: editor.visualLineWidthRatios
    readonly property bool logicalProjectionReady: documentFlow.projectionSourceText === documentFlow.sourceText
    readonly property string resolvedEditorSurfaceHtml: documentFlow.logicalProjectionReady
            ? documentFlow.editorSurfaceHtml
            : documentFlow.lastReadyEditorSurfaceHtml
    readonly property string resolvedDisplayGeometryText: documentFlow.logicalProjectionReady
            ? documentFlow.logicalText
            : documentFlow.lastReadyLogicalText

    signal sourceTextEdited(string text)
    signal viewHookRequested(string reason)

    function normalizedBlocks() {
        return documentFlow.normalizedHtmlBlocks;
    }

    function applyInlineFormatShortcut(event) {
        const tagName = tagInsertionController.tagNameForShortcutKey(Number(event.key) || 0);
        if (tagName.length <= 0)
            return false;

        const currentSourceText = editor.currentPlainText();
        const selectedRange = editor.selectedSourceRange();
        const payload = tagInsertionController.buildTagInsertionPayload(
                    currentSourceText,
                    Number(selectedRange.start) || 0,
                    Number(selectedRange.end) || 0,
                    tagName);
        return editor.applyTagManagementMutationPayload(payload);
    }

    function applyBodyTagShortcut(event) {
        const tagName = tagInsertionController.tagNameForBodyShortcutKey(Number(event.key) || 0);
        if (tagName.length <= 0)
            return false;

        const currentSourceText = editor.currentPlainText();
        const selectedRange = editor.selectedSourceRange();
        const payload = tagInsertionController.buildTagInsertionPayload(
                    currentSourceText,
                    Number(selectedRange.start) || 0,
                    Number(selectedRange.end) || 0,
                    tagName);
        return editor.applyTagManagementMutationPayload(payload);
    }

    function handleTagManagementKeyPress(event) {
        if (editor.eventRequestsInlineFormatShortcut(event))
            return documentFlow.applyInlineFormatShortcut(event);
        if (editor.eventRequestsBodyTagShortcut(event))
            return documentFlow.applyBodyTagShortcut(event);
        return false;
    }

    function requestViewHook(reason) {
        documentFlow.viewHookRequested(reason !== undefined ? String(reason) : "manual");
    }

    function refreshLastReadyProjection() {
        if (!documentFlow.logicalProjectionReady)
            return;
        if (documentFlow.lastReadyEditorSurfaceHtml !== documentFlow.editorSurfaceHtml)
            documentFlow.lastReadyEditorSurfaceHtml = documentFlow.editorSurfaceHtml;
        if (documentFlow.lastReadyLogicalText !== documentFlow.logicalText)
            documentFlow.lastReadyLogicalText = documentFlow.logicalText;
    }

    function pointRequestsTerminalBodyClick(localX, localY) {
        const x = Number(localX) || 0;
        const y = Number(localY) || 0;
        if (x < 0 || x > documentFlow.width || y < 0 || y > documentFlow.height)
            return false;

        const renderedBodyHeight = Math.max(0, Number(editor.displayBodyHeight) || 0);
        if (renderedBodyHeight <= 0)
            return true;
        return y > renderedBodyHeight;
    }

    function focusTerminalBodyFromPoint(localX, localY) {
        if (!documentFlow.pointRequestsTerminalBodyClick(localX, localY))
            return false;
        editor.focusTerminalBodyPosition();
        return true;
    }

    function terminalBodySurfaceY() {
        const renderedBodyHeight = Math.max(0, Number(editor.displayBodyHeight) || 0);
        if (renderedBodyHeight <= 0)
            return 0;
        const minimumBodyLineHeight = Math.max(1, Number(LV.Theme.textBodyLineHeight) || 1);
        return Math.min(
                    documentFlow.height,
                    renderedBodyHeight + minimumBodyLineHeight + 1);
    }

    onEditorSurfaceHtmlChanged: documentFlow.refreshLastReadyProjection()
    onLogicalProjectionReadyChanged: documentFlow.refreshLastReadyProjection()
    onLogicalTextChanged: documentFlow.refreshLastReadyProjection()

    Component.onCompleted: documentFlow.refreshLastReadyProjection()

    ContentsInlineFormatEditor {
        id: editor

        anchors.fill: parent
        coordinateMapper: documentFlow.coordinateMapper
        documentBlocks: documentFlow.documentBlocks
        displayGeometryText: documentFlow.resolvedDisplayGeometryText
        normalizedHtmlBlocks: documentFlow.normalizedHtmlBlocks
        objectName: "contentsStructuredDocumentInlineEditor"
        renderedText: documentFlow.resolvedEditorSurfaceHtml
        resourceVisualBlocks: documentFlow.resourceVisualBlocks
        showRenderedOutput: true
        tagManagementKeyPressHandler: function (event) {
            return documentFlow.handleTagManagementKeyPress(event);
        }
        text: documentFlow.sourceText
        textColor: documentFlow.textColor

        onTextEdited: function (text) {
            if (text !== documentFlow.sourceText)
                documentFlow.sourceTextEdited(text);
        }

        onViewHookRequested: function (reason) {
            documentFlow.viewHookRequested(reason);
        }
    }

    ContentsEditorTagInsertionController {
        id: tagInsertionController
    }

    MouseArea {
        id: terminalBodyClickSurface

        acceptedButtons: Qt.LeftButton
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Math.max(0, documentFlow.height - y)
        objectName: "contentsStructuredDocumentTerminalBodyClickSurface"
        y: documentFlow.terminalBodySurfaceY()
        propagateComposedEvents: true
        z: 1

        onClicked: function (mouse) {
            if (!documentFlow.focusTerminalBodyFromPoint(mouse.x, terminalBodyClickSurface.y + mouse.y))
                mouse.accepted = false;
        }

        onPressed: function (mouse) {
            mouse.accepted = documentFlow.pointRequestsTerminalBodyClick(
                        mouse.x,
                        terminalBodyClickSurface.y + mouse.y);
        }
    }
}
