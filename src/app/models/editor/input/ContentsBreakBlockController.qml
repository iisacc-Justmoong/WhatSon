pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller
    objectName: "contentsBreakBlockController"

    property var breakBlock: null
    property var divider: null

    function selectBreakBlock() {
        if (!controller.breakBlock)
            return;
        controller.breakBlock.forceActiveFocus();
        controller.breakBlock.activated();
    }

    function applyFocusRequest(request) {
        if (!controller.breakBlock)
            return false;
        const safeRequest = request && typeof request === "object" ? request : ({});
        const sourceOffset = Number(safeRequest.sourceOffset);
        if (!isFinite(sourceOffset))
            return false;
        if (sourceOffset < controller.breakBlock.sourceStart || sourceOffset > controller.breakBlock.sourceEnd)
            return false;
        controller.selectBreakBlock();
        return true;
    }

    function visiblePlainText() {
        return "";
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText);
        if (normalizedLineText.length > 0)
            return normalizedLineText.length;
        return 8;
    }

    function logicalLineLayoutEntries() {
        if (!controller.breakBlock || !controller.divider)
            return [];
        const mappedOrigin = controller.divider.mapToItem !== undefined
                ? controller.divider.mapToItem(controller.breakBlock, 0, 0)
                : ({
                       "x": 0,
                       "y": Math.max(
                                0,
                                (controller.breakBlock.height - controller.divider.height) / 2)
                   });
        return [{
                    "contentHeight": Math.max(1, Number(controller.divider.height) || 1),
                    "contentY": Math.max(0, Number(mappedOrigin.y) || 0)
                }];
    }

    function currentCursorRowRect() {
        const entries = controller.logicalLineLayoutEntries();
        if (entries.length > 0)
            return entries[0];
        return ({
                    "contentHeight": 1,
                    "contentY": 0
                });
    }

    function handleKeyPress(event) {
        if (!controller.breakBlock || !event)
            return false;
        if (controller.breakBlock.tagManagementShortcutKeyPressHandler
                && typeof controller.breakBlock.tagManagementShortcutKeyPressHandler === "function") {
            const shortcutHandled = !!controller.breakBlock.tagManagementShortcutKeyPressHandler(event);
            if (shortcutHandled || event.accepted)
                return true;
        }
        const modifiers = Number(event.modifiers) || 0;
        const moveUp = event.key === Qt.Key_Up;
        const moveDown = event.key === Qt.Key_Down;
        const macCommandDocumentNavigation = Qt.platform.os === "osx"
                && (moveUp || moveDown)
                && modifiers === Qt.MetaModifier;
        if (macCommandDocumentNavigation) {
            controller.breakBlock.boundaryNavigationRequested("document", moveUp ? "before" : "after");
            event.accepted = true;
            return true;
        }
        if (modifiers !== Qt.NoModifier)
            return false;
        if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
            controller.breakBlock.blockDeletionRequested();
            event.accepted = true;
            return true;
        }
        if (event.key === Qt.Key_Left) {
            controller.breakBlock.boundaryNavigationRequested("horizontal", "before");
            event.accepted = true;
            return true;
        }
        if (event.key === Qt.Key_Right) {
            controller.breakBlock.boundaryNavigationRequested("horizontal", "after");
            event.accepted = true;
            return true;
        }
        if (event.key === Qt.Key_Up) {
            controller.breakBlock.boundaryNavigationRequested("vertical", "before");
            event.accepted = true;
            return true;
        }
        if (event.key === Qt.Key_Down) {
            controller.breakBlock.boundaryNavigationRequested("vertical", "after");
            event.accepted = true;
            return true;
        }
        return false;
    }

    function handleTap(tapCount) {
        if (!controller.breakBlock)
            return;
        if (tapCount >= 2) {
            controller.breakBlock.documentEndEditRequested();
            return;
        }
        controller.selectBreakBlock();
    }
}
