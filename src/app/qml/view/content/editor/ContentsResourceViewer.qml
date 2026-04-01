pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Pdf
import LVRS 1.0 as LV

Item {
    id: resourceViewer

    property var resourceEntry: ({})
    readonly property string resourceDisplayName: resourceEntry.displayName !== undefined ? String(resourceEntry.displayName) : ""
    readonly property string resourceType: resourceEntry.type !== undefined ? String(resourceEntry.type) : ""
    readonly property string resourceFormat: resourceEntry.format !== undefined ? String(resourceEntry.format) : ""
    readonly property string resourceRenderMode: resourceEntry.renderMode !== undefined ? String(resourceEntry.renderMode) : ""
    readonly property string resourceSource: resourceEntry.source !== undefined ? String(resourceEntry.source) : ""
    readonly property string resourceResolvedPath: resourceEntry.resolvedPath !== undefined ? String(resourceEntry.resolvedPath) : ""
    readonly property string resourceOpenTarget: {
        const sourceValue = resourceViewer.resourceSource.trim();
        if (sourceValue.length > 0)
            return sourceValue;
        return resourceViewer.toViewerTarget(resourceViewer.resourceResolvedPath);
    }
    readonly property bool imageRenderable: resourceViewer.resourceRenderMode === "image"
                                            && resourceViewer.resourceOpenTarget.length > 0
    readonly property bool pdfRenderable: resourceViewer.resourceRenderMode === "pdf"
                                          && resourceViewer.resourceOpenTarget.length > 0
    readonly property bool openable: resourceViewer.resourceOpenTarget.length > 0
    readonly property string modeTitle: {
        if (resourceViewer.resourceRenderMode === "image")
            return "Image Resource";
        if (resourceViewer.resourceRenderMode === "pdf")
            return "PDF Resource";
        if (resourceViewer.resourceRenderMode === "video")
            return "Video Resource";
        if (resourceViewer.resourceRenderMode === "audio")
            return "Audio Resource";
        if (resourceViewer.resourceRenderMode === "text")
            return "Text Resource";
        return "Document Resource";
    }

    function toViewerTarget(pathOrUrl) {
        const value = pathOrUrl === undefined || pathOrUrl === null ? "" : String(pathOrUrl).trim();
        if (value.length === 0)
            return "";
        if (value.indexOf("://") >= 0 || value.startsWith("qrc:/"))
            return value;
        if (value.startsWith("/"))
            return "file://" + value;
        return value;
    }

    clip: true

    Rectangle {
        anchors.fill: parent
        color: "#CC0F141A"
        radius: LV.Theme.radiusSm
    }

    PdfDocument {
        id: resourcePdfDocument

        source: resourceViewer.pdfRenderable ? resourceViewer.resourceOpenTarget : ""
    }

    Image {
        id: resourceImageViewer

        anchors.fill: parent
        anchors.margins: LV.Theme.gap2
        asynchronous: true
        cache: true
        fillMode: Image.PreserveAspectFit
        mipmap: true
        source: resourceViewer.imageRenderable ? resourceViewer.resourceOpenTarget : ""
        visible: resourceViewer.imageRenderable
    }

    PdfMultiPageView {
        id: resourcePdfViewer

        anchors.fill: parent
        anchors.margins: LV.Theme.gap2
        clip: true
        document: resourcePdfDocument
        visible: resourceViewer.pdfRenderable
    }

    LV.VStack {
        anchors.centerIn: parent
        spacing: LV.Theme.gap2
        visible: !resourceViewer.imageRenderable && !resourceViewer.pdfRenderable

        LV.Label {
            color: LV.Theme.textPrimary
            horizontalAlignment: Text.AlignHCenter
            style: body
            text: resourceViewer.resourceDisplayName.length > 0
                  ? resourceViewer.resourceDisplayName
                  : resourceViewer.modeTitle
            width: 320
        }
        LV.Label {
            color: LV.Theme.textSecondary
            horizontalAlignment: Text.AlignHCenter
            style: body
            text: resourceViewer.modeTitle
            width: 320
        }
        LV.IconButton {
            anchors.horizontalCenter: parent.horizontalCenter
            iconName: "generalshow"
            iconSize: 14
            visible: resourceViewer.openable

            onClicked: Qt.openUrlExternally(resourceViewer.resourceOpenTarget)
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.margins: LV.Theme.gap2
        anchors.top: parent.top
        color: "#D91A1D22"
        radius: LV.Theme.radiusSm

        LV.HStack {
            anchors.fill: parent
            anchors.margins: LV.Theme.gap2
            spacing: LV.Theme.gap2

            LV.Label {
                color: LV.Theme.textPrimary
                style: body
                text: resourceViewer.resourceDisplayName.length > 0
                      ? resourceViewer.resourceDisplayName
                      : resourceViewer.modeTitle
            }
            LV.Label {
                color: LV.Theme.textSecondary
                style: body
                text: resourceViewer.resourceType.length > 0 || resourceViewer.resourceFormat.length > 0
                      ? "type=" + resourceViewer.resourceType + "  format=" + resourceViewer.resourceFormat
                      : resourceViewer.modeTitle
            }
        }
    }
}
