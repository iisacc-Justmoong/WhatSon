pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.TextEditor {
    id: textEditor

    property bool editorReadOnly: false
    property string noteBodyFilePath: ""
    property var viewportFlickable: null
    property int editorLineMetricsRevision: 0
    property real editorBottomViewportPaddingRatio: 0.5
    readonly property string editorDocumentText: textEditor.text !== undefined ? String(textEditor.text) : ""
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
    readonly property real editorMeasuredContentHeight: {
        const editorSurface = textEditor.editorItem !== undefined ? textEditor.editorItem : null;
        if (!editorSurface || editorSurface.contentHeight === undefined)
            return 0;

        const measuredHeight = Number(editorSurface.contentHeight) || 0;
        return Math.max(0, measuredHeight - textEditor.editorBottomViewportPadding);
    }
    readonly property int editorRenderedLineCount: {
        const editorSurface = textEditor.editorItem !== undefined ? textEditor.editorItem : null;
        if (editorSurface && editorSurface.lineCount !== undefined)
            return Math.max(0, Math.floor(Number(editorSurface.lineCount) || 0));

        const documentText = textEditor.editorDocumentText;
        return documentText.trim().length > 0
                ? Math.max(1, documentText.split("\n").length)
                : 0;
    }
    readonly property int editorCursorLineIndex: textEditor.cursorLineIndexForVisualCursor()
    readonly property real editorVisualLineHeight: {
        const editorSurface = textEditor.editorItem !== undefined ? textEditor.editorItem : null;
        if (editorSurface
                && editorSurface.contentHeight !== undefined
                && editorSurface.lineCount !== undefined) {
            const visualLineCount = Math.max(1, textEditor.editorRenderedLineCount);
            const visualContentHeight = textEditor.editorMeasuredContentHeight;
            if (visualContentHeight > 0)
                return Math.max(1, visualContentHeight / visualLineCount);
        }
        return Math.max(1, Number(textEditor.lineHeight) || 1);
    }

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

    function bumpEditorLineMetricsRevision() {
        textEditor.editorLineMetricsRevision = (textEditor.editorLineMetricsRevision + 1) % 1000000;
    }

    function cursorLineIndexFor(documentText, cursorPosition) {
        const normalizedText = documentText === undefined || documentText === null
                ? ""
                : String(documentText);
        const safeCursorPosition = Math.max(
                    0,
                    Math.min(
                        normalizedText.length,
                        Math.floor(Number(cursorPosition) || 0)));
        return normalizedText.slice(0, safeCursorPosition).split("\n").length - 1;
    }

    function cursorLineIndexForVisualCursor() {
        textEditor.cursorPosition;
        textEditor.editorLineMetricsRevision;
        const editorSurface = textEditor.editorItem !== undefined ? textEditor.editorItem : null;
        if (editorSurface
                && editorSurface.cursorRectangle !== undefined) {
            const fallbackHeight = Math.max(1, Number(textEditor.editorVisualLineHeight) || 1);
            const renderedLineCount = Math.max(1, textEditor.editorRenderedLineCount);
            const metrics = textEditor.buildEditorVisualLineMetrics(renderedLineCount, fallbackHeight);
            const cursorY = Math.max(
                        0,
                        textEditor.numberOrFallback(editorSurface.y, 0)
                        + textEditor.numberOrFallback(editorSurface.cursorRectangle.y, 0));

            for (let metricIndex = 0; metricIndex < metrics.length; ++metricIndex) {
                const metric = metrics[metricIndex];
                const top = Math.max(0, textEditor.numberOrFallback(metric.y, metricIndex * fallbackHeight));
                const height = Math.max(1, textEditor.numberOrFallback(metric.height, fallbackHeight));
                const bottom = top + height;
                if (cursorY >= top && cursorY < bottom)
                    return metricIndex;
            }

            if (metrics.length > 0)
                return cursorY < Math.max(0, textEditor.numberOrFallback(metrics[0].y, 0))
                        ? 0
                        : metrics.length - 1;
        }

        return textEditor.cursorLineIndexFor(
                    textEditor.editorDocumentText,
                    textEditor.cursorPosition);
    }

    function buildEditorVisualLineMetrics(requiredCount, fallbackHeight) {
        const editorSurface = textEditor.editorItem !== undefined ? textEditor.editorItem : null;
        const safeFallbackHeight = Math.max(1, Number(fallbackHeight) || 1);
        const safeRequiredCount = Math.max(0, Math.floor(Number(requiredCount) || 0));
        if (!editorSurface
                || editorSurface.positionAt === undefined
                || editorSurface.positionToRectangle === undefined) {
            const fallbackMetrics = [];
            for (let fallbackIndex = 0; fallbackIndex < safeRequiredCount; ++fallbackIndex) {
                fallbackMetrics.push({
                    y: fallbackIndex * safeFallbackHeight,
                    height: safeFallbackHeight
                });
            }
            return fallbackMetrics;
        }

        const editorY = textEditor.numberOrFallback(editorSurface.y, 0);
        const editorWidth = Math.max(1, textEditor.numberOrFallback(editorSurface.width, 1));
        const probeX = Math.min(Math.max(0, editorWidth - 1), 1);
        const availableLineCount = Math.max(0, Math.floor(Number(editorSurface.lineCount) || 0));
        const scanCount = Math.min(safeRequiredCount, availableLineCount);
        const metrics = [];
        let probeY = 0;
        let previousTop = -1;

        for (let lineIndex = 0; lineIndex < scanCount; ++lineIndex) {
            const position = editorSurface.positionAt(probeX, Math.max(0, probeY));
            const rectangle = editorSurface.positionToRectangle(position);
            if (!rectangle) {
                const fallbackTop = lineIndex * safeFallbackHeight;
                metrics.push({
                    y: editorY + fallbackTop,
                    height: safeFallbackHeight
                });
                previousTop = fallbackTop;
                probeY = fallbackTop + safeFallbackHeight + 0.5;
                continue;
            }

            let top = textEditor.numberOrFallback(rectangle.y, lineIndex * safeFallbackHeight);
            const rectangleHeight = Math.max(
                        1,
                        textEditor.numberOrFallback(rectangle.height, safeFallbackHeight));
            if (previousTop >= 0 && top <= previousTop)
                top = previousTop + safeFallbackHeight;

            metrics.push({
                y: Math.max(0, editorY + top),
                height: rectangleHeight
            });
            previousTop = top;
            probeY = top + rectangleHeight + 0.5;
        }

        for (let metricIndex = 0; metricIndex < metrics.length - 1; ++metricIndex)
            metrics[metricIndex].height = Math.max(1, metrics[metricIndex + 1].y - metrics[metricIndex].y);

        while (metrics.length < safeRequiredCount) {
            const previousMetric = metrics.length > 0 ? metrics[metrics.length - 1] : null;
            const nextY = previousMetric
                    ? previousMetric.y + Math.max(1, previousMetric.height)
                    : metrics.length * safeFallbackHeight;
            metrics.push({
                y: nextY,
                height: safeFallbackHeight
            });
        }

        return metrics;
    }

    function editorLineMetricsFor(lineIndex) {
        textEditor.editorLineMetricsRevision;
        const normalizedIndex = Math.max(0, Math.floor(Number(lineIndex) || 0));
        const fallbackHeight = Math.max(1, Number(textEditor.editorVisualLineHeight) || 1);
        const metrics = textEditor.buildEditorVisualLineMetrics(normalizedIndex + 2, fallbackHeight);
        if (normalizedIndex < metrics.length)
            return metrics[normalizedIndex];

        return {
            y: normalizedIndex * fallbackHeight,
            height: fallbackHeight
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
    preferNativeGestures: LV.Theme.mobileTarget
    readOnly: textEditor.editorReadOnly || textEditor.noteBodyFilePath.trim().length === 0
    showScrollBar: false
    textColor: LV.Theme.bodyColor
    textColorDisabled: textColor

    Component.onCompleted: {
        textEditor.viewportFlickable = textEditor.findDescendantByObjectName(
                    textEditor,
                    "editorViewportFlickable");
        textEditor.bumpEditorLineMetricsRevision();
    }

    onHeightChanged: textEditor.bumpEditorLineMetricsRevision()
    onWidthChanged: textEditor.bumpEditorLineMetricsRevision()
    onTextChanged: textEditor.bumpEditorLineMetricsRevision()
    onEditorBottomViewportPaddingChanged: textEditor.bumpEditorLineMetricsRevision()

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
