pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: documentFlow

    property var htmlTokens: []
    property var normalizedHtmlBlocks: []
    property bool paperPaletteEnabled: false
    property string editorSurfaceHtml: ""
    property string sourceText: ""
    property color textColor: LV.Theme.bodyColor

    signal sourceTextEdited(string text)
    signal viewHookRequested(string reason)

    function normalizedBlocks() {
        return documentFlow.normalizedHtmlBlocks;
    }

    function requestViewHook(reason) {
        documentFlow.viewHookRequested(reason !== undefined ? String(reason) : "manual");
    }

    ContentsInlineFormatEditor {
        id: editor

        anchors.fill: parent
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
}
