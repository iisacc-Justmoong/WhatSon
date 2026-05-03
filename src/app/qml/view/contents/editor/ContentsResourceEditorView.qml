pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0

Item {
    id: resourceEditor

    property color displayColor: "transparent"
    property var resourceEntry: ({})
    readonly property bool hasResourceSelection: resourceEntry !== null
                                                && resourceEntry !== undefined
                                                && typeof resourceEntry === "object"
                                                && Object.keys(resourceEntry).length > 0

    signal viewHookRequested

    ResourceBitmapViewer {
        id: resourceBitmapState

        resourceEntry: resourceEditor.resourceEntry
    }

    ContentsResourceViewer {
        anchors.fill: parent
        bitmapViewer: resourceBitmapState
        resourceEntry: resourceEditor.resourceEntry
        visible: resourceEditor.hasResourceSelection && resourceBitmapState.bitmapRenderable
    }
}
