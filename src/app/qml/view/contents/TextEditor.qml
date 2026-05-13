pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.TextEditor {
    id: textEditor

    property bool editorReadOnly: false
    property string noteBodyFilePath: ""
    property var viewportFlickable: null
    property int editorPlainTextRevision: 0
    property int editorLineMetricsRevision: 0
    property real editorBottomViewportPaddingRatio: 0.75
    readonly property string editorDocumentText: textEditor.text !== undefined ? String(textEditor.text) : ""
    readonly property string editorSelectedText: textEditor.normalizedEditorPlainText(
            textEditor.selectedText !== undefined ? String(textEditor.selectedText) : "")
    readonly property int editorSelectionStart: textEditor.normalizedSelectionStart()
    readonly property int editorSelectionLength: Math.max(0, textEditor.normalizedSelectionEnd() - textEditor.editorSelectionStart)
    readonly property real viewportContentY: textEditor.viewportFlickable
            && textEditor.viewportFlickable.contentY !== undefined
            ? Number(textEditor.viewportFlickable.contentY)
            : 0
    readonly property real editorViewportHeight: textEditor.viewportFlickable
            && textEditor.viewportFlickable.height !== undefined
            ? Math.max(1, Number(textEditor.viewportFlickable.height) || 1)
            : Math.max(1, Number(textEditor.height) || 1)
    readonly property real editorViewportContentHeight: textEditor.viewportFlickable
            && textEditor.viewportFlickable.contentHeight !== undefined
            ? Math.max(textEditor.editorViewportHeight, Number(textEditor.viewportFlickable.contentHeight) || 0)
            : Math.max(textEditor.editorViewportHeight, textEditor.editorItem && textEditor.editorItem.contentHeight !== undefined
                       ? Number(textEditor.editorItem.contentHeight) || 0
                       : 0)
    readonly property real editorViewportWidth: textEditor.editorItem
            && textEditor.editorItem.width !== undefined
            ? Math.max(1, Number(textEditor.editorItem.width) || 1)
            : Math.max(1, Number(textEditor.width) || 1)
    readonly property real editorBottomViewportPadding: Math.max(
            0,
            textEditor.editorViewportHeight * Math.max(0, Number(textEditor.editorBottomViewportPaddingRatio) || 0))
    readonly property real editorLogicalLineHeight: Math.max(1, Number(textEditor.lineHeight) || 1)
    readonly property string editorPlainText: {
        textEditor.editorPlainTextRevision;
        return textEditor.editorSurfacePlainText();
    }
    readonly property int editorCursorLineIndex: textEditor.cursorLineIndexForLogicalCursor()

    function numberOrFallback(value, fallbackValue) {
        const numericValue = Number(value);
        return Number.isFinite(numericValue) ? numericValue : fallbackValue;
    }

    function normalizedSelectionStart() {
        if (textEditor.selectionStart !== undefined)
            return Math.max(0, Math.floor(Number(textEditor.selectionStart) || 0));
        if (textEditor.selectedText !== undefined) {
            const selectedLength = String(textEditor.selectedText).length;
            if (selectedLength > 0)
                return Math.max(0, Math.floor(Number(textEditor.cursorPosition) || 0) - selectedLength);
        }
        return Math.max(0, Math.floor(Number(textEditor.cursorPosition) || 0));
    }

    function normalizedSelectionEnd() {
        if (textEditor.selectionEnd !== undefined)
            return Math.max(textEditor.normalizedSelectionStart(), Math.floor(Number(textEditor.selectionEnd) || 0));
        if (textEditor.selectedText !== undefined) {
            const selectedLength = String(textEditor.selectedText).length;
            if (selectedLength > 0)
                return textEditor.normalizedSelectionStart() + selectedLength;
        }
        return textEditor.normalizedSelectionStart();
    }

    function bumpEditorPlainTextRevision() {
        textEditor.editorPlainTextRevision = (textEditor.editorPlainTextRevision + 1) % 1000000;
    }

    function bumpEditorLineMetricsRevision() {
        textEditor.editorLineMetricsRevision = (textEditor.editorLineMetricsRevision + 1) % 1000000;
    }

    function normalizedEditorPlainText(value) {
        return (value === undefined || value === null ? "" : String(value))
                .replace(/\r\n/g, "\n")
                .replace(/\r/g, "\n")
                .replace(/\u2028/g, "\n")
                .replace(/\u2029/g, "\n");
    }

    function cursorLineIndexFor(documentText, cursorPosition) {
        const normalizedText = textEditor.normalizedEditorPlainText(documentText);
        const safeCursorPosition = Math.max(
                    0,
                    Math.min(
                        normalizedText.length,
                        Math.floor(Number(cursorPosition) || 0)));
        return normalizedText.slice(0, safeCursorPosition).split("\n").length - 1;
    }

    function editorSurfacePlainText() {
        const editorSurface = textEditor.editorItem !== undefined ? textEditor.editorItem : null;
        if (editorSurface
                && editorSurface.getText !== undefined
                && editorSurface.length !== undefined) {
            const safeLength = Math.max(0, Math.floor(Number(editorSurface.length) || 0));
            return textEditor.normalizedEditorPlainText(editorSurface.getText(0, safeLength));
        }

        return textEditor.normalizedEditorPlainText(textEditor.editorDocumentText);
    }

    function cursorLineIndexForLogicalCursor() {
        textEditor.cursorPosition;
        textEditor.editorPlainTextRevision;
        return textEditor.cursorLineIndexFor(
                    textEditor.editorPlainText,
                    textEditor.cursorPosition);
    }

    function logicalLineStartPositionFor(lineIndex) {
        const normalizedIndex = Math.max(0, Math.floor(Number(lineIndex) || 0));
        const documentText = textEditor.editorPlainText;
        if (normalizedIndex <= 0)
            return 0;

        let currentLineIndex = 0;
        for (let position = 0; position < documentText.length; ++position) {
            if (documentText.charAt(position) === "\n") {
                ++currentLineIndex;
                if (currentLineIndex >= normalizedIndex)
                    return position + 1;
            }
        }
        return documentText.length;
    }

    function editorLogicalLineMetricFor(lineIndex) {
        textEditor.editorLineMetricsRevision;
        const normalizedIndex = Math.max(0, Math.floor(Number(lineIndex) || 0));
        const fallbackHeight = Math.max(1, Number(textEditor.editorLogicalLineHeight) || 1);
        const fallbackMetric = {
            y: normalizedIndex * fallbackHeight,
            height: fallbackHeight
        };
        const editorSurface = textEditor.editorItem !== undefined ? textEditor.editorItem : null;
        if (!editorSurface || editorSurface.positionToRectangle === undefined)
            return fallbackMetric;

        const lineStartPosition = textEditor.logicalLineStartPositionFor(normalizedIndex);
        const rectangle = editorSurface.positionToRectangle(lineStartPosition);
        if (!rectangle)
            return fallbackMetric;

        return {
            y: Math.max(
                    0,
                    textEditor.numberOrFallback(editorSurface.y, 0)
                    + textEditor.numberOrFallback(rectangle.y, fallbackMetric.y)),
            height: Math.max(1, textEditor.numberOrFallback(rectangle.height, fallbackHeight))
        };
    }

    function scrollEditorViewportTo(contentY) {
        const viewport = textEditor.viewportFlickable;
        if (!viewport || viewport.contentY === undefined)
            return false;

        const contentHeight = Math.max(0, Number(viewport.contentHeight) || 0);
        const viewportHeight = Math.max(1, Number(viewport.height) || 1);
        const maxContentY = Math.max(0, contentHeight - viewportHeight);
        const requestedContentY = textEditor.numberOrFallback(contentY, viewport.contentY);
        viewport.contentY = Math.max(0, Math.min(maxContentY, requestedContentY));
        return true;
    }

    function boundedCursorPosition(value) {
        const textLength = textEditor.length !== undefined
                ? Math.max(0, Math.floor(Number(textEditor.length) || 0))
                : textEditor.editorDocumentText.length;
        return Math.max(
                    0,
                    Math.min(
                        textLength,
                        Math.floor(Number(value) || 0)));
    }

    function restoreEditorCursorPosition(nextCursorPosition) {
        const targetCursorPosition = textEditor.boundedCursorPosition(nextCursorPosition);
        textEditor.forceEditorFocus();
        textEditor.cursorPosition = targetCursorPosition;
        textEditor.deselect();
        Qt.callLater(function () {
            const deferredCursorPosition = textEditor.boundedCursorPosition(targetCursorPosition);
            textEditor.forceEditorFocus();
            textEditor.cursorPosition = deferredCursorPosition;
            textEditor.deselect();
        });
    }

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
        textEditor.restoreEditorCursorPosition(nextCursorPosition);
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
    preferNativeGestures: LV.Theme.mobileTarget
    readOnly: textEditor.editorReadOnly || textEditor.noteBodyFilePath.trim().length === 0
    showScrollBar: false
    textColor: LV.Theme.bodyColor
    textColorDisabled: textColor

    Component.onCompleted: {
        textEditor.viewportFlickable = textEditor.findDescendantByObjectName(
                    textEditor,
                    "editorViewportFlickable");
        textEditor.bumpEditorPlainTextRevision();
        textEditor.bumpEditorLineMetricsRevision();
    }

    onTextChanged: {
        textEditor.bumpEditorPlainTextRevision();
        textEditor.bumpEditorLineMetricsRevision();
    }
    onWidthChanged: textEditor.bumpEditorLineMetricsRevision()

    Binding {
        property: "bottomPadding"
        restoreMode: Binding.RestoreBindingOrValue
        target: textEditor.editorItem
        value: textEditor.editorBottomViewportPadding
    }

    Connections {
        target: textEditor.editorItem

        function onContentHeightChanged() {
            textEditor.bumpEditorLineMetricsRevision();
        }

        function onLineCountChanged() {
            textEditor.bumpEditorLineMetricsRevision();
        }

        function onWidthChanged() {
            textEditor.bumpEditorLineMetricsRevision();
        }
    }
}
