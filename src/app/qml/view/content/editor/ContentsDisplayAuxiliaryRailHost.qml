pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: auxiliaryHost

    required property var bodyResourceRenderer
    required property var contentsAgendaBackend
    required property var contentsCalloutBackend
    required property var contentsView
    required property var editorProjection
    required property var editorTypingController
    required property var resourceImportController
    required property var structuredBlockRenderer
    required property var viewportCoordinator

    readonly property var editorViewport: surfaceHost.editorViewport
    readonly property var inputCommandSurface: surfaceHost.inputCommandSurface
    readonly property var minimapLayer: minimapLayerItem
    readonly property var printDocumentViewport: surfaceHost.printDocumentViewport
    readonly property var structuredDocumentFlow: surfaceHost.structuredDocumentFlow
    readonly property var structuredDocumentViewport: surfaceHost.structuredDocumentViewport

    function pointInsideMappedItem(targetItem, sourceItem, sourceX, sourceY) {
        if (!targetItem || !sourceItem || sourceItem.mapToItem === undefined)
            return false
        if (targetItem.visible !== undefined && !targetItem.visible)
            return false
        const mappedPoint = sourceItem.mapToItem(targetItem, sourceX, sourceY)
        const targetWidth = Math.max(0, Number(targetItem.width) || 0)
        const targetHeight = Math.max(0, Number(targetItem.height) || 0)
        return mappedPoint.x >= 0 && mappedPoint.y >= 0
                && mappedPoint.x <= targetWidth
                && mappedPoint.y <= targetHeight
    }

    function requestTerminalBodyClickFromActivationPoint(localX, localY) {
        if (!auxiliaryHost.contentsView.hasSelectedNote
                || auxiliaryHost.contentsView.noteDocumentExceptionVisible)
            return false
        if (auxiliaryHost.pointInsideMappedItem(gutterHost, editorActivationSurface, localX, localY))
            return false
        if (auxiliaryHost.pointInsideMappedItem(minimapLayerItem, editorActivationSurface, localX, localY))
            return false
        if (!surfaceHost || surfaceHost.requestTerminalBodyClickFromEditorAreaPoint === undefined)
            return false
        return surfaceHost.requestTerminalBodyClickFromEditorAreaPoint(
                    editorActivationSurface,
                    localX,
                    localY)
    }

    Item {
        id: editorActivationSurface

        anchors.fill: parent
        enabled: visible
        visible: auxiliaryHost.contentsView.hasSelectedNote
                 && !auxiliaryHost.contentsView.noteDocumentExceptionVisible
        z: 0

        TapHandler {
            id: editorWholeSurfaceTrailingMarginTapHandler

            acceptedButtons: Qt.LeftButton
            gesturePolicy: TapHandler.ReleaseWithinBounds
            grabPermissions: PointerHandler.ApprovesTakeOverByAnything
            target: null

            onTapped: function (eventPoint, button) {
                if (button !== Qt.LeftButton)
                    return
                const localX = eventPoint && eventPoint.position ? Number(eventPoint.position.x) || 0 : 0
                const localY = eventPoint && eventPoint.position ? Number(eventPoint.position.y) || 0 : 0
                auxiliaryHost.requestTerminalBodyClickFromActivationPoint(localX, localY)
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: auxiliaryHost.contentsView.effectiveFrameHorizontalInset
        anchors.rightMargin: auxiliaryHost.contentsView.effectiveFrameHorizontalInset
                             + (auxiliaryHost.contentsView.showMinimapRail ? auxiliaryHost.contentsView.minimapOuterWidth : 0)
        LayoutMirroring.childrenInherit: false
        LayoutMirroring.enabled: false
        layoutDirection: Qt.LeftToRight
        spacing: 0
        visible: auxiliaryHost.contentsView.hasSelectedNote
                 && !auxiliaryHost.contentsView.noteDocumentExceptionVisible
        z: 1

        ContentsDisplayGutterHost {
            id: gutterHost

            contentsView: auxiliaryHost.contentsView
        }

        ContentsDisplaySurfaceHost {
            id: surfaceHost

            bodyResourceRenderer: auxiliaryHost.bodyResourceRenderer
            contentsAgendaBackend: auxiliaryHost.contentsAgendaBackend
            contentsCalloutBackend: auxiliaryHost.contentsCalloutBackend
            contentsView: auxiliaryHost.contentsView
            editorProjection: auxiliaryHost.editorProjection
            editorTypingController: auxiliaryHost.editorTypingController
            resourceImportController: auxiliaryHost.resourceImportController
            structuredBlockRenderer: auxiliaryHost.structuredBlockRenderer
        }

        ContentsDisplayMinimapRailHost {
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.fillHeight: true
            Layout.maximumWidth: 0
            Layout.minimumWidth: 0
            Layout.preferredWidth: 0
            contentsView: auxiliaryHost.contentsView
            viewportCoordinator: auxiliaryHost.viewportCoordinator
            visible: false
        }
    }

    ContentsDisplayMinimapRailHost {
        id: minimapLayerItem

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: contentsView.effectiveFrameHorizontalInset
        anchors.top: parent.top
        contentsView: auxiliaryHost.contentsView
        viewportCoordinator: auxiliaryHost.viewportCoordinator
        visible: contentsView.showMinimapRail && contentsView.noteDocumentParseMounted
        width: visible ? contentsView.minimapOuterWidth : 0
    }
}
