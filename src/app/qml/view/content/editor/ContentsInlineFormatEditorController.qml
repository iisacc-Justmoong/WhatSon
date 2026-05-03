pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: helper

    property var control: null
    property var textInput: null
    property bool hasDeferredProgrammaticText: false
    property string deferredProgrammaticText: ""

    function resolvedText(value) {
        return value === undefined || value === null ? "" : String(value);
    }

    function forceActiveFocus() {
        if (helper.control)
            helper.control.forceActiveFocus();
        if (helper.textInput)
            helper.textInput.forceActiveFocus();
    }

    function currentPlainText() {
        return helper.textInput ? helper.textInput.text : "";
    }

    function selectionSnapshot() {
        if (!helper.textInput)
            return {};
        return {
            "cursorPosition": helper.textInput.cursorPosition,
            "selectionStart": helper.textInput.selectionStart,
            "selectionEnd": helper.textInput.selectionEnd,
            "selectedText": helper.textInput.selectedText
        };
    }

    function clearSelection() {
        if (helper.textInput)
            helper.textInput.deselect();
    }

    function clearCachedSelectionSnapshot() {
    }

    function cacheCurrentSelectionSnapshot() {
        return selectionSnapshot();
    }

    function maybeDiscardCachedSelectionSnapshot() {
    }

    function inlineFormatSelectionSnapshot() {
        return selectionSnapshot();
    }

    function nativeCompositionActive() {
        return Boolean(helper.textInput
                       && (helper.textInput.inputMethodComposing
                           || String(helper.textInput.preeditText).length > 0));
    }

    function clampLogicalPosition(position, maximumLength) {
        const maxLength = Math.max(0, Number(maximumLength) || 0);
        return Math.max(0, Math.min(Number(position) || 0, maxLength));
    }

    function setCursorPositionPreservingNativeInput(position) {
        if (!helper.textInput)
            return 0;
        if (nativeCompositionActive())
            return helper.textInput.cursorPosition;
        helper.textInput.cursorPosition = clampLogicalPosition(position, helper.textInput.length);
        return helper.textInput.cursorPosition;
    }

    function selectionCursorUsesStartEdge(cursorPosition, selectionStart, selectionEnd) {
        return Number(cursorPosition) === Number(selectionStart)
                && Number(selectionStart) !== Number(selectionEnd);
    }

    function restoreSelectionRange(selectionStart, selectionEnd, cursorPosition) {
        if (!helper.textInput)
            return false;
        const start = clampLogicalPosition(selectionStart, helper.textInput.length);
        const end = Math.max(start, clampLogicalPosition(selectionEnd, helper.textInput.length));
        helper.textInput.select(start, end);
        helper.textInput.cursorPosition = Math.max(start, Math.min(Number(cursorPosition) || end, end));
        return true;
    }

    function programmaticTextSyncPolicy(nextText) {
        const text = resolvedText(nextText);
        if (!helper.textInput)
            return { "action": "ignore", "text": text, "defer": false, "reject": false };
        if (helper.textInput.text === text)
            return { "action": "noop", "text": text, "defer": false, "reject": false };
        if (nativeCompositionActive())
            return { "action": "defer", "text": text, "defer": true, "reject": false };
        return { "action": "apply", "text": text, "defer": false, "reject": false };
    }

    function canDeferProgrammaticTextSync(nextText) {
        return programmaticTextSyncPolicy(nextText).defer === true;
    }

    function shouldRejectFocusedProgrammaticTextSync(nextText) {
        return programmaticTextSyncPolicy(nextText).reject === true;
    }

    function flushDeferredProgrammaticText(force) {
        if (!helper.textInput || !helper.hasDeferredProgrammaticText)
            return;
        if (!force && nativeCompositionActive())
            return;
        helper.textInput.text = helper.deferredProgrammaticText;
        clearDeferredProgrammaticText();
    }

    function clearDeferredProgrammaticText() {
        helper.hasDeferredProgrammaticText = false;
        helper.deferredProgrammaticText = "";
    }

    function dispatchCommittedTextEditedIfReady() {
        if (!helper.control || !helper.textInput || nativeCompositionActive())
            return false;
        helper.control.textEdited(helper.textInput.text);
        return true;
    }

    function setProgrammaticText(nextText) {
        const policy = programmaticTextSyncPolicy(nextText);
        if (policy.defer) {
            helper.deferredProgrammaticText = policy.text;
            helper.hasDeferredProgrammaticText = true;
            return;
        }
        if (!helper.textInput || policy.reject)
            return;
        helper.textInput.text = policy.text;
    }

    function scheduleCommittedTextEditedDispatch() {
        dispatchCommittedTextEditedIfReady();
    }
}
