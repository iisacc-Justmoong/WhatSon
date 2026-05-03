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

    LV.TextEditor {
        id: editorTextSurface

        anchors.fill: parent
        anchors.leftMargin: LV.Theme.gap16
        anchors.rightMargin: LV.Theme.gap16
        backgroundColor: "transparent"
        backgroundColorDisabled: "transparent"
        backgroundColorFocused: "transparent"
        backgroundColorHover: "transparent"
        backgroundColorPressed: "transparent"
        centeredTextHeight: Math.max(1, editorTextSurface.resolvedEditorHeight - editorTextSurface.insetVertical * 2)
        cornerRadius: LV.Theme.gapNone
        editorHeight: Math.max(1, editorView.height)
        enforceModeDefaults: true
        fieldMinHeight: LV.Theme.gap16
        fontFamily: LV.Theme.fontBody
        fontPixelSize: LV.Theme.textBody
        fontWeight: LV.Theme.textBodyWeight
        insetHorizontal: LV.Theme.gapNone
        insetVertical: LV.Theme.gapNone
        mode: plainTextMode
        readOnly: true
        selectByMouse: true
        selectedTextColor: editorView.textColor
        selectionColor: LV.Theme.primaryOverlay
        showRenderedOutput: false
        showScrollBar: false
        text: editorView.editorText
        textColor: editorView.textColor
        textColorDisabled: editorView.textColor
        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.Wrap
    }
}
