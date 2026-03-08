pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: contentsView

    readonly property color activeLineNumberColor: "#9DA0A8"
    readonly property int currentCursorLineNumber: {
        const value = contentsView.editorText || "";
        const cursor = Math.max(0, Math.min(value.length, Number(contentEditor.cursorPosition) || 0));
        return value.slice(0, cursor).split(/\r\n|\n/).length;
    }
    readonly property color decorativeMarkerGreen: "#0AFF60"
    readonly property var decorativeMarkerSpecs: [
        {
            "color": contentsView.decorativeMarkerYellow,
            "lineSpan": 3,
            "startLine": 2
        },
        {
            "color": contentsView.decorativeMarkerGreen,
            "lineSpan": 8,
            "startLine": 5
        }
    ]
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
    readonly property real editorLineHeight: {
        const logicalLineCount = Math.max(1, Number(contentEditor.lineCount) || 1);
        const renderedContentHeight = Number(contentEditor.contentHeight) || 0;
        return renderedContentHeight > 0 ? renderedContentHeight / logicalLineCount : contentsView.editorTextLineBoxHeight;
    }
    property string editorText: ""
    readonly property int editorTextLineBoxHeight: 12
    readonly property int editorVerticalInset: 16
    readonly property int frameHorizontalInset: 2
    readonly property color gutterColor: LV.Theme.panelBackground04
    readonly property int gutterCommentMarkerOffset: 2
    readonly property int gutterCommentRailLeft: 4
    readonly property int gutterCommentRailWidth: 10
    readonly property int gutterIconRailLeft: 40
    readonly property int gutterIconRailWidth: 18
    readonly property int gutterWidth: 74
    readonly property bool hasSelectedNote: contentsView.selectedNoteId.length > 0
    readonly property color lineNumberColor: "#4E5157"
    readonly property int lineNumberColumnLeft: 14
    readonly property int lineNumberColumnTextWidth: 22
    readonly property int lineNumberColumnWidth: 26
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    readonly property int minEditorHeight: LV.Theme.gap20 * 12
    property var noteListModel: null
    property color panelColor: LV.Theme.panelBackground07
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ContentViewLayout") : null
    readonly property string selectedNoteBodyText: noteListModel && noteListModel.currentBodyText !== undefined ? String(noteListModel.currentBodyText) : ""
    readonly property string selectedNoteId: noteListModel && noteListModel.currentNoteId !== undefined ? String(noteListModel.currentNoteId) : ""
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    readonly property real textOriginY: {
        if (!contentEditor.editorItem)
            return contentsView.editorVerticalInset;
        return (Number(contentEditor.editorItem.y) || contentsView.editorVerticalInset) + contentsView.editorContentOffsetY;
    }

    signal drawerHeightDragRequested(int value)
    signal editorTextEdited(string text)
    signal viewHookRequested

    function clampDrawerHeight(value) {
        var maxDrawer = Math.max(contentsView.minDrawerHeight, contentsView.height - contentsView.minDisplayHeight - contentsView.splitterThickness);
        return Math.max(contentsView.minDrawerHeight, Math.min(maxDrawer, value));
    }
    function markerY(markerSpec) {
        if (!markerSpec)
            return contentsView.textOriginY;
        const startLine = Math.max(1, Number(markerSpec.startLine) || 1);
        return contentsView.textOriginY + (startLine - 1) * contentsView.editorLineHeight;
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function visibleLineNumbers() {
        const logicalLineCount = Math.max(1, Number(contentEditor.lineCount) || 1);
        const lineHeight = Math.max(1, contentsView.editorLineHeight);
        const viewportTop = Math.max(0, -contentsView.textOriginY);
        const viewportLineCount = Math.max(1, Math.ceil(lineNumberViewport.height / lineHeight) + 2);
        const startLine = Math.max(1, Math.floor(viewportTop / lineHeight) + 1);
        const endLine = Math.min(logicalLineCount, startLine + viewportLineCount);
        const visibleLines = [];
        for (let lineNumber = startLine; lineNumber <= endLine; ++lineNumber)
            visibleLines.push(lineNumber);

    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true

    Component.onCompleted: {
        contentsView.editorText = contentsView.selectedNoteBodyText;
    }
    onSelectedNoteBodyTextChanged: {
        if (contentsView.editorText !== contentsView.selectedNoteBodyText)
            contentsView.editorText = contentsView.selectedNoteBodyText;
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
                            model: contentsView.decorativeMarkerSpecs

                            delegate: Rectangle {
                                required property var modelData

                                color: modelData.color
                                height: Math.max(20, (Number(modelData.lineSpan) || 1) * contentsView.editorLineHeight)
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
                                y: contentsView.textOriginY + (modelData - 1) * contentsView.editorLineHeight
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
                        wrapMode: TextEdit.NoWrap

                        onTextEdited: function (text) {
                            if (contentsView.editorText !== text)
                                contentsView.editorText = text;
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
