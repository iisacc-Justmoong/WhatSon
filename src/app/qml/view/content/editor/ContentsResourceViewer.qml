pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Pdf
import WhatSon.App.Internal 1.0

Item {
    id: resourceViewer

    property var resourceEntry: ({})
    property int imageFillMode: Image.PreserveAspectFit
    property bool imageAllowUpscale: true
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
    readonly property real resolvedImageScale: {
        const naturalWidth = Number(resourceViewer.imagePixelWidth) || 0
        const naturalHeight = Number(resourceViewer.imagePixelHeight) || 0
        const availableWidth = Number(resourceViewer.width) || 0
        const availableHeight = Number(resourceViewer.height) || 0
        if (naturalWidth <= 0 || naturalHeight <= 0 || availableWidth <= 0 || availableHeight <= 0)
            return 0
        const fittedScale = Math.min(availableWidth / naturalWidth, availableHeight / naturalHeight)
        if (!isFinite(fittedScale) || fittedScale <= 0)
            return 0
        return resourceViewer.imageAllowUpscale ? fittedScale : Math.min(1, fittedScale)
    }
    readonly property real resolvedImageWidth: {
        const naturalWidth = Number(resourceViewer.imagePixelWidth) || 0
        if (naturalWidth <= 0)
            return 0
        const resolvedScale = Number(resourceViewer.resolvedImageScale) || 0
        return resolvedScale > 0 ? naturalWidth * resolvedScale : 0
    }
    readonly property real resolvedImageHeight: {
        const naturalHeight = Number(resourceViewer.imagePixelHeight) || 0
        if (naturalHeight <= 0)
            return 0
        const resolvedScale = Number(resourceViewer.resolvedImageScale) || 0
        return resolvedScale > 0 ? naturalHeight * resolvedScale : 0
    }

    clip: true

    ResourceBitmapViewer {
        id: bitmapViewer

        resourceEntry: resourceViewer.resourceEntry
    }

    PdfDocument {
        id: resourcePdfDocument

        source: resourceViewer.pdfRenderable ? resourceViewer.resourceOpenTarget : ""
    }

    Item {
        id: imageViewport

        anchors.centerIn: parent
        height: resourceViewer.resolvedImageHeight
        visible: resourceViewer.imageRenderable
        width: resourceViewer.resolvedImageWidth

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
    }

    PdfMultiPageView {
        id: resourcePdfViewer

        anchors.fill: parent
        clip: true
        document: resourcePdfDocument
        visible: resourceViewer.pdfRenderable
    }
}
