pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Pdf
import WhatSon.App.Internal 1.0

Item {
    id: resourceViewer

    property var resourceEntry: ({})
    property int imageFillMode: Image.PreserveAspectFit
    readonly property string resourceRenderMode: resourceEntry.renderMode !== undefined ? String(resourceEntry.renderMode) : ""
    readonly property string resourceOpenTarget: bitmapViewer.openTarget
    readonly property bool imageRenderable: bitmapViewer.bitmapRenderable
    readonly property bool pdfRenderable: resourceViewer.resourceRenderMode === "pdf"
                                          && resourceViewer.resourceOpenTarget.length > 0
    readonly property bool resourceRenderable: resourceViewer.imageRenderable || resourceViewer.pdfRenderable
    readonly property string renderFailureReason: bitmapViewer.incompatibilityReason
    readonly property real imagePixelWidth: {
        const sourceWidth = Number(resourceImageViewer.sourceSize.width) || 0
        if (sourceWidth > 0)
            return sourceWidth
        const implicitWidth = Number(resourceImageViewer.implicitWidth) || 0
        return implicitWidth > 0 ? implicitWidth : 0
    }
    readonly property real imagePixelHeight: {
        const sourceHeight = Number(resourceImageViewer.sourceSize.height) || 0
        if (sourceHeight > 0)
            return sourceHeight
        const implicitHeight = Number(resourceImageViewer.implicitHeight) || 0
        return implicitHeight > 0 ? implicitHeight : 0
    }
    readonly property real imageAspectRatio: resourceViewer.imagePixelWidth > 0
                                             && resourceViewer.imagePixelHeight > 0
                                             ? resourceViewer.imagePixelWidth / resourceViewer.imagePixelHeight
                                             : 0

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
        fillMode: resourceViewer.imageFillMode
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
