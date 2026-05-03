pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0

Item {
    id: resourceViewer

    property var bitmapViewer: null
    property var resourceEntry: ({})
    readonly property var resolvedBitmapViewer: resourceViewer.bitmapViewer
                                                ? resourceViewer.bitmapViewer
                                                : internalBitmapState

    signal viewHookRequested(string reason)

    ResourceBitmapViewer {
        id: internalBitmapState

        resourceEntry: resourceViewer.resourceEntry
    }

    Image {
        id: bitmapImage

        anchors.fill: parent
        asynchronous: true
        fillMode: Image.PreserveAspectFit
        smooth: true
        source: resourceViewer.resolvedBitmapViewer.viewerSource
        visible: resourceViewer.resolvedBitmapViewer.bitmapRenderable
    }
}
