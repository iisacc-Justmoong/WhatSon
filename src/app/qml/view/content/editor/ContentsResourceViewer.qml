pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Pdf
import WhatSon.App.Internal 1.0

Item {
    id: resourceViewer

    property var resourceEntry: ({})
    readonly property string resourceRenderMode: resourceEntry.renderMode !== undefined ? String(resourceEntry.renderMode) : ""
    readonly property string resourceOpenTarget: bitmapViewer.openTarget
    readonly property bool imageRenderable: resourceViewer.resourceRenderMode === "image"
                                            && bitmapViewer.bitmapRenderable
    readonly property bool pdfRenderable: resourceViewer.resourceRenderMode === "pdf"
                                          && resourceViewer.resourceOpenTarget.length > 0

    clip: true

    ResourceBitmapViewer {
        id: bitmapViewer

        resourceEntry: resourceViewer.resourceEntry
    }

    PdfDocument {
        id: resourcePdfDocument

        source: resourceViewer.pdfRenderable ? resourceViewer.resourceOpenTarget : ""
    }

    Image {
        id: resourceImageViewer

        anchors.fill: parent
        asynchronous: true
        cache: true
        fillMode: Image.PreserveAspectFit
        mipmap: true
        source: resourceViewer.imageRenderable ? bitmapViewer.viewerSource : ""
        visible: resourceViewer.imageRenderable
    }

    PdfMultiPageView {
        id: resourcePdfViewer

        anchors.fill: parent
        clip: true
        document: resourcePdfDocument
        visible: resourceViewer.pdfRenderable
    }
}
