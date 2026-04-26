pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0
import "../structure/ContentsStructuredCursorSupport.js" as StructuredCursorSupport

QtObject {
    id: controller

    property var blockRepeater: null
    property var documentFlow: null
    property bool paperPaletteEnabled: false
    property QtObject inlineStyleRenderer: ContentsTextFormatRenderer {
        paperPaletteEnabled: controller.paperPaletteEnabled
    }

    function normalizedInlineStyleTag(tagName) {
        const normalizedTagName = tagName === undefined || tagName === null
                ? ""
                : String(tagName).trim().toLowerCase()
        if (normalizedTagName === "bold" || normalizedTagName === "b" || normalizedTagName === "strong")
            return "bold"
        if (normalizedTagName === "italic" || normalizedTagName === "i" || normalizedTagName === "em")
            return "italic"
        if (normalizedTagName === "underline" || normalizedTagName === "u")
            return "underline"
        if (normalizedTagName === "strikethrough" || normalizedTagName === "strike" || normalizedTagName === "s" || normalizedTagName === "del")
            return "strikethrough"
        if (normalizedTagName === "highlight" || normalizedTagName === "mark")
            return "highlight"
        if (normalizedTagName === "plain" || normalizedTagName === "clear" || normalizedTagName === "none")
            return "plain"
        return ""
    }

    function adjustedCursorPositionForSelectionMutation(selectionSnapshot, previousSelectionStart, previousSelectionEnd, nextSelectionStart, nextSelectionEnd) {
        const boundedPreviousCursor = Math.max(
                    previousSelectionStart,
                    Math.min(
                        previousSelectionEnd,
                        Math.floor(Number(selectionSnapshot && selectionSnapshot.cursorPosition !== undefined
                                          ? selectionSnapshot.cursorPosition
                                          : previousSelectionEnd) || previousSelectionEnd)))
        const relativeOffset = Math.max(0, boundedPreviousCursor - previousSelectionStart)
        return Math.max(
                    nextSelectionStart,
                    Math.min(
                        nextSelectionEnd,
                        nextSelectionStart + relativeOffset))
    }

    function selectionSnapshotIsValid(selectionSnapshot) {
        if (!documentFlow || documentFlow.selectionSnapshotIsValid === undefined)
            return false
        return !!documentFlow.selectionSnapshotIsValid(selectionSnapshot)
    }

    function normalizedBlockIndex(blockIndex) {
        if (!documentFlow || documentFlow.normalizedBlocks === undefined)
            return -1
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return -1
        const numericBlockIndex = Number(blockIndex)
        if (!isFinite(numericBlockIndex))
            return -1
        return Math.max(0, Math.min(blocks.length - 1, Math.floor(numericBlockIndex)))
    }

    function interactiveBlockEntry(blockIndex) {
        const safeBlockIndex = controller.normalizedBlockIndex(blockIndex)
        if (safeBlockIndex < 0 || !documentFlow || documentFlow.normalizedBlocks === undefined)
            return null
        const blocks = documentFlow.normalizedBlocks()
        const blockEntry = blocks[safeBlockIndex]
        return blockEntry && typeof blockEntry === "object" ? blockEntry : null
    }

    function delegateItemForBlockIndex(blockIndex) {
        const safeBlockIndex = controller.normalizedBlockIndex(blockIndex)
        if (safeBlockIndex < 0 || !blockRepeater || blockRepeater.itemAt === undefined)
            return null
        const blockHost = blockRepeater.itemAt(safeBlockIndex)
        if (!blockHost || !documentFlow || documentFlow.delegateItemForBlockHost === undefined)
            return null
        return documentFlow.delegateItemForBlockHost(blockHost)
    }

    function blockSelectionSnapshot(blockIndex) {
        const delegateItem = controller.delegateItemForBlockIndex(blockIndex)
        if (!delegateItem)
            return ({})
        if (delegateItem.inlineFormatSelectionSnapshot !== undefined)
            return delegateItem.inlineFormatSelectionSnapshot()
        if (delegateItem.selectionSnapshot !== undefined)
            return delegateItem.selectionSnapshot()
        return ({})
    }

    function inlineFormatTargetState() {
        if (!documentFlow || documentFlow.normalizedBlocks === undefined)
            return ({ "valid": false })
        const blocks = documentFlow.normalizedBlocks()
        if (blocks.length === 0)
            return ({ "valid": false })
        const resolvedActiveBlockIndex = documentFlow.normalizedResolvedInteractiveBlockIndex !== undefined
                ? documentFlow.normalizedResolvedInteractiveBlockIndex()
                : -1
        const safeActiveBlockIndex = Math.max(0, Math.min(blocks.length - 1, resolvedActiveBlockIndex))
        const selectionSnapshot = controller.blockSelectionSnapshot(safeActiveBlockIndex)
        return {
            "blockIndex": safeActiveBlockIndex,
            "selectionSnapshot": selectionSnapshot,
            "valid": controller.selectionSnapshotIsValid(selectionSnapshot)
        }
    }

    function applyInlineFormatToBlockSelection(blockIndex, tagName, explicitSelectionSnapshot) {
        if (!documentFlow || documentFlow.normalizedBlocks === undefined
                || documentFlow.normalizedSourceText === undefined
                || documentFlow.replaceSourceRange === undefined) {
            return false
        }
        const normalizedTagName = controller.normalizedInlineStyleTag(tagName)
        if (normalizedTagName.length === 0)
            return false
        const safeBlockIndex = controller.normalizedBlockIndex(blockIndex)
        if (safeBlockIndex < 0)
            return false
        const blockEntry = controller.interactiveBlockEntry(safeBlockIndex)
        if (!blockEntry)
            return false
        if (documentFlow.blockTextEditable !== undefined
                && !documentFlow.blockTextEditable(null, blockEntry)) {
            return false
        }

        const currentSourceText = documentFlow.normalizedSourceText(documentFlow.sourceText)
        const blockSourceStart = Math.max(
                    0,
                    Math.min(
                        currentSourceText.length,
                        Math.floor(Number(blockEntry.sourceStart) || 0)))
        const blockSourceEnd = Math.max(
                    blockSourceStart,
                    Math.min(
                        currentSourceText.length,
                        Math.floor(Number(blockEntry.sourceEnd) || blockSourceStart)))
        if (blockSourceEnd <= blockSourceStart)
            return false

        const blockSourceText = currentSourceText.slice(blockSourceStart, blockSourceEnd)
        const currentPlainText = StructuredCursorSupport.plainTextFromInlineTaggedSource(blockSourceText)
        const selectionSnapshot = explicitSelectionSnapshot
                && typeof explicitSelectionSnapshot === "object"
                && controller.selectionSnapshotIsValid(explicitSelectionSnapshot)
                ? explicitSelectionSnapshot
                : controller.blockSelectionSnapshot(safeBlockIndex)
        const selectionStart = Math.max(
                    0,
                    Math.min(
                        currentPlainText.length,
                        Math.floor(Number(selectionSnapshot.selectionStart) || 0)))
        const selectionEnd = Math.max(
                    selectionStart,
                    Math.min(
                        currentPlainText.length,
                        Math.floor(Number(selectionSnapshot.selectionEnd) || 0)))
        if (selectionEnd <= selectionStart)
            return false

        const nextBlockSourceText = String(controller.inlineStyleRenderer.applyInlineStyleToLogicalSelectionSource(
                                               blockSourceText,
                                               selectionStart,
                                               selectionEnd,
                                               normalizedTagName))
        if (nextBlockSourceText === blockSourceText)
            return false

        const cursorPosition = controller.adjustedCursorPositionForSelectionMutation(
                    selectionSnapshot,
                    selectionStart,
                    selectionEnd,
                    selectionStart,
                    selectionEnd)
        documentFlow.replaceSourceRange(
                    blockSourceStart,
                    blockSourceEnd,
                    nextBlockSourceText,
                    {
                        "localCursorPosition": cursorPosition,
                        "selectionEnd": selectionEnd,
                        "selectionStart": selectionStart,
                        "sourceOffset": StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                                            nextBlockSourceText,
                                            cursorPosition,
                                            blockSourceStart),
                        "targetBlockIndex": safeBlockIndex
                    })
        return true
    }

    function applyInlineFormatToActiveSelection(tagName) {
        const targetState = controller.inlineFormatTargetState()
        if (!targetState.valid)
            return false
        return controller.applyInlineFormatToBlockSelection(
                    targetState.blockIndex,
                    tagName,
                    targetState.selectionSnapshot)
    }
}
