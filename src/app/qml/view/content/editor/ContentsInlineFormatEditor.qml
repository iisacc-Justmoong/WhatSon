pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

FocusScope {
    id: control

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
    property bool showRenderedOutput: true
    property bool showScrollBar: false
    property string text: ""
    property color textColor: LV.Theme.bodyColor
    property int textFormat: TextEdit.PlainText
    property int wrapMode: TextEdit.NoWrap
    property bool preferNativeInputHandling: false

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

    property int _programmaticTextSyncDepth: 0
    property bool _compositionEditPending: false
    property bool _hasDeferredProgrammaticText: false
    property string _deferredProgrammaticText: ""

    signal textEdited(string text)

    function forceActiveFocus() {
        textInput.forceActiveFocus();
    }

    function getText(start, end) {
        return textInput.getText(start, end);
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

    function clampLogicalPosition(position, maximumLength) {
        const numericPosition = Number(position);
        if (!isFinite(numericPosition))
            return 0;
        return Math.max(0, Math.min(Math.floor(numericPosition), Math.max(0, maximumLength)));
    }

    function canDeferProgrammaticTextSync() {
        return !!control.preferNativeInputHandling
                && (control.focused || control.inputMethodComposing || control.preeditText.length > 0);
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

    function setProgrammaticText(nextText) {
        const normalizedText = nextText === undefined || nextText === null ? "" : String(nextText);
        if (textInput.text === normalizedText)
            return;
        const previousCursorPosition = Number(textInput.cursorPosition);
        const previousSelectionStart = Number(textInput.selectionStart);
        const previousSelectionEnd = Number(textInput.selectionEnd);
        const hadSelection = isFinite(previousSelectionStart) && isFinite(previousSelectionEnd)
                && previousSelectionEnd > previousSelectionStart;
        textChangedDispatchTimer.stop();
        control._compositionEditPending = false;
        control._programmaticTextSyncDepth += 1;
        textInput.text = normalizedText;
        const maximumLength = Number(textInput.length) || 0;
        const restoredSelectionStart = control.clampLogicalPosition(previousSelectionStart, maximumLength);
        const restoredSelectionEnd = control.clampLogicalPosition(previousSelectionEnd, maximumLength);
        const restoredCursorPosition = control.clampLogicalPosition(previousCursorPosition, maximumLength);
        if (hadSelection && restoredSelectionEnd > restoredSelectionStart) {
            textInput.select(restoredSelectionStart, restoredSelectionEnd);
        } else {
            if (textInput.deselect !== undefined)
                textInput.deselect();
            textInput.cursorPosition = restoredCursorPosition;
        }
        control._programmaticTextSyncDepth -= 1;
    }

    function scheduleCommittedTextEditedDispatch() {
        if (control._programmaticTextSyncDepth > 0)
            return;
        if (control.inputMethodComposing || control.preeditText.length > 0) {
            control._compositionEditPending = true;
            return;
        }
        if (!textChangedDispatchTimer.running)
            textChangedDispatchTimer.start();
    }

    onFocusedChanged: {
        if (!focused)
            control.flushDeferredProgrammaticText(true);
    }
    onTextChanged: {
        const normalizedText = text === undefined || text === null ? "" : String(text);
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
            height: Math.max(
                        1,
                        Math.max(
                            Number(control.fieldMinHeight) || 0,
                            Number(control.editorHeight) || 0,
                            Number(textInput.contentHeight) || 0))

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
            property alias textFormat: textInput.textFormat
            property alias topPadding: textInput.topPadding

            function forceActiveFocus() {
                textInput.forceActiveFocus();
            }

            function positionToRectangle(position) {
                return textInput.positionToRectangle(position);
            }

            TextEdit {
                id: textInput

                anchors.fill: parent
                activeFocusOnPress: control.autoFocusOnPress
                bottomPadding: control.insetVertical
                color: control.textColor
                cursorVisible: control.activeFocus
                font.family: control.fontFamily
                font.letterSpacing: control.fontLetterSpacing
                font.pixelSize: control.fontPixelSize
                font.weight: control.fontWeight
                leftPadding: control.insetHorizontal
                persistentSelection: true
                readOnly: false
                rightPadding: control.insetHorizontal
                selectByMouse: control.selectByMouse
                selectedTextColor: control.selectedTextColor
                selectionColor: control.selectionColor
                textFormat: control.textFormat
                topPadding: 0
                wrapMode: control.wrapMode

                readonly property bool focused: activeFocus
                property bool showRenderedOutput: control.showRenderedOutput

                onTextChanged: {
                    control.scheduleCommittedTextEditedDispatch();
                }
            }
        }

        Controls.ScrollBar.vertical: control.showScrollBar ? verticalScrollBar : null
    }

    Controls.ScrollBar {
        id: verticalScrollBar
    }

    Timer {
        id: textChangedDispatchTimer

        interval: 0
        repeat: false

        onTriggered: {
            if (control._programmaticTextSyncDepth > 0)
                return;
            if (control.inputMethodComposing || control.preeditText.length > 0) {
                control._compositionEditPending = true;
                return;
            }
            control._compositionEditPending = false;
            control.textEdited(textInput.text);
        }
    }

    Connections {
        function onInputMethodComposingChanged() {
            if (!textInput.inputMethodComposing && control._compositionEditPending)
                control.scheduleCommittedTextEditedDispatch();
            if (!textInput.inputMethodComposing)
                control.flushDeferredProgrammaticText(false);
        }

        function onPreeditTextChanged() {
            if (!textInput.inputMethodComposing && control._compositionEditPending && control.preeditText.length === 0)
                control.scheduleCommittedTextEditedDispatch();
            if (!textInput.inputMethodComposing && control.preeditText.length === 0)
                control.flushDeferredProgrammaticText(false);
        }

        function onTextEdited() {
            control.scheduleCommittedTextEditedDispatch();
        }

        ignoreUnknownSignals: true
        target: textInput
    }

    Component.onCompleted: setProgrammaticText(text)
}
