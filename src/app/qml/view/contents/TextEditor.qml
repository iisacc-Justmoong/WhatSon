pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.TextEditor {
    id: textEditor

    property bool editorReadOnly: false
    property string noteBodyFilePath: ""
    property var viewportFlickable: null
    readonly property string editorDocumentText: textEditor.text !== undefined ? String(textEditor.text) : ""
    readonly property real viewportContentY: textEditor.viewportFlickable
            && textEditor.viewportFlickable.contentY !== undefined
            ? Number(textEditor.viewportFlickable.contentY)
            : 0

    function findDescendantByObjectName(root, objectName) {
        if (!root)
            return null;

        const objectLists = [];
        if (root.children !== undefined)
            objectLists.push(root.children);
        if (root.data !== undefined)
            objectLists.push(root.data);

        for (let listIndex = 0; listIndex < objectLists.length; ++listIndex) {
            const childItems = objectLists[listIndex];
            for (let childIndex = 0; childIndex < childItems.length; ++childIndex) {
                const child = childItems[childIndex];
                if (!child || child === root)
                    continue;
                if (child.objectName === objectName)
                    return child;

                const descendant = textEditor.findDescendantByObjectName(child, objectName);
                if (descendant)
                    return descendant;
            }
        }
        return null;
    }

    function replaceEditorDocumentText(nextText, nextCursorPosition) {
        textEditor.text = nextText === undefined || nextText === null
                ? ""
                : String(nextText);
        textEditor.cursorPosition = Math.max(0, Number(nextCursorPosition) || 0);
        return true;
    }

    function pasteNativeClipboardText() {
        textEditor.forceEditorFocus();
        textEditor.paste();
        return true;
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
