pragma ComponentBehavior: Bound

import QtQuick

FocusScope {
    id: resourceBlock

    required property var blockData
    property var resourceEntry: ({})

    signal activated()
    signal documentEndEditRequested()

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property var normalizedResourceEntry: resourceEntry && typeof resourceEntry === "object" ? resourceEntry : ({})
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

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < resourceBlock.sourceStart || sourceOffset > resourceBlock.sourceEnd)
            return false
        resourceBlock.forceActiveFocus()
        resourceBlock.activated()
        return true
    }

    function shortcutInsertionSourceOffset() {
        return resourceBlock.focusSourceOffset
    }

    ContentsResourceRenderCard {
        id: resourceCard

        anchors.left: parent.left
        anchors.right: parent.right
        inlinePresentation: true
        resourceEntry: resourceBlock.effectiveResourceEntry
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: {
            resourceBlock.documentEndEditRequested()
        }
    }
}
