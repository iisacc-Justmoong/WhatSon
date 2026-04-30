pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: editorView

    property string editorText: ""
    property color textColor: LV.Theme.bodyColor

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        editorView.viewHookRequested(hookReason);
    }

    clip: true
    objectName: "figma-155-5352-EditorView"

    TextEdit {
        id: editorTextSurface

        anchors.fill: parent
        anchors.leftMargin: LV.Theme.gap16
        anchors.rightMargin: LV.Theme.gap16
        color: editorView.textColor
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        font.weight: LV.Theme.textBodyWeight
        padding: LV.Theme.gapNone
        readOnly: true
        selectByMouse: true
        selectedTextColor: editorView.textColor
        selectionColor: LV.Theme.primaryOverlay
        text: editorView.editorText
        textFormat: TextEdit.PlainText
        verticalAlignment: TextEdit.AlignTop
        wrapMode: TextEdit.Wrap
    }
}
