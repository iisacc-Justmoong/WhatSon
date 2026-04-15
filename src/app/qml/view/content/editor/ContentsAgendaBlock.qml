pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: agendaBlock

    required property var blockData

    signal activated()
    signal taskDoneToggled(int openTagStart, int openTagEnd, bool checked)
    signal taskEnterRequested(var blockData, var taskData)
    signal taskTextChanged(var taskData, string text, int cursorPosition)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property int currentLogicalLineNumber: agendaBlock.currentFocusedTaskLineNumber()
    readonly property bool focused: agendaBlock.activeFocus || agendaBlock.hasFocusedTaskRow()
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
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || 0)

    implicitHeight: agendaFrame.implicitHeight
    width: parent ? parent.width : implicitWidth

    function hasFocusedTaskRow() {
        for (let index = 0; index < taskRepeater.count; ++index) {
            const item = taskRepeater.itemAt(index)
            if (item && item.focused !== undefined && item.focused)
                return true
        }
        return false
    }

    function currentFocusedTaskLineNumber() {
        for (let index = 0; index < taskRepeater.count; ++index) {
            const item = taskRepeater.itemAt(index)
            if (item && item.focused !== undefined && item.focused)
                return index + 1
        }
        return 1
    }

    function focusFirstTask() {
        for (let index = 0; index < taskRepeater.count; ++index) {
            const item = taskRepeater.itemAt(index)
            if (!item || item.focusEditor === undefined)
                continue
            item.focusEditor()
            return true
        }
        return false
    }

    function focusTaskAtSourceOffset(sourceOffset) {
        const numericSourceOffset = Number(sourceOffset)
        if (!isFinite(numericSourceOffset))
            return false
        for (let index = 0; index < taskRepeater.count; ++index) {
            const item = taskRepeater.itemAt(index)
            if (!item || item.taskContentStart === undefined || item.taskContentEnd === undefined)
                continue
            const contentStart = Math.max(0, Math.floor(Number(item.taskContentStart) || 0))
            const contentEnd = Math.max(contentStart, Math.floor(Number(item.taskContentEnd) || 0))
            if (numericSourceOffset < contentStart || numericSourceOffset > contentEnd)
                continue
            item.focusEditor(StructuredCursorSupport.plainCursorForSourceOffset(item.taskText, numericSourceOffset - contentStart))
            return true
        }
        return false
    }

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const taskOpenTagStart = Number(safeRequest.taskOpenTagStart)
        const localCursorPosition = Number(safeRequest.localCursorPosition)
        if (isFinite(taskOpenTagStart)) {
            for (let index = 0; index < taskRepeater.count; ++index) {
                const item = taskRepeater.itemAt(index)
                if (!item || item.taskOpenTagStart === undefined)
                    continue
                if (Number(item.taskOpenTagStart) !== Math.floor(taskOpenTagStart))
                    continue
                item.focusEditor(localCursorPosition)
                return true
            }
        }

        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < agendaBlock.sourceStart || sourceOffset > agendaBlock.sourceEnd)
            return false
        if (agendaBlock.focusTaskAtSourceOffset(sourceOffset))
            return true
        return agendaBlock.focusFirstTask()
    }

    function shortcutInsertionSourceOffset() {
        // Agenda/callout shortcuts must stay block-scoped here so new wrappers do not nest inside task content.
        return agendaBlock.sourceEnd
    }

    Rectangle {
        id: agendaFrame

        anchors.left: parent.left
        anchors.right: parent.right
        color: "#262728"
        border.color: "#343536"
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
                    color: "#80FFFFFF"
                    font.pixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(11)))
                    font.weight: Font.Normal
                    style: caption
                    text: "Agenda"
                }
                LV.Label {
                    color: "#80FFFFFF"
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

                        required property var modelData
                        readonly property var taskData: modelData && typeof modelData === "object" ? modelData : ({})
                        readonly property int taskOpenTagStart: Math.floor(Number(taskData.openTagStart) || -1)
                        readonly property int taskOpenTagEnd: Math.floor(Number(taskData.openTagEnd) || -1)
                        readonly property int taskContentStart: Math.floor(Number(taskData.contentStart) || 0)
                        readonly property int taskContentEnd: Math.max(taskContentStart, Math.floor(Number(taskData.contentEnd) || taskContentStart))
                        readonly property bool focused: taskEditor.focused
                                                         || (taskToggle.activeFocus !== undefined && taskToggle.activeFocus)
                        readonly property string taskText: taskData.text !== undefined ? String(taskData.text) : ""

                        Layout.fillWidth: true
                        implicitHeight: taskEditor.implicitHeight
                        width: parent ? parent.width : implicitWidth

                        function focusEditor(cursorPosition) {
                            taskEditor.forceActiveFocus()
                            const numericCursorPosition = Number(cursorPosition)
                            const targetCursorPosition = isFinite(numericCursorPosition)
                                                       ? Math.max(0, Math.min(Math.floor(numericCursorPosition), Math.max(0, taskEditor.length || 0)))
                                                       : Math.max(0, taskEditor.length || 0)
                            if (taskEditor.setCursorPositionPreservingInputMethod !== undefined)
                                taskEditor.setCursorPositionPreservingInputMethod(targetCursorPosition)
                            else if (taskEditor.cursorPosition !== undefined)
                                taskEditor.cursorPosition = targetCursorPosition
                            agendaBlock.activated()
                        }

                        RowLayout {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            spacing: Math.round(LV.Theme.scaleMetric(6))

                            LV.CheckBox {
                                id: taskToggle

                                Layout.alignment: Qt.AlignTop
                                boxBorderColorCheckedEnabled: "#CCFFFFFF"
                                boxBorderColorCheckedDisabled: "#CCFFFFFF"
                                boxBorderColorUncheckedEnabled: "#CCFFFFFF"
                                boxBorderColorUncheckedDisabled: "#CCFFFFFF"
                                boxBorderWidthCheckedEnabled: 0.5
                                boxBorderWidthCheckedDisabled: 0.5
                                boxSize: Math.round(LV.Theme.scaleMetric(17))
                                checkColor: "#262728"
                                checked: !!taskRow.taskData.done
                                checkedColor: "#CCFFFFFF"
                                disabledCheckedColor: "#CCFFFFFF"
                                disabledUncheckedColor: "#CCFFFFFF"
                                text: ""
                                uncheckedColor: "#CCFFFFFF"

                                onToggled: {
                                    if (checked === !!taskRow.taskData.done)
                                        return
                                    agendaBlock.taskDoneToggled(taskRow.taskOpenTagStart, taskRow.taskOpenTagEnd, checked)
                                    agendaBlock.activated()
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
                                placeholderText: ""
                                selectByMouse: true
                                selectedTextColor: LV.Theme.textPrimary
                                selectionColor: LV.Theme.accent
                                shortcutKeyPressHandler: function (event) {
                                    const noModifiers = (event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier | Qt.ShiftModifier)) === 0
                                    if (!noModifiers)
                                        return false
                                    if (event.key !== Qt.Key_Return && event.key !== Qt.Key_Enter)
                                        return false
                                    event.accepted = true
                                    agendaBlock.taskEnterRequested(agendaBlock.blockData, taskRow.taskData)
                                    return true
                                }
                                showRenderedOutput: false
                                showScrollBar: false
                                text: taskRow.taskText
                                textColor: "#FFFFFF"
                                textFormat: TextEdit.PlainText
                                wrapMode: TextEdit.Wrap

                                onFocusedChanged: {
                                    if (focused)
                                        agendaBlock.activated()
                                }
                                onTextEdited: function (text) {
                                    agendaBlock.taskTextChanged(
                                                taskRow.taskData,
                                                String(text || ""),
                                                Math.max(0, Number(taskEditor.cursorPosition) || 0))
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
