.pragma library

function normalizedString(value) {
    if (value === undefined || value === null)
        return ""
    return String(value)
}

function sanitizedDetail(detail) {
    const text = normalizedString(detail).replace(/\n/g, "\\n").replace(/\r/g, "\\r")
    if (text.length <= 320)
        return text
    return text.slice(0, 320) + "...(truncated)"
}

function describeText(value, previewLength) {
    const text = normalizedString(value)
    const safePreviewLength = Math.max(0, Number(previewLength) || 96)
    const preview = sanitizedDetail(text).slice(0, safePreviewLength)
    if (text.length <= safePreviewLength)
        return "len=" + text.length + " preview=\"" + preview + "\""
    return "len=" + text.length + " preview=\"" + preview + "\"...(truncated)"
}

function describeValue(value) {
    if (value === undefined)
        return "undefined"
    if (value === null)
        return "null"
    if (typeof value === "string")
        return describeText(value, 72)
    if (typeof value === "number" || typeof value === "boolean")
        return String(value)
    if (Array.isArray(value))
        return "array(len=" + value.length + ")"
    if (typeof value === "object") {
        try {
            const keys = Object.keys(value)
            return "object(keys=" + keys.slice(0, 8).join(",") + (keys.length > 8 ? ",..." : "") + ")"
        } catch (error) {
            return "object(error=" + normalizedString(error) + ")"
        }
    }
    return normalizedString(value)
}

function describeObject(objectValue, keys) {
    if (!objectValue || typeof objectValue !== "object")
        return describeValue(objectValue)
    const fieldNames = Array.isArray(keys) && keys.length > 0 ? keys : Object.keys(objectValue)
    const parts = []
    for (let index = 0; index < fieldNames.length; ++index) {
        const key = normalizedString(fieldNames[index])
        if (key.length === 0)
            continue
        parts.push(key + "=" + describeValue(objectValue[key]))
    }
    return parts.join(" ")
}

function describeFocusRequest(request) {
    return describeObject(request, [
                              "sourceOffset",
                              "targetBlockIndex",
                              "entryBoundary",
                              "preferNearestTextBlock",
                              "localCursorPosition",
                              "taskOpenTagStart"
                          ])
}

function describeSelection(selection) {
    return describeObject(selection, [
                              "selectionStart",
                              "selectionEnd",
                              "cursorPosition",
                              "selectedText"
                          ])
}

function scopeName(scope) {
    const normalizedScope = normalizedString(scope).trim()
    return normalizedScope.length > 0 ? normalizedScope : "unknown"
}

function trace(scope, action, detail, self) {
    let message = "[whatson:debug][editor.qml][" + scopeName(scope) + "][" + scopeName(action) + "]"
    if (self && self.objectName !== undefined) {
        const objectName = normalizedString(self.objectName).trim()
        if (objectName.length > 0)
            message += " objectName=" + objectName
    }
    const normalizedDetail = sanitizedDetail(detail)
    if (normalizedDetail.length > 0)
        message += " " + normalizedDetail
    console.warn(message)
}
