pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: documentFlow

    property var agendaBackend: null
    property var calloutBackend: null
    property var documentBlocks: []
    property int lineHeightHint: Math.max(1, Math.round(LV.Theme.scaleMetric(12)))
    property var renderedResources: []
    property var shortcutKeyPressHandler: null
    property string sourceText: ""
    property int activeBlockIndex: -1
    property int activeBlockCursorRevision: 0
    property var pendingFocusRequest: null
    property int pendingFocusBlockIndex: -1
    property bool pendingFocusApplyQueued: false
    readonly property bool focused: documentFlow.activeFocus || documentFlow.hasFocusedBlock()
    readonly property int focusedBlockIndexValue: documentFlow.focusedBlockIndex()
    readonly property int resolvedInteractiveBlockIndexValue: {
        const focusedIndex = Number(documentFlow.focusedBlockIndexValue)
        if (isFinite(focusedIndex) && focusedIndex >= 0)
            return Math.floor(focusedIndex)
        const activeIndex = Number(documentFlow.activeBlockIndex)
        if (isFinite(activeIndex) && activeIndex >= 0)
            return Math.floor(activeIndex)
        return -1
    }
    readonly property int currentLogicalLineNumber: documentFlow.activeLogicalLineNumber()

    signal sourceMutationRequested(string nextSourceText, var focusRequest)

    Keys.onPressed: function (event) {
        if (documentFlow.handleActiveBlockDeleteKeyPress(event))
            event.accepted = true
    }

    function blockAtomic(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const delegateItem = documentFlow.delegateItemForBlockHost(host)
        if (delegateItem && delegateItem.atomicBlock !== undefined)
            return !!delegateItem.atomicBlock
        if (blockEntry.atomicBlock !== undefined)
            return !!blockEntry.atomicBlock
        const blockType = blockEntry.type !== undefined ? String(blockEntry.type).trim().toLowerCase() : "text"
        return blockType === "resource" || blockType === "break"
    }

    function blockTextEditable(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const delegateItem = documentFlow.delegateItemForBlockHost(host)
        if (delegateItem && delegateItem.textEditable !== undefined)
            return !!delegateItem.textEditable
        if (blockEntry.textEditable !== undefined)
            return !!blockEntry.textEditable
        return !documentFlow.blockAtomic(host, blockEntry)
    }

    function visiblePlainTextForBlockHost(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const delegateItem = documentFlow.delegateItemForBlockHost(host)
        if (delegateItem && delegateItem.visiblePlainText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(String(delegateItem.visiblePlainText() || ""))
        if (blockEntry.plainText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(String(blockEntry.plainText || ""))
        if (blockEntry.sourceText !== undefined)
            return StructuredCursorSupport.normalizedPlainText(String(blockEntry.sourceText || ""))
        return ""
    }

    function blockRepresentativeCharCountForHost(blockHost, blockEntryOverride, lineText) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const delegateItem = documentFlow.delegateItemForBlockHost(host)
        if (delegateItem && delegateItem.representativeCharCount !== undefined)
            return Math.max(0, Number(delegateItem.representativeCharCount(lineText)) || 0)
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
        if (normalizedLineText.length > 0)
            return normalizedLineText.length
        return Math.max(0, Number(blockEntry.minimapRepresentativeCharCount) || 0)
    }

    function logicalLineCountForBlockHost(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const plainText = documentFlow.visiblePlainTextForBlockHost(host, blockEntry)
        const logicalLines = plainText.length > 0 ? plainText.split("\n") : [""]
        const explicitHint = Math.max(0, Number(blockEntry.logicalLineCountHint) || 0)
        return Math.max(1, explicitHint > 0 ? explicitHint : logicalLines.length)
    }

    function blockGutterCollapsed(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const delegateItem = documentFlow.delegateItemForBlockHost(host)
        if (delegateItem && delegateItem.gutterCollapsed !== undefined)
            return !!delegateItem.gutterCollapsed
        return !!blockEntry.gutterCollapsed
    }

    function blockMinimapVisualKind(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const delegateItem = documentFlow.delegateItemForBlockHost(host)
        if (delegateItem && delegateItem.minimapVisualKind !== undefined)
            return String(delegateItem.minimapVisualKind || "text")
        return blockEntry.minimapVisualKind !== undefined ? String(blockEntry.minimapVisualKind || "text") : "text"
    }

    function logicalLineCount() {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return 1
        let lineCount = 0
        for (let index = 0; index < blocks.length; ++index) {
            const blockHost = blockRepeater.itemAt(index)
            const blockEntry = blocks[index] && typeof blocks[index] === "object" ? blocks[index] : ({})
            lineCount += documentFlow.logicalLineCountForBlockHost(blockHost, blockEntry)
        }
        return Math.max(1, lineCount)
    }

    function visualLineCharCount(blockHost, blockEntry, logicalLines, visualLineCount, visualLineIndex) {
        const safeLogicalLines = Array.isArray(logicalLines) && logicalLines.length > 0 ? logicalLines : [""]
        const safeVisualLineCount = Math.max(1, Math.floor(Number(visualLineCount) || 1))
        const safeVisualLineIndex = Math.max(0, Math.min(safeVisualLineCount - 1, Math.floor(Number(visualLineIndex) || 0)))
        if (safeVisualLineCount === safeLogicalLines.length)
            return Math.max(
                        0,
                        documentFlow.blockRepresentativeCharCountForHost(
                            blockHost,
                            blockEntry,
                            safeLogicalLines[safeVisualLineIndex]))

        let totalCharCount = 0
        for (let index = 0; index < safeLogicalLines.length; ++index)
            totalCharCount += Math.max(0, String(safeLogicalLines[index] || "").length)
        if (totalCharCount <= 0)
            return Math.max(0, documentFlow.blockRepresentativeCharCountForHost(blockHost, blockEntry, ""))

        const baseCount = Math.floor(totalCharCount / safeVisualLineCount)
        const remainder = totalCharCount % safeVisualLineCount
        return Math.max(1, baseCount + (safeVisualLineIndex < remainder ? 1 : 0))
    }

    function normalizedDelegateLineLayoutEntries(delegateItem, expectedLineCount) {
        const safeExpectedLineCount = Math.max(1, Math.floor(Number(expectedLineCount) || 1))
        if (!delegateItem || delegateItem.logicalLineLayoutEntries === undefined)
            return []
        const rawEntries = delegateItem.logicalLineLayoutEntries()
        const normalized = []
        if (Array.isArray(rawEntries)) {
            for (let index = 0; index < rawEntries.length; ++index)
                normalized.push(rawEntries[index])
        } else {
            const explicitLength = Number(rawEntries && rawEntries.length)
            if (isFinite(explicitLength) && explicitLength >= 0) {
                for (let index = 0; index < Math.floor(explicitLength); ++index)
                    normalized.push(rawEntries[index])
            }
        }
        if (normalized.length !== safeExpectedLineCount)
            return []
        return normalized
    }

    function delegateItemForBlockHost(blockHost) {
        return blockHost && blockHost.delegateLoader && blockHost.delegateLoader.item
                ? blockHost.delegateLoader.item
                : null
    }

    function currentLocalLogicalLineNumberForBlockHost(blockHost, blockEntry) {
        const delegateItem = documentFlow.delegateItemForBlockHost(blockHost)
        if (delegateItem && delegateItem.currentLogicalLineNumber !== undefined)
            return Math.max(1, Number(delegateItem.currentLogicalLineNumber) || 1)
        return Math.max(1, documentFlow.logicalLineCountForBlockHost(blockHost, blockEntry))
    }

    function blockHeightForLayout(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        return Math.max(
                    1,
                    host ? (Number(host.implicitHeight) || Number(host.height) || 0) : 0,
                    documentFlow.lineHeightHint * documentFlow.logicalLineCountForBlockHost(host, blockEntry))
    }

    function blockTopYForIndex(blockIndex) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return 0
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        const blockSpacing = Math.max(0, Number(documentColumn && documentColumn.spacing !== undefined ? documentColumn.spacing : 0) || 0)
        let blockTopY = 0
        for (let index = 0; index < safeBlockIndex; ++index) {
            const blockHost = blockRepeater.itemAt(index)
            const blockEntry = blocks[index] && typeof blocks[index] === "object" ? blocks[index] : ({})
            blockTopY += documentFlow.blockHeightForLayout(blockHost, blockEntry)
            blockTopY += blockSpacing
        }
        return blockTopY
    }

    function blockLogicalLineEntries(blockHost, blockEntryOverride, blockBaseYOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const plainText = documentFlow.visiblePlainTextForBlockHost(host, blockEntry)
        const logicalLines = plainText.length > 0 ? plainText.split("\n") : [""]
        const blockHeight = documentFlow.blockHeightForLayout(host, blockEntry)
        const numericBlockBaseY = Number(blockBaseYOverride)
        const blockBaseY = isFinite(numericBlockBaseY)
                ? Math.max(0, numericBlockBaseY)
                : host && host.blockIndex !== undefined
                  ? documentFlow.blockTopYForIndex(host.blockIndex)
                  : Math.max(0, Number(host && host.y !== undefined ? host.y : 0) || 0)
        const entries = []
        const lineCount = documentFlow.logicalLineCountForBlockHost(host, blockEntry)
        const delegateItem = documentFlow.delegateItemForBlockHost(host)
        const delegateLineLayoutEntries = documentFlow.normalizedDelegateLineLayoutEntries(delegateItem, lineCount)

        for (let index = 0; index < lineCount; ++index) {
            const delegateLineLayout = delegateLineLayoutEntries[index] && typeof delegateLineLayoutEntries[index] === "object"
                    ? delegateLineLayoutEntries[index]
                    : null
            const fallbackLocalLineTop = blockHeight * index / lineCount
            const fallbackLocalLineBottom = index + 1 < lineCount
                    ? blockHeight * (index + 1) / lineCount
                    : blockHeight
            const localLineTop = delegateLineLayout && delegateLineLayout.contentY !== undefined
                    ? Math.max(0, Number(delegateLineLayout.contentY) || 0)
                    : fallbackLocalLineTop
            const localLineBottom = delegateLineLayout && delegateLineLayout.contentHeight !== undefined
                    ? Math.max(
                          localLineTop + documentFlow.lineHeightHint,
                          localLineTop + (Number(delegateLineLayout.contentHeight) || 0))
                    : fallbackLocalLineBottom
            const fallbackLineTop = blockBaseY + localLineTop
            const fallbackLineBottom = blockBaseY + Math.max(localLineTop + documentFlow.lineHeightHint, localLineBottom)
            const mappedLineTopPoint = delegateItem && delegateItem.mapToItem !== undefined
                    ? delegateItem.mapToItem(documentFlow, 0, localLineTop)
                    : host && host.mapToItem !== undefined
                      ? host.mapToItem(documentFlow, 0, localLineTop)
                      : ({ "x": 0, "y": fallbackLineTop })
            const mappedLineBottomPoint = delegateItem && delegateItem.mapToItem !== undefined
                    ? delegateItem.mapToItem(
                        documentFlow,
                        0,
                        Math.max(localLineTop + documentFlow.lineHeightHint, localLineBottom))
                    : host && host.mapToItem !== undefined
                      ? host.mapToItem(
                          documentFlow,
                          0,
                          Math.max(localLineTop + documentFlow.lineHeightHint, localLineBottom))
                      : ({ "x": 0, "y": fallbackLineBottom })
            const mappedLineTopY = Number(mappedLineTopPoint && mappedLineTopPoint.y !== undefined ? mappedLineTopPoint.y : fallbackLineTop)
            const mappedLineBottomY = Number(mappedLineBottomPoint && mappedLineBottomPoint.y !== undefined ? mappedLineBottomPoint.y : fallbackLineBottom)
            const lineTop = Math.max(
                        0,
                        fallbackLineTop,
                        isFinite(mappedLineTopY) ? mappedLineTopY : fallbackLineTop)
            const lineBottom = Math.max(
                        lineTop + documentFlow.lineHeightHint,
                        fallbackLineBottom,
                        isFinite(mappedLineBottomY) ? mappedLineBottomY : fallbackLineBottom)
            const lineHeight = Math.max(1, lineBottom - lineTop)
            const gutterCollapsed = documentFlow.blockGutterCollapsed(host, blockEntry)
            const minimapVisualKind = documentFlow.blockMinimapVisualKind(host, blockEntry)
            const minimapRowCharCount = documentFlow.blockRepresentativeCharCountForHost(host, blockEntry, "")
            entries.push({
                "charCount": documentFlow.visualLineCharCount(host, blockEntry, logicalLines, lineCount, index),
                "contentHeight": lineHeight,
                "contentY": lineTop,
                "gutterCollapsed": gutterCollapsed,
                "gutterContentHeight": gutterCollapsed ? Math.max(1, documentFlow.lineHeightHint) : lineHeight,
                "minimapRowCharCount": minimapRowCharCount,
                "minimapVisualKind": minimapVisualKind,
                "rowCount": Math.max(1, Math.round(lineHeight / Math.max(1, documentFlow.lineHeightHint)))
            })
        }

        return entries
    }

    function logicalLineEntries() {
        const entries = []
        const blocks = documentFlow.normalizedBlocks()
        const blockSpacing = Math.max(0, Number(documentColumn && documentColumn.spacing !== undefined ? documentColumn.spacing : 0) || 0)
        let nextBlockBaseY = 0
        let nextGutterContentY = 0
        for (let blockIndex = 0; blockIndex < blockRepeater.count; ++blockIndex) {
            const blockHost = blockRepeater.itemAt(blockIndex)
            const blockEntry = blocks[blockIndex] && typeof blocks[blockIndex] === "object" ? blocks[blockIndex] : ({})
            const blockEntries = documentFlow.blockLogicalLineEntries(blockHost, blockEntry, nextBlockBaseY)
            for (let lineIndex = 0; lineIndex < blockEntries.length; ++lineIndex) {
                const lineEntry = blockEntries[lineIndex] && typeof blockEntries[lineIndex] === "object"
                        ? blockEntries[lineIndex]
                        : ({})
                entries.push({
                    "charCount": Math.max(0, Number(lineEntry.charCount) || 0),
                    "contentHeight": Math.max(1, Number(lineEntry.contentHeight) || documentFlow.lineHeightHint),
                    "contentY": Math.max(0, Number(lineEntry.contentY) || 0),
                    "gutterCollapsed": !!lineEntry.gutterCollapsed,
                    "gutterContentHeight": Math.max(
                                               1,
                                               Number(lineEntry.gutterContentHeight) || documentFlow.lineHeightHint),
                    "gutterContentY": nextGutterContentY,
                    "lineNumber": entries.length + 1,
                    "minimapRowCharCount": Math.max(0, Number(lineEntry.minimapRowCharCount) || 0),
                    "minimapVisualKind": lineEntry.minimapVisualKind !== undefined
                                         ? String(lineEntry.minimapVisualKind)
                                         : "text",
                    "rowCount": Math.max(1, Number(lineEntry.rowCount) || 1)
                })
                nextGutterContentY += Math.max(
                            1,
                            Number(lineEntry.gutterContentHeight) || documentFlow.lineHeightHint)
            }
            nextBlockBaseY += documentFlow.blockHeightForLayout(blockHost, blockEntry)
            if (blockIndex + 1 < blockRepeater.count)
                nextBlockBaseY += blockSpacing
        }

        if (entries.length === 0) {
            entries.push({
                "charCount": 0,
                "contentHeight": Math.max(1, documentFlow.lineHeightHint),
                "contentY": 0,
                "gutterCollapsed": false,
                "gutterContentHeight": Math.max(1, documentFlow.lineHeightHint),
                "gutterContentY": 0,
                "lineNumber": 1,
                "minimapRowCharCount": 0,
                "minimapVisualKind": "text",
                "rowCount": 1
            })
        }

        return entries
    }

    function activeLogicalLineNumber() {
        const cursorRevision = documentFlow.activeBlockCursorRevision
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return 1
        const resolvedActiveBlockIndex = Math.max(-1, Number(documentFlow.resolvedInteractiveBlockIndexValue) || -1)
        const safeActiveBlockIndex = Math.max(0, Math.min(blocks.length - 1, resolvedActiveBlockIndex))
        let lineNumber = 1
        for (let blockIndex = 0; blockIndex < safeActiveBlockIndex; ++blockIndex) {
            const blockHost = blockRepeater.itemAt(blockIndex)
            lineNumber += documentFlow.blockLogicalLineEntries(blockHost, blocks[blockIndex]).length
        }
        const activeBlockHost = blockRepeater.itemAt(safeActiveBlockIndex)
        const activeBlockLineCount = Math.max(1, documentFlow.blockLogicalLineEntries(activeBlockHost, blocks[safeActiveBlockIndex]).length)
        const localLineNumber = Math.max(1, Math.min(
                                             activeBlockLineCount,
                                             documentFlow.currentLocalLogicalLineNumberForBlockHost(
                                                 activeBlockHost,
                                                 blocks[safeActiveBlockIndex])))
        return Math.max(1, lineNumber + localLineNumber - 1)
    }

    function focusedBlockIndex() {
        for (let index = 0; index < blockRepeater.count; ++index) {
            const host = blockRepeater.itemAt(index)
            if (host && host.delegateFocused !== undefined && host.delegateFocused)
                return index
        }
        return -1
    }

    function blockIndexAtPoint(localX, localY) {
        const safeLocalX = Number(localX)
        const safeLocalY = Number(localY)
        if (!isFinite(safeLocalX) || !isFinite(safeLocalY))
            return -1
        for (let index = 0; index < blockRepeater.count; ++index) {
            const host = blockRepeater.itemAt(index)
            if (!host)
                continue
            const hostX = Number(host.x) || 0
            const hostY = Number(host.y) || 0
            const hostWidth = Math.max(0, Number(host.width) || Number(host.implicitWidth) || 0)
            const hostHeight = Math.max(0, Number(host.height) || Number(host.implicitHeight) || 0)
            if (hostWidth <= 0 || hostHeight <= 0)
                continue
            if (safeLocalX < hostX || safeLocalX > hostX + hostWidth)
                continue
            if (safeLocalY < hostY || safeLocalY > hostY + hostHeight)
                continue
            return index
        }
        return -1
    }

    function hasBlockAtPoint(localX, localY) {
        return documentFlow.blockIndexAtPoint(localX, localY) >= 0
    }

    function currentCursorVisualRowRect() {
        const cursorRevision = documentFlow.activeBlockCursorRevision
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return ({
                        "height": Math.max(1, documentFlow.lineHeightHint),
                        "y": 0
                    })
        const resolvedActiveBlockIndex = Math.max(-1, Number(documentFlow.resolvedInteractiveBlockIndexValue) || -1)
        const safeActiveBlockIndex = Math.max(0, Math.min(blocks.length - 1, resolvedActiveBlockIndex))
        const activeBlockHost = blockRepeater.itemAt(safeActiveBlockIndex)
        const delegateItem = documentFlow.delegateItemForBlockHost(activeBlockHost)
        if (delegateItem && delegateItem.currentCursorRowRect !== undefined) {
            const localRect = delegateItem.currentCursorRowRect()
            const localRectY = Math.max(0, Number(localRect && localRect.contentY !== undefined ? localRect.contentY : 0) || 0)
            const fallbackRowY = documentFlow.blockTopYForIndex(safeActiveBlockIndex) + localRectY
            const mappedPoint = delegateItem.mapToItem !== undefined
                    ? delegateItem.mapToItem(
                        documentFlow,
                        0,
                        localRectY)
                    : ({ "x": 0, "y": fallbackRowY })
            const mappedY = Number(mappedPoint && mappedPoint.y !== undefined ? mappedPoint.y : fallbackRowY)
            return {
                "height": Math.max(1, Number(localRect && localRect.contentHeight !== undefined ? localRect.contentHeight : 0) || documentFlow.lineHeightHint),
                "y": Math.max(0, fallbackRowY, isFinite(mappedY) ? mappedY : fallbackRowY)
            }
        }
        const lineNumber = documentFlow.activeLogicalLineNumber()
        const lineEntries = documentFlow.logicalLineEntries()
        const entry = lineEntries.length >= lineNumber && lineNumber > 0 ? lineEntries[lineNumber - 1] : ({})
        return {
            "height": Math.max(
                         1,
                         Number(entry && entry.gutterContentHeight !== undefined ? entry.gutterContentHeight : 0)
                         || documentFlow.lineHeightHint),
            "y": Math.max(0, Number(entry && entry.contentY !== undefined ? entry.contentY : 0) || 0)
        }
    }

    function noteActiveBlockInteraction(blockIndex) {
        const safeBlockIndex = Math.max(-1, Math.floor(Number(blockIndex) || -1))
        if (documentFlow.activeBlockIndex !== safeBlockIndex)
            documentFlow.activeBlockIndex = safeBlockIndex
        documentFlow.activeBlockCursorRevision += 1
    }

    function selectionSnapshotIsValid(selectionSnapshot) {
        if (!selectionSnapshot || typeof selectionSnapshot !== "object")
            return false
        const selectionStart = Number(selectionSnapshot.selectionStart)
        const selectionEnd = Number(selectionSnapshot.selectionEnd)
        return isFinite(selectionStart)
                && isFinite(selectionEnd)
                && Math.floor(selectionEnd) > Math.floor(selectionStart)
    }

    function inlineFormatTargetState() {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return ({ "valid": false })
        const resolvedActiveBlockIndex = Math.max(-1, Number(documentFlow.resolvedInteractiveBlockIndexValue) || -1)
        const safeActiveBlockIndex = Math.max(0, Math.min(blocks.length - 1, resolvedActiveBlockIndex))
        const activeBlockHost = blockRepeater.itemAt(safeActiveBlockIndex)
        const delegateItem = documentFlow.delegateItemForBlockHost(activeBlockHost)
        if (!delegateItem)
            return ({
                        "blockIndex": safeActiveBlockIndex,
                        "selectionSnapshot": ({ }),
                        "valid": false
                    })
        const selectionSnapshot = delegateItem.inlineFormatSelectionSnapshot !== undefined
                ? delegateItem.inlineFormatSelectionSnapshot()
                : delegateItem.selectionSnapshot !== undefined
                  ? delegateItem.selectionSnapshot()
                  : ({})
        return {
            "blockIndex": safeActiveBlockIndex,
            "selectionSnapshot": selectionSnapshot,
            "valid": documentFlow.selectionSnapshotIsValid(selectionSnapshot)
        }
    }

    function applyInlineFormatToBlockSelection(blockIndex, tagName, selectionSnapshot) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return false
        const numericBlockIndex = Number(blockIndex)
        if (!isFinite(numericBlockIndex))
            return false
        const safeActiveBlockIndex = Math.max(
                    0,
                    Math.min(
                        blocks.length - 1,
                        Math.floor(numericBlockIndex)))
        const activeBlockHost = blockRepeater.itemAt(safeActiveBlockIndex)
        const delegateItem = documentFlow.delegateItemForBlockHost(activeBlockHost)
        if (!delegateItem || delegateItem.applyInlineFormatToSelection === undefined)
            return false
        return !!delegateItem.applyInlineFormatToSelection(tagName, selectionSnapshot)
    }

    function applyInlineFormatToActiveSelection(tagName) {
        const targetState = documentFlow.inlineFormatTargetState()
        if (!targetState.valid)
            return false
        return documentFlow.applyInlineFormatToBlockSelection(
                    targetState.blockIndex,
                    tagName,
                    targetState.selectionSnapshot)
    }

    function handleActiveBlockDeleteKeyPress(event) {
        if (!event)
            return false
        const key = Number(event.key)
        if (key !== Qt.Key_Backspace && key !== Qt.Key_Delete)
            return false
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return false
        const resolvedActiveBlockIndex = Math.max(-1, Number(documentFlow.resolvedInteractiveBlockIndexValue) || -1)
        if (resolvedActiveBlockIndex < 0 || resolvedActiveBlockIndex >= blocks.length)
            return false
        const activeBlockHost = blockRepeater.itemAt(resolvedActiveBlockIndex)
        const delegateItem = documentFlow.delegateItemForBlockHost(activeBlockHost)
        if (!delegateItem || delegateItem.handleDeleteKeyPress === undefined)
            return false
        return !!delegateItem.handleDeleteKeyPress(event)
    }

    function hasFocusedBlock() {
        for (let index = 0; index < blockRepeater.count; ++index) {
            const host = blockRepeater.itemAt(index)
            if (host && host.delegateFocused !== undefined && host.delegateFocused)
                return true
        }
        return false
    }

    function requestDocumentEndEdit() {
        const blocks = documentFlow.normalizedBlocks()
        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        if (blocks.length === 0) {
            documentFlow.requestFocus({ "sourceOffset": currentSourceText.length })
            return true
        }
        const lastBlock = blocks[blocks.length - 1] && typeof blocks[blocks.length - 1] === "object"
                ? blocks[blocks.length - 1]
                : ({})
        const lastBlockHost = blockRepeater.itemAt(blocks.length - 1)
        if (documentFlow.blockTextEditable(lastBlockHost, lastBlock)) {
            documentFlow.requestFocus({
                                          "sourceOffset": Math.max(
                                                              0,
                                                              Math.floor(Number(lastBlock.sourceEnd) || currentSourceText.length))
                                      })
            return true
        }
        if (currentSourceText.length > 0 && currentSourceText.charAt(currentSourceText.length - 1) === "\n") {
            documentFlow.requestFocus({ "sourceOffset": currentSourceText.length })
            return true
        }
        documentFlow.sourceMutationRequested(
                    currentSourceText + "\n",
                    { "sourceOffset": currentSourceText.length + 1 })
        return true
    }

    function normalizedBlocks() {
        if (Array.isArray(documentBlocks))
            return documentBlocks

        const explicitLength = documentBlocks ? Number(documentBlocks.length) : NaN
        if (isFinite(explicitLength) && explicitLength >= 0) {
            const normalized = []
            for (let index = 0; index < Math.floor(explicitLength); ++index)
                normalized.push(documentBlocks[index])
            return normalized
        }

        const explicitCount = documentBlocks ? Number(documentBlocks.count) : NaN
        if (isFinite(explicitCount) && explicitCount >= 0) {
            const normalized = []
            for (let index = 0; index < Math.floor(explicitCount); ++index)
                normalized.push(documentBlocks[index])
            return normalized
        }

        if (documentBlocks) {
            const indexedKeys = Object.keys(documentBlocks).filter(function (key) {
                return /^\d+$/.test(key)
            }).sort(function (lhs, rhs) {
                return Number(lhs) - Number(rhs)
            })
            if (indexedKeys.length > 0) {
                const normalized = []
                for (let index = 0; index < indexedKeys.length; ++index)
                    normalized.push(documentBlocks[indexedKeys[index]])
                return normalized
            }
        }

        return []
    }

    function normalizedResourceEntries() {
        if (Array.isArray(renderedResources))
            return renderedResources

        const explicitLength = renderedResources ? Number(renderedResources.length) : NaN
        if (isFinite(explicitLength) && explicitLength >= 0) {
            const normalized = []
            for (let index = 0; index < Math.floor(explicitLength); ++index)
                normalized.push(renderedResources[index])
            return normalized
        }

        const explicitCount = renderedResources ? Number(renderedResources.count) : NaN
        if (isFinite(explicitCount) && explicitCount >= 0) {
            const normalized = []
            for (let index = 0; index < Math.floor(explicitCount); ++index)
                normalized.push(renderedResources[index])
            return normalized
        }

        if (renderedResources) {
            const indexedKeys = Object.keys(renderedResources).filter(function (key) {
                return /^\d+$/.test(key)
            }).sort(function (lhs, rhs) {
                return Number(lhs) - Number(rhs)
            })
            if (indexedKeys.length > 0) {
                const normalized = []
                for (let index = 0; index < indexedKeys.length; ++index)
                    normalized.push(renderedResources[indexedKeys[index]])
                return normalized
            }
        }

        return []
    }

    function normalizedSourceText(value) {
        let text = value === undefined || value === null ? "" : String(value)
        text = text.replace(/\r\n/g, "\n")
        text = text.replace(/\r/g, "\n")
        text = text.replace(/\u2028/g, "\n")
        text = text.replace(/\u2029/g, "\n")
        text = text.replace(/\u00A0/g, " ")
        return text
    }

    function spliceSourceRange(start, end, replacementText) {
        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const safeStart = Math.max(0, Math.min(currentSourceText.length, Math.floor(Number(start) || 0)))
        const safeEnd = Math.max(safeStart, Math.min(currentSourceText.length, Math.floor(Number(end) || 0)))
        const replacement = replacementText === undefined || replacementText === null ? "" : String(replacementText)
        return currentSourceText.slice(0, safeStart) + replacement + currentSourceText.slice(safeEnd)
    }

    function requestFocus(request) {
        const baseRequest = request && typeof request === "object" ? request : ({})
        documentFlow.pendingFocusRequest = Object.assign({}, baseRequest)
        documentFlow.pendingFocusBlockIndex = documentFlow.focusTargetBlockIndex(documentFlow.pendingFocusRequest)
        documentFlow.schedulePendingFocusApply()
    }

    function normalizedFocusTaskOpenTagStart(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const taskOpenTagStart = Number(safeRequest.taskOpenTagStart)
        if (!isFinite(taskOpenTagStart))
            return NaN
        return Math.floor(taskOpenTagStart)
    }

    function normalizedFocusTargetBlockIndex(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const targetBlockIndex = Number(safeRequest.targetBlockIndex)
        if (!isFinite(targetBlockIndex))
            return -1
        return Math.max(0, Math.floor(targetBlockIndex))
    }

    function normalizedFocusSourceOffset(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return NaN
        return Math.max(0, Math.floor(sourceOffset))
    }

    function requestPrefersNearestTextBlock(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        return !!safeRequest.preferNearestTextBlock
    }

    function floorNumberOrFallback(value, fallbackValue) {
        const numericValue = Number(value)
        if (!isFinite(numericValue))
            return fallbackValue
        return Math.floor(numericValue)
    }

    function blockContainsTaskOpenTagStart(blockEntry, taskOpenTagStart) {
        if (!isFinite(taskOpenTagStart))
            return false
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const rawTasks = safeBlock.tasks
        if (!rawTasks || rawTasks.length === undefined)
            return false
        for (let index = 0; index < rawTasks.length; ++index) {
            const taskData = rawTasks[index] && typeof rawTasks[index] === "object" ? rawTasks[index] : ({})
            if (documentFlow.floorNumberOrFallback(taskData.openTagStart, -1) === taskOpenTagStart)
                return true
        }
        return false
    }

    function blockUsesExclusiveTrailingBoundary(blockEntry) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        return documentFlow.blockAtomic(null, safeBlock)
    }

    function blockContainsSourceOffset(blockEntry, sourceOffset) {
        if (!isFinite(sourceOffset))
            return false
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const sourceStart = Math.max(0, Math.floor(Number(safeBlock.sourceStart) || 0))
        const sourceEnd = Math.max(sourceStart, Math.floor(Number(safeBlock.sourceEnd) || sourceStart))
        if (sourceStart === sourceEnd)
            return sourceOffset === sourceStart
        if (documentFlow.blockUsesExclusiveTrailingBoundary(safeBlock))
            return sourceOffset >= sourceStart && sourceOffset < sourceEnd
        return sourceOffset >= sourceStart && sourceOffset <= sourceEnd
    }

    function resourceEntryHasResolvedPayload(entry) {
        const safeEntry = entry && typeof entry === "object" ? entry : ({})
        const entrySource = safeEntry.source !== undefined ? String(safeEntry.source).trim() : ""
        const entryResolvedPath = safeEntry.resolvedPath !== undefined ? String(safeEntry.resolvedPath).trim() : ""
        return entrySource.length > 0 || entryResolvedPath.length > 0
    }

    function resourceEntryForBlock(blockEntry) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const blockResourceIndex = documentFlow.floorNumberOrFallback(safeBlock.resourceIndex, -1)
        const blockSourceStart = Math.max(0, Math.floor(Number(safeBlock.sourceStart) || 0))
        const blockSourceEnd = Math.max(blockSourceStart, Math.floor(Number(safeBlock.sourceEnd) || blockSourceStart))
        const blockResourceId = safeBlock.resourceId !== undefined ? String(safeBlock.resourceId).trim() : ""
        const blockResourcePath = safeBlock.resourcePath !== undefined ? String(safeBlock.resourcePath).trim() : ""
        const resourceEntries = documentFlow.normalizedResourceEntries()
        let fallbackMatch = null

        for (let index = 0; index < resourceEntries.length; ++index) {
            const entry = resourceEntries[index] && typeof resourceEntries[index] === "object" ? resourceEntries[index] : ({})
            const entryIndex = documentFlow.floorNumberOrFallback(entry.index, -1)
            if (blockResourceIndex < 0 || entryIndex !== blockResourceIndex)
                continue
            if (documentFlow.resourceEntryHasResolvedPayload(entry))
                return entry
            if (!fallbackMatch)
                fallbackMatch = entry
        }

        for (let index = 0; index < resourceEntries.length; ++index) {
            const entry = resourceEntries[index] && typeof resourceEntries[index] === "object" ? resourceEntries[index] : ({})
            const entrySourceStart = Math.max(0, Math.floor(Number(entry.sourceStart) || 0))
            const entrySourceEnd = Math.max(entrySourceStart, Math.floor(Number(entry.sourceEnd) || entrySourceStart))
            if (entrySourceStart !== blockSourceStart || entrySourceEnd !== blockSourceEnd)
                continue
            if (documentFlow.resourceEntryHasResolvedPayload(entry))
                return entry
            if (!fallbackMatch)
                fallbackMatch = entry
        }

        for (let index = 0; index < resourceEntries.length; ++index) {
            const entry = resourceEntries[index] && typeof resourceEntries[index] === "object" ? resourceEntries[index] : ({})
            const entryResourceId = entry.resourceId !== undefined ? String(entry.resourceId).trim() : ""
            const entryResourcePath = entry.resourcePath !== undefined ? String(entry.resourcePath).trim() : ""
            const idMatched = blockResourceId.length > 0 && entryResourceId === blockResourceId
            const pathMatched = blockResourcePath.length > 0 && entryResourcePath === blockResourcePath
            if (!idMatched && !pathMatched)
                continue
            if (documentFlow.resourceEntryHasResolvedPayload(entry))
                return entry
            if (!fallbackMatch)
                fallbackMatch = entry
        }

        return fallbackMatch ? fallbackMatch : ({})
    }

    function focusTargetBlockIndex(request) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return -1
        const explicitTargetBlockIndex = documentFlow.normalizedFocusTargetBlockIndex(request)
        if (explicitTargetBlockIndex >= 0 && explicitTargetBlockIndex < blocks.length)
            return explicitTargetBlockIndex
        const taskOpenTagStart = documentFlow.normalizedFocusTaskOpenTagStart(request)
        if (isFinite(taskOpenTagStart)) {
            for (let index = 0; index < blocks.length; ++index) {
                if (documentFlow.blockContainsTaskOpenTagStart(blocks[index], taskOpenTagStart))
                    return index
            }
        }
        const sourceOffset = documentFlow.normalizedFocusSourceOffset(request)
        if (isFinite(sourceOffset)) {
            let fallbackBeforeIndex = -1
            let fallbackAfterIndex = -1
            let fallbackBeforeEditableIndex = -1
            let fallbackAfterEditableIndex = -1
            for (let index = 0; index < blocks.length; ++index) {
                const blockEntry = blocks[index] && typeof blocks[index] === "object" ? blocks[index] : ({})
                if (documentFlow.blockContainsSourceOffset(blockEntry, sourceOffset))
                    return index
                const sourceStart = Math.max(0, Math.floor(Number(blockEntry.sourceStart) || 0))
                const sourceEnd = Math.max(sourceStart, Math.floor(Number(blockEntry.sourceEnd) || sourceStart))
                if (sourceEnd < sourceOffset) {
                    fallbackBeforeIndex = index
                    if (documentFlow.blockTextEditable(blockRepeater.itemAt(index), blockEntry))
                        fallbackBeforeEditableIndex = index
                    continue
                }
                if (sourceStart > sourceOffset && fallbackAfterIndex < 0) {
                    fallbackAfterIndex = index
                    if (documentFlow.blockTextEditable(blockRepeater.itemAt(index), blockEntry))
                        fallbackAfterEditableIndex = index
                }
            }
            if (documentFlow.requestPrefersNearestTextBlock(request)) {
                if (fallbackAfterEditableIndex >= 0)
                    return fallbackAfterEditableIndex
                if (fallbackBeforeEditableIndex >= 0)
                    return fallbackBeforeEditableIndex
                if (fallbackAfterIndex >= 0)
                    return fallbackAfterIndex
                if (fallbackBeforeIndex >= 0)
                    return fallbackBeforeIndex
            }
        }
        if (documentFlow.activeBlockIndex >= 0 && documentFlow.activeBlockIndex < blocks.length)
            return documentFlow.activeBlockIndex
        return -1
    }

    function refreshPendingFocusBlockIndex() {
        if (!documentFlow.pendingFocusRequest || typeof documentFlow.pendingFocusRequest !== "object") {
            if (documentFlow.pendingFocusBlockIndex !== -1)
                documentFlow.pendingFocusBlockIndex = -1
            return -1
        }
        const nextIndex = documentFlow.focusTargetBlockIndex(documentFlow.pendingFocusRequest)
        if (documentFlow.pendingFocusBlockIndex !== nextIndex)
            documentFlow.pendingFocusBlockIndex = nextIndex
        return nextIndex
    }

    function applyFocusToBlockIndex(blockIndex) {
        const request = documentFlow.pendingFocusRequest
        if (!request || typeof request !== "object")
            return false
        const safeIndex = Math.max(-1, documentFlow.floorNumberOrFallback(blockIndex, -1))
        if (safeIndex < 0 || safeIndex >= blockRepeater.count)
            return false
        const host = blockRepeater.itemAt(safeIndex)
        const loader = host && host.delegateLoader ? host.delegateLoader : null
        const delegateItem = loader && loader.item ? loader.item : null
        if (!delegateItem || delegateItem.applyFocusRequest === undefined)
            return false
        if (!delegateItem.applyFocusRequest(request))
            return false
        documentFlow.pendingFocusRequest = null
        documentFlow.pendingFocusBlockIndex = -1
        documentFlow.activeBlockIndex = safeIndex
        return true
    }

    function schedulePendingFocusApply() {
        if (!documentFlow.pendingFocusRequest || typeof documentFlow.pendingFocusRequest !== "object")
            return
        if (documentFlow.pendingFocusApplyQueued)
            return
        documentFlow.pendingFocusApplyQueued = true
        Qt.callLater(function () {
            documentFlow.pendingFocusApplyQueued = false
            if (!documentFlow.pendingFocusRequest || typeof documentFlow.pendingFocusRequest !== "object")
                return
            documentFlow.applyPendingFocus()
        })
    }

    function applyPendingFocus() {
        const targetBlockIndex = documentFlow.pendingFocusBlockIndex >= 0
                               ? documentFlow.pendingFocusBlockIndex
                               : documentFlow.refreshPendingFocusBlockIndex()
        return documentFlow.applyFocusToBlockIndex(targetBlockIndex)
    }

    function replaceSourceRange(start, end, replacementText, focusRequest) {
        documentFlow.sourceMutationRequested(
                    documentFlow.spliceSourceRange(start, end, replacementText),
                    focusRequest && typeof focusRequest === "object" ? focusRequest : ({ }))
    }

    function blockIndexForEntry(blockData) {
        const blocks = documentFlow.normalizedBlocks()
        const safeBlock = blockData && typeof blockData === "object" ? blockData : ({})
        const targetSourceStart = Math.max(0, Math.floor(Number(safeBlock.sourceStart) || 0))
        const targetSourceEnd = Math.max(targetSourceStart, Math.floor(Number(safeBlock.sourceEnd) || targetSourceStart))
        const targetType = safeBlock.type !== undefined ? String(safeBlock.type).trim().toLowerCase() : ""
        for (let index = 0; index < blocks.length; ++index) {
            const blockEntry = blocks[index] && typeof blocks[index] === "object" ? blocks[index] : ({})
            if (blockEntry === safeBlock)
                return index
            const blockSourceStart = Math.max(0, Math.floor(Number(blockEntry.sourceStart) || 0))
            const blockSourceEnd = Math.max(blockSourceStart, Math.floor(Number(blockEntry.sourceEnd) || blockSourceStart))
            const blockType = blockEntry.type !== undefined ? String(blockEntry.type).trim().toLowerCase() : ""
            if (blockSourceStart === targetSourceStart && blockSourceEnd === targetSourceEnd && blockType === targetType)
                return index
        }
        const activeIndex = Math.floor(Number(documentFlow.activeBlockIndex) || -1)
        if (activeIndex >= 0 && activeIndex < blocks.length)
            return activeIndex
        return -1
    }

    function focusRequestAfterBlockDeletion(blockData, nextSourceText) {
        const safeBlock = blockData && typeof blockData === "object" ? blockData : ({})
        const boundedNextSourceText = documentFlow.normalizedSourceText(nextSourceText)
        const blockSourceStart = Math.max(0, Math.floor(Number(safeBlock.sourceStart) || 0))
        const blockSourceEnd = Math.max(blockSourceStart, Math.floor(Number(safeBlock.sourceEnd) || blockSourceStart))
        const blockIndex = documentFlow.blockIndexForEntry(safeBlock)
        const deletedLength = Math.max(0, blockSourceEnd - blockSourceStart)
        const previousEditableOffset = blockIndex >= 0
                ? Math.max(-1, documentFlow.previousEditableBlockFocusSourceOffset(blockIndex))
                : -1
        const nextEditableOffset = blockIndex >= 0
                ? Math.max(-1, documentFlow.nextEditableBlockFocusSourceOffset(blockIndex))
                : -1
        const adjustedNextEditableOffset = nextEditableOffset >= 0
                ? Math.max(0, nextEditableOffset - deletedLength)
                : -1
        if (adjustedNextEditableOffset >= 0) {
            return {
                "preferNearestTextBlock": true,
                "sourceOffset": Math.min(boundedNextSourceText.length, adjustedNextEditableOffset)
            }
        }
        if (previousEditableOffset >= 0) {
            return {
                "preferNearestTextBlock": true,
                "sourceOffset": Math.min(boundedNextSourceText.length, previousEditableOffset)
            }
        }
        return {
            "preferNearestTextBlock": true,
            "sourceOffset": Math.min(boundedNextSourceText.length, blockSourceStart)
        }
    }

    function normalizedDeletionDirection(direction) {
        const normalizedDirection = direction === undefined || direction === null
                ? ""
                : String(direction).trim().toLowerCase()
        return normalizedDirection === "forward" ? "forward" : "backward"
    }

    function emptyTextBlockDeletionRange(blockData, direction, sourceText) {
        const safeBlock = blockData && typeof blockData === "object" ? blockData : ({})
        const currentSourceText = documentFlow.normalizedSourceText(sourceText)
        const anchorOffset = Math.max(
                    0,
                    Math.min(
                        currentSourceText.length,
                        Math.floor(Number(safeBlock.sourceStart) || 0)))
        const previousNewlineStart = anchorOffset > 0 && currentSourceText.charAt(anchorOffset - 1) === "\n"
                ? anchorOffset - 1
                : -1
        const nextNewlineStart = anchorOffset < currentSourceText.length && currentSourceText.charAt(anchorOffset) === "\n"
                ? anchorOffset
                : -1
        const normalizedDirection = documentFlow.normalizedDeletionDirection(direction)

        let deletionStart = -1
        if (normalizedDirection === "forward")
            deletionStart = nextNewlineStart >= 0 ? nextNewlineStart : previousNewlineStart
        else
            deletionStart = previousNewlineStart >= 0 ? previousNewlineStart : nextNewlineStart

        if (deletionStart < 0)
            return null

        return {
            "end": deletionStart + 1,
            "focusRequest": {
                "preferNearestTextBlock": true,
                "sourceOffset": deletionStart
            },
            "start": deletionStart
        }
    }

    function previousEditableBlockFocusSourceOffset(blockIndex) {
        const blocks = documentFlow.normalizedBlocks()
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        for (let index = safeBlockIndex - 1; index >= 0; --index) {
            const blockEntry = blocks[index] && typeof blocks[index] === "object" ? blocks[index] : ({})
            if (!documentFlow.blockTextEditable(blockRepeater.itemAt(index), blockEntry))
                continue
            return Math.max(0, Math.floor(Number(blockEntry.sourceEnd) || 0))
        }
        return -1
    }

    function nextEditableBlockFocusSourceOffset(blockIndex) {
        const blocks = documentFlow.normalizedBlocks()
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        for (let index = safeBlockIndex + 1; index < blocks.length; ++index) {
            const blockEntry = blocks[index] && typeof blocks[index] === "object" ? blocks[index] : ({})
            if (!documentFlow.blockTextEditable(blockRepeater.itemAt(index), blockEntry))
                continue
            return Math.max(0, Math.floor(Number(blockEntry.sourceStart) || 0))
        }
        return -1
    }

    function nextEditableSourceOffsetAfterBlock(sourceText, blockEndOffset) {
        const normalizedText = documentFlow.normalizedSourceText(sourceText)
        const boundedBlockEndOffset = Math.max(
                    0,
                    Math.min(normalizedText.length, Math.floor(Number(blockEndOffset) || 0)))
        if (boundedBlockEndOffset < normalizedText.length
                && normalizedText.charAt(boundedBlockEndOffset) === "\n") {
            return boundedBlockEndOffset + 1
        }
        return boundedBlockEndOffset
    }

    function deleteBlock(blockData, direction) {
        const safeBlock = blockData && typeof blockData === "object" ? blockData : ({})
        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const blockSourceStart = Math.max(0, Math.min(currentSourceText.length, Math.floor(Number(safeBlock.sourceStart) || 0)))
        const blockSourceEnd = Math.max(blockSourceStart, Math.min(currentSourceText.length, Math.floor(Number(safeBlock.sourceEnd) || blockSourceStart)))
        let deletionStart = blockSourceStart
        let deletionEnd = blockSourceEnd
        let focusRequest = null

        if (deletionEnd <= deletionStart) {
            const blockWrapperStart = Math.max(
                        0,
                        Math.min(
                            currentSourceText.length,
                            Math.floor(Number(safeBlock.blockSourceStart) || 0)))
            const blockWrapperEnd = Math.max(
                        blockWrapperStart,
                        Math.min(
                            currentSourceText.length,
                            Math.floor(Number(safeBlock.blockSourceEnd) || blockWrapperStart)))
            if (blockWrapperEnd > blockWrapperStart) {
                deletionStart = blockWrapperStart
                deletionEnd = blockWrapperEnd
            } else {
                const emptyDeletionRange = documentFlow.emptyTextBlockDeletionRange(
                            safeBlock,
                            direction,
                            currentSourceText)
                if (!emptyDeletionRange)
                    return false
                deletionStart = Math.max(0, Math.floor(Number(emptyDeletionRange.start) || 0))
                deletionEnd = Math.max(deletionStart, Math.floor(Number(emptyDeletionRange.end) || deletionStart))
                focusRequest = emptyDeletionRange.focusRequest && typeof emptyDeletionRange.focusRequest === "object"
                        ? emptyDeletionRange.focusRequest
                        : null
            }
        }

        if (deletionEnd <= deletionStart)
            return false
        const nextSourceText = documentFlow.spliceSourceRange(deletionStart, deletionEnd, "")
        documentFlow.sourceMutationRequested(
                    nextSourceText,
                    focusRequest && typeof focusRequest === "object"
                    ? focusRequest
                    : documentFlow.focusRequestAfterBlockDeletion({
                                                                       "sourceEnd": deletionEnd,
                                                                       "sourceStart": deletionStart
                                                                   }, nextSourceText))
        return true
    }

    function blockIsAtomicByIndex(blockIndex) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return false
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        const blockEntry = blocks[safeBlockIndex] && typeof blocks[safeBlockIndex] === "object"
                ? blocks[safeBlockIndex]
                : ({})
        return documentFlow.blockAtomic(blockRepeater.itemAt(safeBlockIndex), blockEntry)
    }

    function blockEntryIsAtomicDeletionTarget(blockEntry) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        return documentFlow.blockAtomic(null, safeBlock)
    }

    function blockEntryIsAtomicFocusTarget(blockEntry) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        return documentFlow.blockAtomic(null, safeBlock)
    }

    function blockHasAdjacentAtomicDeletionTarget(blockIndex, side) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return false
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        const normalizedSide = side === undefined || side === null ? "" : String(side).trim().toLowerCase()
        if (normalizedSide !== "before" && normalizedSide !== "after")
            return false
        const adjacentBlockIndex = normalizedSide === "before" ? safeBlockIndex - 1 : safeBlockIndex + 1
        if (adjacentBlockIndex < 0 || adjacentBlockIndex >= blocks.length)
            return false
        return documentFlow.blockIsAtomicByIndex(adjacentBlockIndex)
    }

    function adjacentBlockIndex(blockIndex, side) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return -1
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        const normalizedSide = side === undefined || side === null ? "" : String(side).trim().toLowerCase()
        if (normalizedSide !== "before" && normalizedSide !== "after")
            return -1
        const resolvedAdjacentBlockIndex = normalizedSide === "before" ? safeBlockIndex - 1 : safeBlockIndex + 1
        if (resolvedAdjacentBlockIndex < 0 || resolvedAdjacentBlockIndex >= blocks.length)
            return -1
        return resolvedAdjacentBlockIndex
    }

    function requestAtomicBlockFocus(blockIndex) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return false
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        const blockEntry = blocks[safeBlockIndex] && typeof blocks[safeBlockIndex] === "object"
                ? blocks[safeBlockIndex]
                : ({})
        if (!documentFlow.blockEntryIsAtomicFocusTarget(blockEntry))
            return false
        const focusSourceOffset = Math.max(
                    0,
                    documentFlow.floorNumberOrFallback(
                        blockEntry.sourceStart,
                        documentFlow.floorNumberOrFallback(blockEntry.focusSourceOffset, 0)))
        documentFlow.requestFocus({
                                      "targetBlockIndex": safeBlockIndex,
                                      "sourceOffset": focusSourceOffset
                                  })
        return true
    }

    function requestBoundaryFocusForBlock(blockIndex, boundarySide) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return false
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        const normalizedBoundarySide = boundarySide === undefined || boundarySide === null
                ? ""
                : String(boundarySide).trim().toLowerCase()
        if (normalizedBoundarySide !== "before" && normalizedBoundarySide !== "after")
            return false
        const blockEntry = blocks[safeBlockIndex] && typeof blocks[safeBlockIndex] === "object"
                ? blocks[safeBlockIndex]
                : ({})
        if (documentFlow.blockEntryIsAtomicFocusTarget(blockEntry))
            return documentFlow.requestAtomicBlockFocus(safeBlockIndex)
        const focusSourceOffset = normalizedBoundarySide === "before"
                ? Math.max(0, documentFlow.floorNumberOrFallback(blockEntry.sourceStart, 0))
                : Math.max(0, documentFlow.floorNumberOrFallback(blockEntry.sourceEnd, 0))
        documentFlow.requestFocus({
                                      "targetBlockIndex": safeBlockIndex,
                                      "entryBoundary": normalizedBoundarySide,
                                      "sourceOffset": Math.max(0, focusSourceOffset)
                                  })
        return true
    }

    function deleteAdjacentAtomicBlock(blockIndex, side) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return false
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        const normalizedSide = side === undefined || side === null ? "" : String(side).trim().toLowerCase()
        if (normalizedSide !== "before" && normalizedSide !== "after")
            return false
        const adjacentBlockIndex = normalizedSide === "before" ? safeBlockIndex - 1 : safeBlockIndex + 1
        if (adjacentBlockIndex < 0 || adjacentBlockIndex >= blocks.length)
            return false

        const currentBlock = blocks[safeBlockIndex] && typeof blocks[safeBlockIndex] === "object" ? blocks[safeBlockIndex] : ({})
        const adjacentBlock = blocks[adjacentBlockIndex] && typeof blocks[adjacentBlockIndex] === "object" ? blocks[adjacentBlockIndex] : ({})
        if (!documentFlow.blockEntryIsAtomicDeletionTarget(adjacentBlock))
            return false

        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const adjacentSourceStart = Math.max(0, Math.min(currentSourceText.length, Math.floor(Number(adjacentBlock.sourceStart) || 0)))
        const adjacentSourceEnd = Math.max(adjacentSourceStart, Math.min(currentSourceText.length, Math.floor(Number(adjacentBlock.sourceEnd) || adjacentSourceStart)))
        if (adjacentSourceEnd <= adjacentSourceStart)
            return false

        const deletedLength = adjacentSourceEnd - adjacentSourceStart
        const nextSourceText = documentFlow.spliceSourceRange(adjacentSourceStart, adjacentSourceEnd, "")
        const currentBlockSourceStart = Math.max(0, Math.floor(Number(currentBlock.sourceStart) || 0))
        const currentBlockSourceEnd = Math.max(currentBlockSourceStart, Math.floor(Number(currentBlock.sourceEnd) || currentBlockSourceStart))
        const focusSourceOffset = normalizedSide === "before"
                ? Math.max(0, currentBlockSourceStart - deletedLength)
                : Math.min(nextSourceText.length, currentBlockSourceEnd)
        documentFlow.sourceMutationRequested(
                    nextSourceText,
                    {
                        "preferNearestTextBlock": true,
                        "sourceOffset": focusSourceOffset
                    })
        return true
    }

    function navigateDocumentBoundary(blockIndex, axis, side) {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return false
        const safeBlockIndex = Math.max(0, Math.min(blocks.length - 1, Math.floor(Number(blockIndex) || 0)))
        const normalizedAxis = axis === undefined || axis === null ? "" : String(axis).trim().toLowerCase()
        const normalizedSide = side === undefined || side === null ? "" : String(side).trim().toLowerCase()
        if ((normalizedAxis !== "horizontal" && normalizedAxis !== "vertical")
                || (normalizedSide !== "before" && normalizedSide !== "after")) {
            return false
        }
        const targetBlockIndex = documentFlow.adjacentBlockIndex(safeBlockIndex, normalizedSide)
        if (targetBlockIndex < 0)
            return false
        const targetBoundarySide = normalizedSide === "before" ? "after" : "before"
        return documentFlow.requestBoundaryFocusForBlock(targetBlockIndex, targetBoundarySide)
    }

    function replaceTextBlock(blockData, nextBlockSourceText, focusRequest) {
        const safeBlock = blockData && typeof blockData === "object" ? blockData : ({})
        documentFlow.replaceSourceRange(
                    Number(safeBlock.sourceStart) || 0,
                    Number(safeBlock.sourceEnd) || 0,
                    documentFlow.normalizedSourceText(nextBlockSourceText),
                    focusRequest)
    }

    function updateAgendaTaskText(taskData, text, cursorPosition) {
        const safeTask = taskData && typeof taskData === "object" ? taskData : ({})
        const contentStart = Number(safeTask.contentStart)
        const contentEnd = Number(safeTask.contentEnd)
        if (!isFinite(contentStart) || !isFinite(contentEnd) || contentEnd < contentStart)
            return false
        const normalizedText = StructuredCursorSupport.normalizedPlainText(text)
        const localCursorPosition = StructuredCursorSupport.clampedPlainCursor(normalizedText, cursorPosition)
        documentFlow.replaceSourceRange(
                    contentStart,
                    contentEnd,
                    StructuredCursorSupport.replacementSourceText(normalizedText),
                    {
                        "localCursorPosition": localCursorPosition,
                        "sourceOffset": StructuredCursorSupport.sourceOffsetForPlainCursor(normalizedText, localCursorPosition, contentStart),
                        "taskOpenTagStart": documentFlow.floorNumberOrFallback(safeTask.openTagStart, -1)
                    })
        return true
    }

    function toggleAgendaTaskDone(openTagStart, openTagEnd, checked) {
        if (!documentFlow.agendaBackend || documentFlow.agendaBackend.rewriteTaskDoneAttribute === undefined)
            return false
        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const nextSourceText = String(documentFlow.agendaBackend.rewriteTaskDoneAttribute(
                                          currentSourceText,
                                          Math.floor(Number(openTagStart) || 0),
                                          Math.floor(Number(openTagEnd) || 0),
                                          !!checked))
        if (nextSourceText === currentSourceText)
            return false
        documentFlow.sourceMutationRequested(nextSourceText, {
                        "taskOpenTagStart": documentFlow.floorNumberOrFallback(openTagStart, -1)
                    })
        return true
    }

    function agendaTaskReturn(taskData) {
        if (!documentFlow.agendaBackend || documentFlow.agendaBackend.detectAgendaTaskEnterReplacement === undefined)
            return false
        const safeTask = taskData && typeof taskData === "object" ? taskData : ({})
        const insertionOffset = Math.max(0, Math.floor(Number(safeTask.contentEnd) || 0))
        const payload = documentFlow.agendaBackend.detectAgendaTaskEnterReplacement(
                    documentFlow.sourceText,
                    insertionOffset,
                    insertionOffset,
                    "\n")
        if (!payload || !payload.applied)
            return false
        documentFlow.replaceSourceRange(
                    Number(payload.replacementSourceStart) || 0,
                    Number(payload.replacementSourceEnd) || 0,
                    String(payload.replacementSourceText || ""),
                    {
                        "sourceOffset": Math.max(0, (Number(payload.replacementSourceStart) || 0) + (Number(payload.cursorSourceOffsetFromReplacementStart) || 0))
                    })
        return true
    }

    function updateCalloutText(blockData, text, cursorPosition) {
        const safeBlock = blockData && typeof blockData === "object" ? blockData : ({})
        const contentStart = Number(safeBlock.contentStart)
        const contentEnd = Number(safeBlock.contentEnd)
        if (!isFinite(contentStart) || !isFinite(contentEnd) || contentEnd < contentStart)
            return false
        const normalizedText = StructuredCursorSupport.normalizedPlainText(text)
        const localCursorPosition = StructuredCursorSupport.clampedPlainCursor(normalizedText, cursorPosition)
        documentFlow.replaceSourceRange(
                    contentStart,
                    contentEnd,
                    StructuredCursorSupport.replacementSourceText(normalizedText),
                    {
                        "localCursorPosition": localCursorPosition,
                        "sourceOffset": StructuredCursorSupport.sourceOffsetForPlainCursor(normalizedText, localCursorPosition, contentStart)
                    })
        return true
    }

    function exitCallout(blockData) {
        if (!documentFlow.calloutBackend || documentFlow.calloutBackend.detectCalloutEnterReplacement === undefined)
            return false
        const safeBlock = blockData && typeof blockData === "object" ? blockData : ({})
        const insertionOffset = Math.max(0, Math.floor(Number(safeBlock.contentEnd) || 0))
        const payload = documentFlow.calloutBackend.detectCalloutEnterReplacement(
                    documentFlow.sourceText,
                    insertionOffset,
                    insertionOffset,
                    "\n")
        if (!payload || !payload.applied)
            return false
        documentFlow.replaceSourceRange(
                    Number(payload.replacementSourceStart) || 0,
                    Number(payload.replacementSourceEnd) || 0,
                    String(payload.replacementSourceText || ""),
                    {
                        "sourceOffset": Math.max(0, (Number(payload.replacementSourceStart) || 0) + (Number(payload.cursorSourceOffsetFromReplacementStart) || 0))
                    })
        return true
    }

    function structuredShortcutSpec(shortcutKind) {
        const normalizedKind = shortcutKind === undefined || shortcutKind === null ? "" : String(shortcutKind).toLowerCase()
        if (normalizedKind === "agenda") {
            if (!documentFlow.agendaBackend || documentFlow.agendaBackend.buildAgendaInsertionPayload === undefined)
                return ({ "applied": false })
            const payload = documentFlow.agendaBackend.buildAgendaInsertionPayload(false, "")
            return {
                "applied": !!payload.applied,
                "cursorSourceOffsetFromInsertionStart": Math.max(0, Number(payload.cursorSourceOffsetFromInsertionStart) || 0),
                "insertionSourceText": String(payload.insertionSourceText || "")
            }
        }
        if (normalizedKind === "callout") {
            if (!documentFlow.calloutBackend || documentFlow.calloutBackend.buildCalloutInsertionPayload === undefined)
                return ({ "applied": false })
            const payload = documentFlow.calloutBackend.buildCalloutInsertionPayload("")
            return {
                "applied": !!payload.applied,
                "cursorSourceOffsetFromInsertionStart": Math.max(0, Number(payload.cursorSourceOffsetFromInsertionStart) || 0),
                "insertionSourceText": String(payload.insertionSourceText || "")
            }
        }
        if (normalizedKind === "break") {
            return {
                "applied": true,
                "cursorSourceOffsetFromInsertionStart": "</break>".length,
                "insertionSourceText": "</break>"
            }
        }
        return ({ "applied": false })
    }

    function shortcutInsertionSourceOffset() {
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return documentFlow.normalizedSourceText(documentFlow.sourceText).length
        const safeIndex = Math.max(0, Math.min(blocks.length - 1, documentFlow.activeBlockIndex >= 0 ? documentFlow.activeBlockIndex : blocks.length - 1))
        const blockHost = blockRepeater.itemAt(safeIndex)
        const delegateLoader = blockHost && blockHost.delegateLoader ? blockHost.delegateLoader : null
        const delegateItem = delegateLoader ? delegateLoader.item : null
        if (delegateItem && delegateItem.shortcutInsertionSourceOffset !== undefined) {
            const delegateOffset = Number(delegateItem.shortcutInsertionSourceOffset())
            if (isFinite(delegateOffset))
                return Math.max(0, Math.min(documentFlow.normalizedSourceText(documentFlow.sourceText).length, Math.floor(delegateOffset)))
        }
        const block = blocks[safeIndex] && typeof blocks[safeIndex] === "object" ? blocks[safeIndex] : ({})
        return Math.max(0, Math.floor(Number(block.sourceEnd) || 0))
    }

    function insertStructuredShortcutAtActivePosition(shortcutKind) {
        const insertionSpec = documentFlow.structuredShortcutSpec(shortcutKind)
        if (!insertionSpec.applied)
            return false
        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const insertionOffset = Math.max(0, Math.min(currentSourceText.length, documentFlow.shortcutInsertionSourceOffset()))
        const prefixNewline = insertionOffset > 0 && currentSourceText.charAt(insertionOffset - 1) !== "\n" ? "\n" : ""
        const suffixNewline = insertionOffset < currentSourceText.length && currentSourceText.charAt(insertionOffset) !== "\n" ? "\n" : ""
        const insertionSourceText = prefixNewline + String(insertionSpec.insertionSourceText || "") + suffixNewline
        documentFlow.replaceSourceRange(
                    insertionOffset,
                    insertionOffset,
                    insertionSourceText,
                    {
                        "sourceOffset": insertionOffset + prefixNewline.length + Math.max(0, Number(insertionSpec.cursorSourceOffsetFromInsertionStart) || 0)
                    })
        return true
    }

    function insertResourceBlocksAtActivePosition(tagTexts) {
        if (!Array.isArray(tagTexts) || tagTexts.length === 0)
            return false
        const normalizedTagTexts = []
        for (let index = 0; index < tagTexts.length; ++index) {
            const tagText = tagTexts[index] === undefined || tagTexts[index] === null ? "" : String(tagTexts[index]).trim()
            if (tagText.length > 0)
                normalizedTagTexts.push(tagText)
        }
        if (normalizedTagTexts.length === 0)
            return false

        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const insertionOffset = Math.max(0, Math.min(currentSourceText.length, documentFlow.shortcutInsertionSourceOffset()))
        const prefixNewline = insertionOffset > 0 && currentSourceText.charAt(insertionOffset - 1) !== "\n" ? "\n" : ""
        const blockSourceText = normalizedTagTexts.join("\n")
        const suffixOffset = insertionOffset < currentSourceText.length ? insertionOffset : currentSourceText.length
        const suffixNewline = suffixOffset < currentSourceText.length
                && currentSourceText.charAt(suffixOffset) !== "\n"
                ? "\n"
                : ""
        const insertionSourceText = prefixNewline + blockSourceText + suffixNewline
        const nextSourceText = documentFlow.spliceSourceRange(
                    insertionOffset,
                    insertionOffset,
                    insertionSourceText)
        const insertedBlockEndOffset = insertionOffset + prefixNewline.length + blockSourceText.length

        documentFlow.sourceMutationRequested(
                    nextSourceText,
                    suffixNewline.length > 0
                    ? {
                        "sourceOffset": documentFlow.nextEditableSourceOffsetAfterBlock(
                                            nextSourceText,
                                            insertedBlockEndOffset)
                    }
                    : {
                        "sourceOffset": Math.max(
                                            insertionOffset + prefixNewline.length,
                                            insertedBlockEndOffset - 1)
                    })
        return true
    }

    implicitHeight: documentColumn.implicitHeight
    width: parent ? parent.width : implicitWidth

    Column {
        id: documentColumn

        anchors.left: parent.left
        anchors.right: parent.right
        spacing: Math.max(0, Math.round(LV.Theme.scaleMetric(10)))

        Repeater {
            id: blockRepeater

            model: documentFlow.normalizedBlocks()

            delegate: Item {
                id: blockHost

                required property int index
                required property var modelData
                property alias delegateLoader: blockLoader
                readonly property int blockIndex: index
                readonly property var blockEntry: modelData && typeof modelData === "object" ? modelData : ({})
                readonly property bool delegateFocused: {
                    const delegateItem = blockLoader.item
                    if (!delegateItem)
                        return false
                    if (delegateItem.focused !== undefined)
                        return !!delegateItem.focused
                    return delegateItem.activeFocus !== undefined && !!delegateItem.activeFocus
                }
                implicitHeight: blockLoader.item ? Math.max(0, Number(blockLoader.item.implicitHeight) || Number(blockLoader.item.height) || 0) : 0
                width: documentColumn.width
                height: implicitHeight

                Loader {
                    id: blockLoader

                    anchors.fill: parent
                    asynchronous: blockRepeater.count > 12
                    sourceComponent: documentBlockDelegate

                    onLoaded: {
                        if (!item || blockHost.blockIndex !== documentFlow.pendingFocusBlockIndex)
                            return
                        documentFlow.schedulePendingFocusApply()
                    }
                }

                Component {
                    id: documentBlockDelegate

                    ContentsDocumentBlock {
                        blockData: blockHost.blockEntry
                        hasAdjacentAtomicBlockAfter: documentFlow.blockHasAdjacentAtomicDeletionTarget(blockHost.blockIndex, "after")
                        hasAdjacentAtomicBlockBefore: documentFlow.blockHasAdjacentAtomicDeletionTarget(blockHost.blockIndex, "before")
                        hasAdjacentBlockAfter: documentFlow.adjacentBlockIndex(blockHost.blockIndex, "after") >= 0
                        hasAdjacentBlockBefore: documentFlow.adjacentBlockIndex(blockHost.blockIndex, "before") >= 0
                        resourceEntry: documentFlow.resourceEntryForBlock(blockHost.blockEntry)
                        shortcutKeyPressHandler: documentFlow.shortcutKeyPressHandler
                        width: blockHost.width

                        onActivated: documentFlow.noteActiveBlockInteraction(blockHost.blockIndex)
                        onAdjacentAtomicBlockDeleteRequested: function (side) {
                            documentFlow.deleteAdjacentAtomicBlock(blockHost.blockIndex, side)
                        }
                        onBoundaryNavigationRequested: function (axis, side) {
                            documentFlow.navigateDocumentBoundary(blockHost.blockIndex, axis, side)
                        }
                        onBlockDeletionRequested: function (direction) {
                            documentFlow.deleteBlock(blockHost.blockEntry, direction)
                        }
                        onDocumentEndEditRequested: documentFlow.requestDocumentEndEdit()
                        onSourceMutationRequested: function (nextBlockSourceText, focusRequest) {
                            documentFlow.replaceTextBlock(blockHost.blockEntry, nextBlockSourceText, focusRequest)
                        }
                        onTaskDoneToggled: function (openTagStart, openTagEnd, checked) {
                            documentFlow.toggleAgendaTaskDone(openTagStart, openTagEnd, checked)
                        }
                        onTaskEnterRequested: function (_blockData, taskData) {
                            documentFlow.agendaTaskReturn(taskData)
                        }
                        onTaskTextChanged: function (taskData, text, cursorPosition) {
                            documentFlow.updateAgendaTaskText(taskData, text, cursorPosition)
                        }
                        onEnterExitRequested: function (blockData) {
                            documentFlow.exitCallout(blockData)
                        }
                        onTextChanged: function (text, cursorPosition) {
                            documentFlow.updateCalloutText(blockHost.blockEntry, text, cursorPosition)
                        }
                    }
                }
            }
        }
    }

    onDocumentBlocksChanged: {
        if (!documentFlow.pendingFocusRequest || typeof documentFlow.pendingFocusRequest !== "object")
            return
        documentFlow.refreshPendingFocusBlockIndex()
        documentFlow.schedulePendingFocusApply()
    }
}
