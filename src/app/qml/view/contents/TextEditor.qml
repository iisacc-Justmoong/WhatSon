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
    property bool cursorViewportSyncQueued: false
    readonly property string editorDocumentText: textEditor.text !== undefined ? String(textEditor.text) : ""
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
    readonly property real editorTextContentBottomY: {
        textEditor.editorLineMetricsRevision;
        return textEditor.editorTextContentBottom();
    }
    readonly property real editorPaddedEditorItemHeight: Math.max(
            textEditor.editorViewportHeight,
            textEditor.editorTextContentBottomY + textEditor.editorBottomViewportPadding)
    readonly property string editorPlainText: {
        textEditor.editorPlainTextRevision;
        return textEditor.editorSurfacePlainText();
    }
    readonly property int editorCursorLineIndex: textEditor.cursorLineIndexForLogicalCursor()

    function numberOrFallback(value, fallbackValue) {
        const numericValue = Number(value);
        return Number.isFinite(numericValue) ? numericValue : fallbackValue;
    }

    function editorSurfaceObject() {
        return textEditor.editorItem !== undefined ? textEditor.editorItem : null;
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

    function editorSurfacePlainText() {
        const editorSurface = textEditor.editorSurfaceObject();
        if (editorSurface
                && editorSurface.getText !== undefined
                && editorSurface.length !== undefined) {
            const safeLength = Math.max(0, Math.floor(Number(editorSurface.length) || 0));
            return textEditor.normalizedEditorPlainText(editorSurface.getText(0, safeLength));
        }

        return textEditor.normalizedEditorPlainText(textEditor.editorDocumentText);
    }

    function editorTextContentBottom() {
        const editorSurface = textEditor.editorSurfaceObject();
        const fallbackHeight = Math.max(1, Number(textEditor.editorLogicalLineHeight) || 1);
        if (!editorSurface)
            return fallbackHeight;

        const safeLength = editorSurface.length !== undefined
                ? Math.max(0, Math.floor(Number(editorSurface.length) || 0))
                : textEditor.editorDocumentText.length;
        if (editorSurface.positionToRectangle !== undefined) {
            const endRectangle = editorSurface.positionToRectangle(safeLength);
            if (endRectangle) {
                const endY = textEditor.numberOrFallback(endRectangle.y, 0);
                const endHeight = Math.max(
                            1,
                            textEditor.numberOrFallback(endRectangle.height, fallbackHeight));
                return Math.max(fallbackHeight, endY + endHeight);
            }
        }

        return Math.max(
                    fallbackHeight,
                    textEditor.numberOrFallback(editorSurface.contentHeight, fallbackHeight));
    }

    function editorPlainTextLineRecords() {
        const documentText = textEditor.editorPlainText;
        const lineRecords = [];
        let lineStartPosition = 0;

        for (let position = 0; position < documentText.length; ++position) {
            if (documentText.charAt(position) !== "\n")
                continue;

            lineRecords.push({
                                 text: documentText.slice(lineStartPosition, position),
                                 start: lineStartPosition,
                                 end: position
                             });
            lineStartPosition = position + 1;
        }

        lineRecords.push({
                             text: documentText.slice(lineStartPosition),
                             start: lineStartPosition,
                             end: documentText.length
                         });
        return lineRecords;
    }

    function sourceAlignedLineStartPositions() {
        const lineRecords = textEditor.editorPlainTextLineRecords();
        const lineStartPositions = [];

        for (let lineIndex = 0; lineIndex < lineRecords.length; ++lineIndex)
            lineStartPositions.push(lineRecords[lineIndex].start);

        if (lineStartPositions.length <= 0)
            lineStartPositions.push(0);
        return lineStartPositions;
    }

    function sourceAlignedLineIndexForPosition(position) {
        const lineStartPositions = textEditor.sourceAlignedLineStartPositions();
        const safePosition = Math.max(
                    0,
                    Math.min(
                        textEditor.editorPlainText.length,
                        Math.floor(Number(position) || 0)));

        let sourceLineIndex = 0;
        for (let index = 0; index < lineStartPositions.length; ++index) {
            if (lineStartPositions[index] > safePosition)
                break;
            sourceLineIndex = index;
        }
        return sourceLineIndex;
    }

    function sourceAlignedOverflowMetricFor(lineIndex, lineStartPositions, fallbackMetric, fallbackHeight) {
        const editorSurface = textEditor.editorSurfaceObject();
        if (!editorSurface
                || editorSurface.positionToRectangle === undefined
                || !lineStartPositions
                || lineStartPositions.length <= 0)
            return fallbackMetric;

        const lastLineIndex = Math.max(0, lineStartPositions.length - 1);
        const lastRectangle = editorSurface.positionToRectangle(lineStartPositions[lastLineIndex]);
        const endRectangle = editorSurface.positionToRectangle(textEditor.editorPlainText.length);
        if (!lastRectangle && !endRectangle)
            return fallbackMetric;

        const editorSurfaceY = textEditor.numberOrFallback(editorSurface.y, 0);
        const lastY = lastRectangle
                ? Math.max(0, editorSurfaceY + textEditor.numberOrFallback(lastRectangle.y, fallbackMetric.y))
                : fallbackMetric.y;
        const lastHeight = lastRectangle
                ? Math.max(1, textEditor.numberOrFallback(lastRectangle.height, fallbackHeight))
                : fallbackHeight;
        const endY = endRectangle
                ? Math.max(0, editorSurfaceY + textEditor.numberOrFallback(endRectangle.y, lastY + lastHeight))
                : lastY + lastHeight;
        const missingLineOffset = Math.max(0, Math.floor(Number(lineIndex) || 0) - lineStartPositions.length);

        return {
            y: Math.max(endY, lastY + lastHeight) + missingLineOffset * fallbackHeight,
            height: fallbackHeight
        };
    }

    function cursorLineIndexForLogicalCursor() {
        textEditor.cursorPosition;
        textEditor.editorPlainTextRevision;
        return textEditor.sourceAlignedLineIndexForPosition(textEditor.cursorPosition);
    }

    function editorLogicalLineMetricFor(lineIndex) {
        textEditor.editorLineMetricsRevision;
        const normalizedIndex = Math.max(0, Math.floor(Number(lineIndex) || 0));
        const fallbackHeight = Math.max(1, Number(textEditor.editorLogicalLineHeight) || 1);
        const fallbackMetric = {
            y: normalizedIndex * fallbackHeight,
            height: fallbackHeight
        };
        const editorSurface = textEditor.editorSurfaceObject();
        if (!editorSurface || editorSurface.positionToRectangle === undefined)
            return fallbackMetric;

        const lineStartPositions = textEditor.sourceAlignedLineStartPositions();
        if (normalizedIndex >= lineStartPositions.length)
            return textEditor.sourceAlignedOverflowMetricFor(
                        normalizedIndex,
                        lineStartPositions,
                        fallbackMetric,
                        fallbackHeight);

        const lineStartPosition = lineStartPositions[normalizedIndex];
        const nextLineStartPosition = normalizedIndex + 1 < lineStartPositions.length
                ? lineStartPositions[normalizedIndex + 1]
                : -1;
        const rectangle = editorSurface.positionToRectangle(lineStartPosition);
        if (!rectangle)
            return fallbackMetric;

        const y = Math.max(
                    0,
                    textEditor.numberOrFallback(editorSurface.y, 0)
                    + textEditor.numberOrFallback(rectangle.y, fallbackMetric.y));
        let height = Math.max(1, textEditor.numberOrFallback(rectangle.height, fallbackHeight));
        if (nextLineStartPosition >= 0) {
            const nextRectangle = editorSurface.positionToRectangle(nextLineStartPosition);
            if (nextRectangle) {
                const nextY = Math.max(
                            0,
                            textEditor.numberOrFallback(editorSurface.y, 0)
                            + textEditor.numberOrFallback(nextRectangle.y, y + height));
                if (nextY > y)
                    height = Math.max(height, nextY - y);
            }
        }

        return {
            y: y,
            height: height
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

    function cursorRectangleInViewportContent() {
        const editorSurface = textEditor.editorSurfaceObject();
        if (!editorSurface || editorSurface.positionToRectangle === undefined)
            return null;

        const rectangle = editorSurface.positionToRectangle(textEditor.cursorPosition);
        if (!rectangle)
            return null;

        const fallbackHeight = Math.max(1, Number(textEditor.editorLogicalLineHeight) || 1);
        const surfaceY = textEditor.numberOrFallback(editorSurface.y, 0);
        return {
            y: Math.max(0, surfaceY + textEditor.numberOrFallback(rectangle.y, 0)),
            height: Math.max(1, textEditor.numberOrFallback(rectangle.height, fallbackHeight))
        };
    }

    function ensureCursorVisibleInViewport() {
        const viewport = textEditor.viewportFlickable;
        if (!viewport || viewport.contentY === undefined)
            return false;

        const cursorRectangle = textEditor.cursorRectangleInViewportContent();
        if (!cursorRectangle)
            return false;

        const contentHeight = Math.max(0, Number(viewport.contentHeight) || 0);
        const viewportHeight = Math.max(1, Number(viewport.height) || 1);
        const maxContentY = Math.max(0, contentHeight - viewportHeight);
        const currentContentY = textEditor.numberOrFallback(viewport.contentY, 0);
        const cursorMargin = Math.max(
                    0,
                    Math.min(
                        Math.max(1, Number(textEditor.editorLogicalLineHeight) || 1),
                        viewportHeight / 3));
        const visibleTop = currentContentY;
        const visibleBottom = currentContentY + viewportHeight;
        const cursorTop = cursorRectangle.y;
        const cursorBottom = cursorRectangle.y + cursorRectangle.height;
        let nextContentY = currentContentY;

        if (cursorTop < visibleTop + cursorMargin)
            nextContentY = cursorTop - cursorMargin;
        else if (cursorBottom > visibleBottom - cursorMargin)
            nextContentY = cursorBottom + cursorMargin - viewportHeight;

        nextContentY = Math.max(0, Math.min(maxContentY, nextContentY));
        if (Math.abs(nextContentY - currentContentY) < 0.5)
            return false;

        viewport.contentY = Math.max(0, Math.min(maxContentY, nextContentY));
        return true;
    }

    function requestEnsureCursorVisibleInViewport() {
        if (textEditor.cursorViewportSyncQueued)
            return false;

        textEditor.cursorViewportSyncQueued = true;
        Qt.callLater(function () {
            textEditor.cursorViewportSyncQueued = false;
            textEditor.ensureCursorVisibleInViewport();
        });
        return true;
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

    autoFocusOnPress: !LV.Theme.mobileTarget
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
    preferNativeGestures: false
    readOnly: textEditor.editorReadOnly || textEditor.noteBodyFilePath.trim().length === 0
    selectByMouse: true
    showScrollBar: false
    mouseSelectionMode: TextEdit.SelectCharacters
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
    onCursorPositionChanged: textEditor.requestEnsureCursorVisibleInViewport()
    onWidthChanged: textEditor.bumpEditorLineMetricsRevision()

    Binding {
        property: "bottomPadding"
        restoreMode: Binding.RestoreBindingOrValue
        target: textEditor.editorItem
        value: textEditor.editorBottomViewportPadding
    }

    Binding {
        property: "height"
        restoreMode: Binding.RestoreBindingOrValue
        target: textEditor.editorItem
        value: textEditor.editorPaddedEditorItemHeight
    }
}
