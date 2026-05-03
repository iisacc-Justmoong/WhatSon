pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: control

    property bool activeFocusOnPress: true
    property bool autoFocusOnPress: true
    property alias contentHeight: textInput.contentHeight
    property alias cursorPosition: textInput.cursorPosition
    readonly property var editorItem: textInput.editorItem
    property bool focused: textInput.focused
    readonly property var inputItem: textInput.editorItem
    property int inputMethodHints: Qt.ImhNone
    property int mouseSelectionMode: TextEdit.SelectCharacters
    readonly property bool nativeSelectionActive: textInput.selectionStart !== textInput.selectionEnd
    property bool overwriteMode: false
    property bool persistentSelection: true
    readonly property bool inputMethodComposing: textInput.inputMethodComposing
    readonly property string preeditText: String(textInput.editorItem.preeditText)
    property string renderedText: ""
    readonly property bool renderedOverlayVisible: control.showRenderedOutput
            && control.renderedText.length > 0
            && !control.nativeCompositionActive()
            && !control.nativeSelectionActive
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
        const key = event.key;
        const pureModifierKey = key === Qt.Key_Alt
                || key === Qt.Key_Control
                || key === Qt.Key_Meta
                || key === Qt.Key_Shift;
        if (pureModifierKey)
            return false;
        const modifiers = Number(event.modifiers) || 0;
        const commandHeld = (modifiers & Qt.ControlModifier) || (modifiers & Qt.MetaModifier);
        const optionHeld = modifiers & Qt.AltModifier;
        return Boolean((modifiers !== 0 && (key === Qt.Key_Return || key === Qt.Key_Enter))
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
        textInput.forceEditorFocus();
    }

    function inlineFormatSelectionSnapshot() {
        return selectionSnapshot();
    }

    function nativeCompositionActive() {
        return control.inputMethodComposing || control.preeditText.length > 0;
    }

    function positionToRectangle(position) {
        return textInput.editorItem.positionToRectangle(position);
    }

    function mapEditorPointToItem(target, x, y) {
        const mappedPoint = textInput.editorItem.mapToItem(
                    target,
                    Number(x) || LV.Theme.gapNone,
                    Number(y) || LV.Theme.gapNone);
        return {
            "x": mappedPoint.x,
            "y": mappedPoint.y
        };
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
        const cursor = Math.max(start, Math.min(Number(cursorPosition) || end, end));
        if (start === end) {
            textInput.cursorPosition = cursor;
            return true;
        }
        if (cursor === start) {
            textInput.cursorPosition = end;
            textInput.editorItem.moveCursorSelection(start, TextEdit.SelectCharacters);
        } else {
            textInput.cursorPosition = start;
            textInput.editorItem.moveCursorSelection(end, TextEdit.SelectCharacters);
        }
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

    function syntheticTagManagementKeyEvent(key, modifiers) {
        return {
            "accepted": false,
            "key": key,
            "matches": function (standardKey) {
                return false;
            },
            "modifiers": modifiers
        };
    }

    function triggerTagManagementShortcut(key, modifiers) {
        const event = syntheticTagManagementKeyEvent(key, modifiers);
        control.handleTagManagementKeyPress(event);
        return event.accepted === true;
    }

    clip: true

    Keys.priority: Keys.BeforeItem
    Keys.onPressed: function (event) {
        control.handleTagManagementKeyPress(event);
    }

    ContentsInlineFormatEditorController {
        id: inlineEditorController

        control: control
        textInput: textInput.editorItem
    }

    Text {
        id: renderedOverlay

        anchors.fill: textInput
        color: control.textColor
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        linkColor: LV.Theme.primary
        padding: LV.Theme.gapNone
        text: control.renderedText
        textFormat: Text.RichText
        visible: control.renderedOverlayVisible
        wrapMode: Text.Wrap
        z: 1

        onLinkActivated: function (link) {
            Qt.openUrlExternally(link);
        }
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: control.focused
                 && control.tagManagementKeyPressHandler !== null
                 && control.tagManagementKeyPressHandler !== undefined
        sequence: "Ctrl+Alt+C"

        onActivated: control.triggerTagManagementShortcut(Qt.Key_C, Qt.ControlModifier | Qt.AltModifier)
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: control.focused
                 && control.tagManagementKeyPressHandler !== null
                 && control.tagManagementKeyPressHandler !== undefined
        sequence: "Meta+Alt+C"

        onActivated: control.triggerTagManagementShortcut(Qt.Key_C, Qt.MetaModifier | Qt.AltModifier)
    }

    LV.TextEditor {
        id: textInput

        anchors.fill: parent
        anchors.leftMargin: LV.Theme.gap16
        anchors.rightMargin: LV.Theme.gap16
        activeFocusOnPress: control.autoFocusOnPress
        autoFocusOnPress: control.autoFocusOnPress
        backgroundColor: "transparent"
        backgroundColorDisabled: "transparent"
        backgroundColorFocused: "transparent"
        backgroundColorHover: "transparent"
        backgroundColorPressed: "transparent"
        centeredTextHeight: Math.max(1, textInput.resolvedEditorHeight - textInput.insetVertical * 2)
        cornerRadius: LV.Theme.gapNone
        editorHeight: Math.max(1, control.height)
        enforceModeDefaults: true
        fieldMinHeight: LV.Theme.gap16
        fontFamily: LV.Theme.fontBody
        fontPixelSize: LV.Theme.textBody
        insetHorizontal: LV.Theme.gapNone
        insetVertical: LV.Theme.gapNone
        inputMethodHints: control.inputMethodHints
        mode: plainTextMode
        mouseSelectionMode: control.mouseSelectionMode
        overwriteMode: control.overwriteMode
        persistentSelection: control.persistentSelection
        selectByKeyboard: control.selectByKeyboard
        selectByMouse: control.selectByMouse
        selectedTextColor: control.textColor
        selectionColor: LV.Theme.primaryOverlay
        showRenderedOutput: false
        showScrollBar: false
        textColor: control.renderedOverlayVisible ? "transparent" : control.textColor
        textColorDisabled: textColor
        textFormat: TextEdit.PlainText
        wrapMode: TextEdit.Wrap

        Keys.priority: Keys.BeforeItem
        Keys.onPressed: function (event) {
            control.handleTagManagementKeyPress(event);
        }

        onTextEdited: function (text) {
            control.textEdited(text);
        }
    }
}
