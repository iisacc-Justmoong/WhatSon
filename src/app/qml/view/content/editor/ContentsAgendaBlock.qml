pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "../../../../models/editor/structure/ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: agendaBlock

    required property var blockData
    property bool nativeTextInputPriority: false
    property bool paperPaletteEnabled: false
    property var tagManagementShortcutKeyPressHandler: null
    readonly property int editorMouseSelectionMode: Qt.platform.os === "ios"
                                                    ? TextEdit.SelectWords
                                                    : TextEdit.SelectCharacters

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

    function invokeTagManagementShortcut(event) {
        const handler = agendaBlock.tagManagementShortcutKeyPressHandler
        if (!handler || typeof handler !== "function")
            return false
        return !!handler(event)
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
                                mouseSelectionMode: agendaBlock.editorMouseSelectionMode
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
                                tagManagementKeyPressHandler: function (event) {
                                    return agendaBlock.invokeTagManagementShortcut(event)
                                }
                                text: taskRow.taskText
                                textColor: agendaBlock.taskTextColor
                                wrapMode: TextEdit.Wrap

                            }



Item {
    id: controller
    objectName: "contentsAgendaTaskRowController"

    property var agendaBlock: null
    property var taskRow: null
    property var taskEditor: null
    property bool hasLiveTaskTextSnapshot: false
    property string liveTaskText: ""

    function focusEditor(cursorPosition) {
        if (!controller.agendaBlock || !controller.taskEditor)
            return;
        controller.taskEditor.forceActiveFocus();
        const numericCursorPosition = Number(cursorPosition);
        const targetCursorPosition = isFinite(numericCursorPosition)
                                   ? Math.max(
                                         0,
                                         Math.min(
                                             Math.floor(numericCursorPosition),
                                             Math.max(0, controller.taskEditor.length || 0)))
                                   : Math.max(0, controller.taskEditor.length || 0);
        if (controller.taskEditor.setCursorPositionPreservingNativeInput !== undefined)
            controller.taskEditor.setCursorPositionPreservingNativeInput(targetCursorPosition);
        else if (controller.taskEditor.cursorPosition !== undefined)
            controller.taskEditor.cursorPosition = targetCursorPosition;
        controller.agendaBlock.activated();
    }

    function clearSelection(preserveFocusedEditor) {
        if (!controller.taskEditor || controller.taskEditor.clearSelection === undefined)
            return false;
        if (preserveFocusedEditor === true && controller.taskEditor.focused)
            return false;
        controller.taskEditor.clearSelection();
        return true;
    }

    function currentEditorPlainText() {
        if (!controller.taskRow)
            return "";
        if (!controller.taskEditor)
            return StructuredCursorSupport.normalizedPlainText(controller.taskRow.taskText);
        if (controller.taskEditor.currentPlainText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(controller.taskEditor.currentPlainText());
        return StructuredCursorSupport.normalizedPlainText(controller.taskRow.taskText);
    }

    function syncLiveTaskTextFromHost() {
        if (!controller.taskRow)
            return;
        const hostText = StructuredCursorSupport.normalizedPlainText(controller.taskRow.taskText);
        if (controller.taskRow.focused
                && controller.hasLiveTaskTextSnapshot
                && hostText !== controller.liveTaskText
                && controller.currentEditorPlainText() !== hostText) {
            return;
        }
        controller.liveTaskText = hostText;
        controller.hasLiveTaskTextSnapshot = true;
    }

    function currentCursorRowRect() {
        if (!controller.agendaBlock || !controller.taskRow)
            return ({ "contentHeight": Math.round(LV.Theme.scaleMetric(12)), "contentY": 0 });
        const editorItem = controller.taskEditor && controller.taskEditor.editorItem ? controller.taskEditor.editorItem : null;
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        Math.max(0, controller.taskEditor ? controller.taskEditor.length || 0 : 0),
                        Number(controller.taskEditor && controller.taskEditor.cursorPosition !== undefined
                               ? controller.taskEditor.cursorPosition
                               : 0) || 0));
        if (!editorItem || editorItem.positionToRectangle === undefined)
            return ({
                        "contentHeight": Math.max(
                                             1,
                                             Number(controller.taskEditor ? controller.taskEditor.inputContentHeight : 0)
                                             || Math.round(LV.Theme.scaleMetric(12))),
                        "contentY": Math.max(0, Number(controller.taskRow.y) || 0)
                    });
        const rect = editorItem.positionToRectangle(cursorPosition);
        const mappedPoint = editorItem.mapToItem !== undefined
                ? editorItem.mapToItem(controller.agendaBlock, 0, Number(rect.y) || 0)
                : ({
                       "x": 0,
                       "y": Math.max(0, Number(controller.taskRow.y) || 0) + (Number(rect.y) || 0)
                   });
        return {
            "contentHeight": Math.max(1, Number(rect.height) || Math.round(LV.Theme.scaleMetric(12))),
            "contentY": Math.max(0, Number(mappedPoint.y) || 0)
        };
    }

    function cursorOnFirstVisualRow() {
        const rowRect = controller.currentCursorRowRect();
        const localTaskY = controller.taskRow ? Math.max(0, Number(controller.taskRow.y) || 0) : 0;
        return Math.max(0, Number(rowRect.contentY) || 0) <= localTaskY + 1;
    }

    function cursorOnLastVisualRow() {
        const rowRect = controller.currentCursorRowRect();
        const rowBottom = Math.max(
                    0,
                    (Number(rowRect.contentY) || 0) + (Number(rowRect.contentHeight) || 0));
        const localTaskY = controller.taskRow ? Math.max(0, Number(controller.taskRow.y) || 0) : 0;
        const contentHeight = Math.max(
                    1,
                    Number(controller.taskEditor && controller.taskEditor.inputContentHeight !== undefined
                           ? controller.taskEditor.inputContentHeight
                           : 0)
                    || Math.max(1, rowBottom - localTaskY));
        const localRowBottom = Math.max(0, rowBottom - localTaskY);
        return localRowBottom >= contentHeight - 1;
    }

    function handleToggleChanged(checked) {
        if (!controller.agendaBlock || !controller.taskRow)
            return;
        if (checked === !!controller.taskRow.taskData.done)
            return;
        controller.agendaBlock.taskDoneToggled(
                    controller.taskRow.taskOpenTagStart,
                    controller.taskRow.taskOpenTagEnd,
                    checked);
        controller.agendaBlock.activated();
    }

    function handleEditorFocusedChanged() {
        if (!controller.agendaBlock || !controller.taskEditor)
            return;
        if (controller.taskEditor.focused) {
            controller.syncLiveTaskTextFromHost();
            controller.agendaBlock.activated();
        }
    }

    function handleEditorCursorPositionChanged() {
        if (controller.agendaBlock && controller.taskEditor && controller.taskEditor.focused)
            controller.agendaBlock.cursorInteraction();
    }

    function handleEditorTextEdited(text) {
        if (!controller.agendaBlock || !controller.taskRow || !controller.taskEditor)
            return;
        const previousText = controller.hasLiveTaskTextSnapshot
                ? controller.liveTaskText
                : StructuredCursorSupport.normalizedPlainText(controller.taskRow.taskText);
        const nextText = StructuredCursorSupport.normalizedPlainText(String(text || ""));
        if (previousText === nextText)
            return;
        controller.liveTaskText = nextText;
        controller.hasLiveTaskTextSnapshot = true;
        controller.agendaBlock.taskTextChanged(
                    controller.taskRow.taskData,
                    nextText,
                    Math.max(0, Number(controller.taskEditor.cursorPosition) || 0),
                    previousText);
    }

    Connections {
        function onCursorPositionChanged() {
            controller.handleEditorCursorPositionChanged();
        }

        function onFocusedChanged() {
            controller.handleEditorFocusedChanged();
        }

        function onTextEdited(text) {
            controller.handleEditorTextEdited(text);
        }

        target: controller.taskEditor
    }

    Component.onCompleted: controller.syncLiveTaskTextFromHost()
}
                        }
                    }
                }
            }
        }
    }



QtObject {
    id: controller
    objectName: "contentsAgendaBlockController"

    property var agendaBlock: null
    property var taskRepeater: null

    function itemAt(index) {
        return controller.taskRepeater && controller.taskRepeater.itemAt !== undefined
                ? controller.taskRepeater.itemAt(index)
                : null;
    }

    function taskCount() {
        return controller.taskRepeater && controller.taskRepeater.count !== undefined
                ? Math.max(0, Math.floor(Number(controller.taskRepeater.count) || 0))
                : 0;
    }

    function hasFocusedTaskRow() {
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            if (item && item["focused"] !== undefined && item["focused"])
                return true;
        }
        return false;
    }

    function focusedTaskInputMethodComposing() {
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            if (!item || item["focused"] === undefined || !item["focused"])
                continue;
            return item["inputMethodComposing"] !== undefined && !!item["inputMethodComposing"];
        }
        return false;
    }

    function focusedTaskPreeditText() {
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            if (!item || item["focused"] === undefined || !item["focused"])
                continue;
            return item["preeditText"] !== undefined && item["preeditText"] !== null
                    ? String(item["preeditText"])
                    : "";
        }
        return "";
    }

    function nativeCompositionActive() {
        return controller.focusedTaskInputMethodComposing()
                || controller.focusedTaskPreeditText().length > 0;
    }

    function currentFocusedTaskLineNumber() {
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            if (item && item["focused"] !== undefined && item["focused"])
                return index + 1;
        }
        return 1;
    }

    function currentCursorRowRect() {
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            if (!item || item["focused"] === undefined || !item["focused"] || item["currentCursorRowRect"] === undefined)
                continue;
            return item["currentCursorRowRect"]();
        }
        return ({
                    "contentHeight": Math.max(1, Math.round(LV.Theme.scaleMetric(12))),
                    "contentY": 0
                });
    }

    function currentVisiblePlainText() {
        if (!controller.agendaBlock)
            return "";
        const lines = [];
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            if (item && item["currentEditorPlainText"] !== undefined) {
                lines.push(String(item["currentEditorPlainText"]() || ""));
                continue;
            }
            const taskData = controller.agendaBlock.tasks[index] && typeof controller.agendaBlock.tasks[index] === "object"
                    ? controller.agendaBlock.tasks[index]
                    : ({});
            lines.push(StructuredCursorSupport.normalizedPlainText(String(taskData.text || "")));
        }
        if (lines.length === 0)
            lines.push("");
        return lines.join("\n");
    }

    function visiblePlainText() {
        return controller.currentVisiblePlainText();
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText);
        return Math.max(0, normalizedLineText.length);
    }

    function focusLastTask() {
        return controller.focusTaskBoundary(controller.taskCount() - 1, "after");
    }

    function focusTaskBoundary(taskIndex, side) {
        const numericTaskIndex = Number(taskIndex);
        const resolvedTaskIndex = isFinite(numericTaskIndex) ? Math.floor(numericTaskIndex) : -1;
        const item = controller.itemAt(resolvedTaskIndex);
        if (!item || item["focusEditor"] === undefined)
            return false;
        const normalizedSide = side === undefined || side === null ? "" : String(side).trim().toLowerCase();
        const plainText = item["currentEditorPlainText"] !== undefined
                ? String(item["currentEditorPlainText"]() || "")
                : String(item["taskText"] || "");
        item["focusEditor"](normalizedSide === "after" ? plainText.length : 0);
        return true;
    }

    function focusBoundary(side) {
        const normalizedSide = side === undefined || side === null ? "" : String(side).trim().toLowerCase();
        if (normalizedSide === "after")
            return controller.focusLastTask();
        return controller.focusFirstTask();
    }

    function focusFirstTask() {
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            if (!item || item["focusEditor"] === undefined)
                continue;
            item["focusEditor"](0);
            return true;
        }
        return false;
    }

    function focusTaskAtSourceOffset(sourceOffset) {
        const numericSourceOffset = Number(sourceOffset);
        if (!isFinite(numericSourceOffset))
            return false;
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            if (!item || item["taskContentStart"] === undefined || item["taskContentEnd"] === undefined)
                continue;
            const contentStart = Math.max(0, Math.floor(Number(item["taskContentStart"]) || 0));
            const contentEnd = Math.max(contentStart, Math.floor(Number(item["taskContentEnd"]) || 0));
            if (numericSourceOffset < contentStart || numericSourceOffset > contentEnd)
                continue;
            item["focusEditor"](
                        StructuredCursorSupport.plainCursorForSourceOffset(
                            item["taskText"],
                            numericSourceOffset - contentStart));
            return true;
        }
        return false;
    }

    function applyFocusRequest(request) {
        if (!controller.agendaBlock)
            return false;
        const safeRequest = request && typeof request === "object" ? request : ({});
        const taskOpenTagStart = Number(safeRequest.taskOpenTagStart);
        const localCursorPosition = Number(safeRequest.localCursorPosition);
        const entryBoundary = safeRequest.entryBoundary === undefined || safeRequest.entryBoundary === null
                ? ""
                : String(safeRequest.entryBoundary).trim().toLowerCase();
        if (isFinite(taskOpenTagStart)) {
            for (let index = 0; index < controller.taskCount(); ++index) {
                const item = controller.itemAt(index);
                if (!item || item["taskOpenTagStart"] === undefined)
                    continue;
                if (Number(item["taskOpenTagStart"]) !== Math.floor(taskOpenTagStart))
                    continue;
                item["focusEditor"](localCursorPosition);
                return true;
            }
        }

        const sourceOffset = Number(safeRequest.sourceOffset);
        if (!isFinite(sourceOffset))
            return false;
        if (sourceOffset < controller.agendaBlock.sourceStart || sourceOffset > controller.agendaBlock.sourceEnd)
            return false;
        if (entryBoundary === "before" || sourceOffset <= controller.agendaBlock.sourceStart)
            return controller.focusBoundary("before");
        if (entryBoundary === "after" || sourceOffset >= controller.agendaBlock.sourceEnd)
            return controller.focusBoundary("after");
        if (controller.focusTaskAtSourceOffset(sourceOffset))
            return true;
        return controller.focusBoundary("before");
    }

    function clearSelection(preserveFocusedEditor) {
        let cleared = false;
        for (let index = 0; index < controller.taskCount(); ++index) {
            const item = controller.itemAt(index);
            const clearSelectionFunction = item && item["clearSelection"] !== undefined
                    ? item["clearSelection"]
                    : null;
            if (!clearSelectionFunction)
                continue;
            if (clearSelectionFunction(preserveFocusedEditor === true))
                cleared = true;
        }
        return cleared;
    }

    function shortcutInsertionSourceOffset() {
        if (!controller.agendaBlock)
            return 0;
        return controller.agendaBlock.sourceEnd;
    }
}
}
