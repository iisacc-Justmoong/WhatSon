pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "../structure/ContentsStructuredCursorSupport.js" as StructuredCursorSupport

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
