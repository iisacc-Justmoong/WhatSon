pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../../../../models/editor/input" as EditorInputModel

FocusScope {
    id: agendaBlock

    required property var blockData
    property bool nativeTextInputPriority: false
    property bool paperPaletteEnabled: false

    signal activated()
    signal boundaryNavigationRequested(string axis, string side)
    signal cursorInteraction()
    signal taskDoneToggled(int openTagStart, int openTagEnd, bool checked)
    signal taskEnterRequested(var blockData, var taskData)
    signal taskTextChanged(var taskData, string text, int cursorPosition, string expectedPreviousText)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property int currentLogicalLineNumber: agendaBlockController.currentFocusedTaskLineNumber()
    readonly property bool focused: agendaBlock.activeFocus || agendaBlockController.hasFocusedTaskRow()
    readonly property bool inputMethodComposing: agendaBlockController.focusedTaskInputMethodComposing()
    readonly property bool textEditable: true
    readonly property bool atomicBlock: false
    readonly property bool gutterCollapsed: false
    readonly property string minimapVisualKind: "text"
    readonly property int minimapRepresentativeCharCount: 0
    readonly property var tasks: {
        const rawTasks = normalizedBlock.tasks
        if (Array.isArray(rawTasks))
            return rawTasks
        if (rawTasks && rawTasks.length !== undefined) {
            const normalized = []
            for (let index = 0; index < rawTasks.length; ++index)
                normalized.push(rawTasks[index])
            return normalized
        }
        return []
    }
    readonly property string dateText: normalizedBlock.date !== undefined ? String(normalizedBlock.date) : "yyyy-mm-dd"
    readonly property string preeditText: agendaBlockController.focusedTaskPreeditText()
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || 0)
    readonly property color frameColor: paperPaletteEnabled ? "#F7F3EA" : "#262728"
    readonly property color frameBorderColor: paperPaletteEnabled ? "#D2C7B3" : "#343536"
    readonly property color headerTextColor: paperPaletteEnabled ? "#6A5B44" : "#80FFFFFF"
    readonly property color taskBoxColor: paperPaletteEnabled ? "#4E5763" : "#CCFFFFFF"
    readonly property color taskCheckColor: paperPaletteEnabled ? agendaBlock.frameColor : "#262728"
    readonly property color taskTextColor: paperPaletteEnabled ? "#111111" : "#FFFFFF"

    implicitHeight: agendaFrame.implicitHeight
    width: parent ? parent.width : implicitWidth

    function hasFocusedTaskRow() {
        return agendaBlockController.hasFocusedTaskRow()
    }

    function focusedTaskInputMethodComposing() {
        return agendaBlockController.focusedTaskInputMethodComposing()
    }

    function focusedTaskPreeditText() {
        return agendaBlockController.focusedTaskPreeditText()
    }

    function nativeCompositionActive() {
        return agendaBlockController.nativeCompositionActive()
    }

    function currentFocusedTaskLineNumber() {
        return agendaBlockController.currentFocusedTaskLineNumber()
    }

    function currentCursorRowRect() {
        return agendaBlockController.currentCursorRowRect()
    }

    function currentVisiblePlainText() {
        return agendaBlockController.currentVisiblePlainText()
    }

    function visiblePlainText() {
        return agendaBlockController.visiblePlainText()
    }

    function representativeCharCount(lineText) {
        return agendaBlockController.representativeCharCount(lineText)
    }

    function focusLastTask() {
        return agendaBlockController.focusLastTask()
    }

    function focusTaskBoundary(taskIndex, side) {
        return agendaBlockController.focusTaskBoundary(taskIndex, side)
    }

    function focusBoundary(side) {
        return agendaBlockController.focusBoundary(side)
    }

    function focusFirstTask() {
        return agendaBlockController.focusFirstTask()
    }

    function focusTaskAtSourceOffset(sourceOffset) {
        return agendaBlockController.focusTaskAtSourceOffset(sourceOffset)
    }

    function applyFocusRequest(request) {
        return agendaBlockController.applyFocusRequest(request)
    }

    function clearSelection(preserveFocusedEditor) {
        return agendaBlockController.clearSelection(preserveFocusedEditor)
    }

    function shortcutInsertionSourceOffset() {
        return agendaBlockController.shortcutInsertionSourceOffset()
    }

    Rectangle {
        id: agendaFrame

        anchors.left: parent.left
        anchors.right: parent.right
        color: agendaBlock.frameColor
        border.color: agendaBlock.frameBorderColor
        border.width: 1
        implicitHeight: agendaLayout.implicitHeight + Math.round(LV.Theme.scaleMetric(16))
        radius: Math.round(LV.Theme.scaleMetric(12))

        LV.VStack {
            id: agendaLayout

            anchors.fill: parent
            anchors.margins: Math.round(LV.Theme.scaleMetric(8))
            spacing: Math.round(LV.Theme.scaleMetric(8))

            RowLayout {
                Layout.fillWidth: true

                LV.Label {
                    Layout.fillWidth: true
                    color: agendaBlock.headerTextColor
                    font.pixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(11)))
                    font.weight: Font.Normal
                    style: caption
                    text: "Agenda"
                }
                LV.Label {
                    color: agendaBlock.headerTextColor
                    font.pixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(11)))
                    font.weight: Font.Normal
                    style: caption
                    text: agendaBlock.dateText
                }
            }

            LV.VStack {
                Layout.fillWidth: true
                Layout.leftMargin: Math.round(LV.Theme.scaleMetric(8))
                Layout.rightMargin: Math.round(LV.Theme.scaleMetric(8))
                spacing: Math.round(LV.Theme.scaleMetric(4))

                Repeater {
                    id: taskRepeater

                    model: agendaBlock.tasks

                    delegate: FocusScope {
                        id: taskRow

                        required property int index
                        required property var modelData
                        readonly property var taskData: modelData && typeof modelData === "object" ? modelData : ({})
                        readonly property int taskIndex: index
                        readonly property int taskOpenTagStart: Math.floor(Number(taskData.openTagStart) || -1)
                        readonly property int taskOpenTagEnd: Math.floor(Number(taskData.openTagEnd) || -1)
                        readonly property int taskContentStart: Math.floor(Number(taskData.contentStart) || 0)
                        readonly property int taskContentEnd: Math.max(taskContentStart, Math.floor(Number(taskData.contentEnd) || taskContentStart))
                        readonly property bool focused: taskEditor.focused
                                                         || (taskToggle.activeFocus !== undefined && taskToggle.activeFocus)
                        readonly property bool inputMethodComposing: taskEditor.inputMethodComposing !== undefined
                                                                     && taskEditor.inputMethodComposing
                        readonly property string preeditText: taskEditor.preeditText === undefined || taskEditor.preeditText === null
                                                              ? ""
                                                              : String(taskEditor.preeditText)
                        property string taskText: taskData.text !== undefined ? String(taskData.text) : ""

                        Layout.fillWidth: true
                        implicitHeight: taskEditor.implicitHeight
                        width: parent ? parent.width : implicitWidth

                        function focusEditor(cursorPosition) {
                            taskRowController.focusEditor(cursorPosition)
                        }

                        function clearSelection(preserveFocusedEditor) {
                            return taskRowController.clearSelection(preserveFocusedEditor)
                        }

                        function currentEditorPlainText() {
                            return taskRowController.currentEditorPlainText()
                        }

                        function syncLiveTaskTextFromHost() {
                            taskRowController.syncLiveTaskTextFromHost()
                        }

                        function currentCursorRowRect() {
                            return taskRowController.currentCursorRowRect()
                        }

                        function cursorOnFirstVisualRow() {
                            return taskRowController.cursorOnFirstVisualRow()
                        }

                        function cursorOnLastVisualRow() {
                            return taskRowController.cursorOnLastVisualRow()
                        }

                        RowLayout {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            spacing: Math.round(LV.Theme.scaleMetric(6))

                            LV.CheckBox {
                                id: taskToggle

                                Layout.alignment: Qt.AlignTop
                                boxBorderColorCheckedEnabled: agendaBlock.taskBoxColor
                                boxBorderColorCheckedDisabled: agendaBlock.taskBoxColor
                                boxBorderColorUncheckedEnabled: agendaBlock.taskBoxColor
                                boxBorderColorUncheckedDisabled: agendaBlock.taskBoxColor
                                boxBorderWidthCheckedEnabled: 0.5
                                boxBorderWidthCheckedDisabled: 0.5
                                boxSize: Math.round(LV.Theme.scaleMetric(17))
                                checkColor: agendaBlock.taskCheckColor
                                checked: !!taskRow.taskData.done
                                checkedColor: agendaBlock.taskBoxColor
                                disabledCheckedColor: agendaBlock.taskBoxColor
                                disabledUncheckedColor: agendaBlock.taskBoxColor
                                text: ""
                                uncheckedColor: agendaBlock.taskBoxColor

                                onToggled: {
                                    taskRowController.handleToggleChanged(checked)
                                }
                            }

                            ContentsInlineFormatEditor {
                                id: taskEditor

                                Layout.fillWidth: true
                                backgroundColor: "transparent"
                                backgroundColorDisabled: "transparent"
                                backgroundColorFocused: "transparent"
                                backgroundColorHover: "transparent"
                                backgroundColorPressed: "transparent"
                                centeredTextHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
                                cornerRadius: 0
                                fieldMinHeight: Math.max(Math.round(LV.Theme.scaleMetric(20)), inputContentHeight)
                                fontFamily: LV.Theme.fontBody
                                fontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
                                fontWeight: Font.Medium
                                insetHorizontal: 0
                                insetVertical: 0
                                inputMethodHints: Qt.ImhNone
                                mouseSelectionMode: TextEdit.SelectCharacters
                                overwriteMode: false
                                persistentSelection: true
                                placeholderText: ""
                                selectByKeyboard: true
                                selectByMouse: true
                                selectedTextColor: LV.Theme.textPrimary
                                selectionColor: LV.Theme.accent
                                preferNativeInputHandling: true
                                showRenderedOutput: false
                                showScrollBar: false
                                text: taskRow.taskText
                                textColor: agendaBlock.taskTextColor
                                wrapMode: TextEdit.Wrap

                            }

                            EditorInputModel.ContentsAgendaTaskRowController {
                                id: taskRowController

                                agendaBlock: agendaBlock
                                taskEditor: taskEditor
                                taskRow: taskRow
                            }
                        }
                    }
                }
            }
        }
    }

    EditorInputModel.ContentsAgendaBlockController {
        id: agendaBlockController

        agendaBlock: agendaBlock
        taskRepeater: taskRepeater
    }
}
