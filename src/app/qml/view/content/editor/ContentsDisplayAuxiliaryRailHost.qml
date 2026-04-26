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

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: contentsView.effectiveFrameHorizontalInset
        anchors.rightMargin: contentsView.effectiveFrameHorizontalInset
                             + (contentsView.showMinimapRail ? contentsView.minimapOuterWidth : 0)
        LayoutMirroring.childrenInherit: false
        LayoutMirroring.enabled: false
        layoutDirection: Qt.LeftToRight
        spacing: 0
        visible: contentsView.hasSelectedNote && contentsView.noteDocumentSurfaceVisible

        ContentsDisplayGutterHost {
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
