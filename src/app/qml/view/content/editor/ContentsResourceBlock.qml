pragma ComponentBehavior: Bound

import QtQuick

FocusScope {
    id: resourceBlock

    required property var blockData
    property var resourceEntry: ({})

    signal activated()

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
        const hasResolvedPayload = resolvedEntry
                && typeof resolvedEntry === "object"
                && ((resolvedEntry.source !== undefined && String(resolvedEntry.source).trim().length > 0)
                    || (resolvedEntry.resolvedPath !== undefined && String(resolvedEntry.resolvedPath).trim().length > 0)
                    || (resolvedEntry.renderMode !== undefined && String(resolvedEntry.renderMode).trim().length > 0))
        if (hasResolvedPayload)
            return resolvedEntry
        return {
            "displayName": resourceBlock.blockResourceId.length > 0
                               ? resourceBlock.blockResourceId
                               : resourceBlock.blockResourcePath,
            "focusSourceOffset": resourceBlock.focusSourceOffset,
            "format": resourceBlock.blockResourceFormat,
            "renderMode": resourceBlock.fallbackRenderMode,
            "resourceId": resourceBlock.blockResourceId,
            "resourcePath": resourceBlock.blockResourcePath,
            "sourceEnd": resourceBlock.sourceEnd,
            "sourceStart": resourceBlock.sourceStart,
            "type": resourceBlock.blockResourceType
        }
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
            resourceBlock.forceActiveFocus()
            resourceBlock.activated()
        }
    }
}
