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
    property string sourceText: ""
    property int activeBlockIndex: -1
    property var pendingFocusRequest: null
    property int pendingFocusBlockIndex: -1
    property bool pendingFocusApplyQueued: false
    readonly property bool focused: documentFlow.activeFocus || documentFlow.hasFocusedBlock()
    readonly property int currentLogicalLineNumber: documentFlow.activeLogicalLineNumber()

    signal sourceMutationRequested(string nextSourceText, var focusRequest)

    function blockUsesTextDelegate(blockEntry) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const blockType = safeBlock.type !== undefined ? String(safeBlock.type).toLowerCase() : "text"
        return blockType !== "agenda"
                && blockType !== "callout"
                && blockType !== "resource"
                && blockType !== "break"
    }

    function normalizedAgendaTasks(blockEntry) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const rawTasks = safeBlock.tasks
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

    function visiblePlainTextForBlock(blockEntry) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const blockType = safeBlock.type !== undefined ? String(safeBlock.type).toLowerCase() : "text"
        if (blockType === "agenda") {
            const tasks = documentFlow.normalizedAgendaTasks(safeBlock)
            const taskLines = []
            for (let index = 0; index < tasks.length; ++index) {
                const taskData = tasks[index] && typeof tasks[index] === "object" ? tasks[index] : ({})
                taskLines.push(StructuredCursorSupport.normalizedPlainText(String(taskData.text || "")))
            }
            return taskLines.join("\n")
        }
        if (blockType === "callout")
            return StructuredCursorSupport.normalizedPlainText(String(safeBlock.text || ""))
        if (blockType === "resource" || blockType === "break")
            return ""
        return StructuredCursorSupport.plainTextFromInlineTaggedSource(String(safeBlock.sourceText || ""))
    }

    function blockRepresentativeCharCount(blockEntry, lineText) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const blockType = safeBlock.type !== undefined ? String(safeBlock.type).toLowerCase() : "text"
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
        if (normalizedLineText.length > 0)
            return normalizedLineText.length
        if (blockType === "resource") {
            const resourceLabel = safeBlock.resourceId !== undefined && String(safeBlock.resourceId).trim().length > 0
                    ? String(safeBlock.resourceId).trim()
                    : (safeBlock.resourcePath !== undefined && String(safeBlock.resourcePath).trim().length > 0
                       ? String(safeBlock.resourcePath).trim()
                       : "resource")
            return Math.max(12, resourceLabel.length)
        }
        if (blockType === "break")
            return 8
        return 0
    }

    function logicalLineCountForBlock(blockEntry, logicalLines) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const blockType = safeBlock.type !== undefined ? String(safeBlock.type).toLowerCase() : "text"
        const logicalLineCount = Math.max(1, Array.isArray(logicalLines) ? logicalLines.length : 1)
        if (blockType === "resource" || blockType === "break")
            return 1
        return logicalLineCount
    }

    function visualLineCharCount(blockEntry, logicalLines, visualLineCount, visualLineIndex) {
        const safeLogicalLines = Array.isArray(logicalLines) && logicalLines.length > 0 ? logicalLines : [""]
        const safeVisualLineCount = Math.max(1, Math.floor(Number(visualLineCount) || 1))
        const safeVisualLineIndex = Math.max(0, Math.min(safeVisualLineCount - 1, Math.floor(Number(visualLineIndex) || 0)))
        if (safeVisualLineCount === safeLogicalLines.length)
            return Math.max(0, documentFlow.blockRepresentativeCharCount(blockEntry, safeLogicalLines[safeVisualLineIndex]))

        let totalCharCount = 0
        for (let index = 0; index < safeLogicalLines.length; ++index)
            totalCharCount += Math.max(0, String(safeLogicalLines[index] || "").length)
        if (totalCharCount <= 0)
            return Math.max(0, documentFlow.blockRepresentativeCharCount(blockEntry, ""))

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

    function blockLogicalLineEntries(blockHost, blockEntryOverride) {
        const host = blockHost && typeof blockHost === "object" ? blockHost : null
        const blockEntry = blockEntryOverride && typeof blockEntryOverride === "object"
                ? blockEntryOverride
                : (host && host.blockEntry && typeof host.blockEntry === "object" ? host.blockEntry : ({ }))
        const safeBlockEntry = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const blockType = safeBlockEntry.type !== undefined ? String(safeBlockEntry.type).toLowerCase() : "text"
        const plainText = documentFlow.visiblePlainTextForBlock(blockEntry)
        const logicalLines = plainText.length > 0 ? plainText.split("\n") : [""]
        const blockHeight = Math.max(
                    1,
                    host ? (Number(host.implicitHeight) || Number(host.height) || 0) : 0,
                    documentFlow.lineHeightHint * Math.max(1, logicalLines.length))
        const baseY = Math.max(0, host ? (Number(host.y) || 0) : 0)
        const entries = []
        const lineCount = documentFlow.logicalLineCountForBlock(blockEntry, logicalLines)
        const delegateItem = host && host.delegateLoader && host.delegateLoader.item ? host.delegateLoader.item : null
        const delegateLineLayoutEntries = documentFlow.normalizedDelegateLineLayoutEntries(delegateItem, lineCount)

        for (let index = 0; index < lineCount; ++index) {
            const delegateLineLayout = delegateLineLayoutEntries[index] && typeof delegateLineLayoutEntries[index] === "object"
                    ? delegateLineLayoutEntries[index]
                    : null
            const fallbackLineTop = blockHeight * index / lineCount
            const fallbackLineBottom = index + 1 < lineCount
                    ? blockHeight * (index + 1) / lineCount
                    : blockHeight
            const localLineTop = delegateLineLayout && delegateLineLayout.contentY !== undefined
                    ? Math.max(0, Number(delegateLineLayout.contentY) || 0)
                    : fallbackLineTop
            const localLineBottom = delegateLineLayout && delegateLineLayout.contentHeight !== undefined
                    ? Math.max(
                          localLineTop + documentFlow.lineHeightHint,
                          localLineTop + (Number(delegateLineLayout.contentHeight) || 0))
                    : fallbackLineBottom
            const lineTop = baseY + localLineTop
            const lineBottom = baseY + Math.max(localLineTop + documentFlow.lineHeightHint, localLineBottom)
            const lineHeight = Math.max(1, lineBottom - lineTop)
            const gutterCollapsed = blockType === "resource" || blockType === "break"
            const minimapVisualKind = blockType === "resource" ? "block" : "text"
            const minimapRowCharCount = blockType === "resource" ? 160 : 0
            entries.push({
                "charCount": documentFlow.visualLineCharCount(blockEntry, logicalLines, lineCount, index),
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
        let nextGutterContentY = 0
        for (let blockIndex = 0; blockIndex < blockRepeater.count; ++blockIndex) {
            const blockHost = blockRepeater.itemAt(blockIndex)
            const blockEntry = blocks[blockIndex] && typeof blocks[blockIndex] === "object" ? blocks[blockIndex] : ({})
            const blockEntries = documentFlow.blockLogicalLineEntries(blockHost, blockEntry)
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
        const blocks = documentFlow.normalizedBlocks()
        const safeActiveBlockIndex = Math.max(0, Math.min(blocks.length - 1, Number(documentFlow.activeBlockIndex) || 0))
        if (blocks.length === 0)
            return 1
        let lineNumber = 1
        for (let blockIndex = 0; blockIndex < safeActiveBlockIndex; ++blockIndex) {
            const blockHost = blockRepeater.itemAt(blockIndex)
            lineNumber += documentFlow.blockLogicalLineEntries(blockHost, blocks[blockIndex]).length
        }
        return Math.max(1, lineNumber)
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
        if (documentFlow.blockUsesTextDelegate(lastBlock)) {
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

    function normalizedFocusSourceOffset(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return NaN
        return Math.max(0, Math.floor(sourceOffset))
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

    function blockContainsSourceOffset(blockEntry, sourceOffset) {
        if (!isFinite(sourceOffset))
            return false
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const sourceStart = Math.max(0, Math.floor(Number(safeBlock.sourceStart) || 0))
        const sourceEnd = Math.max(sourceStart, Math.floor(Number(safeBlock.sourceEnd) || sourceStart))
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
        const taskOpenTagStart = documentFlow.normalizedFocusTaskOpenTagStart(request)
        if (isFinite(taskOpenTagStart)) {
            for (let index = 0; index < blocks.length; ++index) {
                if (documentFlow.blockContainsTaskOpenTagStart(blocks[index], taskOpenTagStart))
                    return index
            }
        }
        const sourceOffset = documentFlow.normalizedFocusSourceOffset(request)
        if (isFinite(sourceOffset)) {
            for (let index = 0; index < blocks.length; ++index) {
                if (documentFlow.blockContainsSourceOffset(blocks[index], sourceOffset))
                    return index
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
        const suffixNewline = suffixOffset >= currentSourceText.length
                || currentSourceText.charAt(suffixOffset) !== "\n"
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
                    {
                        "sourceOffset": documentFlow.nextEditableSourceOffsetAfterBlock(
                                            nextSourceText,
                                            insertedBlockEndOffset)
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
                readonly property string blockType: blockEntry.type !== undefined ? String(blockEntry.type) : "text"
                implicitHeight: blockLoader.item ? Math.max(0, Number(blockLoader.item.implicitHeight) || Number(blockLoader.item.height) || 0) : 0
                width: documentColumn.width
                height: implicitHeight

                Loader {
                    id: blockLoader

                    anchors.fill: parent
                    asynchronous: blockRepeater.count > 12
                    sourceComponent: blockHost.blockType === "agenda"
                                     ? agendaBlockDelegate
                                     : blockHost.blockType === "callout"
                                       ? calloutBlockDelegate
                                       : blockHost.blockType === "resource"
                                         ? resourceBlockDelegate
                                       : blockHost.blockType === "break"
                                         ? breakBlockDelegate
                                         : textBlockDelegate

                    onLoaded: {
                        if (!item || blockHost.blockIndex !== documentFlow.pendingFocusBlockIndex)
                            return
                        documentFlow.schedulePendingFocusApply()
                    }
                }

                Component {
                    id: textBlockDelegate

                    ContentsDocumentTextBlock {
                        blockData: blockHost.blockEntry
                        width: blockHost.width

                        onActivated: documentFlow.activeBlockIndex = blockHost.blockIndex
                        onSourceMutationRequested: function (nextBlockSourceText, focusRequest) {
                            documentFlow.replaceTextBlock(blockHost.blockEntry, nextBlockSourceText, focusRequest)
                        }
                    }
                }

                Component {
                    id: agendaBlockDelegate

                    ContentsAgendaBlock {
                        blockData: blockHost.blockEntry
                        width: blockHost.width

                        onActivated: documentFlow.activeBlockIndex = blockHost.blockIndex
                        onTaskDoneToggled: function (openTagStart, openTagEnd, checked) {
                            documentFlow.toggleAgendaTaskDone(openTagStart, openTagEnd, checked)
                        }
                        onTaskEnterRequested: function (_blockData, taskData) {
                            documentFlow.agendaTaskReturn(taskData)
                        }
                        onTaskTextChanged: function (taskData, text, cursorPosition) {
                            documentFlow.updateAgendaTaskText(taskData, text, cursorPosition)
                        }
                    }
                }

                Component {
                    id: calloutBlockDelegate

                    ContentsCalloutBlock {
                        blockData: blockHost.blockEntry
                        width: blockHost.width

                        onActivated: documentFlow.activeBlockIndex = blockHost.blockIndex
                        onEnterExitRequested: function (blockData) {
                            documentFlow.exitCallout(blockData)
                        }
                        onTextChanged: function (text, cursorPosition) {
                            documentFlow.updateCalloutText(blockHost.blockEntry, text, cursorPosition)
                        }
                    }
                }

                Component {
                    id: breakBlockDelegate

                    ContentsBreakBlock {
                        blockData: blockHost.blockEntry
                        width: blockHost.width

                        onDocumentEndEditRequested: documentFlow.requestDocumentEndEdit()
                    }
                }

                Component {
                    id: resourceBlockDelegate

                    ContentsResourceBlock {
                        blockData: blockHost.blockEntry
                        resourceEntry: documentFlow.resourceEntryForBlock(blockHost.blockEntry)
                        width: blockHost.width

                        onActivated: documentFlow.activeBlockIndex = blockHost.blockIndex
                        onDocumentEndEditRequested: documentFlow.requestDocumentEndEdit()
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
