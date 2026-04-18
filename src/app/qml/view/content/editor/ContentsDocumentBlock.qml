pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "ContentsEditorDebugTrace.js" as EditorTrace

FocusScope {
    id: documentBlock
    objectName: "contentsDocumentBlock"

    required property var blockData
    property int blockIndex: -1
    property var resourceEntry: ({})
    property var selectionManager: null
    property var shortcutKeyPressHandler: null
    property bool hasAdjacentAtomicBlockAfter: false
    property bool hasAdjacentAtomicBlockBefore: false
    property bool hasAdjacentBlockAfter: false
    property bool hasAdjacentBlockBefore: false
    property bool paragraphBoundaryOperationsEnabled: false
    property bool paragraphMergeableAfter: false
    property bool paragraphMergeableBefore: false

    signal activated()
    signal adjacentAtomicBlockDeleteRequested(string side)
    signal boundaryNavigationRequested(string axis, string side)
    signal blockDeletionRequested(string direction)
    signal documentEndEditRequested()
    signal paragraphSplitRequested(int sourceOffset)
    signal sourceMutationRequested(string nextBlockSourceText, var focusRequest)
    signal taskDoneToggled(int openTagStart, int openTagEnd, bool checked)
    signal taskEnterRequested(var blockData, var taskData)
    signal taskTextChanged(var taskData, string text, int cursorPosition)
    signal enterExitRequested(var blockData)
    signal textChanged(string text, int cursorPosition)

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
        const blockItem = blockLoader.item
        if (blockItem && blockItem.currentCursorRowRect !== undefined)
            return blockItem.currentCursorRowRect()
        return ({
                    "contentHeight": Math.max(1, Math.round(LV.Theme.scaleMetric(12))),
                    "contentY": 0
                })
    }

    function logicalLineLayoutEntries() {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.logicalLineLayoutEntries !== undefined)
            return blockItem.logicalLineLayoutEntries()
        return []
    }

    function applyFocusRequest(request) {
        EditorTrace.trace(
                    "documentBlock",
                    "applyFocusRequest",
                    "blockType=" + documentBlock.blockType
                    + " atomic=" + documentBlock.atomicBlock
                    + " request={" + EditorTrace.describeFocusRequest(request) + "}",
                    documentBlock)
        if (documentBlock.atomicBlock) {
            const safeRequest = request && typeof request === "object" ? request : ({})
            const sourceOffset = Number(safeRequest.sourceOffset)
            if (!isFinite(sourceOffset))
                return false
            const sourceStart = Math.max(0, Number(documentBlock.normalizedBlock.sourceStart) || 0)
            const sourceEnd = Math.max(sourceStart, Number(documentBlock.normalizedBlock.sourceEnd) || sourceStart)
            if (sourceOffset < sourceStart || sourceOffset > sourceEnd)
                return false
            documentBlock.forceActiveFocus()
            documentBlock.activated()
            return true
        }
        const blockItem = blockLoader.item
        if (!blockItem || blockItem.applyFocusRequest === undefined)
            return false
        return !!blockItem.applyFocusRequest(request)
    }

    function handleDeleteKeyPress(event) {
        EditorTrace.trace(
                    "documentBlock",
                    "handleDeleteKeyPress",
                    "blockType=" + documentBlock.blockType
                    + " atomic=" + documentBlock.atomicBlock
                    + " key=" + (event ? Number(event.key) : -1),
                    documentBlock)
        if (documentBlock.atomicBlock) {
            if (!event)
                return false
            if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
                documentBlock.blockDeletionRequested(
                            event.key === Qt.Key_Delete
                            ? "forward"
                            : "backward")
                event.accepted = true
                return true
            }
            return false
        }
        const blockItem = blockLoader.item
        if (!blockItem || blockItem.handleDeleteKeyPress === undefined)
            return false
        return !!blockItem.handleDeleteKeyPress(event)
    }

    function inlineFormatSelectionSnapshot() {
        const blockItem = blockLoader.item
        if (!blockItem)
            return ({})
        if (blockItem.inlineFormatSelectionSnapshot !== undefined)
            return blockItem.inlineFormatSelectionSnapshot()
        if (blockItem.selectionSnapshot !== undefined)
            return blockItem.selectionSnapshot()
        return ({})
    }

    function applyInlineFormatToSelection(tagName, selectionSnapshot) {
        const blockItem = blockLoader.item
        if (!blockItem || blockItem.applyInlineFormatToSelection === undefined)
            return false
        return !!blockItem.applyInlineFormatToSelection(tagName, selectionSnapshot)
    }

    function clearSelection(preserveFocusedEditor) {
        const blockItem = blockLoader.item
        if (!blockItem || blockItem.clearSelection === undefined)
            return false
        return !!blockItem.clearSelection(preserveFocusedEditor === true)
    }

    function shortcutInsertionSourceOffset() {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.shortcutInsertionSourceOffset !== undefined) {
            const blockOffset = Number(blockItem.shortcutInsertionSourceOffset())
            if (isFinite(blockOffset))
                return Math.max(0, Math.floor(blockOffset))
        }
        return Math.max(0, Number(normalizedBlock.sourceEnd) || 0)
    }

    function visiblePlainText() {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.visiblePlainText !== undefined)
            return String(blockItem.visiblePlainText() || "")
        if (normalizedBlock.plainText !== undefined)
            return String(normalizedBlock.plainText || "")
        if (normalizedBlock.sourceText !== undefined)
            return String(normalizedBlock.sourceText || "")
        return ""
    }

    function representativeCharCount(lineText) {
        const blockItem = blockLoader.item
        if (blockItem && blockItem.representativeCharCount !== undefined)
            return Math.max(0, Number(blockItem.representativeCharCount(lineText)) || 0)
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
        if (normalizedLineText.length > 0)
            return normalizedLineText.length
        return minimapRepresentativeCharCount
    }

    function handleAtomicBoundaryKeyPress(event) {
        if (!documentBlock.atomicBlock || !event)
            return false
        if (documentBlock.shortcutKeyPressHandler
                && typeof documentBlock.shortcutKeyPressHandler === "function") {
            const shortcutHandled = !!documentBlock.shortcutKeyPressHandler(event)
            if (shortcutHandled || event.accepted)
                return true
        }
        if (documentBlock.handleDeleteKeyPress(event))
            return true
        const modifiers = Number(event.modifiers) || 0
        if ((modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier | Qt.ShiftModifier)) !== 0)
            return false
        if (event.key === Qt.Key_Left) {
            documentBlock.boundaryNavigationRequested("horizontal", "before")
            event.accepted = true
            return true
        }
        if (event.key === Qt.Key_Right) {
            documentBlock.boundaryNavigationRequested("horizontal", "after")
            event.accepted = true
            return true
        }
        if (event.key === Qt.Key_Up) {
            documentBlock.boundaryNavigationRequested("vertical", "before")
            event.accepted = true
            return true
        }
        if (event.key === Qt.Key_Down) {
            documentBlock.boundaryNavigationRequested("vertical", "after")
            event.accepted = true
            return true
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            documentBlock.documentEndEditRequested()
            event.accepted = true
            return true
        }
        return false
    }

    Keys.onPressed: function (event) {
        if (documentBlock.handleAtomicBoundaryKeyPress(event))
            return
    }

    Component.onCompleted: {
        EditorTrace.trace(
                    "documentBlock",
                    "mount",
                    "block={" + EditorTrace.describeObject(documentBlock.normalizedBlock, ["type", "sourceStart", "sourceEnd"]) + "}",
                    documentBlock)
    }

    Component.onDestruction: {
        EditorTrace.trace(
                    "documentBlock",
                    "unmount",
                    "block={" + EditorTrace.describeObject(documentBlock.normalizedBlock, ["type", "sourceStart", "sourceEnd"]) + "}",
                    documentBlock)
    }

    Connections {
        target: documentBlock.selectionManager
        ignoreUnknownSignals: true

        function onSelectionClearRevisionChanged() {
            const manager = documentBlock.selectionManager
            if (!manager)
                return
            const retainedBlockIndex = manager.selectionClearRetainedBlockIndex !== undefined
                    ? Math.floor(Number(manager.selectionClearRetainedBlockIndex) || -1)
                    : -1
            documentBlock.clearSelection(retainedBlockIndex === documentBlock.blockIndex)
        }
    }

    onFocusedChanged: {
        EditorTrace.trace(
                    "documentBlock",
                    "focusedChanged",
                    "blockType=" + documentBlock.blockType + " focused=" + documentBlock.focused,
                    documentBlock)
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

    Connections {
        target: blockLoader.item
        ignoreUnknownSignals: true

        function onActivated() {
            if (documentBlock.atomicBlock)
                documentBlock.forceActiveFocus()
            documentBlock.activated()
        }

        function onAdjacentAtomicBlockDeleteRequested(side) {
            documentBlock.adjacentAtomicBlockDeleteRequested(side)
        }

        function onBoundaryNavigationRequested(axis, side) {
            documentBlock.boundaryNavigationRequested(axis, side)
        }

        function onBlockDeletionRequested(direction) {
            documentBlock.blockDeletionRequested(direction)
        }

        function onDocumentEndEditRequested() {
            documentBlock.documentEndEditRequested()
        }

        function onParagraphSplitRequested(sourceOffset) {
            documentBlock.paragraphSplitRequested(sourceOffset)
        }

        function onSourceMutationRequested(nextBlockSourceText, focusRequest) {
            documentBlock.sourceMutationRequested(nextBlockSourceText, focusRequest)
        }

        function onTaskDoneToggled(openTagStart, openTagEnd, checked) {
            documentBlock.taskDoneToggled(openTagStart, openTagEnd, checked)
        }

        function onTaskEnterRequested(blockData, taskData) {
            documentBlock.taskEnterRequested(blockData, taskData)
        }

        function onTaskTextChanged(taskData, text, cursorPosition) {
            documentBlock.taskTextChanged(taskData, text, cursorPosition)
        }

        function onEnterExitRequested(blockData) {
            documentBlock.enterExitRequested(blockData)
        }

        function onTextChanged(text, cursorPosition) {
            documentBlock.textChanged(text, cursorPosition)
        }
    }

    Component {
        id: textBlockComponent

        ContentsDocumentTextBlock {
            blockData: documentBlock.blockData
            hasAdjacentAtomicBlockAfter: documentBlock.hasAdjacentAtomicBlockAfter
            hasAdjacentAtomicBlockBefore: documentBlock.hasAdjacentAtomicBlockBefore
            hasAdjacentBlockAfter: documentBlock.hasAdjacentBlockAfter
            hasAdjacentBlockBefore: documentBlock.hasAdjacentBlockBefore
            paragraphBoundaryOperationsEnabled: documentBlock.paragraphBoundaryOperationsEnabled
            paragraphMergeableAfter: documentBlock.paragraphMergeableAfter
            paragraphMergeableBefore: documentBlock.paragraphMergeableBefore
            shortcutKeyPressHandler: documentBlock.shortcutKeyPressHandler
            width: documentBlock.width
        }
    }

    Component {
        id: agendaBlockComponent

        ContentsAgendaBlock {
            blockData: documentBlock.blockData
            shortcutKeyPressHandler: documentBlock.shortcutKeyPressHandler
            width: documentBlock.width
        }
    }

    Component {
        id: calloutBlockComponent

        ContentsCalloutBlock {
            blockData: documentBlock.blockData
            shortcutKeyPressHandler: documentBlock.shortcutKeyPressHandler
            width: documentBlock.width
        }
    }

    Component {
        id: breakBlockComponent

        ContentsBreakBlock {
            blockData: documentBlock.blockData
            shortcutKeyPressHandler: documentBlock.shortcutKeyPressHandler
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
            documentBlock.forceActiveFocus()
            documentBlock.activated()
            if (tapCount >= 2)
                documentBlock.documentEndEditRequested()
        }
    }
}
