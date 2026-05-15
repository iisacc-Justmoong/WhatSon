pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: imageEditor

    property color backgroundColor: LV.Theme.panelBackground01
    property var resourceEntry: ({})
    readonly property string displayName: imageEditor.stringEntryValue("displayName")
    readonly property string entryFormat: imageEditor.stringEntryValue("format").toLowerCase()
    readonly property string entryResourcePath: imageEditor.stringEntryValue("resourcePath")
    readonly property string entryResolvedPath: imageEditor.stringEntryValue("resolvedPath")
    readonly property string entrySource: imageEditor.stringEntryValue("source")
    readonly property string entryType: imageEditor.stringEntryValue("type").toLowerCase()
    readonly property string effectiveImageSource: imageEditor.imageResource
            ? imageEditor.urlSourceFor(imageEditor.entrySource.length > 0
                                       ? imageEditor.entrySource
                                       : (imageEditor.entryResolvedPath.length > 0
                                          ? imageEditor.entryResolvedPath
                                          : imageEditor.entryResourcePath))
            : ""
    readonly property bool imageResource: imageEditor.entryType === "image"
            || imageEditor.formatLooksLikeImage(imageEditor.entryFormat)

    function formatLooksLikeImage(format) {
        const normalizedFormat = format === undefined || format === null
                ? ""
                : String(format).trim().toLowerCase();
        const suffixedFormat = normalizedFormat.startsWith(".")
                ? normalizedFormat
                : "." + normalizedFormat;
        return suffixedFormat === ".avif"
                || suffixedFormat === ".bmp"
                || suffixedFormat === ".gif"
                || suffixedFormat === ".heic"
                || suffixedFormat === ".heif"
                || suffixedFormat === ".jpeg"
                || suffixedFormat === ".jpg"
                || suffixedFormat === ".png"
                || suffixedFormat === ".svg"
                || suffixedFormat === ".webp";
    }
    function stringEntryValue(key) {
        if (!imageEditor.resourceEntry || typeof imageEditor.resourceEntry !== "object")
            return "";
        const value = imageEditor.resourceEntry[key];
        if (value === undefined || value === null)
            return "";
        return String(value).trim();
    }
    function urlSourceFor(value) {
        const normalizedValue = value === undefined || value === null
                ? ""
                : String(value).trim();
        if (normalizedValue.length === 0)
            return "";
        if (normalizedValue.match(/^[A-Za-z][A-Za-z0-9+.-]*:/))
            return normalizedValue;
        if (normalizedValue.startsWith("/"))
            return "file://" + normalizedValue;
        return normalizedValue;
    }

    objectName: "contentsImageEditor"

    Rectangle {
        anchors.fill: parent
        color: imageEditor.backgroundColor
    }

    Item {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap24
        clip: true

        Image {
            id: imagePreview

            anchors.fill: parent
            asynchronous: true
            fillMode: Image.PreserveAspectFit
            mipmap: true
            smooth: true
            source: imageEditor.effectiveImageSource
            visible: imageEditor.effectiveImageSource.length > 0
        }
    }

    LV.Label {
        anchors.centerIn: parent
        color: LV.Theme.descriptionColor
        horizontalAlignment: Text.AlignHCenter
        style: caption
        text: imageEditor.displayName
        verticalAlignment: Text.AlignVCenter
        visible: imageEditor.effectiveImageSource.length === 0 && imageEditor.displayName.length > 0
    }
}
