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

    function canDeferProgrammaticTextSync() {
        return control.inputMethodSessionActive()
                || (!!control.preferNativeInputHandling && control.focused);
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
        control._compositionEditPending = false;
        control._programmaticTextSyncDepth += 1;
        textInput.text = normalizedText;
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
        if (control.inputMethodComposing || control.preeditText.length > 0) {
            control._compositionEditPending = true;
            return;
        }
        control._compositionEditPending = false;
        control.textEdited(textInput.text);
    }

    onFocusedChanged: {
        control.notifyInputMethod(Qt.ImQueryAll);
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

                onActiveFocusChanged: {
                    control.notifyInputMethod(Qt.ImQueryAll);
                }
                onCursorPositionChanged: {
                    control.notifyInputMethod(control.inputMethodSelectionQueryMask());
                }
                onCursorRectangleChanged: {
                    control.notifyInputMethod(control.inputMethodGeometryQueryMask());
                }
                onSelectionEndChanged: {
                    control.notifyInputMethod(control.inputMethodSelectionQueryMask());
                }
                onSelectionStartChanged: {
                    control.notifyInputMethod(control.inputMethodSelectionQueryMask());
                }
                onTextChanged: {
                    control.scheduleCommittedTextEditedDispatch();
                    control.notifyInputMethod(control.inputMethodSelectionQueryMask());
                }

                TapHandler {
                    acceptedDevices: PointerDevice.TouchScreen
                    enabled: control.preferNativeInputHandling
                    gesturePolicy: TapHandler.DragThreshold

                    onDoubleTapped: function (eventPoint, _button) {
                        control.selectWordAtPoint(eventPoint.position);
                    }
                    onTapped: function (eventPoint, _button) {
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

        ignoreUnknownSignals: true
        target: textInput
    }

    Component.onCompleted: {
        setProgrammaticText(text);
        control.notifyInputMethod(Qt.ImQueryAll);
    }
}
