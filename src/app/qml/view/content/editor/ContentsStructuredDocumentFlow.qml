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
    property var renderedResources: []
    property string sourceText: ""
    property int activeBlockIndex: -1
    property var pendingFocusRequest: null
    property int pendingFocusBlockIndex: -1
    property bool pendingFocusApplyQueued: false
    readonly property bool focused: activeFocus

    signal sourceMutationRequested(string nextSourceText, var focusRequest)

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
        const lastBlockType = lastBlock.type !== undefined ? String(lastBlock.type) : "text"
        if (lastBlockType === "text") {
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
        if (documentBlocks && documentBlocks.length !== undefined) {
            const normalized = []
            for (let index = 0; index < documentBlocks.length; ++index)
                normalized.push(documentBlocks[index])
            return normalized
        }
        return []
    }

    function normalizedResourceEntries() {
        if (Array.isArray(renderedResources))
            return renderedResources
        if (renderedResources && renderedResources.length !== undefined) {
            const normalized = []
            for (let index = 0; index < renderedResources.length; ++index)
                normalized.push(renderedResources[index])
            return normalized
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

    function blockContainsTaskOpenTagStart(blockEntry, taskOpenTagStart) {
        if (!isFinite(taskOpenTagStart))
            return false
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const rawTasks = safeBlock.tasks
        if (!rawTasks || rawTasks.length === undefined)
            return false
        for (let index = 0; index < rawTasks.length; ++index) {
            const taskData = rawTasks[index] && typeof rawTasks[index] === "object" ? rawTasks[index] : ({})
            if (Math.floor(Number(taskData.openTagStart) || -1) === taskOpenTagStart)
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

    function resourceEntryForBlock(blockEntry) {
        const safeBlock = blockEntry && typeof blockEntry === "object" ? blockEntry : ({})
        const blockResourceIndex = Math.floor(Number(safeBlock.resourceIndex) || -1)
        const blockSourceStart = Math.max(0, Math.floor(Number(safeBlock.sourceStart) || 0))
        const blockSourceEnd = Math.max(blockSourceStart, Math.floor(Number(safeBlock.sourceEnd) || blockSourceStart))
        const blockResourceId = safeBlock.resourceId !== undefined ? String(safeBlock.resourceId).trim() : ""
        const blockResourcePath = safeBlock.resourcePath !== undefined ? String(safeBlock.resourcePath).trim() : ""
        const resourceEntries = documentFlow.normalizedResourceEntries()

        for (let index = 0; index < resourceEntries.length; ++index) {
            const entry = resourceEntries[index] && typeof resourceEntries[index] === "object" ? resourceEntries[index] : ({})
            const entryIndex = Math.floor(Number(entry.index) || -1)
            if (blockResourceIndex >= 0 && entryIndex === blockResourceIndex)
                return entry
        }

        for (let index = 0; index < resourceEntries.length; ++index) {
            const entry = resourceEntries[index] && typeof resourceEntries[index] === "object" ? resourceEntries[index] : ({})
            const entrySourceStart = Math.max(0, Math.floor(Number(entry.sourceStart) || 0))
            const entrySourceEnd = Math.max(entrySourceStart, Math.floor(Number(entry.sourceEnd) || entrySourceStart))
            if (entrySourceStart === blockSourceStart && entrySourceEnd === blockSourceEnd)
                return entry
        }

        for (let index = 0; index < resourceEntries.length; ++index) {
            const entry = resourceEntries[index] && typeof resourceEntries[index] === "object" ? resourceEntries[index] : ({})
            const entryResourceId = entry.resourceId !== undefined ? String(entry.resourceId).trim() : ""
            const entryResourcePath = entry.resourcePath !== undefined ? String(entry.resourcePath).trim() : ""
            if (blockResourceId.length > 0 && entryResourceId === blockResourceId)
                return entry
            if (blockResourcePath.length > 0 && entryResourcePath === blockResourcePath)
                return entry
        }

        return ({})
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
        const safeIndex = Math.max(-1, Math.floor(Number(blockIndex) || -1))
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
                        "taskOpenTagStart": Math.floor(Number(safeTask.openTagStart) || -1)
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
        documentFlow.sourceMutationRequested(nextSourceText, { "taskOpenTagStart": Math.floor(Number(openTagStart) || -1) })
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

    implicitHeight: documentColumn.implicitHeight
    width: parent ? parent.width : implicitWidth

    Column {
        id: documentColumn

        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Repeater {
            id: blockRepeater

            model: documentFlow.normalizedBlocks()

            delegate: Item {
                id: blockHost

                required property var modelData
                property alias delegateLoader: blockLoader
                readonly property int blockIndex: index
                readonly property var blockEntry: modelData && typeof modelData === "object" ? modelData : ({})
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
