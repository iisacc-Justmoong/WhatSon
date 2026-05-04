pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0

QtObject {
    id: helper

    property var control: null
    property var textInput: null
    property int programmaticTextSyncDepth: 0
    property bool hasDeferredProgrammaticText: false
    property string deferredProgrammaticText: ""
    property bool localSelectionInteractionSinceFocus: false
    property bool localTextEditSinceFocus: false

    property ContentsEditorInputPolicyAdapter inputPolicyAdapter: ContentsEditorInputPolicyAdapter {
    }

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
        if (!helper.textInput)
            return "";
        const maximumLength = Number(helper.textInput.length) || 0;
        if (helper.textInput.getText !== undefined)
            return helper.textInput.getText(0, maximumLength);
        return resolvedText(helper.textInput.text);
    }

    function controlFocused() {
        return Boolean(helper.control
                       && helper.control.focused !== undefined
                       && helper.control.focused);
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

    function nativeSelectionActive() {
        return Boolean(helper.textInput
                       && Number(helper.textInput.selectionStart) !== Number(helper.textInput.selectionEnd));
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
        const cursor = Math.max(start, Math.min(Number(cursorPosition) || end, end));
        if (start === end) {
            helper.textInput.cursorPosition = cursor;
            return true;
        }
        if (cursor === start) {
            helper.textInput.cursorPosition = end;
            helper.textInput.moveCursorSelection(start, TextEdit.SelectCharacters);
        } else {
            helper.textInput.cursorPosition = start;
            helper.textInput.moveCursorSelection(end, TextEdit.SelectCharacters);
        }
        return true;
    }

    function programmaticTextSyncPolicy(nextText) {
        const text = nextText === undefined ? helper.deferredProgrammaticText : resolvedText(nextText);
        if (!helper.textInput)
            return { "action": "ignore", "apply": false, "text": text, "defer": false, "reject": false };
        if (currentPlainText() === text)
            return { "action": "noop", "apply": false, "text": text, "defer": false, "reject": false };

        const policy = helper.inputPolicyAdapter.programmaticTextSyncPolicy(
                    currentPlainText(),
                    text,
                    nativeCompositionActive(),
                    controlFocused(),
                    Boolean(helper.control
                            && helper.control.preferNativeInputHandling !== undefined
                            && helper.control.preferNativeInputHandling),
                    helper.localTextEditSinceFocus,
                    helper.localSelectionInteractionSinceFocus || nativeSelectionActive());
        const apply = Boolean(policy.apply);
        const defer = Boolean(policy.defer);
        return {
            "action": defer ? "defer" : (apply ? "apply" : "reject"),
            "apply": apply,
            "text": text,
            "defer": defer,
            "reject": !apply && !defer
        };
    }

    function canDeferProgrammaticTextSync(nextText) {
        return programmaticTextSyncPolicy(nextText).defer === true;
    }

    function shouldRejectFocusedProgrammaticTextSync(nextText) {
        const policy = programmaticTextSyncPolicy(nextText);
        return policy.action !== "noop" && policy.apply !== true;
    }

    function flushDeferredProgrammaticText(force) {
        if (!helper.textInput || !helper.hasDeferredProgrammaticText)
            return;
        if (!force && canDeferProgrammaticTextSync(helper.deferredProgrammaticText))
            return;
        const deferredText = helper.deferredProgrammaticText;
        clearDeferredProgrammaticText();
        if (force)
            applyProgrammaticText(deferredText);
        else
            setProgrammaticText(deferredText);
    }

    function clearDeferredProgrammaticText() {
        helper.hasDeferredProgrammaticText = false;
        helper.deferredProgrammaticText = "";
    }

    function dispatchCommittedTextEditedIfReady() {
        if (helper.programmaticTextSyncDepth > 0)
            return false;
        if (!helper.control || !helper.textInput || nativeCompositionActive())
            return false;
        helper.control.textEdited(helper.textInput.text);
        return true;
    }

    function applyProgrammaticText(nextText) {
        if (!helper.textInput)
            return;
        const normalizedText = resolvedText(nextText);
        if (currentPlainText() === normalizedText)
            return;

        const previousCursorPosition = Number(helper.textInput.cursorPosition);
        const previousSelectionStart = Number(helper.textInput.selectionStart);
        const previousSelectionEnd = Number(helper.textInput.selectionEnd);
        const hadSelection = isFinite(previousSelectionStart)
                && isFinite(previousSelectionEnd)
                && previousSelectionEnd > previousSelectionStart;

        helper.programmaticTextSyncDepth += 1;
        helper.textInput.text = normalizedText;
        if (hadSelection) {
            restoreSelectionRange(previousSelectionStart, previousSelectionEnd, previousCursorPosition);
        } else {
            const restoredCursorPosition = clampLogicalPosition(
                        previousCursorPosition,
                        helper.textInput.length);
            if (helper.textInput.deselect !== undefined)
                helper.textInput.deselect();
            setCursorPositionPreservingNativeInput(restoredCursorPosition);
        }
        helper.programmaticTextSyncDepth = Math.max(0, helper.programmaticTextSyncDepth - 1);
    }

    function setProgrammaticText(nextText) {
        const policy = programmaticTextSyncPolicy(nextText);
        if (policy.defer) {
            helper.deferredProgrammaticText = policy.text;
            helper.hasDeferredProgrammaticText = true;
            return;
        }
        if (!helper.textInput || policy.reject || policy.action === "noop")
            return;
        applyProgrammaticText(policy.text);
    }

    function applyImmediateProgrammaticText(nextText) {
        clearDeferredProgrammaticText();
        applyProgrammaticText(resolvedText(nextText));
    }

    function scheduleCommittedTextEditedDispatch() {
        dispatchCommittedTextEditedIfReady();
    }

    function handleControlFocusedChanged() {
        if (!controlFocused()) {
            flushDeferredProgrammaticText(true);
            helper.localSelectionInteractionSinceFocus = false;
            helper.localTextEditSinceFocus = false;
        }
    }

    function handleTextInputCursorPositionChanged() {
        if (controlFocused())
            helper.localSelectionInteractionSinceFocus = true;
    }

    function handleTextInputSelectionChanged() {
        if (controlFocused())
            helper.localSelectionInteractionSinceFocus = true;
    }

    function handleTextInputTextChanged() {
        if (helper.programmaticTextSyncDepth > 0)
            return;
        if (controlFocused()) {
            helper.localSelectionInteractionSinceFocus = true;
            helper.localTextEditSinceFocus = true;
        }
        clearDeferredProgrammaticText();
    }

    function handleNativeCompositionSettled() {
        if (nativeCompositionActive())
            return;
        flushDeferredProgrammaticText(false);
    }

    Connections {
        function onFocusedChanged() {
            helper.handleControlFocusedChanged();
        }

        target: helper.control
    }

    Connections {
        function onCursorPositionChanged() {
            helper.handleTextInputCursorPositionChanged();
        }

        function onInputMethodComposingChanged() {
            helper.handleNativeCompositionSettled();
        }

        function onPreeditTextChanged() {
            helper.handleNativeCompositionSettled();
        }

        function onSelectionEndChanged() {
            helper.handleTextInputSelectionChanged();
        }

        function onSelectionStartChanged() {
            helper.handleTextInputSelectionChanged();
        }

        function onTextChanged() {
            helper.handleTextInputTextChanged();
        }

        ignoreUnknownSignals: true
        target: helper.textInput
    }
}
