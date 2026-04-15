pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "ContentsStructuredCursorSupport.js" as StructuredCursorSupport

FocusScope {
    id: resourceBlock

    required property var blockData
    property var resourceEntry: ({})
    property int afterTextFocusSourceOffset: -1
    property int beforeTextFocusSourceOffset: -1

    signal activated()
    signal adjacentPlainTextInsertionRequested(string side, string text, int cursorPosition)
    signal adjacentTextFocusRequested(int sourceOffset)
    signal documentEndEditRequested()

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property var normalizedResourceEntry: resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({})
    readonly property real boundaryEditorHeight: Math.max(Math.round(LV.Theme.scaleMetric(18)), Math.round(LV.Theme.scaleMetric(12)))
    readonly property real boundaryEditorInset: Math.max(0, Math.round(LV.Theme.scaleMetric(6)))
    readonly property real boundaryEditorWidth: Math.max(Math.round(LV.Theme.scaleMetric(24)), Math.round(LV.Theme.scaleMetric(18)))
    readonly property bool focused: resourceBlock.activeFocus || beforeBoundaryEditor.focused || afterBoundaryEditor.focused
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || 0)
    readonly property int focusSourceOffset: Math.max(
                                                 sourceStart,
                                                 Number(normalizedResourceEntry.focusSourceOffset)
                                                 || Number(normalizedBlock.focusSourceOffset)
                                                 || Number(normalizedResourceEntry.sourceEnd)
                                                 || sourceEnd)
    readonly property string blockResourceType: normalizedBlock.resourceType !== undefined ? String(normalizedBlock.resourceType).trim().toLowerCase() : ""
    readonly property string blockResourceFormat: normalizedBlock.resourceFormat !== undefined ? String(normalizedBlock.resourceFormat).trim().toLowerCase() : ""
    readonly property string blockResourceId: normalizedBlock.resourceId !== undefined ? String(normalizedBlock.resourceId).trim() : ""
    readonly property string blockResourcePath: normalizedBlock.resourcePath !== undefined ? String(normalizedBlock.resourcePath).trim() : ""
    readonly property bool resourceEntryHasResolvedPayload: {
        const entrySource = normalizedResourceEntry.source !== undefined ? String(normalizedResourceEntry.source).trim() : ""
        const entryResolvedPath = normalizedResourceEntry.resolvedPath !== undefined ? String(normalizedResourceEntry.resolvedPath).trim() : ""
        return entrySource.length > 0 || entryResolvedPath.length > 0
    }
    readonly property string fallbackRenderMode: {
        if (resourceBlock.blockResourceType === "image")
            return "image"
        if (resourceBlock.blockResourceType === "video")
            return "video"
        if (resourceBlock.blockResourceType === "audio" || resourceBlock.blockResourceType === "music")
            return "audio"
        if (resourceBlock.blockResourceFormat === ".pdf")
            return "pdf"
        return "document"
    }
    readonly property var effectiveResourceEntry: {
        const resolvedEntry = resourceBlock.normalizedResourceEntry
        const mergedEntry = {}
        if (resolvedEntry && typeof resolvedEntry === "object") {
            for (const key in resolvedEntry)
                mergedEntry[key] = resolvedEntry[key]
        }

        const mergedType = mergedEntry.type !== undefined ? String(mergedEntry.type).trim().toLowerCase() : ""
        const mergedFormat = mergedEntry.format !== undefined ? String(mergedEntry.format).trim().toLowerCase() : ""
        const mergedResourceId = mergedEntry.resourceId !== undefined ? String(mergedEntry.resourceId).trim() : ""
        const mergedResourcePath = mergedEntry.resourcePath !== undefined ? String(mergedEntry.resourcePath).trim() : ""
        const mergedDisplayName = mergedEntry.displayName !== undefined ? String(mergedEntry.displayName).trim() : ""
        const mergedRenderMode = mergedEntry.renderMode !== undefined ? String(mergedEntry.renderMode).trim().toLowerCase() : ""

        if (mergedDisplayName.length === 0) {
            mergedEntry.displayName = resourceBlock.blockResourceId.length > 0
                                      ? resourceBlock.blockResourceId
                                      : resourceBlock.blockResourcePath
        }
        if (mergedEntry.focusSourceOffset === undefined || !isFinite(Number(mergedEntry.focusSourceOffset)))
            mergedEntry.focusSourceOffset = resourceBlock.focusSourceOffset
        if (mergedEntry.sourceStart === undefined || !isFinite(Number(mergedEntry.sourceStart)))
            mergedEntry.sourceStart = resourceBlock.sourceStart
        if (mergedEntry.sourceEnd === undefined || !isFinite(Number(mergedEntry.sourceEnd)))
            mergedEntry.sourceEnd = resourceBlock.sourceEnd
        if (mergedType.length === 0 && resourceBlock.blockResourceType.length > 0)
            mergedEntry.type = resourceBlock.blockResourceType
        if (mergedFormat.length === 0 && resourceBlock.blockResourceFormat.length > 0)
            mergedEntry.format = resourceBlock.blockResourceFormat
        if (mergedResourceId.length === 0 && resourceBlock.blockResourceId.length > 0)
            mergedEntry.resourceId = resourceBlock.blockResourceId
        if (mergedResourcePath.length === 0 && resourceBlock.blockResourcePath.length > 0)
            mergedEntry.resourcePath = resourceBlock.blockResourcePath

        const effectiveType = mergedEntry.type !== undefined ? String(mergedEntry.type).trim().toLowerCase() : ""
        const effectiveFormat = mergedEntry.format !== undefined ? String(mergedEntry.format).trim().toLowerCase() : ""
        if (!resourceBlock.resourceEntryHasResolvedPayload
                || mergedRenderMode.length === 0
                || (mergedRenderMode === "document"
                    && (effectiveType === "image"
                        || effectiveType === "video"
                        || effectiveType === "audio"
                        || effectiveType === "music"
                        || effectiveFormat === ".pdf"))) {
            mergedEntry.renderMode = resourceBlock.fallbackRenderMode
        }

        return mergedEntry
    }
    property string interactionMode: "selected"
    readonly property bool afterBoundaryActive: resourceBlock.interactionMode === "after"
    readonly property bool beforeBoundaryActive: resourceBlock.interactionMode === "before"
    readonly property bool blockSelected: resourceBlock.interactionMode === "selected"

    implicitHeight: resourceCard.implicitHeight
    width: parent ? parent.width : implicitWidth

    function normalizedInteractionMode(mode) {
        const normalizedMode = mode === undefined || mode === null ? "" : String(mode).trim().toLowerCase()
        if (normalizedMode === "before" || normalizedMode === "selected" || normalizedMode === "after")
            return normalizedMode
        return "selected"
    }

    function setInteractionMode(mode) {
        const normalizedMode = resourceBlock.normalizedInteractionMode(mode)
        if (resourceBlock.interactionMode !== normalizedMode)
            resourceBlock.interactionMode = normalizedMode
    }

    function activateBoundaryEditor(mode) {
        const normalizedMode = resourceBlock.normalizedInteractionMode(mode)
        resourceBlock.setInteractionMode(normalizedMode)
        resourceBlock.forceActiveFocus()
        resourceBlock.activated()
        Qt.callLater(function () {
            if (normalizedMode === "before") {
                beforeBoundaryEditor.forceActiveFocus()
                return
            }
            if (normalizedMode === "after") {
                afterBoundaryEditor.forceActiveFocus()
                return
            }
            resourceBlock.forceActiveFocus()
        })
    }

    function selectResourceBlock() {
        resourceBlock.setInteractionMode("selected")
        resourceBlock.forceActiveFocus()
        resourceBlock.activated()
    }

    function tapInteractionMode(localX) {
        const blockWidth = Math.max(1, Number(resourceBlock.width) || Number(resourceCard.width) || 1)
        const normalizedX = Math.max(0, Math.min(blockWidth, Number(localX) || 0))
        const leftBoundary = blockWidth * 0.28
        const rightBoundary = blockWidth * 0.72
        if (normalizedX <= leftBoundary)
            return "before"
        if (normalizedX >= rightBoundary)
            return "after"
        return "selected"
    }

    function focusAdjacentTextOrBoundary(mode) {
        const normalizedMode = resourceBlock.normalizedInteractionMode(mode)
        const adjacentSourceOffset = normalizedMode === "before"
                ? Math.max(-1, Math.floor(Number(resourceBlock.beforeTextFocusSourceOffset) || -1))
                : Math.max(-1, Math.floor(Number(resourceBlock.afterTextFocusSourceOffset) || -1))
        if (adjacentSourceOffset >= 0) {
            resourceBlock.adjacentTextFocusRequested(adjacentSourceOffset)
            return true
        }
        resourceBlock.activateBoundaryEditor(normalizedMode)
        return true
    }

    function commitBoundaryPlainText(mode, text, cursorPosition) {
        const normalizedText = StructuredCursorSupport.normalizedPlainText(text)
        if (normalizedText.length === 0)
            return false
        resourceBlock.adjacentPlainTextInsertionRequested(
                    resourceBlock.normalizedInteractionMode(mode),
                    normalizedText,
                    Math.max(0, Math.floor(Number(cursorPosition) || 0)))
        return true
    }

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < resourceBlock.sourceStart || sourceOffset > resourceBlock.sourceEnd)
            return false
        if (sourceOffset <= resourceBlock.sourceStart) {
            resourceBlock.activateBoundaryEditor("before")
            return true
        }
        if (sourceOffset >= resourceBlock.sourceEnd) {
            resourceBlock.activateBoundaryEditor("after")
            return true
        }
        resourceBlock.selectResourceBlock()
        return true
    }

    function shortcutInsertionSourceOffset() {
        if (resourceBlock.beforeBoundaryActive)
            return resourceBlock.sourceStart
        return resourceBlock.focusSourceOffset
    }

    ContentsResourceRenderCard {
        id: resourceCard

        anchors.left: parent.left
        anchors.right: parent.right
        inlinePresentation: true
        resourceEntry: resourceBlock.effectiveResourceEntry
    }

    Rectangle {
        anchors.fill: resourceCard
        border.color: LV.Theme.panelBackground10
        border.width: resourceBlock.blockSelected ? Math.max(1, Math.round(LV.Theme.strokeThin)) : 0
        color: "transparent"
        radius: Math.max(Math.round(LV.Theme.scaleMetric(12)), Number(resourceCard.radius) || 0)
        visible: resourceBlock.blockSelected
        z: 2
    }

    ContentsInlineFormatEditor {
        id: beforeBoundaryEditor

        anchors.left: resourceCard.left
        anchors.leftMargin: resourceBlock.boundaryEditorInset
        anchors.verticalCenter: resourceCard.verticalCenter
        autoFocusOnPress: false
        backgroundColor: "transparent"
        backgroundColorDisabled: "transparent"
        backgroundColorFocused: "transparent"
        backgroundColorHover: "transparent"
        backgroundColorPressed: "transparent"
        centeredTextHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
        cornerRadius: 0
        fieldMinHeight: resourceBlock.boundaryEditorHeight
        fontFamily: LV.Theme.fontBody
        fontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
        fontWeight: Font.Medium
        height: resourceBlock.boundaryEditorHeight
        insetHorizontal: 0
        insetVertical: 0
        placeholderText: ""
        selectByMouse: false
        selectedTextColor: LV.Theme.textPrimary
        selectionColor: LV.Theme.accent
        showRenderedOutput: false
        showScrollBar: false
        text: ""
        textColor: LV.Theme.textPrimary
        textFormat: TextEdit.PlainText
        visible: resourceBlock.beforeBoundaryActive
        width: resourceBlock.boundaryEditorWidth
        wrapMode: TextEdit.NoWrap
        z: 3

        onCursorPositionChanged: {
            if (focused)
                resourceBlock.activated()
        }
        onFocusedChanged: {
            if (focused) {
                resourceBlock.setInteractionMode("before")
                resourceBlock.activated()
                return
            }
            if (resourceBlock.beforeBoundaryActive)
                resourceBlock.setInteractionMode("selected")
        }
        onTextEdited: function (text) {
            resourceBlock.commitBoundaryPlainText(
                        "before",
                        String(text || ""),
                        Math.max(0, Number(beforeBoundaryEditor.cursorPosition) || 0))
        }
    }

    ContentsInlineFormatEditor {
        id: afterBoundaryEditor

        anchors.right: resourceCard.right
        anchors.rightMargin: resourceBlock.boundaryEditorInset
        anchors.verticalCenter: resourceCard.verticalCenter
        autoFocusOnPress: false
        backgroundColor: "transparent"
        backgroundColorDisabled: "transparent"
        backgroundColorFocused: "transparent"
        backgroundColorHover: "transparent"
        backgroundColorPressed: "transparent"
        centeredTextHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
        cornerRadius: 0
        fieldMinHeight: resourceBlock.boundaryEditorHeight
        fontFamily: LV.Theme.fontBody
        fontPixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
        fontWeight: Font.Medium
        height: resourceBlock.boundaryEditorHeight
        insetHorizontal: 0
        insetVertical: 0
        placeholderText: ""
        selectByMouse: false
        selectedTextColor: LV.Theme.textPrimary
        selectionColor: LV.Theme.accent
        showRenderedOutput: false
        showScrollBar: false
        text: ""
        textColor: LV.Theme.textPrimary
        textFormat: TextEdit.PlainText
        visible: resourceBlock.afterBoundaryActive
        width: resourceBlock.boundaryEditorWidth
        wrapMode: TextEdit.NoWrap
        z: 3

        onCursorPositionChanged: {
            if (focused)
                resourceBlock.activated()
        }
        onFocusedChanged: {
            if (focused) {
                resourceBlock.setInteractionMode("after")
                resourceBlock.activated()
                return
            }
            if (resourceBlock.afterBoundaryActive)
                resourceBlock.setInteractionMode("selected")
        }
        onTextEdited: function (text) {
            resourceBlock.commitBoundaryPlainText(
                        "after",
                        String(text || ""),
                        Math.max(0, Number(afterBoundaryEditor.cursorPosition) || 0))
        }
    }

    Keys.onPressed: function (event) {
        if (!event)
            return
        if (event.key === Qt.Key_Left) {
            if (resourceBlock.focusAdjacentTextOrBoundary("before"))
                event.accepted = true
            return
        }
        if (event.key === Qt.Key_Right) {
            if (resourceBlock.focusAdjacentTextOrBoundary("after"))
                event.accepted = true
            return
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: function (eventPoint) {
            const interactionMode = resourceBlock.tapInteractionMode(
                        eventPoint && eventPoint.position ? eventPoint.position.x : 0)
            if (interactionMode === "before") {
                resourceBlock.focusAdjacentTextOrBoundary("before")
                return
            }
            if (interactionMode === "after") {
                resourceBlock.focusAdjacentTextOrBoundary("after")
                return
            }
            resourceBlock.selectResourceBlock()
        }
    }
}
