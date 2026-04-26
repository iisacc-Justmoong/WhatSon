pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV
import "../../../../models/editor/input" as EditorInputModel

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

    function macOsTextUnitIsWord(character) {
        return inlineEditorController.macOsTextUnitIsWord(character);
    }

    function macOsOptionWordNavigationTarget(direction, cursorPosition) {
        return inlineEditorController.macOsOptionWordNavigationTarget(direction, cursorPosition);
    }

    function handleMacOsOptionWordNavigation(event) {
        return inlineEditorController.handleMacOsOptionWordNavigation(event);
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

                Keys.priority: Keys.BeforeItem
                Keys.onPressed: function (event) {
                    inlineEditorController.handleMacOsOptionWordNavigation(event);
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

    EditorInputModel.ContentsInlineFormatEditorController {
        id: inlineEditorController

        control: control
        textInput: textInput
    }
}
