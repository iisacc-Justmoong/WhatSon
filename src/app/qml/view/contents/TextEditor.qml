pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.TextEditor {
    id: textEditor

    backgroundColor: "transparent"
    backgroundColorDisabled: "transparent"
    backgroundColorFocused: "transparent"
    backgroundColorHover: "transparent"
    backgroundColorPressed: "transparent"
    cornerRadius: LV.Theme.gapNone
    editorHeight: Math.max(1, height)
    fieldMinHeight: LV.Theme.gap16
    filePath: ""
    fontFamily: LV.Theme.fontBody
    fontPixelSize: LV.Theme.textBody
    fontWeight: LV.Theme.textBodyWeight
    insetHorizontal: LV.Theme.gapNone
    insetVertical: LV.Theme.gapNone
    objectName: "contentsTextEditor"
    preferNativeGestures: true
    showScrollBar: false
    textColor: LV.Theme.bodyColor
    textColorDisabled: textColor
}
