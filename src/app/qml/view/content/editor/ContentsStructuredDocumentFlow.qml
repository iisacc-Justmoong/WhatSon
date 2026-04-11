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
    property string sourceText: ""
    property int activeBlockIndex: -1
    property var pendingFocusRequest: ({ "token": 0 })
    readonly property bool focused: activeFocus

    signal sourceMutationRequested(string nextSourceText, var focusRequest)

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
        documentFlow.pendingFocusRequest = Object.assign({}, baseRequest, {
                                                             "token": Math.max(0, Number(documentFlow.pendingFocusRequest.token) || 0) + 1
                                                         })
    }

    function applyPendingFocus() {
        for (let index = 0; index < blockRepeater.count; ++index) {
            const host = blockRepeater.itemAt(index)
            const loader = host && host.delegateLoader ? host.delegateLoader : null
            if (!loader || !loader.item || loader.item.applyFocusRequest === undefined)
                continue
            if (loader.item.applyFocusRequest(documentFlow.pendingFocusRequest))
                return true
        }
        return false
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
                    sourceComponent: blockHost.blockType === "agenda"
                                     ? agendaBlockDelegate
                                     : blockHost.blockType === "callout"
                                       ? calloutBlockDelegate
                                       : blockHost.blockType === "break"
                                         ? breakBlockDelegate
                                         : textBlockDelegate
                }

                Component {
                    id: textBlockDelegate

                    ContentsDocumentTextBlock {
                        blockData: blockHost.blockEntry
                        focusRequest: documentFlow.pendingFocusRequest
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
                        focusRequest: documentFlow.pendingFocusRequest
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
                        focusRequest: documentFlow.pendingFocusRequest
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
                    }
                }
            }
        }
    }

    onPendingFocusRequestChanged: {
        Qt.callLater(function () {
            documentFlow.applyPendingFocus()
        })
    }
}
