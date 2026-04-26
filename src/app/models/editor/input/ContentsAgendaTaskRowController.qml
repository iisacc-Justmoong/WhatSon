pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "../structure/ContentsStructuredCursorSupport.js" as StructuredCursorSupport

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
