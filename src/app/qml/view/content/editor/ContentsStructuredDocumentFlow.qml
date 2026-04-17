pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "ContentsEditorDebugTrace.js" as EditorTrace
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: documentFlow
    objectName: "contentsStructuredDocumentFlow"

    property var agendaBackend: null
    property var calloutBackend: null
    property int lineHeightHint: Math.max(1, Math.round(LV.Theme.scaleMetric(12)))
    property var shortcutKeyPressHandler: null
    property alias documentBlocks: documentHost.documentBlocks
    property alias renderedResources: documentHost.renderedResources
    property alias sourceText: documentHost.sourceText
    property alias activeBlockIndex: documentHost.activeBlockIndex
    property alias activeBlockCursorRevision: documentHost.activeBlockCursorRevision
    property alias pendingFocusRequest: documentHost.pendingFocusRequest
    property alias pendingFocusBlockIndex: documentHost.pendingFocusBlockIndex
    property alias pendingFocusApplyQueued: documentHost.pendingFocusApplyQueued
    property alias cachedLogicalLineEntries: documentHost.cachedLogicalLineEntries
    property alias cachedBlockLayoutSummaries: documentHost.cachedBlockLayoutSummaries
    property alias layoutCacheRefreshQueued: documentHost.layoutCacheRefreshQueued
    property alias viewportContentY: documentHost.viewportContentY
    property alias viewportHeight: documentHost.viewportHeight
    readonly property bool focused: documentFlow.activeFocus || documentFlow.hasFocusedBlock()
    readonly property real blockDelegateOverscan: Math.max(
                                                      Math.round(LV.Theme.scaleMetric(240)),
                                                      documentFlow.lineHeightHint * 12)
    readonly property real delegateVirtualizationTopY: Math.max(
                                                           0,
                                                           (Number(documentFlow.viewportContentY) || 0)
                                                           - documentFlow.blockDelegateOverscan)
    readonly property real delegateVirtualizationBottomY: Math.max(
                                                              documentFlow.delegateVirtualizationTopY,
                                                              (Number(documentFlow.viewportContentY) || 0)
                                                              + Math.max(0, Number(documentFlow.viewportHeight) || 0)
                                                              + documentFlow.blockDelegateOverscan)
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

    ContentsStructuredDocumentHost {
        id: documentHost
        objectName: "contentsStructuredDocumentHost"
    }

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

    function minimapRowCountForBlockHost(blockHost, blockEntryOverride, lineHeight) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const blockType = blockEntry.type !== undefined ? String(blockEntry.type).trim().toLowerCase() : "text"
        const resolvedLineHeight = Math.max(1, Number(lineHeight) || documentFlow.lineHeightHint)
        const rawRowCount = Math.max(1, Math.ceil(resolvedLineHeight / Math.max(1, documentFlow.lineHeightHint)))
        if (blockType === "resource")
            return Math.min(10, rawRowCount)
        return rawRowCount
    }

    function logicalLineCount() {
        return Math.max(1, documentFlow.cachedLogicalLineEntries.length)
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

    function estimatedBlockHeight(blockEntryOverride) {
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : ({ })
        const explicitHeightHint = Math.max(0, Number(blockEntry.blockHeightHint) || 0)
        if (explicitHeightHint > 0)
            return explicitHeightHint
        const blockType = blockEntry.type !== undefined ? String(blockEntry.type).trim().toLowerCase() : "text"
        const logicalLineCount = Math.max(1, documentFlow.logicalLineCountForBlockHost(null, blockEntry))
        if (blockType === "resource")
            return Math.max(Math.round(LV.Theme.scaleMetric(160)), documentFlow.lineHeightHint * 6)
        if (blockType === "agenda") {
            return Math.max(
                        Math.round(LV.Theme.scaleMetric(72)),
                        documentFlow.lineHeightHint * Math.max(2, logicalLineCount)
                        + Math.round(LV.Theme.scaleMetric(36)))
        }
        if (blockType === "callout") {
            return Math.max(
                        Math.round(LV.Theme.scaleMetric(48)),
                        documentFlow.lineHeightHint * Math.max(1, logicalLineCount)
                        + Math.round(LV.Theme.scaleMetric(24)))
        }
        if (blockType === "break")
            return Math.max(documentFlow.lineHeightHint, Math.round(LV.Theme.scaleMetric(24)))
        return Math.max(documentFlow.lineHeightHint, documentFlow.lineHeightHint * logicalLineCount)
    }

    function measuredBlockHeightForHost(blockHost) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        if (!host)
            return 0
        const cachedDelegateHeight = Math.max(0, Number(host.cachedDelegateHeight) || 0)
        if (cachedDelegateHeight > 0)
            return cachedDelegateHeight
        const measuredDelegateHeight = Math.max(0, Number(host.measuredDelegateHeight) || 0)
        if (measuredDelegateHeight > 0)
            return measuredDelegateHeight
        return 0
    }

    function blockHeightForLayout(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const measuredHeight = documentFlow.measuredBlockHeightForHost(host)
        if (measuredHeight > 0)
            return Math.max(1, measuredHeight)
        return Math.max(1, documentFlow.estimatedBlockHeight(blockEntry))
    }

    function cachedBlockLayoutSummaryAt(blockIndex) {
        const safeBlockIndex = Math.max(0, Math.floor(Number(blockIndex) || 0))
        if (safeBlockIndex >= documentFlow.cachedBlockLayoutSummaries.length)
            return null
        const entry = documentFlow.cachedBlockLayoutSummaries[safeBlockIndex]
        return entry && typeof entry === "object" ? entry : null
    }

    function blockTopYForIndex(blockIndex) {
        const cachedSummary = documentFlow.cachedBlockLayoutSummaryAt(blockIndex)
        if (cachedSummary && cachedSummary.blockTopY !== undefined)
            return Math.max(0, Number(cachedSummary.blockTopY) || 0)
        return 0
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
                "rowCount": documentFlow.minimapRowCountForBlockHost(host, blockEntry, lineHeight)
            })
        }

        return entries
    }

    function refreshLayoutCache() {
        const blocks = documentFlow.normalizedBlocks()
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "refreshLayoutCache",
                    "blockCount=" + blocks.length
                    + " viewportHeight=" + documentFlow.viewportHeight
                    + " viewportContentY=" + documentFlow.viewportContentY,
                    documentFlow)
        const blockSpacing = Math.max(0, Number(documentColumn && documentColumn.spacing !== undefined ? documentColumn.spacing : 0) || 0)
        const entries = []
        const blockSummaries = []
        let nextBlockBaseY = 0
        for (let blockIndex = 0; blockIndex < blockRepeater.count; ++blockIndex) {
            const blockHost = blockRepeater.itemAt(blockIndex)
            const blockEntry = blocks[blockIndex] && typeof blocks[blockIndex] === "object" ? blocks[blockIndex] : ({})
            const blockHeight = documentFlow.blockHeightForLayout(blockHost, blockEntry)
            const blockEntries = documentFlow.blockLogicalLineEntries(blockHost, blockEntry, nextBlockBaseY)
            const startLineNumber = entries.length + 1
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
                    "gutterContentY": Math.max(0, Number(lineEntry.contentY) || 0),
                    "lineNumber": entries.length + 1,
                    "minimapRowCharCount": Math.max(0, Number(lineEntry.minimapRowCharCount) || 0),
                    "minimapVisualKind": lineEntry.minimapVisualKind !== undefined
                                         ? String(lineEntry.minimapVisualKind)
                                         : "text",
                    "rowCount": Math.max(1, Number(lineEntry.rowCount) || 1)
                })
            }
            blockSummaries.push({
                "blockHeight": blockHeight,
                "blockIndex": blockIndex,
                "blockTopY": nextBlockBaseY,
                "lineCount": Math.max(1, blockEntries.length),
                "startLineNumber": startLineNumber
            })
            nextBlockBaseY += blockHeight
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

        documentFlow.cachedBlockLayoutSummaries = blockSummaries
        documentFlow.cachedLogicalLineEntries = entries
    }

    function scheduleLayoutCacheRefresh() {
        if (documentFlow.layoutCacheRefreshQueued)
            return
        documentFlow.layoutCacheRefreshQueued = true
        Qt.callLater(function () {
            documentFlow.layoutCacheRefreshQueued = false
            documentFlow.refreshLayoutCache()
        })
    }

    function logicalLineEntries() {
        return documentFlow.cachedLogicalLineEntries
    }

    function activeLogicalLineNumber() {
        const cursorRevision = documentFlow.activeBlockCursorRevision
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return 1
        const resolvedActiveBlockIndex = Math.max(-1, Number(documentFlow.resolvedInteractiveBlockIndexValue) || -1)
        const safeActiveBlockIndex = Math.max(0, Math.min(blocks.length - 1, resolvedActiveBlockIndex))
        const cachedSummary = documentFlow.cachedBlockLayoutSummaryAt(safeActiveBlockIndex)
        const lineNumber = Math.max(
                    1,
                    Number(cachedSummary && cachedSummary.startLineNumber !== undefined
                           ? cachedSummary.startLineNumber
                           : 1) || 1)
        const activeBlockHost = blockRepeater.itemAt(safeActiveBlockIndex)
        const activeBlockLineCount = Math.max(
                    1,
                    Number(cachedSummary && cachedSummary.lineCount !== undefined
                           ? cachedSummary.lineCount
                           : documentFlow.logicalLineCountForBlockHost(activeBlockHost, blocks[safeActiveBlockIndex])) || 1)
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
        const lineEntries = documentFlow.cachedLogicalLineEntries
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
        documentHost.noteActiveBlockInteraction(Math.max(-1, Math.floor(Number(blockIndex) || -1)))
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
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "requestDocumentEndEdit",
                    "activeBlockIndex=" + documentFlow.activeBlockIndex,
                    documentFlow)
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
        return documentHost.collectionPolicy.normalizeEntries(documentFlow.documentBlocks)
    }

    function normalizedResourceEntries() {
        return documentHost.collectionPolicy.normalizeEntries(documentFlow.renderedResources)
    }

    function normalizedSourceText(value) {
        return documentHost.collectionPolicy.normalizeSourceText(
                    value === undefined || value === null ? "" : String(value))
    }

    function spliceSourceRange(start, end, replacementText) {
        return documentHost.collectionPolicy.spliceSourceRange(
                    documentFlow.sourceText,
                    Math.floor(Number(start) || 0),
                    Math.floor(Number(end) || 0),
                    replacementText === undefined || replacementText === null ? "" : String(replacementText))
    }

    function requestFocus(request) {
        const baseRequest = request && typeof request === "object" ? request : ({})
        documentFlow.pendingFocusRequest = Object.assign({}, baseRequest)
        documentFlow.pendingFocusBlockIndex = documentFlow.focusTargetBlockIndex(documentFlow.pendingFocusRequest)
        documentFlow.schedulePendingFocusApply()
    }

    function hasPendingFocusRequest() {
        return !!(documentFlow.pendingFocusRequest
                  && typeof documentFlow.pendingFocusRequest === "object"
                  && Object.keys(documentFlow.pendingFocusRequest).length > 0)
    }

    function floorNumberOrFallback(value, fallbackValue) {
        const numericValue = Number(value)
        if (!isFinite(numericValue))
            return fallbackValue
        return Math.floor(numericValue)
    }

    function resourceEntryForBlock(blockEntry) {
        return documentHost.collectionPolicy.resourceEntryForBlock(
                    blockEntry && typeof blockEntry === "object" ? blockEntry : ({ }),
                    documentFlow.renderedResources)
    }

    function shouldKeepBlockDelegateLoaded(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        if (!host)
            return true
        if (!documentFlow.visible)
            return host.blockIndex === documentFlow.pendingFocusBlockIndex
        if (host.blockIndex === documentFlow.pendingFocusBlockIndex || host.delegateFocused)
            return true
        const resolvedActiveBlockIndex = Math.max(-1, Number(documentFlow.resolvedInteractiveBlockIndexValue) || -1)
        if (resolvedActiveBlockIndex >= 0 && Math.abs(host.blockIndex - resolvedActiveBlockIndex) <= 1)
            return true
        if ((Number(documentFlow.viewportHeight) || 0) <= 0)
            return true
        const cachedSummary = documentFlow.cachedBlockLayoutSummaryAt(host.blockIndex)
        const blockTopY = Math.max(
                    0,
                    Number(cachedSummary && cachedSummary.blockTopY !== undefined
                           ? cachedSummary.blockTopY
                           : host.y) || 0)
        const blockHeight = Math.max(
                    1,
                    Number(cachedSummary && cachedSummary.blockHeight !== undefined
                           ? cachedSummary.blockHeight
                           : documentFlow.blockHeightForLayout(host, blockEntry)) || 1)
        const blockBottomY = blockTopY + Math.max(1, blockHeight)
        return blockBottomY >= documentFlow.delegateVirtualizationTopY
                && blockTopY <= documentFlow.delegateVirtualizationBottomY
    }

    function focusTargetBlockIndex(request) {
        return documentHost.focusPolicy.focusTargetBlockIndex(
                    documentFlow.documentBlocks,
                    documentFlow.activeBlockIndex,
                    request && typeof request === "object" ? request : ({ }))
    }

    function refreshPendingFocusBlockIndex() {
        if (!documentFlow.hasPendingFocusRequest()) {
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
        if (!documentFlow.hasPendingFocusRequest())
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
        documentHost.clearPendingFocusRequest()
        documentFlow.activeBlockIndex = safeIndex
        return true
    }

    function schedulePendingFocusApply() {
        if (!documentFlow.hasPendingFocusRequest())
            return
        if (documentFlow.pendingFocusApplyQueued)
            return
        documentFlow.pendingFocusApplyQueued = true
        Qt.callLater(function () {
            documentFlow.pendingFocusApplyQueued = false
            if (!documentFlow.hasPendingFocusRequest())
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
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "replaceSourceRange",
                    "start=" + start
                    + " end=" + end
                    + " focusRequest={" + EditorTrace.describeFocusRequest(focusRequest) + "} "
                    + EditorTrace.describeText(replacementText),
                    documentFlow)
        documentFlow.sourceMutationRequested(
                    documentFlow.spliceSourceRange(start, end, replacementText),
                    focusRequest && typeof focusRequest === "object" ? focusRequest : ({ }))
    }

    function focusRequestAfterBlockDeletion(blockData, nextSourceText) {
        return documentHost.focusPolicy.focusRequestAfterBlockDeletion(
                    documentFlow.documentBlocks,
                    documentFlow.activeBlockIndex,
                    blockData && typeof blockData === "object" ? blockData : ({ }),
                    nextSourceText)
    }

    function emptyTextBlockDeletionRange(blockData, direction, sourceText) {
        return documentHost.mutationPolicy.emptyTextBlockDeletionRange(
                    blockData && typeof blockData === "object" ? blockData : ({ }),
                    direction === undefined || direction === null ? "" : String(direction),
                    sourceText === undefined || sourceText === null ? "" : String(sourceText))
    }

    function nextEditableSourceOffsetAfterBlock(sourceText, blockEndOffset) {
        return documentHost.mutationPolicy.nextEditableSourceOffsetAfterBlock(
                    sourceText === undefined || sourceText === null ? "" : String(sourceText),
                    Math.floor(Number(blockEndOffset) || 0))
    }

    function deleteBlock(blockData, direction) {
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "deleteBlock",
                    "direction=" + String(direction || "")
                    + " block={" + EditorTrace.describeObject(blockData, ["type", "sourceStart", "sourceEnd"]) + "}",
                    documentFlow)
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
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "updateAgendaTaskText",
                    "cursorPosition=" + cursorPosition
                    + " task={" + EditorTrace.describeObject(taskData, ["openTagStart", "contentStart", "contentEnd"]) + "} "
                    + EditorTrace.describeText(text),
                    documentFlow)
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
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "toggleAgendaTaskDone",
                    "openTagStart=" + openTagStart
                    + " openTagEnd=" + openTagEnd
                    + " checked=" + checked,
                    documentFlow)
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
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "agendaTaskReturn",
                    "task={" + EditorTrace.describeObject(taskData, ["openTagStart", "contentEnd"]) + "}",
                    documentFlow)
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
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "updateCalloutText",
                    "cursorPosition=" + cursorPosition
                    + " block={" + EditorTrace.describeObject(blockData, ["type", "contentStart", "contentEnd"]) + "} "
                    + EditorTrace.describeText(text),
                    documentFlow)
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
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "exitCallout",
                    "block={" + EditorTrace.describeObject(blockData, ["type", "contentEnd"]) + "}",
                    documentFlow)
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

    function pendingShortcutInsertionSourceOffset() {
        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const pendingRequest = documentFlow.pendingFocusRequest && typeof documentFlow.pendingFocusRequest === "object"
                ? documentFlow.pendingFocusRequest
                : null
        const pendingSourceOffset = Number(
                    pendingRequest && pendingRequest.sourceOffset !== undefined
                    ? pendingRequest.sourceOffset
                    : NaN)
        if (!isFinite(pendingSourceOffset))
            return NaN
        return Math.max(0, Math.min(currentSourceText.length, Math.floor(pendingSourceOffset)))
    }

    function shortcutInsertionSourceOffset() {
        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const pendingSourceOffset = documentFlow.pendingShortcutInsertionSourceOffset()
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return currentSourceText.length === 0 ? 0 : pendingSourceOffset
        const resolvedActiveBlockIndex = Math.max(-1, Number(documentFlow.resolvedInteractiveBlockIndexValue) || -1)
        if (resolvedActiveBlockIndex < 0 || resolvedActiveBlockIndex >= blocks.length)
            return pendingSourceOffset
        const safeIndex = resolvedActiveBlockIndex
        const blockHost = blockRepeater.itemAt(safeIndex)
        const delegateLoader = blockHost && blockHost.delegateLoader ? blockHost.delegateLoader : null
        const delegateItem = delegateLoader ? delegateLoader.item : null
        if (delegateItem && delegateItem.shortcutInsertionSourceOffset !== undefined) {
            const delegateOffset = Number(delegateItem.shortcutInsertionSourceOffset())
            if (isFinite(delegateOffset))
                return Math.max(0, Math.min(currentSourceText.length, Math.floor(delegateOffset)))
        }
        const block = blocks[safeIndex] && typeof blocks[safeIndex] === "object" ? blocks[safeIndex] : ({})
        if (documentFlow.blockTextEditable(blockHost, block) && isFinite(pendingSourceOffset))
            return pendingSourceOffset
        return Math.max(0, Math.floor(Number(block.sourceEnd) || 0))
    }

    function insertStructuredShortcutAtActivePosition(shortcutKind) {
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "insertStructuredShortcutAtActivePosition",
                    "shortcutKind=" + String(shortcutKind || "")
                    + " activeBlockIndex=" + documentFlow.activeBlockIndex,
                    documentFlow)
        const insertionSpec = documentFlow.structuredShortcutSpec(shortcutKind)
        if (!insertionSpec.applied)
            return false
        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const resolvedInsertionOffset = Number(documentFlow.shortcutInsertionSourceOffset())
        if (!isFinite(resolvedInsertionOffset))
            return false
        const insertionOffset = Math.max(0, Math.min(currentSourceText.length, Math.floor(resolvedInsertionOffset)))
        const payload = documentHost.mutationPolicy.buildStructuredInsertionPayload(
                    currentSourceText,
                    insertionOffset,
                    String(insertionSpec.insertionSourceText || ""),
                    Math.max(0, Number(insertionSpec.cursorSourceOffsetFromInsertionStart) || 0))
        documentFlow.sourceMutationRequested(
                    String(payload.nextSourceText || currentSourceText),
                    {
                        "sourceOffset": Math.max(0, Number(payload.sourceOffset) || 0)
                    })
        return true
    }

    function insertResourceBlocksAtActivePosition(tagTexts) {
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "insertResourceBlocksAtActivePosition",
                    "tagCount=" + (Array.isArray(tagTexts) ? tagTexts.length : 0),
                    documentFlow)
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
        const resolvedInsertionOffset = Number(documentFlow.shortcutInsertionSourceOffset())
        if (!isFinite(resolvedInsertionOffset))
            return false
        const insertionOffset = Math.max(0, Math.min(currentSourceText.length, Math.floor(resolvedInsertionOffset)))
        const payload = documentHost.mutationPolicy.buildResourceInsertionPayload(
                    currentSourceText,
                    insertionOffset,
                    normalizedTagTexts)
        const nextSourceText = payload.nextSourceText !== undefined && payload.nextSourceText !== null
                ? String(payload.nextSourceText)
                : currentSourceText
        if (nextSourceText === currentSourceText)
            return false
        documentFlow.sourceMutationRequested(
                    nextSourceText,
                    payload.focusRequest && typeof payload.focusRequest === "object"
                    ? payload.focusRequest
                    : ({ }))
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
                property real cachedDelegateHeight: 0
                property real measuredDelegateHeight: blockLoader.item
                                                     ? Math.max(
                                                           0,
                                                           Number(blockLoader.item.implicitHeight) || Number(blockLoader.item.height) || 0)
                                                     : 0
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
                readonly property bool keepDelegateLoaded: documentFlow.shouldKeepBlockDelegateLoaded(blockHost, blockHost.blockEntry)
                implicitHeight: Math.max(
                                    1,
                                    blockHost.cachedDelegateHeight > 0
                                    ? blockHost.cachedDelegateHeight
                                    : documentFlow.estimatedBlockHeight(blockHost.blockEntry))
                width: documentColumn.width
                height: implicitHeight

                onMeasuredDelegateHeightChanged: {
                    const nextMeasuredHeight = Math.max(0, Number(blockHost.measuredDelegateHeight) || 0)
                    if (nextMeasuredHeight <= 0 || Math.abs(nextMeasuredHeight - blockHost.cachedDelegateHeight) <= 0.5)
                        return
                    blockHost.cachedDelegateHeight = nextMeasuredHeight
                }
                onCachedDelegateHeightChanged: documentFlow.scheduleLayoutCacheRefresh()

                Loader {
                    id: blockLoader

                    active: blockHost.keepDelegateLoaded
                    anchors.fill: parent
                    asynchronous: blockRepeater.count > 12 && blockHost.keepDelegateLoaded
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
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "documentBlocksChanged",
                    "blockCount=" + documentFlow.documentBlocks.length,
                    documentFlow)
        documentFlow.refreshLayoutCache()
        if (!documentFlow.hasPendingFocusRequest())
            return
        documentFlow.refreshPendingFocusBlockIndex()
        documentFlow.schedulePendingFocusApply()
    }
    onLineHeightHintChanged: documentFlow.refreshLayoutCache()
    onViewportHeightChanged: documentFlow.scheduleLayoutCacheRefresh()
    onWidthChanged: documentFlow.scheduleLayoutCacheRefresh()

    Component.onCompleted: {
        EditorTrace.trace("structuredDocumentFlow", "mount", "blockCount=" + documentFlow.documentBlocks.length, documentFlow)
        documentFlow.refreshLayoutCache()
    }

    Component.onDestruction: {
        EditorTrace.trace(
                    "structuredDocumentFlow",
                    "unmount",
                    "activeBlockIndex=" + documentFlow.activeBlockIndex
                    + " blockCount=" + documentFlow.documentBlocks.length,
                    documentFlow)
    }
}
