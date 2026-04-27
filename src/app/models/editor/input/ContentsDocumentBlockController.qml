pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "../diagnostics/ContentsEditorDebugTrace.js" as EditorTrace

Item {
    id: controller
    objectName: "contentsDocumentBlockController"

    property var documentBlock: null
    property var blockLoader: null

    function blockItem() {
        return controller.blockLoader && controller.blockLoader.item ? controller.blockLoader.item : null;
    }

    function currentCursorRowRect() {
        const item = controller.blockItem();
        if (item && item.currentCursorRowRect !== undefined)
            return item.currentCursorRowRect();
        return ({
                    "contentHeight": Math.max(1, Math.round(LV.Theme.scaleMetric(12))),
                    "contentY": 0
                });
    }

    function logicalLineLayoutEntries() {
        const item = controller.blockItem();
        if (item && item.logicalLineLayoutEntries !== undefined)
            return item.logicalLineLayoutEntries();
        return [];
    }

    function applyFocusRequest(request) {
        if (!controller.documentBlock)
            return false;
        EditorTrace.trace(
                    "documentBlock",
                    "applyFocusRequest",
                    "blockType=" + controller.documentBlock.blockType
                    + " atomic=" + controller.documentBlock.atomicBlock
                    + " request={" + EditorTrace.describeFocusRequest(request) + "}",
                    controller.documentBlock);
        if (controller.documentBlock.atomicBlock) {
            const safeRequest = request && typeof request === "object" ? request : ({});
            const sourceOffset = Number(safeRequest.sourceOffset);
            if (!isFinite(sourceOffset))
                return false;
            const sourceStart = Math.max(0, Number(controller.documentBlock.normalizedBlock.sourceStart) || 0);
            const sourceEnd = Math.max(sourceStart, Number(controller.documentBlock.normalizedBlock.sourceEnd) || sourceStart);
            if (sourceOffset < sourceStart || sourceOffset > sourceEnd)
                return false;
            controller.documentBlock.forceActiveFocus();
            controller.documentBlock.activated();
            return true;
        }
        const item = controller.blockItem();
        if (!item || item.applyFocusRequest === undefined)
            return false;
        return !!item.applyFocusRequest(request);
    }

    function handleAtomicTagDeleteKeyPress(event) {
        if (!controller.documentBlock)
            return false;
        EditorTrace.trace(
                    "documentBlock",
                    "handleAtomicTagDeleteKeyPress",
                    "blockType=" + controller.documentBlock.blockType
                    + " atomic=" + controller.documentBlock.atomicBlock
                    + " key=" + (event ? Number(event.key) : -1),
                    controller.documentBlock);
        if (controller.documentBlock.atomicBlock) {
            if (!event)
                return false;
            const modifiers = Number(event.modifiers) || 0;
            if (modifiers !== Qt.NoModifier)
                return false;
            if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
                controller.documentBlock.blockDeletionRequested(
                            event.key === Qt.Key_Delete
                            ? "forward"
                            : "backward");
                event.accepted = true;
                return true;
            }
            return false;
        }
        return false;
    }

    function inlineFormatSelectionSnapshot() {
        const item = controller.blockItem();
        if (!item)
            return ({});
        if (item.inlineFormatSelectionSnapshot !== undefined)
            return item.inlineFormatSelectionSnapshot();
        if (item.selectionSnapshot !== undefined)
            return item.selectionSnapshot();
        return ({});
    }

    function clearSelection(preserveFocusedEditor) {
        const item = controller.blockItem();
        if (!item || item.clearSelection === undefined)
            return false;
        return !!item.clearSelection(preserveFocusedEditor === true);
    }

    function shortcutInsertionSourceOffset() {
        if (!controller.documentBlock)
            return 0;
        const item = controller.blockItem();
        if (item && item.shortcutInsertionSourceOffset !== undefined) {
            const blockOffset = Number(item.shortcutInsertionSourceOffset());
            if (isFinite(blockOffset))
                return Math.max(0, Math.floor(blockOffset));
        }
        return Math.max(0, Number(controller.documentBlock.normalizedBlock.sourceEnd) || 0);
    }

    function visiblePlainText() {
        if (!controller.documentBlock)
            return "";
        const item = controller.blockItem();
        if (item && item.visiblePlainText !== undefined)
            return String(item.visiblePlainText() || "");
        if (controller.documentBlock.normalizedBlock.plainText !== undefined)
            return String(controller.documentBlock.normalizedBlock.plainText || "");
        if (controller.documentBlock.normalizedBlock.sourceText !== undefined)
            return String(controller.documentBlock.normalizedBlock.sourceText || "");
        return "";
    }

    function representativeCharCount(lineText) {
        if (!controller.documentBlock)
            return 0;
        const item = controller.blockItem();
        if (item && item.representativeCharCount !== undefined)
            return Math.max(0, Number(item.representativeCharCount(lineText)) || 0);
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText);
        if (normalizedLineText.length > 0)
            return normalizedLineText.length;
        return controller.documentBlock.minimapRepresentativeCharCount;
    }

    function handleAtomicTagManagementKeyPress(event) {
        if (!controller.documentBlock || !controller.documentBlock.atomicBlock || !event)
            return false;
        if (controller.documentBlock.tagManagementShortcutKeyPressHandler
                && typeof controller.documentBlock.tagManagementShortcutKeyPressHandler === "function") {
            const shortcutHandled = !!controller.documentBlock.tagManagementShortcutKeyPressHandler(event);
            if (shortcutHandled || event.accepted)
                return true;
        }
        if (controller.handleAtomicTagDeleteKeyPress(event))
            return true;
        const modifiers = Number(event.modifiers) || 0;
        const moveUp = event.key === Qt.Key_Up;
        const moveDown = event.key === Qt.Key_Down;
        const macCommandDocumentNavigation = Qt.platform.os === "osx"
                && (moveUp || moveDown)
                && modifiers === Qt.MetaModifier;
        if (macCommandDocumentNavigation) {
            controller.documentBlock.boundaryNavigationRequested("document", moveUp ? "before" : "after");
            event.accepted = true;
            return true;
        }
        if (modifiers !== Qt.NoModifier)
            return false;
        if (event.key === Qt.Key_Left) {
            controller.documentBlock.boundaryNavigationRequested("horizontal", "before");
            event.accepted = true;
            return true;
        }
        if (event.key === Qt.Key_Right) {
            controller.documentBlock.boundaryNavigationRequested("horizontal", "after");
            event.accepted = true;
            return true;
        }
        if (event.key === Qt.Key_Up) {
            controller.documentBlock.boundaryNavigationRequested("vertical", "before");
            event.accepted = true;
            return true;
        }
        if (event.key === Qt.Key_Down) {
            controller.documentBlock.boundaryNavigationRequested("vertical", "after");
            event.accepted = true;
            return true;
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            controller.documentBlock.documentEndEditRequested();
            event.accepted = true;
            return true;
        }
        return false;
    }

    function handleSelectionClearRevisionChanged() {
        if (!controller.documentBlock)
            return;
        const manager = controller.documentBlock.selectionManager;
        if (!manager)
            return;
        const retainedBlockIndex = manager.selectionClearRetainedBlockIndex !== undefined
                ? Math.floor(Number(manager.selectionClearRetainedBlockIndex) || -1)
                : -1;
        controller.clearSelection(retainedBlockIndex === controller.documentBlock.blockIndex);
    }

    function handleFocusedChanged() {
        if (!controller.documentBlock)
            return;
        EditorTrace.trace(
                    "documentBlock",
                    "focusedChanged",
                    "blockType=" + controller.documentBlock.blockType
                    + " focused=" + controller.documentBlock.focused,
                    controller.documentBlock);
    }

    function handleBlockItemActivated() {
        if (!controller.documentBlock)
            return;
        if (controller.documentBlock.atomicBlock)
            controller.documentBlock.forceActiveFocus();
        controller.documentBlock.activated();
    }

    function handleAtomicTap(tapCount) {
        if (!controller.documentBlock)
            return;
        controller.documentBlock.forceActiveFocus();
        controller.documentBlock.activated();
        if (tapCount >= 2)
            controller.documentBlock.documentEndEditRequested();
    }

    function mount() {
        if (!controller.documentBlock)
            return;
        EditorTrace.trace(
                    "documentBlock",
                    "mount",
                    "block={" + EditorTrace.describeObject(
                        controller.documentBlock.normalizedBlock,
                        ["type", "sourceStart", "sourceEnd"]) + "}",
                    controller.documentBlock);
    }

    function unmount() {
        if (!controller.documentBlock)
            return;
        EditorTrace.trace(
                    "documentBlock",
                    "unmount",
                    "block={" + EditorTrace.describeObject(
                        controller.documentBlock.normalizedBlock,
                        ["type", "sourceStart", "sourceEnd"]) + "}",
                    controller.documentBlock);
    }

    Connections {
        function onSelectionClearRevisionChanged() {
            controller.handleSelectionClearRevisionChanged();
        }

        ignoreUnknownSignals: true
        target: controller.documentBlock ? controller.documentBlock.selectionManager : null
    }

    Connections {
        function onFocusedChanged() {
            controller.handleFocusedChanged();
        }

        target: controller.documentBlock
    }

    Connections {
        function onActivated() {
            controller.handleBlockItemActivated();
        }

        function onAdjacentAtomicBlockDeleteRequested(side) {
            controller.documentBlock.adjacentAtomicBlockDeleteRequested(side);
        }

        function onBoundaryNavigationRequested(axis, side) {
            controller.documentBlock.boundaryNavigationRequested(axis, side);
        }

        function onBlockDeletionRequested(direction) {
            controller.documentBlock.blockDeletionRequested(direction);
        }

        function onCursorInteraction() {
            controller.documentBlock.cursorInteraction();
        }

        function onDocumentEndEditRequested() {
            controller.documentBlock.documentEndEditRequested();
        }

        function onEnterExitRequested(blockData, sourceOffset) {
            controller.documentBlock.enterExitRequested(blockData, sourceOffset);
        }

        function onParagraphSplitRequested(sourceOffset) {
            controller.documentBlock.paragraphSplitRequested(sourceOffset);
        }

        function onSourceMutationRequested(nextBlockSourceText, focusRequest, expectedPreviousSourceText) {
            controller.documentBlock.sourceMutationRequested(nextBlockSourceText, focusRequest, expectedPreviousSourceText);
        }

        function onTaskDoneToggled(openTagStart, openTagEnd, checked) {
            controller.documentBlock.taskDoneToggled(openTagStart, openTagEnd, checked);
        }

        function onTaskEnterRequested(blockData, taskData) {
            controller.documentBlock.taskEnterRequested(blockData, taskData);
        }

        function onTaskTextChanged(taskData, text, cursorPosition, expectedPreviousText) {
            controller.documentBlock.taskTextChanged(taskData, text, cursorPosition, expectedPreviousText);
        }

        function onTextChanged(text, cursorPosition, expectedPreviousText) {
            controller.documentBlock.textChanged(text, cursorPosition, expectedPreviousText);
        }

        ignoreUnknownSignals: true
        target: controller.blockLoader && controller.blockLoader.item ? controller.blockLoader.item : null
    }

    Component.onCompleted: controller.mount()
    Component.onDestruction: controller.unmount()
}
