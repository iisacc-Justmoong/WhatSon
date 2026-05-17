pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.TextEditor {
    id: textEditor

    property bool editorReadOnly: false
    property string noteBodyFilePath: ""
    property var inAppClipboard: null
    property var clipboardEditorPaste: null
    property var editorInputCommandFilter: null
    property var noteEditorSession: null
    property var viewportFlickable: null
    property int editorPlainTextRevision: 0
    property int editorLineMetricsRevision: 0
    property real editorBottomViewportPaddingRatio: 0.75
    property bool cursorViewportSyncQueued: false
    readonly property string editorDocumentText: textEditor.text !== undefined ? String(textEditor.text) : ""
    readonly property string editorSelectedText: textEditor.editorSelectedTextForCurrentSelection()
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
    readonly property real editorTextContentBottomY: {
        textEditor.editorLineMetricsRevision;
        return textEditor.editorTextContentBottom();
    }
    readonly property real editorPaddedEditorItemHeight: Math.max(
            textEditor.editorViewportHeight,
            textEditor.editorTextContentBottomY + textEditor.editorBottomViewportPadding)
    readonly property real editorBottomViewportPaddingAreaTop: textEditor.editorItem
            ? textEditor.numberOrFallback(textEditor.editorItem.y, 0)
              + textEditor.editorTextContentBottomY
              - textEditor.viewportContentY
            : textEditor.height
    readonly property real editorBottomViewportPaddingAreaBottom: textEditor.editorItem
            ? textEditor.numberOrFallback(textEditor.editorItem.y, 0)
              + textEditor.editorPaddedEditorItemHeight
              - textEditor.viewportContentY
            : textEditor.height
    readonly property real editorBottomViewportPaddingHitAreaY: Math.max(
            0,
            textEditor.editorBottomViewportPaddingAreaTop)
    readonly property real editorBottomViewportPaddingHitAreaHeight: Math.max(
            0,
            Math.min(textEditor.height, textEditor.editorBottomViewportPaddingAreaBottom)
            -textEditor.editorBottomViewportPaddingHitAreaY)
    readonly property string editorPlainText: {
        textEditor.editorPlainTextRevision;
        return textEditor.editorSurfacePlainText();
    }
    readonly property string editorResourceObjectReplacementText: "\uFFFC"
    readonly property int editorCursorLineIndex: textEditor.cursorLineIndexForLogicalCursor()

    function numberOrFallback(value, fallbackValue) {
        const numericValue = Number(value);
        return Number.isFinite(numericValue) ? numericValue : fallbackValue;
    }

    function editorSurfaceObject() {
        return textEditor.editorItem !== undefined ? textEditor.editorItem : null;
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

    function editorSelectedTextForCurrentSelection() {
        const selectionStart = textEditor.normalizedSelectionStart();
        const selectionEnd = textEditor.normalizedSelectionEnd();
        const editorSurface = textEditor.editorSurfaceObject();
        if (selectionEnd > selectionStart
                && editorSurface
                && editorSurface.getText !== undefined)
            return textEditor.normalizedEditorPlainText(
                        editorSurface.getText(selectionStart, selectionEnd));

        return textEditor.normalizedEditorPlainText(
                    textEditor.selectedText !== undefined ? String(textEditor.selectedText) : "");
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

    function logicalLineStartPositionFor(lineIndex) {
        const normalizedIndex = Math.max(0, Math.floor(Number(lineIndex) || 0));
        const lineStartPositions = textEditor.sourceAlignedLineStartPositions();
        if (normalizedIndex < lineStartPositions.length)
            return lineStartPositions[normalizedIndex];
        return textEditor.editorPlainText.length;
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

        const lineStartPosition = normalizedIndex < lineStartPositions.length
                ? lineStartPositions[normalizedIndex]
                : textEditor.editorPlainText.length;
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

        const rectangle = editorSurface.positionToRectangle(textEditor.boundedCursorPosition(textEditor.cursorPosition));
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
        textEditor.recordEditorUserActivity();
        if (textEditor.cursorViewportSyncQueued)
            return false;

        textEditor.cursorViewportSyncQueued = true;
        Qt.callLater(function () {
            textEditor.cursorViewportSyncQueued = false;
            textEditor.ensureCursorVisibleInViewport();
        });
        return true;
    }

    function recordEditorUserActivity() {
        if (textEditor.noteEditorSession
                && textEditor.noteEditorSession.recordEditorUserActivity !== undefined)
            textEditor.noteEditorSession.recordEditorUserActivity();
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
        textEditor.ensureCursorVisibleInViewport();
        Qt.callLater(function () {
            const deferredCursorPosition = textEditor.boundedCursorPosition(targetCursorPosition);
            textEditor.forceEditorFocus();
            textEditor.cursorPosition = deferredCursorPosition;
            textEditor.deselect();
            textEditor.ensureCursorVisibleInViewport();
        });
    }

    function focusEditorAtDocumentEnd() {
        const editorSurface = textEditor.editorSurfaceObject();
        const documentEnd = editorSurface && editorSurface.length !== undefined
                ? Math.max(0, Math.floor(Number(editorSurface.length) || 0))
                : textEditor.editorDocumentText.length;
        textEditor.restoreEditorCursorPosition(documentEnd);
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
        textEditor.restoreEditorCursorPosition(nextCursorPosition);
        return true;
    }

    function refreshEditorResourceFrameViewportWidth() {
        if (!textEditor.noteEditorSession
                || textEditor.noteEditorSession.reprojectResourceFramesForEditorWidth === undefined)
            return false;

        const editorWidth = Math.max(1, Math.round(textEditor.editorViewportWidth));
        const result = textEditor.noteEditorSession.reprojectResourceFramesForEditorWidth(
                    textEditor.editorDocumentText,
                    editorWidth);
        if (!result || !result.valid || !result.changed || result.editorDocumentText === undefined)
            return false;

        textEditor.replaceEditorDocumentText(result.editorDocumentText, textEditor.cursorPosition);
        return true;
    }

    function pasteNativeClipboardText() {
        textEditor.forceEditorFocus();
        textEditor.paste();
        return true;
    }

    function refreshEditorInputCommandFilterOwner() {
        if (!textEditor.editorInputCommandFilter
                || textEditor.editorInputCommandFilter.attachEditorInputOwner === undefined
                || !textEditor.editorItem
                || !textEditor.clipboardEditorPaste
                || !textEditor.inAppClipboard
                || !textEditor.noteEditorSession)
            return false;
        return textEditor.editorInputCommandFilter.attachEditorInputOwner(
                    textEditor.editorItem,
                    textEditor,
                    textEditor.clipboardEditorPaste,
                    textEditor.inAppClipboard,
                    textEditor.noteEditorSession);
    }

    function appendEditorGestureUiTokens(tokens, value) {
        if (value === undefined || value === null)
            return;
        if (typeof value === "string" || typeof value === "number" || typeof value === "boolean") {
            tokens.push(String(value));
            return;
        }
        if (Array.isArray(value)) {
            for (let index = 0; index < value.length; index += 1)
                textEditor.appendEditorGestureUiTokens(tokens, value[index]);
            return;
        }

        const fields = [
            "objectName",
            "hitObjectName",
            "path",
            "hitPath",
            "qmlId",
            "hitQmlId",
            "componentName",
            "hitComponentName",
            "hierarchy"
        ];
        for (let index = 0; index < fields.length; index += 1)
            textEditor.appendEditorGestureUiTokens(tokens, value[fields[index]]);
    }

    function editorGestureUiText(ui) {
        const tokens = [];
        textEditor.appendEditorGestureUiTokens(tokens, ui);
        return tokens.join(" ");
    }

    function editorGestureUiMatchesEditor(ui) {
        const uiText = textEditor.editorGestureUiText(ui);
        return uiText.indexOf("contentsTextEditor") >= 0
                || uiText.indexOf("contentsDisplayTextEditor") >= 0
                || uiText.indexOf("textEditorRichTextEdit") >= 0
                || uiText.indexOf("editorViewportFlickable") >= 0;
    }

    function editorPointFromGlobal(item, x, y) {
        const sceneX = Number(x);
        const sceneY = Number(y);
        if (!Number.isFinite(sceneX) || !Number.isFinite(sceneY) || !item)
            return null;

        if (item.mapFromGlobal !== undefined)
            return item.mapFromGlobal(sceneX, sceneY);
        if (item.mapFromItem !== undefined)
            return item.mapFromItem(null, sceneX, sceneY);
        return null;
    }

    function editorGesturePointWithinEditor(x, y) {
        const localPoint = textEditor.editorPointFromGlobal(textEditor, x, y);
        if (!localPoint)
            return false;

        return localPoint.x >= 0
                && localPoint.y >= 0
                && localPoint.x <= textEditor.width
                && localPoint.y <= textEditor.height;
    }

    function editorGestureMatchesEditor(eventData) {
        if (!eventData)
            return false;
        if (textEditor.editorGestureUiMatchesEditor(eventData.originUi)
                || textEditor.editorGestureUiMatchesEditor(eventData.ui))
            return true;
        return textEditor.editorGesturePointWithinEditor(eventData.startX, eventData.startY)
                || textEditor.editorGesturePointWithinEditor(eventData.x, eventData.y);
    }

    function editorPointFromGesture(eventData) {
        const editorSurface = textEditor.editorSurfaceObject();
        if (!eventData || !editorSurface || editorSurface.mapFromItem === undefined)
            return null;

        const globalX = eventData.globalX !== undefined ? Number(eventData.globalX) : Number(eventData.x);
        const globalY = eventData.globalY !== undefined ? Number(eventData.globalY) : Number(eventData.y);
        return textEditor.editorPointFromGlobal(editorSurface, globalX, globalY);
    }

    function focusEditorAtEditorPoint(editorPoint) {
        if (textEditor.readOnly)
            return false;

        const editorSurface = textEditor.editorSurfaceObject();
        if (editorSurface
                && editorPoint
                && editorSurface.positionAt !== undefined) {
            textEditor.cursorPosition = textEditor.boundedCursorPosition(
                        editorSurface.positionAt(editorPoint.x, editorPoint.y));
            textEditor.deselect();
        }

        textEditor.forceEditorFocus();
        return true;
    }

    function focusEditorFromGesture(eventData) {
        if (!textEditor.editorGestureMatchesEditor(eventData))
            return false;
        return textEditor.focusEditorAtEditorPoint(textEditor.editorPointFromGesture(eventData));
    }

    function handleEditorPressEnded(eventData) {
        if (!LV.Theme.mobileTarget || !eventData)
            return false;

        const finalInteractionKind = eventData.finalInteractionKind !== undefined
                ? String(eventData.finalInteractionKind)
                : String(eventData.interactionKind || "");
        const maximumFingerCount = Math.max(0, Math.floor(Number(eventData.maximumFingerCount) || 0));
        if (finalInteractionKind !== "tap" || maximumFingerCount > 1)
            return false;

        return textEditor.focusEditorFromGesture(eventData);
    }

    function handleEditorHoldStarted(eventData) {
        if (!LV.Theme.mobileTarget || !eventData)
            return false;

        const maximumFingerCount = Math.max(0, Math.floor(Number(eventData.maximumFingerCount) || 0));
        if (maximumFingerCount > 1 || eventData.scrollActive || eventData.dragActive)
            return false;

        return textEditor.focusEditorFromGesture(eventData);
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
    showScrollBar: false
    textColor: LV.Theme.bodyColor
    textColorDisabled: textColor

    onInAppClipboardChanged: textEditor.refreshEditorInputCommandFilterOwner()
    onClipboardEditorPasteChanged: textEditor.refreshEditorInputCommandFilterOwner()
    onEditorInputCommandFilterChanged: textEditor.refreshEditorInputCommandFilterOwner()
    onNoteEditorSessionChanged: {
        textEditor.refreshEditorInputCommandFilterOwner();
        Qt.callLater(textEditor.refreshEditorResourceFrameViewportWidth);
    }
    onReadFinished: textEditor.refreshEditorResourceFrameViewportWidth()

    Component.onCompleted: {
        textEditor.viewportFlickable = textEditor.findDescendantByObjectName(
                    textEditor,
                    "editorViewportFlickable");
        textEditor.bumpEditorPlainTextRevision();
        textEditor.bumpEditorLineMetricsRevision();
        Qt.callLater(textEditor.refreshEditorInputCommandFilterOwner);
    }
    Component.onDestruction: {
        if (textEditor.editorInputCommandFilter
                && textEditor.editorInputCommandFilter.detachEditorInputOwner !== undefined
                && textEditor.editorItem)
            textEditor.editorInputCommandFilter.detachEditorInputOwner(textEditor.editorItem);
    }

    onTextChanged: {
        textEditor.recordEditorUserActivity();
        textEditor.bumpEditorPlainTextRevision();
        textEditor.bumpEditorLineMetricsRevision();
    }
    onCursorPositionChanged: textEditor.requestEnsureCursorVisibleInViewport()
    onWidthChanged: textEditor.bumpEditorLineMetricsRevision()
    onEditorViewportWidthChanged: Qt.callLater(textEditor.refreshEditorResourceFrameViewportWidth)

    LV.EventListener {
        enabled: LV.Theme.mobileTarget && !textEditor.readOnly
        includeUiHit: true
        trigger: "pressEnded"
        action: function(eventData) {
            textEditor.handleEditorPressEnded(eventData);
        }
    }

    LV.EventListener {
        enabled: LV.Theme.mobileTarget && !textEditor.readOnly
        includeUiHit: true
        trigger: "holdStarted"
        action: function(eventData) {
            textEditor.handleEditorHoldStarted(eventData);
        }
    }

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

    Binding {
        property: "editorViewportWidth"
        restoreMode: Binding.RestoreBindingOrValue
        target: textEditor.noteEditorSession
        value: Math.round(textEditor.editorViewportWidth)
        when: textEditor.noteEditorSession
              && textEditor.noteEditorSession.editorViewportWidth !== undefined
    }

    MouseArea {
        id: editorBottomViewportPaddingHitArea

        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.IBeamCursor
        enabled: !textEditor.readOnly
                 && textEditor.editorBottomViewportPadding > 0
                 && textEditor.editorBottomViewportPaddingHitAreaHeight > 0
        height: textEditor.editorBottomViewportPaddingHitAreaHeight
        objectName: "contentsTextEditorBottomViewportPaddingHitArea"
        preventStealing: false
        visible: enabled
        width: textEditor.editorItem
               ? Math.max(1, Number(textEditor.editorItem.width) || textEditor.width)
               : textEditor.width
        x: textEditor.editorItem
           ? Math.max(0, Number(textEditor.editorItem.x) || 0)
           : 0
        y: textEditor.editorBottomViewportPaddingHitAreaY

        onClicked: function(mouse) {
            if (mouse.button !== Qt.LeftButton) {
                mouse.accepted = false;
                return;
            }
            textEditor.focusEditorAtDocumentEnd();
            mouse.accepted = true;
        }
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
