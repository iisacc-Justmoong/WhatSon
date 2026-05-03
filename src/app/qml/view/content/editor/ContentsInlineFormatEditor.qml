pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: control

    property bool activeFocusOnPress: true
    property bool autoFocusOnPress: true
    property alias contentHeight: textInput.contentHeight
    property alias cursorPosition: textInput.cursorPosition
    property alias editorItem: textInput
    property bool focused: textInput.activeFocus
    property alias inputItem: textInput
    property int inputMethodHints: Qt.ImhNone
    property int mouseSelectionMode: TextEdit.SelectCharacters
    property bool overwriteMode: false
    property bool persistentSelection: true
    readonly property bool inputMethodComposing: textInput.inputMethodComposing
    readonly property string preeditText: textInput.preeditText
    property string renderedText: ""
    property bool selectByKeyboard: true
    property bool selectByMouse: true
    property alias selectedText: textInput.selectedText
    property alias selectionEnd: textInput.selectionEnd
    property alias selectionStart: textInput.selectionStart
    property bool showRenderedOutput: false
    property var tagManagementKeyPressHandler: null
    property alias text: textInput.text
    property color textColor: LV.Theme.bodyColor

    signal textEdited(string text)
    signal viewHookRequested(string reason)

    focus: true

    function clearCachedSelectionSnapshot() {
    }

    function clearSelection() {
        textInput.deselect();
    }

    function currentPlainText() {
        return textInput.text;
    }

    function eventRequestsBodyTagShortcut(event) {
        const modifiers = Number(event.modifiers) || 0;
        const commandHeld = (modifiers & Qt.ControlModifier) || (modifiers & Qt.MetaModifier);
        const optionHeld = modifiers & Qt.AltModifier;
        return Boolean((modifiers !== 0 && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter))
                       || (commandHeld && optionHeld));
    }

    function eventRequestsInlineFormatShortcut(event) {
        const modifiers = Number(event.modifiers) || 0;
        const commandHeld = (modifiers & Qt.ControlModifier) || (modifiers & Qt.MetaModifier);
        const optionHeld = modifiers & Qt.AltModifier;
        return Boolean(commandHeld && !optionHeld
                       && (event.key === Qt.Key_B
                           || event.key === Qt.Key_I
                           || event.key === Qt.Key_U
                           || event.key === Qt.Key_H));
    }

    function eventRequestsPasteShortcut(event) {
        return event.matches(StandardKey.Paste);
    }

    function forceActiveFocus() {
        control.focus = true;
        textInput.forceActiveFocus();
    }

    function inlineFormatSelectionSnapshot() {
        return selectionSnapshot();
    }

    function nativeCompositionActive() {
        return control.inputMethodComposing || control.preeditText.length > 0;
    }

    function positionToRectangle(position) {
        return textInput.positionToRectangle(position);
    }

    function requestViewHook(reason) {
        control.viewHookRequested(reason !== undefined ? String(reason) : "manual");
    }

    function programmaticTextSyncPolicy(nextText) {
        return inlineEditorController.programmaticTextSyncPolicy(nextText);
    }

    function restoreSelectionRange(selectionStart, selectionEnd, cursorPosition) {
        const start = Math.max(0, Math.min(Number(selectionStart) || 0, textInput.length));
        const end = Math.max(start, Math.min(Number(selectionEnd) || start, textInput.length));
        textInput.select(start, end);
        textInput.cursorPosition = Math.max(start, Math.min(Number(cursorPosition) || end, end));
        return true;
    }

    function selectionSnapshot() {
        return {
            "cursorPosition": textInput.cursorPosition,
            "selectionStart": textInput.selectionStart,
            "selectionEnd": textInput.selectionEnd,
            "selectedText": textInput.selectedText
        };
    }

    function setCursorPositionPreservingNativeInput(position) {
        if (nativeCompositionActive())
            return textInput.cursorPosition;
        textInput.cursorPosition = Math.max(0, Math.min(Number(position) || 0, textInput.length));
        return textInput.cursorPosition;
    }

    function setProgrammaticText(nextText) {
        const resolvedText = nextText === undefined || nextText === null ? "" : String(nextText);
        if (textInput.text !== resolvedText)
            textInput.text = resolvedText;
    }

    function handleTagManagementKeyPress(event) {
        const key = event.key;
        if (key !== Qt.Key_Backspace
                && !control.eventRequestsPasteShortcut(event)
                && !control.eventRequestsInlineFormatShortcut(event)
                && !control.eventRequestsBodyTagShortcut(event)) {
            return;
        }

        if (control.tagManagementKeyPressHandler === null
                || control.tagManagementKeyPressHandler === undefined) {
            event.accepted = false;
            return;
        }

        const handled = Boolean(control.tagManagementKeyPressHandler(event));
        event.accepted = handled;
        if (!handled)
            event.accepted = false;
    }

    clip: true

    Keys.priority: Keys.BeforeItem
    Keys.onPressed: function (event) {
        control.handleTagManagementKeyPress(event);
    }

    ContentsInlineFormatEditorController {
        id: inlineEditorController

        control: control
        textInput: textInput
    }

    Text {
        id: renderedOverlay

        anchors.fill: textInput
        color: control.textColor
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        linkColor: LV.Theme.primary
        padding: textInput.padding
        text: control.renderedText
        textFormat: Text.RichText
        visible: control.showRenderedOutput && control.renderedText.length > 0 && !control.nativeCompositionActive()
        wrapMode: Text.Wrap
        z: 1

        onLinkActivated: function (link) {
            Qt.openUrlExternally(link);
        }
    }

    TextEdit {
        id: textInput

        anchors.fill: parent
        activeFocusOnPress: control.autoFocusOnPress
        color: renderedOverlay.visible ? "transparent" : control.textColor
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        inputMethodHints: control.inputMethodHints
        mouseSelectionMode: control.mouseSelectionMode
        overwriteMode: control.overwriteMode
        padding: LV.Theme.gap16
        persistentSelection: control.persistentSelection
        selectByKeyboard: control.selectByKeyboard
        selectByMouse: control.selectByMouse
        selectedTextColor: control.textColor
        selectionColor: LV.Theme.primaryOverlay
        textFormat: TextEdit.PlainText
        verticalAlignment: TextEdit.AlignTop
        wrapMode: TextEdit.Wrap

        Keys.priority: Keys.BeforeItem
        Keys.onPressed: function (event) {
            control.handleTagManagementKeyPress(event);
        }

        onTextChanged: {
            control.textEdited(textInput.text);
        }
    }
}
