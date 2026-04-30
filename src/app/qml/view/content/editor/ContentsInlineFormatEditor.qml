pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "../../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace

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
    property int inputMethodHints: Qt.ImhNone
    property int mouseSelectionMode: TextEdit.SelectCharacters
    property bool overwriteMode: false
    property bool persistentSelection: true
    property string placeholderText: ""
    property bool selectByMouse: true
    property bool selectByKeyboard: true
    property color selectedTextColor: LV.Theme.textPrimary
    property color selectionColor: LV.Theme.accent
    property int shapeStyle: 0
    property bool cursorVisibleWhenFocused: true
    property bool showRenderedOutput: true
    property bool showScrollBar: false
    property bool blockExternalDropMutation: false
    property bool suppressCommittedTextEditedDispatch: false
    property int tabIndentSpaceCount: 4
    property var tagManagementKeyPressHandler: null
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
                                                  && !control.nativeCompositionActive()
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

    signal textEdited(string text)

    function forceActiveFocus() {
        inlineEditorController.forceActiveFocus();
    }

    function getText(start, end) {
        return textInput.getText(start, end);
    }

    function currentPlainText() {
        return inlineEditorController.currentPlainText();
    }

    function getFormattedText(start, end) {
        return textInput.getFormattedText(start, end);
    }

    function selectionSnapshot() {
        return inlineEditorController.selectionSnapshot();
    }

    function clearSelection() {
        inlineEditorController.clearSelection();
    }

    function clearCachedSelectionSnapshot() {
        inlineEditorController.clearCachedSelectionSnapshot();
    }

    function cacheCurrentSelectionSnapshot() {
        return inlineEditorController.cacheCurrentSelectionSnapshot();
    }

    function maybeDiscardCachedSelectionSnapshot() {
        inlineEditorController.maybeDiscardCachedSelectionSnapshot();
    }

    function inlineFormatSelectionSnapshot() {
        return inlineEditorController.inlineFormatSelectionSnapshot();
    }

    function nativeCompositionActive() {
        return inlineEditorController.nativeCompositionActive();
    }

    function clampLogicalPosition(position, maximumLength) {
        return inlineEditorController.clampLogicalPosition(position, maximumLength);
    }

    function setCursorPositionPreservingNativeInput(position) {
        return inlineEditorController.setCursorPositionPreservingNativeInput(position);
    }

    function selectionCursorUsesStartEdge(cursorPosition, selectionStart, selectionEnd) {
        return inlineEditorController.selectionCursorUsesStartEdge(cursorPosition, selectionStart, selectionEnd);
    }

    function restoreSelectionRange(selectionStart, selectionEnd, cursorPosition) {
        return inlineEditorController.restoreSelectionRange(selectionStart, selectionEnd, cursorPosition);
    }

    function programmaticTextSyncPolicy(nextText) {
        return inlineEditorController.programmaticTextSyncPolicy(nextText);
    }

    function canDeferProgrammaticTextSync(nextText) {
        return inlineEditorController.canDeferProgrammaticTextSync(nextText);
    }

    function shouldRejectFocusedProgrammaticTextSync(nextText) {
        return inlineEditorController.shouldRejectFocusedProgrammaticTextSync(nextText);
    }

    function flushDeferredProgrammaticText(force) {
        inlineEditorController.flushDeferredProgrammaticText(force);
    }

    function clearDeferredProgrammaticText() {
        inlineEditorController.clearDeferredProgrammaticText();
    }

    function dispatchCommittedTextEditedIfReady() {
        return inlineEditorController.dispatchCommittedTextEditedIfReady();
    }

    function setProgrammaticText(nextText) {
        inlineEditorController.setProgrammaticText(nextText);
    }

    function scheduleCommittedTextEditedDispatch() {
        inlineEditorController.scheduleCommittedTextEditedDispatch();
    }

    function handleTagManagementKeyPress(event) {
        if (!control.tagManagementKeyPressHandler
                || typeof control.tagManagementKeyPressHandler !== "function") {
            event.accepted = false;
            return false;
        }
        const handled = !!control.tagManagementKeyPressHandler(event);
        if (handled || event.accepted) {
            event.accepted = true;
            return true;
        }
        event.accepted = false;
        return false;
    }

    function eventRequestsPasteShortcut(event) {
        if (!event)
            return false;
        if (event.matches !== undefined && event.matches(StandardKey.Paste))
            return true;

        const modifiers = Number(event.modifiers) || 0;
        const metaPressed = !!(modifiers & Qt.MetaModifier);
        const controlPressed = !!(modifiers & Qt.ControlModifier);
        const altPressed = !!(modifiers & Qt.AltModifier);
        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        const normalizedText = event.text === undefined || event.text === null ? "" : String(event.text).toUpperCase();
        if (!altPressed
                && !shiftPressed
                && (metaPressed || controlPressed)
                && (event.key === Qt.Key_V || normalizedText === "V")) {
            return true;
        }
        if (!metaPressed
                && !controlPressed
                && !altPressed
                && shiftPressed
                && event.key === Qt.Key_Insert) {
            return true;
        }
        return false;
    }

    function inlineFormatShortcutTag(event) {
        if (!event)
            return "";

        const modifiers = Number(event.modifiers) || 0;
        const metaPressed = !!(modifiers & Qt.MetaModifier);
        const controlPressed = !!(modifiers & Qt.ControlModifier);
        const altPressed = !!(modifiers & Qt.AltModifier);
        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        const commandPressed = metaPressed || controlPressed;
        if (altPressed || !commandPressed)
            return "";

        const normalizedText = event.text === undefined || event.text === null ? "" : String(event.text).toUpperCase();
        if (!shiftPressed) {
            if (event.key === Qt.Key_B || normalizedText === "B")
                return "bold";
            if (event.key === Qt.Key_I || normalizedText === "I")
                return "italic";
            if (event.key === Qt.Key_U || normalizedText === "U")
                return "underline";
            return "";
        }
        if (event.key === Qt.Key_X || normalizedText === "X")
            return "strikethrough";
        if (event.key === Qt.Key_E || normalizedText === "E")
            return "highlight";
        return "";
    }

    function eventRequestsInlineFormatShortcut(event) {
        return control.inlineFormatShortcutTag(event).length > 0;
    }

    function bodyTagShortcutKind(event) {
        if (!event)
            return "";

        const modifiers = Number(event.modifiers) || 0;
        const metaPressed = !!(modifiers & Qt.MetaModifier);
        const controlPressed = !!(modifiers & Qt.ControlModifier);
        const altPressed = !!(modifiers & Qt.AltModifier);
        const shiftPressed = !!(modifiers & Qt.ShiftModifier);
        const commandPressed = metaPressed || controlPressed;
        if (!commandPressed)
            return "";

        const normalizedText = event.text === undefined || event.text === null ? "" : String(event.text).toUpperCase();
        const key = Number(event.key);
        if (altPressed && !shiftPressed) {
            if (key === Qt.Key_T || normalizedText === "T")
                return "agenda";
            if (key === Qt.Key_C || normalizedText === "C")
                return "callout";
            return "";
        }
        if (shiftPressed && !altPressed && (key === Qt.Key_H || normalizedText === "H"))
            return "break";
        return "";
    }

    function eventRequestsBodyTagShortcut(event) {
        return control.bodyTagShortcutKind(event).length > 0;
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
                onLinkActivated: function (link) {
                    Qt.openUrlExternally(link);
                }
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
                inputMethodHints: control.inputMethodHints
                leftPadding: control.insetHorizontal
                mouseSelectionMode: control.mouseSelectionMode
                overwriteMode: control.overwriteMode
                persistentSelection: control.persistentSelection
                readOnly: control.blockExternalDropMutation
                rightPadding: control.insetHorizontal
                selectByKeyboard: control.selectByKeyboard
                selectByMouse: control.selectByMouse
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

                Keys.onPressed: function (event) {
                    const key = Number(event.key);
                    if (key !== Qt.Key_Backspace
                            && !control.eventRequestsPasteShortcut(event)
                            && !control.eventRequestsInlineFormatShortcut(event)
                            && !control.eventRequestsBodyTagShortcut(event)) {
                        event.accepted = false;
                        return;
                    }
                    control.handleTagManagementKeyPress(event);
                }

                Keys.onEnterPressed: function (event) {
                    control.handleTagManagementKeyPress(event);
                }

                Keys.onReturnPressed: function (event) {
                    control.handleTagManagementKeyPress(event);
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



Item {
    id: controller
    objectName: "contentsInlineFormatEditorController"

    property var control: null
    property var textInput: null
    property int programmaticTextSyncDepth: 0
    property bool hasDeferredProgrammaticText: false
    property string deferredProgrammaticText: ""
    property bool fallbackTextEditedDispatchQueued: false
    property int fallbackTextEditedDispatchRevision: 0
    property bool localSelectionInteractionSinceFocus: false
    property bool localTextEditSinceFocus: false
    property string cachedSelectedText: ""
    property int cachedSelectionCursorPosition: -1
    property int cachedSelectionEnd: -1
    property int cachedSelectionStart: -1

    ContentsEditorInputPolicyAdapter {
        id: inlineInputPolicyAdapter
    }

    function controlText() {
        return controller.control && controller.control.text !== undefined && controller.control.text !== null
                ? String(controller.control.text)
                : "";
    }

    function controlFocused() {
        return controller.control && controller.control.focused !== undefined
                ? !!controller.control.focused
                : false;
    }

    function forceActiveFocus() {
        EditorTrace.trace("inlineFormatEditor", "forceActiveFocus", "", controller.control)
        if (controller.textInput && controller.textInput.forceActiveFocus !== undefined)
            controller.textInput.forceActiveFocus();
    }

    function currentPlainText() {
        if (!controller.textInput)
            return "";
        const maximumLength = Number(controller.textInput.length) || 0;
        if (controller.textInput.getText !== undefined)
            return controller.textInput.getText(0, maximumLength);
        return controller.textInput.text === undefined || controller.textInput.text === null
                ? ""
                : String(controller.textInput.text);
    }

    function selectionSnapshot() {
        if (!controller.textInput) {
            return {
                "cursorPosition": 0,
                "selectedText": "",
                "selectionEnd": 0,
                "selectionStart": 0
            };
        }
        return {
            "cursorPosition": Number(controller.textInput.cursorPosition),
            "selectedText": controller.textInput.selectedText,
            "selectionEnd": Number(controller.textInput.selectionEnd),
            "selectionStart": Number(controller.textInput.selectionStart)
        };
    }

    function clearSelection() {
        if (controller.textInput && controller.textInput.deselect !== undefined)
            controller.textInput.deselect();
        controller.clearCachedSelectionSnapshot();
    }

    function clearCachedSelectionSnapshot() {
        controller.cachedSelectedText = "";
        controller.cachedSelectionCursorPosition = -1;
        controller.cachedSelectionEnd = -1;
        controller.cachedSelectionStart = -1;
    }

    function cacheCurrentSelectionSnapshot() {
        if (!controller.textInput)
            return false;
        const selectionStart = Math.max(0, Math.floor(Number(controller.textInput.selectionStart) || 0));
        const selectionEnd = Math.max(selectionStart, Math.floor(Number(controller.textInput.selectionEnd) || 0));
        if (selectionEnd <= selectionStart)
            return false;
        controller.cachedSelectedText = controller.textInput.selectedText === undefined || controller.textInput.selectedText === null
                ? ""
                : String(controller.textInput.selectedText);
        controller.cachedSelectionCursorPosition = Math.max(
                    0,
                    Math.floor(Number(controller.textInput.cursorPosition) || selectionEnd));
        controller.cachedSelectionEnd = selectionEnd;
        controller.cachedSelectionStart = selectionStart;
        return true;
    }

    function maybeDiscardCachedSelectionSnapshot() {
        if (!controller.textInput) {
            controller.clearCachedSelectionSnapshot();
            return;
        }
        const selectionStart = Math.max(0, Math.floor(Number(controller.textInput.selectionStart) || 0));
        const selectionEnd = Math.max(selectionStart, Math.floor(Number(controller.textInput.selectionEnd) || 0));
        if (selectionEnd > selectionStart) {
            controller.cacheCurrentSelectionSnapshot();
            return;
        }

        const cachedStart = Math.max(0, Math.floor(Number(controller.cachedSelectionStart) || 0));
        const cachedEnd = Math.max(cachedStart, Math.floor(Number(controller.cachedSelectionEnd) || 0));
        if (cachedEnd <= cachedStart) {
            controller.clearCachedSelectionSnapshot();
            return;
        }

        if (!controller.controlFocused()) {
            controller.clearCachedSelectionSnapshot();
            return;
        }

        const currentCursorPosition = Math.max(0, Math.floor(Number(controller.textInput.cursorPosition) || 0));
        if (currentCursorPosition !== cachedStart && currentCursorPosition !== cachedEnd)
            controller.clearCachedSelectionSnapshot();
    }

    function inlineFormatSelectionSnapshot() {
        if (controller.cacheCurrentSelectionSnapshot())
            return controller.selectionSnapshot();

        const cachedStart = Math.max(0, Math.floor(Number(controller.cachedSelectionStart) || 0));
        const cachedEnd = Math.max(cachedStart, Math.floor(Number(controller.cachedSelectionEnd) || 0));
        if (cachedEnd <= cachedStart)
            return controller.selectionSnapshot();

        if (!controller.textInput)
            return controller.selectionSnapshot();

        const currentCursorPosition = Math.max(0, Math.floor(Number(controller.textInput.cursorPosition) || 0));
        if (!controller.controlFocused()
                || (currentCursorPosition !== cachedStart && currentCursorPosition !== cachedEnd)) {
            return controller.selectionSnapshot();
        }

        return {
            "cursorPosition": Math.max(
                                  0,
                                  Math.floor(Number(controller.cachedSelectionCursorPosition) || cachedEnd)),
            "selectedText": controller.cachedSelectedText,
            "selectionEnd": cachedEnd,
            "selectionStart": cachedStart
        };
    }

    function nativeCompositionActive() {
        if (!controller.control)
            return false;
        const inputMethodComposing = controller.control.inputMethodComposing !== undefined
                ? !!controller.control.inputMethodComposing
                : false;
        const preeditText = controller.control.preeditText === undefined || controller.control.preeditText === null
                ? ""
                : String(controller.control.preeditText);
        return inputMethodComposing || preeditText.length > 0;
    }

    function clampLogicalPosition(position, maximumLength) {
        const numericPosition = Number(position);
        if (!isFinite(numericPosition))
            return 0;
        return Math.max(0, Math.min(Math.floor(numericPosition), Math.max(0, maximumLength)));
    }

    function setCursorPositionPreservingNativeInput(position) {
        if (!controller.textInput || controller.nativeCompositionActive())
            return false;
        const maximumLength = Number(controller.textInput.length) || 0;
        const boundedPosition = controller.clampLogicalPosition(position, maximumLength);
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "setCursorPositionPreservingNativeInput",
                    "requested=" + position + " bounded=" + boundedPosition + " maximumLength=" + maximumLength,
                    controller.control)
        if (Number(controller.textInput.cursorPosition) === boundedPosition)
            return false;
        controller.textInput.cursorPosition = boundedPosition;
        return true;
    }

    function selectionCursorUsesStartEdge(cursorPosition, selectionStart, selectionEnd) {
        const boundedStart = Math.max(0, Math.floor(Number(selectionStart) || 0));
        const boundedEnd = Math.max(boundedStart, Math.floor(Number(selectionEnd) || 0));
        const numericCursor = Number(cursorPosition);
        if (!isFinite(numericCursor))
            return false;
        const boundedCursor = controller.clampLogicalPosition(numericCursor, boundedEnd);
        if (boundedCursor <= boundedStart)
            return true;
        if (boundedCursor >= boundedEnd)
            return false;
        return Math.abs(boundedCursor - boundedStart) <= Math.abs(boundedCursor - boundedEnd);
    }

    function restoreSelectionRange(selectionStart, selectionEnd, cursorPosition) {
        if (!controller.textInput)
            return false;
        const maximumLength = Number(controller.textInput.length) || 0;
        const boundedStart = controller.clampLogicalPosition(selectionStart, maximumLength);
        const boundedEnd = controller.clampLogicalPosition(selectionEnd, maximumLength);
        if (boundedEnd <= boundedStart)
            return false;
        const activeCursor = isFinite(Number(cursorPosition)) ? Number(cursorPosition) : boundedEnd;
        const activeEdgeIsStart = controller.selectionCursorUsesStartEdge(activeCursor, boundedStart, boundedEnd);
        if (controller.textInput.moveCursorSelection !== undefined) {
            const anchorPosition = activeEdgeIsStart ? boundedEnd : boundedStart;
            const activePosition = activeEdgeIsStart ? boundedStart : boundedEnd;
            controller.textInput.cursorPosition = anchorPosition;
            controller.textInput.moveCursorSelection(activePosition, TextEdit.SelectCharacters);
            return true;
        }
        if (controller.textInput.select !== undefined) {
            controller.textInput.select(boundedStart, boundedEnd);
            return true;
        }
        return false;
    }

    function programmaticTextSyncPolicy(nextText) {
        const normalizedNextText = nextText === undefined
                ? controller.deferredProgrammaticText
                : nextText;
        return inlineInputPolicyAdapter.programmaticTextSyncPolicy(
                    controller.currentPlainText(),
                    normalizedNextText,
                    controller.nativeCompositionActive(),
                    controller.controlFocused(),
                    controller.control && controller.control.preferNativeInputHandling !== undefined
                        ? !!controller.control.preferNativeInputHandling
                        : false,
                    controller.localTextEditSinceFocus,
                    controller.localSelectionInteractionSinceFocus);
    }

    function canDeferProgrammaticTextSync(nextText) {
        const syncPolicy = controller.programmaticTextSyncPolicy(nextText);
        return !!syncPolicy.defer;
    }

    function shouldRejectFocusedProgrammaticTextSync(nextText) {
        const syncPolicy = controller.programmaticTextSyncPolicy(nextText);
        return !syncPolicy.apply;
    }

    function flushDeferredProgrammaticText(force) {
        if (!controller.hasDeferredProgrammaticText)
            return;
        if (!force && controller.canDeferProgrammaticTextSync(controller.deferredProgrammaticText))
            return;

        const deferredText = controller.deferredProgrammaticText;
        controller.hasDeferredProgrammaticText = false;
        controller.deferredProgrammaticText = "";
        controller.setProgrammaticText(deferredText);
    }

    function clearDeferredProgrammaticText() {
        controller.hasDeferredProgrammaticText = false;
        controller.deferredProgrammaticText = "";
    }

    function dispatchCommittedTextEditedIfReady() {
        if (controller.programmaticTextSyncDepth > 0)
            return false;
        if (!controller.control)
            return false;
        if (controller.control.blockExternalDropMutation)
            return false;
        if (controller.control.suppressCommittedTextEditedDispatch)
            return false;
        if (controller.nativeCompositionActive())
            return false;
        controller.control.textEdited(controller.currentPlainText());
        return true;
    }

    function setProgrammaticText(nextText) {
        if (!controller.textInput)
            return;
        const normalizedText = nextText === undefined || nextText === null ? "" : String(nextText);
        if (controller.textInput.text === normalizedText)
            return;
        const previousCursorPosition = Number(controller.textInput.cursorPosition);
        const previousSelectionStart = Number(controller.textInput.selectionStart);
        const previousSelectionEnd = Number(controller.textInput.selectionEnd);
        const hadSelection = isFinite(previousSelectionStart) && isFinite(previousSelectionEnd)
                && previousSelectionEnd > previousSelectionStart;
        controller.fallbackTextEditedDispatchRevision += 1;
        controller.fallbackTextEditedDispatchQueued = false;
        controller.programmaticTextSyncDepth += 1;
        controller.textInput.text = normalizedText;
        controller.clearCachedSelectionSnapshot();
        if (hadSelection && previousSelectionEnd > previousSelectionStart) {
            controller.restoreSelectionRange(previousSelectionStart, previousSelectionEnd, previousCursorPosition);
        } else {
            const maximumLength = Number(controller.textInput.length) || 0;
            const restoredCursorPosition = controller.clampLogicalPosition(previousCursorPosition, maximumLength);
            if (controller.textInput.deselect !== undefined)
                controller.textInput.deselect();
            controller.setCursorPositionPreservingNativeInput(restoredCursorPosition);
        }
        controller.programmaticTextSyncDepth -= 1;
    }

    function scheduleCommittedTextEditedDispatch() {
        if (controller.programmaticTextSyncDepth > 0)
            return;
        if (controller.fallbackTextEditedDispatchQueued)
            return;
        const scheduledRevision = controller.fallbackTextEditedDispatchRevision + 1;
        controller.fallbackTextEditedDispatchRevision = scheduledRevision;
        controller.fallbackTextEditedDispatchQueued = true;
        Qt.callLater(function () {
            controller.fallbackTextEditedDispatchQueued = false;
            if (controller.fallbackTextEditedDispatchRevision !== scheduledRevision)
                return;
            controller.dispatchCommittedTextEditedIfReady();
        });
    }

    function handleControlFocusedChanged() {
        const focused = controller.controlFocused();
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "focusedChanged",
                    "focused=" + focused + " nativeCompositionActive=" + controller.nativeCompositionActive(),
                    controller.control)
        const hadLocalTextEditSinceFocus = controller.localTextEditSinceFocus;
        if (!focused && controller.nativeCompositionActive()) {
            Qt.callLater(function () {
                if (!controller.controlFocused() && !controller.nativeCompositionActive()) {
                    if (controller.control && controller.control.preferNativeInputHandling && hadLocalTextEditSinceFocus)
                        controller.clearDeferredProgrammaticText();
                    else
                        controller.flushDeferredProgrammaticText(true);
                }
            });
            return;
        }
        controller.maybeDiscardCachedSelectionSnapshot();
        if (!focused) {
            if (controller.control && controller.control.preferNativeInputHandling && hadLocalTextEditSinceFocus)
                controller.clearDeferredProgrammaticText();
            else
                controller.flushDeferredProgrammaticText(true);
        }
        controller.localSelectionInteractionSinceFocus = false;
        controller.localTextEditSinceFocus = false;
    }

    function handleControlTextChanged() {
        const normalizedText = controller.controlText();
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "textChanged",
                    EditorTrace.describeText(normalizedText),
                    controller.control)
        const syncPolicy = controller.programmaticTextSyncPolicy(normalizedText);
        if (syncPolicy.defer) {
            controller.deferredProgrammaticText = normalizedText;
            controller.hasDeferredProgrammaticText = true;
            return;
        }
        if (!syncPolicy.apply) {
            controller.hasDeferredProgrammaticText = false;
            controller.deferredProgrammaticText = "";
            return;
        }
        controller.hasDeferredProgrammaticText = false;
        controller.deferredProgrammaticText = "";
        controller.setProgrammaticText(normalizedText);
    }

    function handleTextInputActiveFocusChanged() {
        if (!controller.textInput)
            return;
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "textInputActiveFocusChanged",
                    "activeFocus=" + controller.textInput.activeFocus,
                    controller.control)
        controller.maybeDiscardCachedSelectionSnapshot();
    }

    function handleTextInputCursorPositionChanged() {
        if (!controller.textInput)
            return;
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "cursorPositionChanged",
                    "cursorPosition=" + controller.textInput.cursorPosition,
                    controller.control)
        if (controller.controlFocused())
            controller.localSelectionInteractionSinceFocus = true;
        controller.maybeDiscardCachedSelectionSnapshot();
    }

    function handleTextInputSelectionEndChanged() {
        if (!controller.textInput)
            return;
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "selectionEndChanged",
                    "selectionStart=" + controller.textInput.selectionStart
                    + " selectionEnd=" + controller.textInput.selectionEnd,
                    controller.control)
        if (controller.controlFocused())
            controller.localSelectionInteractionSinceFocus = true;
        controller.maybeDiscardCachedSelectionSnapshot();
    }

    function handleTextInputSelectionStartChanged() {
        if (!controller.textInput)
            return;
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "selectionStartChanged",
                    "selectionStart=" + controller.textInput.selectionStart
                    + " selectionEnd=" + controller.textInput.selectionEnd,
                    controller.control)
        if (controller.controlFocused())
            controller.localSelectionInteractionSinceFocus = true;
        controller.maybeDiscardCachedSelectionSnapshot();
    }

    function handleTextInputTextChanged() {
        if (!controller.textInput)
            return;
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "textInputTextChanged",
                    "programmaticDepth=" + controller.programmaticTextSyncDepth
                    + " nativeCompositionActive=" + controller.nativeCompositionActive()
                    + " " + EditorTrace.describeText(controller.textInput.text),
                    controller.control)
        controller.clearCachedSelectionSnapshot();
        if (controller.programmaticTextSyncDepth > 0)
            return;
        controller.localSelectionInteractionSinceFocus = true;
        controller.localTextEditSinceFocus = true;
        controller.clearDeferredProgrammaticText();
        if (controller.control
                && (controller.control.blockExternalDropMutation
                    || controller.control.suppressCommittedTextEditedDispatch)) {
            controller.fallbackTextEditedDispatchRevision += 1;
            controller.fallbackTextEditedDispatchQueued = false;
            return;
        }
        if (controller.nativeCompositionActive()) {
            controller.scheduleCommittedTextEditedDispatch();
            return;
        }
        controller.fallbackTextEditedDispatchRevision += 1;
        controller.dispatchCommittedTextEditedIfReady();
    }

    function handleInputMethodComposingChanged() {
        if (!controller.textInput || controller.textInput.inputMethodComposing)
            return;
        controller.scheduleCommittedTextEditedDispatch();
        controller.flushDeferredProgrammaticText(false);
    }

    function handlePreeditTextChanged() {
        if (!controller.textInput || controller.textInput.inputMethodComposing)
            return;
        const preeditText = controller.control && controller.control.preeditText !== undefined
                && controller.control.preeditText !== null
                ? String(controller.control.preeditText)
                : "";
        if (preeditText.length > 0)
            return;
        controller.scheduleCommittedTextEditedDispatch();
        controller.flushDeferredProgrammaticText(false);
    }

    function mount() {
        EditorTrace.trace("inlineFormatEditor", "mount", EditorTrace.describeText(controller.controlText()), controller.control);
        controller.setProgrammaticText(controller.controlText());
    }

    function unmount() {
        if (!controller.textInput)
            return;
        EditorTrace.trace(
                    "inlineFormatEditor",
                    "unmount",
                    "cursorPosition=" + controller.textInput.cursorPosition
                    + " selectionStart=" + controller.textInput.selectionStart
                    + " selectionEnd=" + controller.textInput.selectionEnd,
                    controller.control)
    }

    Connections {
        function onFocusedChanged() {
            controller.handleControlFocusedChanged();
        }

        function onTextChanged() {
            controller.handleControlTextChanged();
        }

        target: controller.control
    }

    Connections {
        function onActiveFocusChanged() {
            controller.handleTextInputActiveFocusChanged();
        }

        function onCursorPositionChanged() {
            controller.handleTextInputCursorPositionChanged();
        }

        function onInputMethodComposingChanged() {
            controller.handleInputMethodComposingChanged();
        }

        function onPreeditTextChanged() {
            controller.handlePreeditTextChanged();
        }

        function onSelectionEndChanged() {
            controller.handleTextInputSelectionEndChanged();
        }

        function onSelectionStartChanged() {
            controller.handleTextInputSelectionStartChanged();
        }

        function onTextChanged() {
            controller.handleTextInputTextChanged();
        }

        ignoreUnknownSignals: true
        target: controller.textInput
    }

    Component.onCompleted: controller.mount()
    Component.onDestruction: controller.unmount()
}
}
