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

    function cursorSnapshotIsValid(selectionSnapshot) {
        if (!selectionSnapshot || typeof selectionSnapshot !== "object")
            return false
        const cursorPosition = Number(selectionSnapshot.cursorPosition)
        if (isFinite(cursorPosition))
            return true
        const selectionStart = Number(selectionSnapshot.selectionStart)
        const selectionEnd = Number(selectionSnapshot.selectionEnd)
        return isFinite(selectionStart) || isFinite(selectionEnd)
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

    function blockDelegateFocused(blockIndex) {
        const delegateItem = controller.delegateItemForBlockIndex(blockIndex)
        if (!delegateItem)
            return false
        if (delegateItem.focused !== undefined)
            return !!delegateItem.focused
        return delegateItem.activeFocus !== undefined && !!delegateItem.activeFocus
    }

    function blockInlineFormatTargetState(blockIndex) {
        const safeBlockIndex = controller.normalizedBlockIndex(blockIndex)
        if (safeBlockIndex < 0)
            return ({ "valid": false })
        const selectionSnapshot = controller.blockSelectionSnapshot(safeBlockIndex)
        const selectionValid = controller.selectionSnapshotIsValid(selectionSnapshot)
        const focused = controller.blockDelegateFocused(safeBlockIndex)
        const cursorValid = controller.cursorSnapshotIsValid(selectionSnapshot)
        return {
            "blockIndex": safeBlockIndex,
            "cursorValid": cursorValid,
            "focused": focused,
            "selectionSnapshot": selectionSnapshot,
            "selectionValid": selectionValid,
            "valid": selectionValid || (focused && cursorValid)
        }
    }

    function firstMountedSelectionTargetState() {
        if (!documentFlow || documentFlow.normalizedBlocks === undefined)
            return ({ "valid": false })
        const blocks = documentFlow.normalizedBlocks()
        for (let blockIndex = 0; blockIndex < blocks.length; ++blockIndex) {
            const targetState = controller.blockInlineFormatTargetState(blockIndex)
            if (targetState.selectionValid)
                return targetState
        }
        return ({ "valid": false })
    }

    function focusedCursorTargetState() {
        if (!documentFlow || documentFlow.normalizedBlocks === undefined)
            return ({ "valid": false })
        const focusedBlockIndex = documentFlow.focusedBlockIndex !== undefined
                ? documentFlow.focusedBlockIndex()
                : -1
        const focusedTargetState = controller.blockInlineFormatTargetState(focusedBlockIndex)
        if (focusedTargetState.valid)
            return focusedTargetState

        const blocks = documentFlow.normalizedBlocks()
        for (let blockIndex = 0; blockIndex < blocks.length; ++blockIndex) {
            const targetState = controller.blockInlineFormatTargetState(blockIndex)
            if (targetState.focused && targetState.cursorValid)
                return targetState
        }
        return ({ "valid": false })
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
        const activeTargetState = controller.blockInlineFormatTargetState(resolvedActiveBlockIndex)
        if (activeTargetState.valid)
            return activeTargetState

        const selectedTargetState = controller.firstMountedSelectionTargetState()
        if (selectedTargetState.valid)
            return selectedTargetState

        return controller.focusedCursorTargetState()
    }

    function applyInlineFormatAtCollapsedCursor(blockIndex, tagName, selectionSnapshot, blockSourceStart, blockSourceEnd, blockSourceText, currentPlainText) {
        const safeSnapshot = selectionSnapshot && typeof selectionSnapshot === "object"
                ? selectionSnapshot
                : ({})
        const fallbackCursor = isFinite(Number(safeSnapshot.selectionEnd))
                ? Number(safeSnapshot.selectionEnd)
                : currentPlainText.length
        const cursorPosition = Math.max(
                    0,
                    Math.min(
                        currentPlainText.length,
                        Math.floor(isFinite(Number(safeSnapshot.cursorPosition))
                                   ? Number(safeSnapshot.cursorPosition)
                                   : fallbackCursor)))
        const sourceInsertionOffset = StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(
                    blockSourceText,
                    cursorPosition,
                    blockSourceStart)
        const localInsertionOffset = Math.max(
                    0,
                    Math.min(
                        blockSourceText.length,
                        Math.floor(sourceInsertionOffset - blockSourceStart)))
        const openTag = "<" + tagName + ">"
        const closeTag = "</" + tagName + ">"
        const nextBlockSourceText = blockSourceText.slice(0, localInsertionOffset)
                + openTag
                + closeTag
                + blockSourceText.slice(localInsertionOffset)
        if (nextBlockSourceText === blockSourceText)
            return false
        documentFlow.replaceSourceRange(
                    blockSourceStart,
                    blockSourceEnd,
                    nextBlockSourceText,
                    {
                        "localCursorPosition": cursorPosition,
                        "sourceOffset": sourceInsertionOffset + openTag.length,
                        "targetBlockIndex": blockIndex
                    })
        return true
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

        const blockSourceText = currentSourceText.slice(blockSourceStart, blockSourceEnd)
        const currentPlainText = StructuredCursorSupport.plainTextFromInlineTaggedSource(blockSourceText)
        const selectionSnapshot = explicitSelectionSnapshot
                && typeof explicitSelectionSnapshot === "object"
                && (controller.selectionSnapshotIsValid(explicitSelectionSnapshot)
                    || controller.cursorSnapshotIsValid(explicitSelectionSnapshot))
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
        if (selectionEnd <= selectionStart) {
            if (!controller.cursorSnapshotIsValid(selectionSnapshot))
                return false
            return controller.applyInlineFormatAtCollapsedCursor(
                        safeBlockIndex,
                        normalizedTagName,
                        selectionSnapshot,
                        blockSourceStart,
                        blockSourceEnd,
                        blockSourceText,
                        currentPlainText)
        }
        if (blockSourceEnd <= blockSourceStart)
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
