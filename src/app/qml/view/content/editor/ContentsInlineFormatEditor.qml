pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import WhatSon.App.Internal 1.0
import "ContentsEditorDebugTrace.js" as EditorTrace

FocusScope {
    id: control
    objectName: "contentsInlineFormatEditor"

    property bool autoFocusOnPress: true
    property color backgroundColor: "transparent"
    property color backgroundColorDisabled: backgroundColor
    property color backgroundColorFocused: backgroundColor
    property color backgroundColorHover: backgroundColor
    property color backgroundColorPressed: backgroundColor
    property real centeredTextHeight: 0
    property real cornerRadius: 0
    property real editorHeight: 0
    property bool enforceModeDefaults: false
    property bool externalScroll: false
    property var externalScrollViewport: null
    property real fieldMinHeight: 0
    property string fontFamily: LV.Theme.fontBody
    property real fontLetterSpacing: 0
    property int fontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    property int fontWeight: Font.Medium
    property real insetHorizontal: 0
    property real insetVertical: 0
    property string placeholderText: ""
    property bool selectByMouse: true
    property color selectedTextColor: LV.Theme.textPrimary
    property color selectionColor: LV.Theme.accent
    property int shapeStyle: 0
    property var shortcutKeyPressHandler: null
    property bool cursorVisibleWhenFocused: true
    property bool showRenderedOutput: true
    property bool showScrollBar: false
    property bool blockExternalDropMutation: false
    property bool suppressCommittedTextEditedDispatch: false
    property int tabIndentSpaceCount: 4
    property string renderedText: ""
    property int renderedTextFormat: Text.RichText
    property string text: ""
    property color textColor: LV.Theme.bodyColor
    property int wrapMode: TextEdit.NoWrap
    property bool preferNativeInputHandling: false
    readonly property int effectiveTabIndentSpaceCount: Math.max(1, Math.floor(Number(control.tabIndentSpaceCount) || 4))
    readonly property real editorVisualHeight: Math.max(
                                                   1,
                                                   Math.max(
                                                       Number(control.fieldMinHeight) || 0,
                                                       Number(control.editorHeight) || 0,
                                                       Number(textInput.contentHeight) || 0,
                                                       Number(renderedOutputText.contentHeight) || 0))
    readonly property bool renderedOutputVisible: control.showRenderedOutput
                                                  && !control.inputMethodSessionActive()
                                                  && (control.renderedText === undefined
                                                      || control.renderedText === null
                                                      ? ""
                                                      : String(control.renderedText)).length > 0
    implicitHeight: control.editorVisualHeight

    readonly property real contentHeight: control.externalScroll ? editorShell.height : editorFlickable.contentHeight
    readonly property real contentOffsetY: {
        if (control.externalScroll) {
            const viewport = control.resolvedFlickable;
            const viewportContentY = viewport && viewport.contentY !== undefined ? Number(viewport.contentY) || 0 : 0;
            return (Number(control.y) || 0) - viewportContentY;
        }
        if (!editorShell.parent)
            return 0;
        return Number(editorShell.parent.y) || 0;
    }
    property alias cursorPosition: textInput.cursorPosition
    property alias editorItem: editorShell
    readonly property bool empty: textInput.length === 0
    readonly property bool focused: activeFocus || textInput.activeFocus
    readonly property bool hasSelection: textInput.selectionEnd > textInput.selectionStart
    readonly property bool inputMethodComposing: textInput.inputMethodComposing
    readonly property bool inputMethodVisible: Qt.inputMethod && Qt.inputMethod.visible !== undefined
                                               ? Qt.inputMethod.visible
                                               : false
    readonly property int length: textInput.length
    readonly property int lineCount: textInput.lineCount
    readonly property string preeditText: textInput.preeditText === undefined || textInput.preeditText === null
                                           ? ""
                                           : String(textInput.preeditText)
    readonly property real inputContentHeight: Number(textInput.contentHeight) || 0
    readonly property var resolvedFlickable: control.externalScroll && control.externalScrollViewport ? control.externalScrollViewport : editorFlickable
    readonly property string selectedText: textInput.selectedText
    readonly property int selectionEnd: textInput.selectionEnd
    readonly property int selectionStart: textInput.selectionStart

    property int _programmaticTextSyncDepth: 0
    property bool _hasDeferredProgrammaticText: false
    property string _deferredProgrammaticText: ""
    property bool _fallbackTextEditedDispatchQueued: false
    property int _fallbackTextEditedDispatchRevision: 0
    property string _cachedSelectedText: ""
    property int _cachedSelectionCursorPosition: -1
    property int _cachedSelectionEnd: -1
    property int _cachedSelectionStart: -1

    signal textEdited(string text)

    function forceActiveFocus() {
        EditorTrace.trace("inlineFormatEditor", "forceActiveFocus", "", control)
        textInput.forceActiveFocus();
    }

    function getText(start, end) {
        return textInput.getText(start, end);
    }

    function currentPlainText() {
        const maximumLength = Number(textInput.length) || 0
        if (textInput.getText !== undefined)
            return textInput.getText(0, maximumLength)
        return textInput.text === undefined || textInput.text === null ? "" : String(textInput.text)
    }

    function getFormattedText(start, end) {
        return textInput.getFormattedText(start, end);
    }

    function selectionSnapshot() {
        return {
            "cursorPosition": Number(textInput.cursorPosition),
            "selectedText": textInput.selectedText,
            "selectionEnd": Number(textInput.selectionEnd),
            "selectionStart": Number(textInput.selectionStart)
        };
    }

    function clearSelection() {
        if (textInput.deselect !== undefined)
            textInput.deselect();
        control.clearCachedSelectionSnapshot();
        control.notifyInputMethod(control.inputMethodSelectionQueryMask());
    }

    function clearCachedSelectionSnapshot() {
        control._cachedSelectedText = "";
        control._cachedSelectionCursorPosition = -1;
        control._cachedSelectionEnd = -1;
        control._cachedSelectionStart = -1;
    }

    function cacheCurrentSelectionSnapshot() {
        const selectionStart = Math.max(0, Math.floor(Number(textInput.selectionStart) || 0));
        const selectionEnd = Math.max(selectionStart, Math.floor(Number(textInput.selectionEnd) || 0));
        if (selectionEnd <= selectionStart)
            return false;
        control._cachedSelectedText = textInput.selectedText === undefined || textInput.selectedText === null
                ? ""
                : String(textInput.selectedText);
        control._cachedSelectionCursorPosition = Math.max(
                    0,
                    Math.floor(Number(textInput.cursorPosition) || selectionEnd));
        control._cachedSelectionEnd = selectionEnd;
        control._cachedSelectionStart = selectionStart;
        return true;
    }

    function maybeDiscardCachedSelectionSnapshot() {
        const selectionStart = Math.max(0, Math.floor(Number(textInput.selectionStart) || 0));
        const selectionEnd = Math.max(selectionStart, Math.floor(Number(textInput.selectionEnd) || 0));
        if (selectionEnd > selectionStart) {
            control.cacheCurrentSelectionSnapshot();
            return;
        }

        const cachedStart = Math.max(0, Math.floor(Number(control._cachedSelectionStart) || 0));
        const cachedEnd = Math.max(cachedStart, Math.floor(Number(control._cachedSelectionEnd) || 0));
        if (cachedEnd <= cachedStart) {
            control.clearCachedSelectionSnapshot();
            return;
        }

        if (!control.focused) {
            control.clearCachedSelectionSnapshot();
            return;
        }

        const currentCursorPosition = Math.max(0, Math.floor(Number(textInput.cursorPosition) || 0));
        if (currentCursorPosition !== cachedStart && currentCursorPosition !== cachedEnd)
            control.clearCachedSelectionSnapshot();
    }

    function inlineFormatSelectionSnapshot() {
        if (control.cacheCurrentSelectionSnapshot())
            return control.selectionSnapshot();

        const cachedStart = Math.max(0, Math.floor(Number(control._cachedSelectionStart) || 0));
        const cachedEnd = Math.max(cachedStart, Math.floor(Number(control._cachedSelectionEnd) || 0));
        if (cachedEnd <= cachedStart)
            return control.selectionSnapshot();

        const currentCursorPosition = Math.max(0, Math.floor(Number(textInput.cursorPosition) || 0));
        if (!control.focused
                || (currentCursorPosition !== cachedStart && currentCursorPosition !== cachedEnd)) {
            return control.selectionSnapshot();
        }

        return {
            "cursorPosition": Math.max(
                                  0,
                                  Math.floor(Number(control._cachedSelectionCursorPosition) || cachedEnd)),
            "selectedText": control._cachedSelectedText,
            "selectionEnd": cachedEnd,
            "selectionStart": cachedStart
        };
    }

    function inputMethodSessionActive() {
        return control.inputMethodComposing || control.preeditText.length > 0;
    }

    function clampLogicalPosition(position, maximumLength) {
        const numericPosition = Number(position);
        if (!isFinite(numericPosition))
            return 0;
        return Math.max(0, Math.min(Math.floor(numericPosition), Math.max(0, maximumLength)));
    }

    function setCursorPositionPreservingInputMethod(position) {
        const maximumLength = Number(textInput.length) || 0;
        const boundedPosition = control.clampLogicalPosition(position, maximumLength);
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "setCursorPositionPreservingInputMethod",
                    "requested=" + position + " bounded=" + boundedPosition + " maximumLength=" + maximumLength,
                    control)
        if (Number(textInput.cursorPosition) === boundedPosition)
            return false;
        textInput.cursorPosition = boundedPosition;
        control.notifyInputMethod(control.inputMethodSelectionQueryMask());
        control.notifyInputMethod(control.inputMethodGeometryQueryMask());
        return true;
    }

    function inputMethodGeometryQueryMask() {
        return Qt.ImCursorRectangle | Qt.ImAnchorRectangle | Qt.ImInputItemClipRectangle;
    }

    function inputMethodSelectionQueryMask() {
        return Qt.ImQueryInput;
    }

    function notifyInputMethod(queries) {
        const queryMask = queries === undefined || queries === null
                ? control.inputMethodSelectionQueryMask()
                : queries;
        InputMethod.update(queryMask);
    }

    function selectionCursorUsesStartEdge(cursorPosition, selectionStart, selectionEnd) {
        const boundedStart = Math.max(0, Math.floor(Number(selectionStart) || 0));
        const boundedEnd = Math.max(boundedStart, Math.floor(Number(selectionEnd) || 0));
        const numericCursor = Number(cursorPosition);
        if (!isFinite(numericCursor))
            return false;
        const boundedCursor = control.clampLogicalPosition(numericCursor, boundedEnd);
        if (boundedCursor <= boundedStart)
            return true;
        if (boundedCursor >= boundedEnd)
            return false;
        return Math.abs(boundedCursor - boundedStart) <= Math.abs(boundedCursor - boundedEnd);
    }

    function restoreSelectionRange(selectionStart, selectionEnd, cursorPosition) {
        const maximumLength = Number(textInput.length) || 0;
        const boundedStart = control.clampLogicalPosition(selectionStart, maximumLength);
        const boundedEnd = control.clampLogicalPosition(selectionEnd, maximumLength);
        if (boundedEnd <= boundedStart)
            return false;
        const activeCursor = isFinite(Number(cursorPosition)) ? Number(cursorPosition) : boundedEnd;
        const activeEdgeIsStart = control.selectionCursorUsesStartEdge(activeCursor, boundedStart, boundedEnd);
        if (textInput.moveCursorSelection !== undefined) {
            const anchorPosition = activeEdgeIsStart ? boundedEnd : boundedStart;
            const activePosition = activeEdgeIsStart ? boundedStart : boundedEnd;
            textInput.cursorPosition = anchorPosition;
            textInput.moveCursorSelection(activePosition, TextEdit.SelectCharacters);
            return true;
        }
        if (textInput.select !== undefined) {
            textInput.select(boundedStart, boundedEnd);
            return true;
        }
        return false;
    }

    function cursorPositionForPoint(point) {
        const maximumLength = Number(textInput.length) || 0;
        if (textInput.positionAt !== undefined && point) {
            const pointX = Number(point.x);
            const pointY = Number(point.y);
            if (isFinite(pointX) && isFinite(pointY)) {
                const resolvedPosition = Number(textInput.positionAt(
                                                   Math.round(pointX),
                                                   Math.round(pointY)));
                if (isFinite(resolvedPosition))
                    return control.clampLogicalPosition(resolvedPosition, maximumLength);
            }
        }
        return control.clampLogicalPosition(Number(textInput.cursorPosition), maximumLength);
    }

    function paragraphSelectionRangeForPosition(position) {
        const maximumLength = Number(textInput.length) || 0;
        const editorText = textInput.getText !== undefined
                ? String(textInput.getText(0, maximumLength))
                : String(textInput.text === undefined || textInput.text === null ? "" : textInput.text);
        if (editorText.length === 0)
            return ({ "start": 0, "end": 0 });

        let boundedPosition = control.clampLogicalPosition(position, editorText.length);
        if (boundedPosition >= editorText.length)
            boundedPosition = Math.max(0, editorText.length - 1);

        let paragraphStart = boundedPosition;
        while (paragraphStart > 0 && editorText.charAt(paragraphStart - 1) !== "\n")
            --paragraphStart;

        let paragraphEnd = boundedPosition;
        while (paragraphEnd < editorText.length && editorText.charAt(paragraphEnd) !== "\n")
            ++paragraphEnd;

        return ({
                "start": paragraphStart,
                "end": paragraphEnd
            });
    }

    function applyNativeSelectionRange(selectionStart, selectionEnd, cursorPosition) {
        const restored = control.restoreSelectionRange(selectionStart, selectionEnd, cursorPosition);
        if (restored) {
            control.notifyInputMethod(control.inputMethodSelectionQueryMask());
            control.notifyInputMethod(control.inputMethodGeometryQueryMask());
        }
        return restored;
    }

    function selectWordAtPoint(point) {
        const targetPosition = control.cursorPositionForPoint(point);
        textInput.cursorPosition = targetPosition;
        if (textInput.selectWord === undefined)
            return false;
        textInput.selectWord();
        const selectionStart = Number(textInput.selectionStart);
        const selectionEnd = Number(textInput.selectionEnd);
        if (selectionEnd <= selectionStart)
            return false;
        return control.applyNativeSelectionRange(selectionStart, selectionEnd, selectionEnd);
    }

    function selectParagraphAtPoint(point) {
        const targetPosition = control.cursorPositionForPoint(point);
        const paragraphRange = control.paragraphSelectionRangeForPosition(targetPosition);
        if (paragraphRange.end <= paragraphRange.start)
            return false;
        return control.applyNativeSelectionRange(paragraphRange.start, paragraphRange.end, paragraphRange.end);
    }

    function activateInputAtPoint(point) {
        if (!control.preferNativeInputHandling || control.inputMethodVisible)
            return false;
        const targetPosition = control.cursorPositionForPoint(point);
        if (textInput.deselect !== undefined)
            textInput.deselect();
        control.setCursorPositionPreservingInputMethod(targetPosition);
        textInput.forceActiveFocus();
        control.notifyInputMethod(Qt.ImQueryAll);
        if (Qt.inputMethod && Qt.inputMethod.show !== undefined)
            Qt.inputMethod.show();
        return true;
    }

    function canDeferProgrammaticTextSync() {
        return control.inputMethodSessionActive();
    }

    function flushDeferredProgrammaticText(force) {
        if (!control._hasDeferredProgrammaticText)
            return;
        if (!force && control.canDeferProgrammaticTextSync())
            return;

        const deferredText = control._deferredProgrammaticText;
        control._hasDeferredProgrammaticText = false;
        control._deferredProgrammaticText = "";
        control.setProgrammaticText(deferredText);
    }

    function clearDeferredProgrammaticText() {
        control._hasDeferredProgrammaticText = false;
        control._deferredProgrammaticText = "";
    }

    function dispatchCommittedTextEditedIfReady() {
        if (control._programmaticTextSyncDepth > 0)
            return false
        if (control.blockExternalDropMutation)
            return false
        if (control.suppressCommittedTextEditedDispatch)
            return false
        if (control.inputMethodSessionActive())
            return false
        control.textEdited(control.currentPlainText())
        return true
    }

    function setProgrammaticText(nextText) {
        const normalizedText = nextText === undefined || nextText === null ? "" : String(nextText);
        if (textInput.text === normalizedText)
            return;
        const previousCursorPosition = Number(textInput.cursorPosition);
        const previousSelectionStart = Number(textInput.selectionStart);
        const previousSelectionEnd = Number(textInput.selectionEnd);
        const hadSelection = isFinite(previousSelectionStart) && isFinite(previousSelectionEnd)
                && previousSelectionEnd > previousSelectionStart;
        // Cancel any deferred committed-text dispatch that was queued for the previous surface.
        control._fallbackTextEditedDispatchRevision += 1;
        control._fallbackTextEditedDispatchQueued = false;
        control._programmaticTextSyncDepth += 1;
        textInput.text = normalizedText;
        control.clearCachedSelectionSnapshot();
        if (hadSelection && previousSelectionEnd > previousSelectionStart) {
            control.restoreSelectionRange(previousSelectionStart, previousSelectionEnd, previousCursorPosition);
        } else {
            const maximumLength = Number(textInput.length) || 0;
            const restoredCursorPosition = control.clampLogicalPosition(previousCursorPosition, maximumLength);
            if (textInput.deselect !== undefined)
                textInput.deselect();
            control.setCursorPositionPreservingInputMethod(restoredCursorPosition);
        }
        control._programmaticTextSyncDepth -= 1;
        control.notifyInputMethod(control.inputMethodSelectionQueryMask());
        control.notifyInputMethod(control.inputMethodGeometryQueryMask());
    }

    function scheduleCommittedTextEditedDispatch() {
        if (control._programmaticTextSyncDepth > 0)
            return;
        if (control._fallbackTextEditedDispatchQueued)
            return;
        const scheduledRevision = control._fallbackTextEditedDispatchRevision + 1;
        control._fallbackTextEditedDispatchRevision = scheduledRevision;
        control._fallbackTextEditedDispatchQueued = true;
        Qt.callLater(function () {
            control._fallbackTextEditedDispatchQueued = false;
            if (control._fallbackTextEditedDispatchRevision !== scheduledRevision)
                return;
            control.dispatchCommittedTextEditedIfReady();
        });
    }

    function tabIndentText() {
        return Array(control.effectiveTabIndentSpaceCount + 1).join(" ");
    }

    function insertTabIndentAsSpaces() {
        const indentText = control.tabIndentText();
        if (indentText.length === 0)
            return false;

        const cursorPosition = Math.max(0, Math.floor(Number(textInput.cursorPosition) || 0));
        const selectionStart = Math.max(0, Math.floor(Number(textInput.selectionStart) || 0));
        const selectionEnd = Math.max(selectionStart, Math.floor(Number(textInput.selectionEnd) || 0));
        const hasSelection = selectionEnd > selectionStart;
        const insertionPosition = hasSelection ? selectionStart : cursorPosition;

        if (hasSelection && textInput.remove !== undefined)
            textInput.remove(selectionStart, selectionEnd);

        if (textInput.insert !== undefined) {
            textInput.insert(insertionPosition, indentText);
        } else {
            const previousText = textInput.text === undefined || textInput.text === null ? "" : String(textInput.text);
            textInput.text = previousText.slice(0, insertionPosition) + indentText + previousText.slice(insertionPosition);
        }

        if (textInput.deselect !== undefined)
            textInput.deselect();
        control.setCursorPositionPreservingInputMethod(insertionPosition + indentText.length);
        return true;
    }

    onFocusedChanged: {
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "focusedChanged",
                    "focused=" + focused + " inputMethodSessionActive=" + control.inputMethodSessionActive(),
                    control)
        control.notifyInputMethod(Qt.ImQueryAll);
        if (!focused && control.inputMethodSessionActive()) {
            Qt.callLater(function () {
                if (!control.focused && !control.inputMethodSessionActive())
                    control.flushDeferredProgrammaticText(true);
            });
            return;
        }
        control.maybeDiscardCachedSelectionSnapshot();
        if (!focused)
            control.flushDeferredProgrammaticText(true);
    }
    onTextChanged: {
        const normalizedText = text === undefined || text === null ? "" : String(text);
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "textChanged",
                    EditorTrace.describeText(normalizedText),
                    control)
        if (control.canDeferProgrammaticTextSync()) {
            control._deferredProgrammaticText = normalizedText;
            control._hasDeferredProgrammaticText = true;
            return;
        }
        control._hasDeferredProgrammaticText = false;
        control._deferredProgrammaticText = "";
        setProgrammaticText(normalizedText);
    }

    Rectangle {
        anchors.fill: parent
        color: {
            if (!control.enabled)
                return control.backgroundColorDisabled;
            if (control.focused)
                return control.backgroundColorFocused;
            return control.backgroundColor;
        }
        radius: control.cornerRadius
    }

    Flickable {
        id: editorFlickable

        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentHeight: Math.max(height, editorShell.height)
        contentWidth: width
        flickableDirection: Flickable.VerticalFlick
        interactive: !control.externalScroll && contentHeight > height

        Item {
            id: editorShell

            parent: editorFlickable.contentItem
            width: editorFlickable.width
            height: control.editorVisualHeight

            property alias cursorPosition: textInput.cursorPosition
            readonly property bool focused: textInput.activeFocus
            readonly property bool inputMethodComposing: textInput.inputMethodComposing
            property alias inputItem: textInput
            readonly property string preeditText: textInput.preeditText === undefined || textInput.preeditText === null
                                                 ? ""
                                                 : String(textInput.preeditText)
            readonly property string selectedText: textInput.selectedText
            readonly property int selectionEnd: textInput.selectionEnd
            readonly property int selectionStart: textInput.selectionStart
            property bool showRenderedOutput: control.showRenderedOutput
            property alias topPadding: textInput.topPadding

            function forceActiveFocus() {
                textInput.forceActiveFocus();
            }

            function positionToRectangle(position) {
                return textInput.positionToRectangle(position);
            }

            Text {
                id: renderedOutputText

                color: control.textColor
                font.family: control.fontFamily
                font.letterSpacing: control.fontLetterSpacing
                font.pixelSize: control.fontPixelSize
                font.weight: control.fontWeight
                text: control.renderedText
                textFormat: control.renderedTextFormat
                visible: control.renderedOutputVisible
                width: Math.max(
                           0,
                           parent ? (Number(parent.width) || 0) - control.insetHorizontal * 2 : 0)
                wrapMode: Text.Wrap
                x: control.insetHorizontal
                y: 0
                z: 0
            }

            TextEdit {
                id: textInput

                anchors.fill: parent
                activeFocusOnPress: control.autoFocusOnPress
                                    && (!control.preferNativeInputHandling || control.inputMethodVisible)
                bottomPadding: control.insetVertical
                color: control.renderedOutputVisible ? "transparent" : control.textColor
                cursorDelegate: Rectangle {
                    color: control.textColor
                    height: Math.max(1, Number(textInput.cursorRectangle.height) || Number(control.fontPixelSize) || 1)
                    visible: textInput.cursorVisible
                    width: Math.max(1, Math.round(LV.Theme.scaleMetric(1)))
                    z: 2
                }
                cursorVisible: control.focused && control.cursorVisibleWhenFocused
                font.family: control.fontFamily
                font.letterSpacing: control.fontLetterSpacing
                font.pixelSize: control.fontPixelSize
                font.weight: control.fontWeight
                leftPadding: control.insetHorizontal
                persistentSelection: true
                readOnly: control.blockExternalDropMutation
                rightPadding: control.insetHorizontal
                selectByMouse: control.selectByMouse
                               && (!control.preferNativeInputHandling || control.inputMethodVisible)
                selectedTextColor: control.selectedTextColor
                selectionColor: control.selectionColor
                textFormat: TextEdit.PlainText
                tabStopDistance: Math.max(
                                     1,
                                     Number(tabStopTextMetrics.advanceWidth)
                                     || Number(font.pixelSize) * control.effectiveTabIndentSpaceCount)
                topPadding: 0
                wrapMode: control.wrapMode

                readonly property bool focused: activeFocus
                property bool showRenderedOutput: control.showRenderedOutput

                onActiveFocusChanged: {
                    EditorTrace.trace(
                                "inlineFormatEditor",
                                "textInputActiveFocusChanged",
                                "activeFocus=" + activeFocus,
                                control)
                    control.notifyInputMethod(Qt.ImQueryAll);
                    control.maybeDiscardCachedSelectionSnapshot();
                }
                onCursorPositionChanged: {
                    EditorTrace.trace(
                                "inlineFormatEditor",
                                "cursorPositionChanged",
                                "cursorPosition=" + cursorPosition,
                                control)
                    control.notifyInputMethod(control.inputMethodSelectionQueryMask());
                    control.maybeDiscardCachedSelectionSnapshot();
                }
                onCursorRectangleChanged: {
                    control.notifyInputMethod(control.inputMethodGeometryQueryMask());
                }
                onSelectionEndChanged: {
                    EditorTrace.trace(
                                "inlineFormatEditor",
                                "selectionEndChanged",
                                "selectionStart=" + selectionStart + " selectionEnd=" + selectionEnd,
                                control)
                    control.notifyInputMethod(control.inputMethodSelectionQueryMask());
                    control.maybeDiscardCachedSelectionSnapshot();
                }
                onSelectionStartChanged: {
                    EditorTrace.trace(
                                "inlineFormatEditor",
                                "selectionStartChanged",
                                "selectionStart=" + selectionStart + " selectionEnd=" + selectionEnd,
                                control)
                    control.notifyInputMethod(control.inputMethodSelectionQueryMask());
                    control.maybeDiscardCachedSelectionSnapshot();
                }
                onTextChanged: {
                    EditorTrace.trace(
                                "inlineFormatEditor",
                                "textInputTextChanged",
                                "programmaticDepth=" + control._programmaticTextSyncDepth
                                + " inputMethodSessionActive=" + control.inputMethodSessionActive()
                                + " " + EditorTrace.describeText(textInput.text),
                                control)
                    control.notifyInputMethod(control.inputMethodSelectionQueryMask());
                    control.clearCachedSelectionSnapshot();
                    if (control._programmaticTextSyncDepth > 0)
                        return;
                    control.clearDeferredProgrammaticText();
                    if (control.blockExternalDropMutation
                            || control.suppressCommittedTextEditedDispatch) {
                        control._fallbackTextEditedDispatchRevision += 1;
                        control._fallbackTextEditedDispatchQueued = false;
                        return;
                    }
                    if (control.inputMethodSessionActive()) {
                        control.scheduleCommittedTextEditedDispatch();
                        return;
                    }
                    control._fallbackTextEditedDispatchRevision += 1;
                    control.dispatchCommittedTextEditedIfReady();
                }
                Keys.onPressed: function (event) {
                    if (control.shortcutKeyPressHandler
                            && typeof control.shortcutKeyPressHandler === "function") {
                        const shortcutHandled = !!control.shortcutKeyPressHandler(event);
                        if (shortcutHandled || event.accepted)
                            return;
                    }
                    const modifierMask = Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier;
                    if (event.key === Qt.Key_Tab
                            && (event.modifiers & modifierMask) === 0
                            && (event.modifiers & Qt.ShiftModifier) === 0) {
                        if (control.insertTabIndentAsSpaces()) {
                            event.accepted = true;
                            return;
                        }
                    }
                }

                TapHandler {
                    acceptedDevices: PointerDevice.TouchScreen
                    enabled: control.preferNativeInputHandling
                    gesturePolicy: TapHandler.DragThreshold

                    onDoubleTapped: function (eventPoint, _button) {
                        control.selectWordAtPoint(eventPoint.position);
                    }
                    onTapped: function (eventPoint, _button) {
                        if (tapCount === 1) {
                            control.activateInputAtPoint(eventPoint.position);
                            return;
                        }
                        if (tapCount === 3)
                            control.selectParagraphAtPoint(eventPoint.position);
                    }
                }
            }
        }

        Controls.ScrollBar.vertical: control.showScrollBar ? verticalScrollBar : null
    }

    Controls.ScrollBar {
        id: verticalScrollBar
    }

    TextMetrics {
        id: tabStopTextMetrics

        font.family: control.fontFamily
        font.letterSpacing: control.fontLetterSpacing
        font.pixelSize: control.fontPixelSize
        font.weight: control.fontWeight
        text: Array(control.effectiveTabIndentSpaceCount + 1).join(" ")
    }

    Connections {
        function onContentYChanged() {
            control.notifyInputMethod(control.inputMethodGeometryQueryMask());
        }

        function onHeightChanged() {
            control.notifyInputMethod(control.inputMethodGeometryQueryMask());
        }

        ignoreUnknownSignals: true
        target: control.resolvedFlickable
    }

    Connections {
        function onInputMethodComposingChanged() {
            if (!textInput.inputMethodComposing) {
                control.scheduleCommittedTextEditedDispatch();
                control.flushDeferredProgrammaticText(false);
            }
        }

        function onPreeditTextChanged() {
            if (!textInput.inputMethodComposing && control.preeditText.length === 0) {
                control.scheduleCommittedTextEditedDispatch();
                control.flushDeferredProgrammaticText(false);
            }
        }

        ignoreUnknownSignals: true
        target: textInput
    }

    Component.onCompleted: {
        EditorTrace.trace("inlineFormatEditor", "mount", EditorTrace.describeText(text), control)
        setProgrammaticText(text);
        control.notifyInputMethod(Qt.ImQueryAll);
    }

    Component.onDestruction: {
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "unmount",
                    "cursorPosition=" + cursorPosition
                    + " selectionStart=" + selectionStart
                    + " selectionEnd=" + selectionEnd,
                    control)
    }
}
