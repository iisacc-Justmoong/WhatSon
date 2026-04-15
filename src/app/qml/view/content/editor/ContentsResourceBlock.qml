pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

FocusScope {
    id: resourceBlock

    required property var blockData
    property var resourceEntry: ({})

    signal activated()
    signal boundaryNavigationRequested(string axis, string side)
    signal blockDeletionRequested()
    signal documentEndEditRequested()

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property var normalizedResourceEntry: resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({})
    readonly property bool focused: resourceBlock.activeFocus
    readonly property int currentLogicalLineNumber: 1
    readonly property bool textEditable: false
    readonly property bool atomicBlock: true
    readonly property bool gutterCollapsed: true
    readonly property string minimapVisualKind: "block"
    readonly property int minimapRepresentativeCharCount: 160
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

    implicitHeight: resourceCard.implicitHeight
    width: parent ? parent.width : implicitWidth

    function selectResourceBlock() {
        resourceBlock.forceActiveFocus()
        resourceBlock.activated()
    }

    function deleteSelectedBlock() {
        resourceBlock.blockDeletionRequested()
        return true
    }

    function handleDeleteKeyPress(event) {
        if (!event)
            return false
        if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
            if (resourceBlock.deleteSelectedBlock()) {
                event.accepted = true
                return true
            }
        }
        return false
    }

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < resourceBlock.sourceStart || sourceOffset > resourceBlock.sourceEnd)
            return false
        resourceBlock.selectResourceBlock()
        return true
    }

    function logicalLineLayoutEntries() {
        const mappedOrigin = resourceCard.mapToItem !== undefined
                ? resourceCard.mapToItem(resourceBlock, 0, 0)
                : ({ "x": 0, "y": 0 })
        return [{
                    "contentHeight": Math.max(
                                         1,
                                         Number(resourceCard.implicitHeight) || Number(resourceCard.height) || Math.round(LV.Theme.scaleMetric(12))),
                    "contentY": Math.max(0, Number(mappedOrigin.y) || 0)
                }]
    }

    function visiblePlainText() {
        return ""
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
        if (normalizedLineText.length > 0)
            return normalizedLineText.length
        const resourceLabel = resourceBlock.blockResourceId.length > 0
                ? resourceBlock.blockResourceId
                : (resourceBlock.blockResourcePath.length > 0 ? resourceBlock.blockResourcePath : "resource")
        return Math.max(12, resourceLabel.length)
    }

    function currentCursorRowRect() {
        const entries = resourceBlock.logicalLineLayoutEntries()
        if (entries.length > 0)
            return entries[0]
        return ({
                    "contentHeight": Math.max(1, Math.round(LV.Theme.scaleMetric(12))),
                    "contentY": 0
                })
    }

    function shortcutInsertionSourceOffset() {
        return resourceBlock.focusSourceOffset
    }

    ContentsResourceRenderCard {
        id: resourceCard

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        inlinePresentation: true
        resourceEntry: resourceBlock.effectiveResourceEntry
    }

    Rectangle {
        anchors.fill: resourceCard
        border.color: LV.Theme.panelBackground10
        border.width: resourceBlock.focused ? Math.max(1, Math.round(LV.Theme.strokeThin)) : 0
        color: "transparent"
        radius: Math.max(Math.round(LV.Theme.scaleMetric(12)), Number(resourceCard.radius) || 0)
        visible: resourceBlock.focused
        z: 2
    }

    Keys.onPressed: function (event) {
        if (!event)
            return
        if (resourceBlock.handleDeleteKeyPress(event))
            return
        if (event.key === Qt.Key_Up) {
            resourceBlock.boundaryNavigationRequested("vertical", "before")
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Down) {
            resourceBlock.boundaryNavigationRequested("vertical", "after")
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Left) {
            resourceBlock.boundaryNavigationRequested("horizontal", "before")
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Right) {
            resourceBlock.boundaryNavigationRequested("horizontal", "after")
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            resourceBlock.documentEndEditRequested()
            event.accepted = true
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: function () {
            if (tapCount >= 2) {
                resourceBlock.documentEndEditRequested()
                return
            }
            resourceBlock.selectResourceBlock()
        }
    }
}
