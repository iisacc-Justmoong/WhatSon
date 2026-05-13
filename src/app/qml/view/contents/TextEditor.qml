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
        const editorSurface = textEditor.editorSurfaceObject();
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
