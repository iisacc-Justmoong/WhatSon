pragma ComponentBehavior: Bound

import QtQuick
import "../diagnostics/ContentsEditorDebugTrace.js" as EditorTrace

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
