pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: documentFlow

    property var htmlTokens: []
    property var logicalToSourceOffsets: []
    property var normalizedHtmlBlocks: []
    property bool paperPaletteEnabled: false
    property string editorSurfaceHtml: ""
    property string logicalText: ""
    property int logicalCursorPosition: sourceText.length
    property string sourceText: ""
    property color textColor: LV.Theme.bodyColor
    readonly property real editorContentHeight: editor.displayContentHeight
    readonly property int editorCursorPosition: editor.cursorPosition

    signal sourceTextEdited(string text)
    signal viewHookRequested(string reason)

    function normalizedBlocks() {
        return documentFlow.normalizedHtmlBlocks;
    }

    function requestViewHook(reason) {
        documentFlow.viewHookRequested(reason !== undefined ? String(reason) : "manual");
    }

    function logicalOffsetToSourceOffset(logicalOffset) {
        const fallbackOffset = Math.max(0, Math.min(Number(logicalOffset) || 0, documentFlow.sourceText.length));
        const offsets = documentFlow.logicalToSourceOffsets;
        if (!offsets || offsets.length === undefined || offsets.length <= 0)
            return fallbackOffset;
        const safeIndex = Math.max(0, Math.min(Number(logicalOffset) || 0, offsets.length - 1));
        const mappedOffset = Number(offsets[Math.floor(safeIndex)]);
        if (!isFinite(mappedOffset))
            return fallbackOffset;
        return Math.max(0, Math.min(mappedOffset, documentFlow.sourceText.length));
    }

    function pointRequestsTerminalBodyClick(localX, localY) {
        const x = Number(localX) || 0;
        const y = Number(localY) || 0;
        if (x < 0 || x > documentFlow.width || y < 0 || y > documentFlow.height)
            return false;

        const renderedBodyHeight = Math.max(0, Number(editor.displayContentHeight) || 0);
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

    ContentsInlineFormatEditor {
        id: editor

        anchors.fill: parent
        displayGeometryText: documentFlow.logicalText.length > 0 ? documentFlow.logicalText : documentFlow.sourceText
        logicalCursorPosition: documentFlow.logicalCursorPosition
        logicalToSourceOffsets: documentFlow.logicalToSourceOffsets
        normalizedHtmlBlocks: documentFlow.normalizedHtmlBlocks
        renderedText: documentFlow.editorSurfaceHtml
        showRenderedOutput: true
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

    MouseArea {
        id: terminalBodyClickSurface

        acceptedButtons: Qt.LeftButton
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Math.max(0, documentFlow.height - y)
        y: Math.min(
               documentFlow.height,
               Math.max(0, Number(editor.displayContentHeight) || 0))
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
