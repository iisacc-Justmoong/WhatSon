pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: surfaceHost

    property string editorSurfaceHtml: ""
    property color textColor: LV.Theme.bodyColor

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        surfaceHost.viewHookRequested(reason !== undefined ? String(reason) : "manual");
    }

    Text {
        id: renderedText

        anchors.fill: parent
        color: surfaceHost.textColor
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        linkColor: LV.Theme.primary
        padding: LV.Theme.gap16
        text: surfaceHost.editorSurfaceHtml
        textFormat: Text.RichText
        wrapMode: Text.Wrap

        onLinkActivated: function (link) {
            Qt.openUrlExternally(link);
        }
    }
}
