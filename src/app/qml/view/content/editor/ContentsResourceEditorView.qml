pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0

Item {
    id: resourceEditor

    property color displayColor: "transparent"
    property var resourceEntry: ({})

    readonly property string resourceId: resourceEditor.resourceEntry.noteId !== undefined
                                         ? String(resourceEditor.resourceEntry.noteId).trim()
                                         : ""
    readonly property bool hasResourceSelection: resourceEditor.resourceId.length > 0

    signal viewHookRequested

    function requestViewHook(reason) {
        resourceEditor.viewHookRequested();
    }

    clip: true

    ResourceBitmapViewer {
        id: resourceBitmapState

        resourceEntry: resourceEditor.resourceEntry
    }

    ContentsResourceViewer {
        anchors.fill: parent
        imageAllowUpscale: false
        imageFillMode: Image.PreserveAspectFit
        resourceEntry: resourceEditor.resourceEntry
        visible: resourceEditor.hasResourceSelection && resourceBitmapState.bitmapRenderable
    }
}
