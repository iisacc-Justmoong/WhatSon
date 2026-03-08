pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: contentsView

    readonly property color activeLineNumberColor: "#9DA0A8"
    property var contentViewModel: null
    readonly property int currentCursorLineNumber: {
        const value = contentsView.editorText || "";
        const cursor = Math.max(0, Math.min(value.length, Number(contentEditor.cursorPosition) || 0));
        return value.slice(0, cursor).split(/\r\n|\n/).length;
    }
    readonly property color decorativeMarkerYellow: "#FFF567"
    property color displayColor: LV.Theme.panelBackground09
    property color drawerColor: LV.Theme.panelBackground11
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    readonly property real editorContentOffsetY: {
        if (!contentEditor.editorItem || !contentEditor.editorItem.parent)
            return 0;
        return Number(contentEditor.editorItem.parent.y) || 0;
    }
    readonly property int editorHorizontalInset: 16
    readonly property real editorLineHeight: contentsView.editorTextLineBoxHeight
    property string editorText: ""
    readonly property int editorTextLineBoxHeight: 12
    readonly property int editorVerticalInset: 16
    readonly property var effectiveGutterMarkers: {
        const normalizedMarkers = [];
        if (contentsView.showCurrentLineMarker) {
            normalizedMarkers.push({
                "color": contentsView.gutterMarkerCurrentColor,
                "lineSpan": 1,
                "startLine": contentsView.currentCursorLineNumber,
                "type": "current"
            });
        }
        const externalMarkers = Array.isArray(contentsView.gutterMarkers) ? contentsView.gutterMarkers : [];
        for (let i = 0; i < externalMarkers.length; ++i) {
            const normalizedMarker = contentsView.normalizeGutterMarker(externalMarkers[i]);
            if (normalizedMarker)
                normalizedMarkers.push(normalizedMarker);
        }
        return normalizedMarkers;
    }
    readonly property int frameHorizontalInset: 2
    readonly property color gutterColor: LV.Theme.panelBackground04
    readonly property int gutterCommentMarkerOffset: 2
    readonly property int gutterCommentRailLeft: 4
    readonly property int gutterCommentRailWidth: 10
    readonly property int gutterIconRailLeft: 40
    readonly property int gutterIconRailWidth: 18
    readonly property color gutterMarkerChangedColor: contentsView.decorativeMarkerYellow
    readonly property color gutterMarkerConflictColor: LV.Theme.danger
    readonly property color gutterMarkerCurrentColor: LV.Theme.primary
    property var gutterMarkers: []
    readonly property int gutterWidth: 74
    readonly property bool hasSelectedNote: contentsView.selectedNoteId.length > 0
    readonly property color lineNumberColor: "#4E5157"
    readonly property int lineNumberColumnLeft: 14
    readonly property int lineNumberColumnTextWidth: contentsView.gutterWidth - contentsView.lineNumberColumnLeft - contentsView.lineNumberRightInset
    readonly property int lineNumberColumnWidth: 26
    readonly property int lineNumberRightInset: contentsView.editorHorizontalInset
    readonly property int logicalLineCount: Math.max(1, contentsView.logicalLineStartOffsets.length)
    readonly property var logicalLineStartOffsets: contentsView.buildLogicalLineStartOffsets(contentsView.editorText)
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    readonly property int minEditorHeight: LV.Theme.gap20 * 12
    property var noteListModel: null
    property color panelColor: LV.Theme.panelBackground07
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ContentViewLayout") : null
    readonly property string selectedNoteBodyText: noteListModel && noteListModel.currentBodyText !== undefined ? String(noteListModel.currentBodyText) : ""
    readonly property string selectedNoteId: noteListModel && noteListModel.currentNoteId !== undefined ? String(noteListModel.currentNoteId) : ""
    readonly property bool showCurrentLineMarker: contentsView.hasSelectedNote || contentsView.editorText.length > 0 || contentEditor.focused
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property bool syncingEditorTextFromModel: false
    readonly property real textOriginY: {
        if (!contentEditor.editorItem)
            return contentsView.editorVerticalInset;
        return (Number(contentEditor.editorItem.y) || contentsView.editorVerticalInset) + contentsView.editorContentOffsetY;
    }

    signal drawerHeightDragRequested(int value)
    signal editorTextEdited(string text)
    signal viewHookRequested

    function buildLogicalLineStartOffsets(text) {
        const value = text === undefined || text === null ? "" : String(text);
        const offsets = [0];
        for (let index = 0; index < value.length; ++index) {
            if (value.charAt(index) === "\n")
                offsets.push(index + 1);
        }

    }
    function clampDrawerHeight(value) {
        var maxDrawer = Math.max(contentsView.minDrawerHeight, contentsView.height - contentsView.minDisplayHeight - contentsView.splitterThickness);
        return Math.max(contentsView.minDrawerHeight, Math.min(maxDrawer, value));
    }
    function editorViewportYForDocumentY(documentY) {
        const editorY = contentEditor.editorItem ? Number(contentEditor.editorItem.y) || contentsView.editorVerticalInset : contentsView.editorVerticalInset;
        return editorY + documentY + contentsView.editorContentOffsetY;
    }
    function lineDocumentY(lineNumber) {
        const safeLineNumber = Math.max(1, Math.min(contentsView.logicalLineCount, Number(lineNumber) || 1));
        if (!contentEditor.editorItem || contentEditor.editorItem.positionToRectangle === undefined)
            return (safeLineNumber - 1) * contentsView.editorLineHeight;
        const offset = contentsView.logicalLineStartOffsets[safeLineNumber - 1];
        const rect = contentEditor.editorItem.positionToRectangle(offset);
        return Number(rect.y) || 0;
    }
    function lineVisualHeight(startLine, lineSpan) {
        const safeStartLine = Math.max(1, Math.min(contentsView.logicalLineCount, Number(startLine) || 1));
        const safeLineSpan = Math.max(1, Number(lineSpan) || 1);
        const startDocumentY = contentsView.lineDocumentY(safeStartLine);
        const nextLineNumber = safeStartLine + safeLineSpan;
        let endDocumentY = 0;
        if (nextLineNumber <= contentsView.logicalLineCount) {
            endDocumentY = contentsView.lineDocumentY(nextLineNumber);
        } else {
            const contentHeight = Number(contentEditor.contentHeight) || 0;
            endDocumentY = contentHeight > 0 ? contentHeight : startDocumentY + safeLineSpan * contentsView.editorLineHeight;
        }
        return Math.max(contentsView.editorLineHeight, endDocumentY - startDocumentY);
    }
    function lineY(lineNumber) {
        return contentsView.editorViewportYForDocumentY(contentsView.lineDocumentY(lineNumber));
    }
    function markerColorForType(markerType) {
        const normalizedType = markerType === undefined || markerType === null ? "" : String(markerType).toLowerCase();
        if (normalizedType === "conflict")
            return contentsView.gutterMarkerConflictColor;
        if (normalizedType === "changed")


    }
    function markerY(markerSpec) {
        if (!markerSpec)
            return contentsView.editorVerticalInset;
        const startLine = Math.max(1, Number(markerSpec.startLine) || 1);
        return contentsView.lineY(startLine);
    }
    function normalizeGutterMarker(markerSpec) {
        if (!markerSpec)
            return null;
        const rawStartLine = markerSpec.startLine !== undefined ? markerSpec.startLine : markerSpec.line;
        const startLine = Math.max(1, Number(rawStartLine) || 1);
        const hasExplicitSpan = markerSpec.lineSpan !== undefined && markerSpec.lineSpan !== null;
        const explicitSpan = hasExplicitSpan ? Math.max(1, Number(markerSpec.lineSpan) || 1) : 0;
        const endLine = Math.max(startLine, Number(markerSpec.endLine) || startLine);
        const lineSpan = explicitSpan > 0 ? explicitSpan : Math.max(1, endLine - startLine + 1);
        const markerType = markerSpec.type !== undefined ? String(markerSpec.type).toLowerCase() : "";
        if (markerType !== "changed" && markerType !== "conflict" && markerType !== "current")
            return null;
        return {
            "color": contentsView.markerColorForType(markerType),
            "lineSpan": lineSpan,
            "startLine": startLine,
            "type": markerType
        };
    }
    function persistEditorText(text) {
        if (!contentViewModel || contentViewModel.saveCurrentBodyText === undefined)
            return false;
        return Boolean(contentViewModel.saveCurrentBodyText(text));
    }
    function releaseEditorSyncGuard() {
        Qt.callLater(function () {
            contentsView.syncingEditorTextFromModel = false;
        });
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function syncEditorTextFromSelection(text) {
        const nextText = text === undefined || text === null ? "" : String(text);
        contentsView.syncingEditorTextFromModel = true;
        if (contentsView.editorText !== nextText)
            contentsView.editorText = nextText;
        contentsView.releaseEditorSyncGuard();
    }
    function visibleLineNumbers() {
        const visibleLines = [];
        for (let lineNumber = 1; lineNumber <= contentsView.logicalLineCount; ++lineNumber) {
            const lineY = contentsView.lineY(lineNumber);
            if (lineY > lineNumberViewport.height)
                break;
            if (lineY + contentsView.editorLineHeight < 0)
                continue;
            visibleLines.push(lineNumber);
        }

    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true

    Component.onCompleted: {
        contentsView.syncEditorTextFromSelection(contentsView.selectedNoteBodyText);
    }
    onSelectedNoteBodyTextChanged: {
        contentsView.syncEditorTextFromSelection(contentsView.selectedNoteBodyText);
    }

    Rectangle {
        anchors.fill: parent
        color: contentsView.panelColor
    }
    LV.VStack {
        id: drawerView

        anchors.fill: parent
        spacing: LV.Theme.gapNone

        Rectangle {
            id: contentsDisplayView

            Layout.fillHeight: true
            Layout.fillWidth: true
            color: contentsView.displayColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: contentsView.frameHorizontalInset
                anchors.rightMargin: contentsView.frameHorizontalInset
                spacing: 0

                Rectangle {
                    id: lineNumberGutter

                    Layout.fillHeight: true
                    Layout.preferredWidth: contentsView.gutterWidth
                    color: contentsView.gutterColor

                    Item {
                        id: lineNumberViewport

                        anchors.fill: parent
                        clip: true

                        Item {
                            height: parent.height
                            width: contentsView.gutterIconRailWidth
                            x: contentsView.gutterIconRailLeft
                        }
                        Repeater {
                            model: contentsView.effectiveGutterMarkers

                            delegate: Rectangle {
                                required property var modelData

                                color: modelData.color
                                height: Math.max(20, contentsView.lineVisualHeight(modelData.startLine, modelData.lineSpan))
                                radius: width / 2
                                width: 4
                                x: contentsView.gutterCommentRailLeft + contentsView.gutterCommentMarkerOffset
                                y: contentsView.markerY(modelData)
                            }
                        }
                        Repeater {
                            model: contentsView.visibleLineNumbers()

                            delegate: LV.Label {
                                required property int modelData

                                color: modelData === contentsView.currentCursorLineNumber ? contentsView.activeLineNumberColor : contentsView.lineNumberColor
                                font.family: LV.Theme.fontBody
                                font.letterSpacing: 0
                                font.pixelSize: 11
                                font.weight: modelData === contentsView.currentCursorLineNumber ? Font.Medium : Font.Normal
                                height: contentsView.editorLineHeight
                                horizontalAlignment: Text.AlignRight
                                style: caption
                                text: String(modelData)
                                verticalAlignment: Text.AlignVCenter
                                width: contentsView.lineNumberColumnTextWidth
                                x: contentsView.lineNumberColumnLeft
                                y: contentsView.lineY(modelData)
                            }
                        }
                    }
                }
                Item {
                    id: editorViewport

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumHeight: contentsView.minEditorHeight

                    LV.TextEditor {
                        id: contentEditor

                        anchors.fill: parent
                        autoFocusOnPress: true
                        backgroundColor: contentsView.displayColor
                        backgroundColorDisabled: contentsView.displayColor
                        backgroundColorFocused: contentsView.displayColor
                        backgroundColorHover: contentsView.displayColor
                        backgroundColorPressed: contentsView.displayColor
                        centeredTextHeight: contentsView.editorTextLineBoxHeight
                        cornerRadius: 0
                        editorHeight: editorViewport.height
                        enforceModeDefaults: false
                        fieldMinHeight: contentsView.minEditorHeight
                        fontFamily: LV.Theme.fontBody
                        fontLetterSpacing: 0
                        fontPixelSize: 12
                        fontWeight: Font.Medium
                        insetHorizontal: contentsView.editorHorizontalInset
                        insetVertical: contentsView.editorVerticalInset
                        placeholderText: ""
                        selectByMouse: true
                        selectedTextColor: LV.Theme.textPrimary
                        selectionColor: LV.Theme.accentBlueMuted
                        shapeStyle: shapeRoundRect
                        showRenderedOutput: false
                        showScrollBar: false
                        text: contentsView.editorText
                        textColor: LV.Theme.textSecondary
                        textFormat: TextEdit.PlainText
                        wrapMode: TextEdit.Wrap

                        onTextEdited: function (text) {
                            if (contentsView.editorText !== text)
                                contentsView.editorText = text;
                            if (contentsView.syncingEditorTextFromModel)
                                return;
                            contentsView.persistEditorText(text);
                            contentsView.editorTextEdited(text);
                        }
                    }
                    Binding {
                        "y"
                        target: contentEditor.editorItem
                        value: contentsView.editorVerticalInset
                    }
                    LV.Label {
                        anchors.left: parent.left
                        anchors.leftMargin: contentsView.editorHorizontalInset
                        anchors.top: parent.top
                        anchors.topMargin: contentsView.editorVerticalInset
                        color: LV.Theme.textTertiary
                        style: body
                        text: contentsView.hasSelectedNote ? "Start typing here" : "Select a note to view its body text"
                        visible: contentEditor.empty
                    }
                }
            }
        }
        Rectangle {
            id: drawerSplitter

            Layout.fillWidth: true
            Layout.preferredHeight: contentsView.splitterThickness
            color: contentsView.splitterColor

            MouseArea {
                id: drawerSplitterMouse

                property int dragStartDrawerHeight: contentsView.drawerHeight
                property real dragStartGlobalY: 0

                acceptedButtons: Qt.LeftButton
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                cursorShape: Qt.SizeVerCursor
                height: contentsView.splitterHandleThickness
                preventStealing: true

                onPositionChanged: function (mouse) {
                    if (!(pressedButtons & Qt.LeftButton))
                        return;
                    var movePoint = drawerSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    var deltaY = movePoint.y - dragStartGlobalY;
                    var nextDrawerHeight = contentsView.clampDrawerHeight(dragStartDrawerHeight - deltaY);
                    if (nextDrawerHeight !== contentsView.drawerHeight)
                        contentsView.drawerHeightDragRequested(nextDrawerHeight);
                }
                onPressed: function (mouse) {
                    var pressPoint = drawerSplitterMouse.mapToGlobal(Qt.point(mouse.x, mouse.y));
                    dragStartGlobalY = pressPoint.y;
                    dragStartDrawerHeight = contentsView.drawerHeight;
                }
            }
        }
        Rectangle {
            id: drawer

            Layout.fillWidth: true
            Layout.preferredHeight: contentsView.clampDrawerHeight(contentsView.drawerHeight)
            color: contentsView.drawerColor
        }
    }
}
