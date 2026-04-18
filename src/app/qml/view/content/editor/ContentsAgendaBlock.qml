pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: agendaBlock

    required property var blockData
    property var shortcutKeyPressHandler: null
    property bool paperPaletteEnabled: false

    signal activated()
    signal boundaryNavigationRequested(string axis, string side)
    signal taskDoneToggled(int openTagStart, int openTagEnd, bool checked)
    signal taskEnterRequested(var blockData, var taskData)
    signal taskTextChanged(var taskData, string text, int cursorPosition)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property int currentLogicalLineNumber: agendaBlock.currentFocusedTaskLineNumber()
    readonly property bool focused: agendaBlock.activeFocus || agendaBlock.hasFocusedTaskRow()
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

    function currentCursorRowRect() {
        for (let index = 0; index < taskRepeater.count; ++index) {
            const item = taskRepeater.itemAt(index)
            if (!item || item.focused === undefined || !item.focused || item.currentCursorRowRect === undefined)
                continue
            return item.currentCursorRowRect()
        }
        return ({
                    "contentHeight": Math.max(1, Math.round(LV.Theme.scaleMetric(12))),
                    "contentY": 0
                })
    }

    function currentVisiblePlainText() {
        const lines = []
        for (let index = 0; index < taskRepeater.count; ++index) {
            const item = taskRepeater.itemAt(index)
            if (item && item.currentEditorPlainText !== undefined) {
                lines.push(String(item.currentEditorPlainText() || ""))
                continue
            }
            const taskData = agendaBlock.tasks[index] && typeof agendaBlock.tasks[index] === "object"
                    ? agendaBlock.tasks[index]
                    : ({})
            lines.push(StructuredCursorSupport.normalizedPlainText(String(taskData.text || "")))
        }
        if (lines.length === 0)
            lines.push("")
        return lines.join("\n")
    }

    function visiblePlainText() {
        return agendaBlock.currentVisiblePlainText()
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
        return Math.max(0, normalizedLineText.length)
    }

    function focusLastTask() {
        return agendaBlock.focusTaskBoundary(taskRepeater.count - 1, "after")
    }

    function focusTaskBoundary(taskIndex, side) {
        const numericTaskIndex = Number(taskIndex)
        const resolvedTaskIndex = isFinite(numericTaskIndex) ? Math.floor(numericTaskIndex) : -1
        const item = taskRepeater.itemAt(resolvedTaskIndex)
        if (!item || item.focusEditor === undefined)
            return false
        const normalizedSide = side === undefined || side === null ? "" : String(side).trim().toLowerCase()
        const plainText = item.currentEditorPlainText !== undefined
                ? String(item.currentEditorPlainText() || "")
                : String(item.taskText || "")
        item.focusEditor(normalizedSide === "after" ? plainText.length : 0)
        return true
    }

    function focusBoundary(side) {
        const normalizedSide = side === undefined || side === null ? "" : String(side).trim().toLowerCase()
        if (normalizedSide === "after")
            return agendaBlock.focusLastTask()
        return agendaBlock.focusFirstTask()
    }

    function focusFirstTask() {
        for (let index = 0; index < taskRepeater.count; ++index) {
            const item = taskRepeater.itemAt(index)
            if (!item || item.focusEditor === undefined)
                continue
            item.focusEditor(0)
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
        const entryBoundary = safeRequest.entryBoundary === undefined || safeRequest.entryBoundary === null
                ? ""
                : String(safeRequest.entryBoundary).trim().toLowerCase()
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
        if (entryBoundary === "before" || sourceOffset <= agendaBlock.sourceStart)
            return agendaBlock.focusBoundary("before")
        if (entryBoundary === "after" || sourceOffset >= agendaBlock.sourceEnd)
            return agendaBlock.focusBoundary("after")
        if (agendaBlock.focusTaskAtSourceOffset(sourceOffset))
            return true
        return agendaBlock.focusBoundary("before")
    }

    function clearSelection(preserveFocusedEditor) {
        let cleared = false
        for (let index = 0; index < taskRepeater.count; ++index) {
            const item = taskRepeater.itemAt(index)
            const clearSelectionFunction = item && item["clearSelection"] !== undefined
                    ? item["clearSelection"]
                    : null
            if (!clearSelectionFunction)
                continue
            if (clearSelectionFunction(preserveFocusedEditor === true))
                cleared = true
        }
        return cleared
    }

    function shortcutInsertionSourceOffset() {
        // Agenda/callout shortcuts must stay block-scoped here so new wrappers do not nest inside task content.
        return agendaBlock.sourceEnd
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

                        function clearSelection(preserveFocusedEditor) {
                            if (!taskEditor || taskEditor.clearSelection === undefined)
                                return false
                            if (preserveFocusedEditor === true && taskEditor.focused)
                                return false
                            taskEditor.clearSelection()
                            return true
                        }

                        function currentEditorPlainText() {
                            if (!taskEditor)
                                return StructuredCursorSupport.normalizedPlainText(taskRow.taskText)
                            if (taskEditor.currentPlainText !== undefined)
                                return StructuredCursorSupport.normalizedPlainText(taskEditor.currentPlainText())
                            return StructuredCursorSupport.normalizedPlainText(taskRow.taskText)
                        }

                        function currentCursorRowRect() {
                            const editorItem = taskEditor && taskEditor.editorItem ? taskEditor.editorItem : null
                            const cursorPosition = Math.max(
                                        0,
                                        Math.min(
                                            Math.max(0, taskEditor.length || 0),
                                            Number(taskEditor && taskEditor.cursorPosition !== undefined ? taskEditor.cursorPosition : 0) || 0))
                            if (!editorItem || editorItem.positionToRectangle === undefined)
                                return ({
                                            "contentHeight": Math.max(1, Number(taskEditor ? taskEditor.inputContentHeight : 0) || Math.round(LV.Theme.scaleMetric(12))),
                                            "contentY": Math.max(0, Number(taskRow.y) || 0)
                                        })
                            const rect = editorItem.positionToRectangle(cursorPosition)
                            const mappedPoint = editorItem.mapToItem !== undefined
                                    ? editorItem.mapToItem(agendaBlock, 0, Number(rect.y) || 0)
                                    : ({ "x": 0, "y": Math.max(0, Number(taskRow.y) || 0) + (Number(rect.y) || 0) })
                            return {
                                "contentHeight": Math.max(1, Number(rect.height) || Math.round(LV.Theme.scaleMetric(12))),
                                "contentY": Math.max(0, Number(mappedPoint.y) || 0)
                            }
                        }

                        function cursorOnFirstVisualRow() {
                            const rowRect = taskRow.currentCursorRowRect()
                            const localTaskY = Math.max(0, Number(taskRow.y) || 0)
                            return Math.max(0, Number(rowRect.contentY) || 0) <= localTaskY + 1
                        }

                        function cursorOnLastVisualRow() {
                            const rowRect = taskRow.currentCursorRowRect()
                            const rowBottom = Math.max(
                                        0,
                                        (Number(rowRect.contentY) || 0) + (Number(rowRect.contentHeight) || 0))
                            const localTaskY = Math.max(0, Number(taskRow.y) || 0)
                            const contentHeight = Math.max(
                                        1,
                                        Number(taskEditor && taskEditor.inputContentHeight !== undefined
                                               ? taskEditor.inputContentHeight
                                               : 0)
                                        || Math.max(1, rowBottom - localTaskY))
                            const localRowBottom = Math.max(0, rowBottom - localTaskY)
                            return localRowBottom >= contentHeight - 1
                        }

                        function handleBoundaryKeyPress(event) {
                            if (!event)
                                return false
                            const key = Number(event.key)
                            const moveBackward = key === Qt.Key_Left
                            const moveForward = key === Qt.Key_Right
                            const moveUp = key === Qt.Key_Up
                            const moveDown = key === Qt.Key_Down
                            if (!moveBackward && !moveForward && !moveUp && !moveDown)
                                return false
                            const modifiers = Number(event.modifiers) || 0
                            if ((modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier)) !== 0)
                                return false
                            const selectionStart = Math.max(0, Math.floor(Number(taskEditor.selectionStart) || 0))
                            const selectionEnd = Math.max(selectionStart, Math.floor(Number(taskEditor.selectionEnd) || 0))
                            if (selectionEnd > selectionStart)
                                return false
                            if (taskEditor.inputMethodComposing !== undefined && taskEditor.inputMethodComposing)
                                return false
                            const preeditText = taskEditor.preeditText !== undefined && taskEditor.preeditText !== null
                                    ? String(taskEditor.preeditText)
                                    : ""
                            if (preeditText.length > 0)
                                return false
                            if ((modifiers & Qt.ShiftModifier) !== 0)
                                return false

                            const cursorPosition = Math.max(0, Math.floor(Number(taskEditor.cursorPosition) || 0))
                            const plainTextLength = taskRow.currentEditorPlainText().length
                            const taskIndex = Number(taskRow.taskIndex)
                            const previousTaskIndex = Math.max(-1, Math.floor(taskIndex) - 1)
                            const nextTaskIndex = Math.floor(taskIndex) + 1
                            const lastTaskIndex = Math.max(0, taskRepeater.count - 1)

                            if (moveBackward && cursorPosition === 0) {
                                if (previousTaskIndex >= 0) {
                                    agendaBlock.focusTaskBoundary(previousTaskIndex, "after")
                                } else {
                                    agendaBlock.boundaryNavigationRequested("horizontal", "before")
                                }
                                event.accepted = true
                                return true
                            }
                            if (moveForward && cursorPosition === plainTextLength) {
                                if (nextTaskIndex < taskRepeater.count) {
                                    agendaBlock.focusTaskBoundary(nextTaskIndex, "before")
                                } else {
                                    agendaBlock.boundaryNavigationRequested("horizontal", "after")
                                }
                                event.accepted = true
                                return true
                            }
                            if (moveUp && taskRow.cursorOnFirstVisualRow()) {
                                if (previousTaskIndex >= 0) {
                                    agendaBlock.focusTaskBoundary(previousTaskIndex, "after")
                                } else {
                                    agendaBlock.boundaryNavigationRequested("vertical", "before")
                                }
                                event.accepted = true
                                return true
                            }
                            if (moveDown && taskRow.cursorOnLastVisualRow()) {
                                if (nextTaskIndex <= lastTaskIndex) {
                                    agendaBlock.focusTaskBoundary(nextTaskIndex, "before")
                                } else {
                                    agendaBlock.boundaryNavigationRequested("vertical", "after")
                                }
                                event.accepted = true
                                return true
                            }
                            return false
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
                                    if (agendaBlock.shortcutKeyPressHandler
                                            && typeof agendaBlock.shortcutKeyPressHandler === "function") {
                                        const shortcutHandled = !!agendaBlock.shortcutKeyPressHandler(event)
                                        if (shortcutHandled || event.accepted)
                                            return true
                                    }
                                    if (taskRow.handleBoundaryKeyPress(event))
                                        return true
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
                                textColor: agendaBlock.taskTextColor
                                wrapMode: TextEdit.Wrap

                                onFocusedChanged: {
                                    if (focused)
                                        agendaBlock.activated()
                                }
                                onCursorPositionChanged: {
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
