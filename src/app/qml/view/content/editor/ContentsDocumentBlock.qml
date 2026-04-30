pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "../../../../models/editor/diagnostics/ContentsEditorDebugTrace.js" as EditorTrace

FocusScope {
    id: documentBlock
    objectName: "contentsDocumentBlock"

    required property var blockData
    property int blockIndex: -1
    property var resourceEntry: ({})
    property var selectionManager: null
    property var tagManagementShortcutKeyPressHandler: null
    property bool paperPaletteEnabled: false
    property bool hasAdjacentAtomicBlockAfter: false
    property bool hasAdjacentAtomicBlockBefore: false
    property bool hasAdjacentBlockAfter: false
    property bool hasAdjacentBlockBefore: false
    property bool nativeTextInputPriority: false
    property bool paragraphBoundaryOperationsEnabled: false
    property bool paragraphMergeableAfter: false
    property bool paragraphMergeableBefore: false

    signal activated()
    signal adjacentAtomicBlockDeleteRequested(string side)
    signal boundaryNavigationRequested(string axis, string side)
    signal blockDeletionRequested(string direction)
    signal cursorInteraction()
    signal documentEndEditRequested()
    signal inlineFormatRequested(int blockIndex, string tagName, var selectionSnapshot)
    signal paragraphSplitRequested(int sourceOffset)
    signal sourceMutationRequested(string nextBlockSourceText, var focusRequest, string expectedPreviousSourceText)
    signal taskDoneToggled(int openTagStart, int openTagEnd, bool checked)
    signal taskEnterRequested(var blockData, var taskData)
    signal taskTextChanged(var taskData, string text, int cursorPosition, string expectedPreviousText)
    signal enterExitRequested(var blockData, int sourceOffset)
    signal textChanged(string text, int cursorPosition, string expectedPreviousText)

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property string blockType: normalizedBlock.type !== undefined ? String(normalizedBlock.type).trim().toLowerCase() : "text"
    readonly property bool focused: {
        if (documentBlock.activeFocus)
            return true
        const blockItem = blockLoader.item
        if (!blockItem)
            return false
        if (blockItem.focused !== undefined)
            return !!blockItem.focused
        return blockItem.activeFocus !== undefined && !!blockItem.activeFocus
    }
    readonly property bool inputMethodComposing: {
        const blockItem = blockLoader.item
        return blockItem && blockItem.inputMethodComposing !== undefined
                ? !!blockItem.inputMethodComposing
                : false
    }
    readonly property string preeditText: {
        const blockItem = blockLoader.item
        return blockItem && blockItem.preeditText !== undefined && blockItem.preeditText !== null
                ? String(blockItem.preeditText)
                : ""
    }
    readonly property int currentLogicalLineNumber: {
        const blockItem = blockLoader.item
        if (!blockItem || blockItem.currentLogicalLineNumber === undefined)
            return 1
        return Math.max(1, Number(blockItem.currentLogicalLineNumber) || 1)
    }
    readonly property bool textEditable: {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.textEditable !== undefined)
            return !!blockItem.textEditable
        return !!normalizedBlock.textEditable
    }
    readonly property bool atomicBlock: {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.atomicBlock !== undefined)
            return !!blockItem.atomicBlock
        return !!normalizedBlock.atomicBlock
    }
    readonly property bool gutterCollapsed: {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.gutterCollapsed !== undefined)
            return !!blockItem.gutterCollapsed
        return !!normalizedBlock.gutterCollapsed
    }
    readonly property string minimapVisualKind: {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.minimapVisualKind !== undefined)
            return String(blockItem.minimapVisualKind || "text")
        return normalizedBlock.minimapVisualKind !== undefined
                ? String(normalizedBlock.minimapVisualKind || "text")
                : "text"
    }
    readonly property int minimapRepresentativeCharCount: {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.minimapRepresentativeCharCount !== undefined)
            return Math.max(0, Number(blockItem.minimapRepresentativeCharCount) || 0)
        return Math.max(0, Number(normalizedBlock.minimapRepresentativeCharCount) || 0)
    }

    implicitHeight: blockLoader.item
                    ? Math.max(
                          0,
                          Number(blockLoader.item.implicitHeight) || Number(blockLoader.item.height) || 0)
                    : 0
    width: parent ? parent.width : implicitWidth

    function currentCursorRowRect() {
        return documentBlockController.currentCursorRowRect()
    }

    function logicalLineLayoutEntries() {
        return documentBlockController.logicalLineLayoutEntries()
    }

    function applyFocusRequest(request) {
        return documentBlockController.applyFocusRequest(request)
    }

    function handleAtomicTagDeleteKeyPress(event) {
        return documentBlockController.handleAtomicTagDeleteKeyPress(event)
    }

    function inlineFormatSelectionSnapshot() {
        return documentBlockController.inlineFormatSelectionSnapshot()
    }

    function clearSelection(preserveFocusedEditor) {
        return documentBlockController.clearSelection(preserveFocusedEditor)
    }

    function shortcutInsertionSourceOffset() {
        return documentBlockController.shortcutInsertionSourceOffset()
    }

    function visiblePlainText() {
        return documentBlockController.visiblePlainText()
    }

    function representativeCharCount(lineText) {
        return documentBlockController.representativeCharCount(lineText)
    }

    function handleAtomicTagManagementKeyPress(event) {
        return documentBlockController.handleAtomicTagManagementKeyPress(event)
    }

    Keys.onPressed: function (event) {
        if (documentBlock.handleAtomicTagManagementKeyPress(event))
            return
    }

    Loader {
        id: blockLoader

        anchors.fill: parent
        sourceComponent: documentBlock.blockType === "agenda"
                         ? agendaBlockComponent
                         : documentBlock.blockType === "callout"
                           ? calloutBlockComponent
                           : documentBlock.blockType === "resource"
                             ? resourceBlockComponent
                           : documentBlock.blockType === "break"
                             ? breakBlockComponent
                             : textBlockComponent
    }



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

    Component {
        id: textBlockComponent

        ContentsDocumentTextBlock {
            blockData: documentBlock.blockData
            hasAdjacentAtomicBlockAfter: documentBlock.hasAdjacentAtomicBlockAfter
            hasAdjacentAtomicBlockBefore: documentBlock.hasAdjacentAtomicBlockBefore
            hasAdjacentBlockAfter: documentBlock.hasAdjacentBlockAfter
            hasAdjacentBlockBefore: documentBlock.hasAdjacentBlockBefore
            nativeTextInputPriority: documentBlock.nativeTextInputPriority
            paperPaletteEnabled: documentBlock.paperPaletteEnabled
            paragraphBoundaryOperationsEnabled: documentBlock.paragraphBoundaryOperationsEnabled
            paragraphMergeableAfter: documentBlock.paragraphMergeableAfter
            paragraphMergeableBefore: documentBlock.paragraphMergeableBefore
            tagManagementShortcutKeyPressHandler: documentBlock.tagManagementShortcutKeyPressHandler
            width: documentBlock.width

            onInlineFormatRequested: function (tagName, selectionSnapshot) {
                documentBlock.inlineFormatRequested(documentBlock.blockIndex, tagName, selectionSnapshot)
            }
        }
    }

    Component {
        id: agendaBlockComponent

        ContentsAgendaBlock {
            blockData: documentBlock.blockData
            nativeTextInputPriority: documentBlock.nativeTextInputPriority
            paperPaletteEnabled: documentBlock.paperPaletteEnabled
            tagManagementShortcutKeyPressHandler: documentBlock.tagManagementShortcutKeyPressHandler
            width: documentBlock.width
        }
    }

    Component {
        id: calloutBlockComponent

        ContentsCalloutBlock {
            blockData: documentBlock.blockData
            nativeTextInputPriority: documentBlock.nativeTextInputPriority
            paperPaletteEnabled: documentBlock.paperPaletteEnabled
            tagManagementShortcutKeyPressHandler: documentBlock.tagManagementShortcutKeyPressHandler
            width: documentBlock.width
        }
    }

    Component {
        id: breakBlockComponent

        ContentsBreakBlock {
            blockData: documentBlock.blockData
            tagManagementShortcutKeyPressHandler: documentBlock.tagManagementShortcutKeyPressHandler
            width: documentBlock.width
        }
    }

    Component {
        id: resourceBlockComponent

        ContentsResourceBlock {
            blockData: documentBlock.blockData
            blockFocused: documentBlock.activeFocus
            resourceEntry: documentBlock.resourceEntry
            width: documentBlock.width
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        enabled: documentBlock.atomicBlock

        onTapped: function () {
            documentBlockController.handleAtomicTap(tapCount)
        }
    }
}
