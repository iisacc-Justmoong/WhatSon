pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.TextEditor {
    id: textEditor

    property bool editorReadOnly: false
    property string noteBodyFilePath: ""
    property var viewportFlickable: null
    readonly property real viewportContentY: textEditor.viewportFlickable
            && textEditor.viewportFlickable.contentY !== undefined
            ? Number(textEditor.viewportFlickable.contentY)
            : 0

    function findDescendantByObjectName(root, objectName) {
        if (!root || root.children === undefined)
            return null;

        const childItems = root.children;
        for (let childIndex = 0; childIndex < childItems.length; ++childIndex) {
            const child = childItems[childIndex];
            if (child && child.objectName === objectName)
                return child;

            const descendant = textEditor.findDescendantByObjectName(child, objectName);
            if (descendant)
                return descendant;
        }
        return null;
    }

    backgroundColor: "transparent"
    backgroundColorDisabled: "transparent"
    backgroundColorFocused: "transparent"
    backgroundColorHover: "transparent"
    backgroundColorPressed: "transparent"
    cornerRadius: LV.Theme.gapNone
    editorHeight: Math.max(1, height)
    fieldMinHeight: LV.Theme.gap16
    filePath: textEditor.noteBodyFilePath
    fontFamily: LV.Theme.fontBody
    fontPixelSize: LV.Theme.textBody
    fontWeight: LV.Theme.textBodyWeight
    insetHorizontal: LV.Theme.gapNone
    insetVertical: LV.Theme.gapNone
    objectName: "contentsTextEditor"
    preferNativeGestures: true
    readOnly: textEditor.editorReadOnly || textEditor.noteBodyFilePath.trim().length === 0
    showScrollBar: false
    textColor: LV.Theme.bodyColor
    textColorDisabled: textColor

    Component.onCompleted: {
        textEditor.viewportFlickable = textEditor.findDescendantByObjectName(
                    textEditor,
                    "editorViewportFlickable");
    }
}
