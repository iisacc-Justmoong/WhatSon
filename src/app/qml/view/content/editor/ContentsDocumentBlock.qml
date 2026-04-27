pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "../../../../models/editor/input" as EditorInputModel

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

    EditorInputModel.ContentsDocumentBlockController {
        id: documentBlockController

        blockLoader: blockLoader
        documentBlock: documentBlock
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
