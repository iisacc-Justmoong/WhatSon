pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../../../../models/editor/display" as EditorDisplayModel

Item {
    id: surfaceHost

    required property var bodyResourceRenderer
    required property var contentsAgendaBackend
    required property var contentsCalloutBackend
    required property var contentsView
    required property var editorProjection
    required property var editorTypingController
    required property var resourceImportController
    required property var structuredBlockRenderer

    readonly property var editorViewport: surfaceHost
    property alias inputCommandSurface: inputCommandSurfaceItem
    property alias printDocumentViewport: printDocumentViewportItem
    property alias structuredDocumentFlow: structuredDocumentFlowItem
    property alias structuredDocumentViewport: structuredDocumentViewportItem

    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.minimumHeight: contentsView.minEditorHeight
    clip: true
    enabled: !contentsView.selectedNoteBodyLoading

    Flickable {
        id: printDocumentViewportItem

        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentHeight: Math.max(height, printDocumentSurface.height)
        contentWidth: width
        flickableDirection: Flickable.VerticalFlick
        interactive: contentsView.showPrintEditorLayout && contentHeight > height
        visible: contentsView.showPrintEditorLayout
        z: 0

        Item {
            id: printDocumentSurface

            height: contentsView.printDocumentSurfaceHeight
            width: printDocumentViewportItem.width

            Rectangle {
                anchors.fill: parent
                color: contentsView.printCanvasColor
            }

            Item {
                id: printPaperColumn

                height: contentsView.printPaperDocumentHeight
                width: contentsView.printPaperResolvedWidth
                x: Math.max(0, (Number(parent ? parent.width : 0) - width) / 2)
                y: contentsView.printPaperVerticalMargin
            }

            Repeater {
                model: contentsView.printDocumentPageCount

                delegate: Item {
                    required property int index

                    height: contentsView.printPaperResolvedHeight
                    width: printPaperColumn.width
                    x: printPaperColumn.x
                    y: printPaperColumn.y + index * contentsView.printPaperTextHeight

                    Rectangle {
                        color: contentsView.printPaperShadowColor
                        height: parent.height
                        radius: LV.Theme.radiusSm
                        width: parent.width
                        x: contentsView.printPaperShadowOffsetX
                        y: contentsView.printPaperShadowOffsetY
                        z: -2
                    }

                    Rectangle {
                        anchors.fill: parent
                        border.color: contentsView.printPaperBorderColor
                        border.width: contentsView.printPaperSeparatorThickness
                        color: contentsView.printPaperColor
                        gradient: Gradient {
                            GradientStop {
                                color: contentsView.printPaperHighlightColor
                                position: 0.0
                            }
                            GradientStop {
                                color: contentsView.printPaperColor
                                position: 0.72
                            }
                            GradientStop {
                                color: contentsView.printPaperShadeColor
                                position: 1.0
                            }
                        }
                        radius: LV.Theme.radiusSm
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        color: contentsView.printPaperSeparatorColor
                        height: contentsView.printPaperSeparatorThickness
                        visible: index > 0
                    }

                    Canvas {
                        anchors.fill: parent
                        visible: contentsView.showPrintMarginGuides

                        onHeightChanged: requestPaint()
                        onVisibleChanged: {
                            if (visible)
                                requestPaint();
                        }
                        onWidthChanged: requestPaint()
                        onPaint: {
                            const ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            if (!visible)
                                return;

                            const leftInset = Math.max(1, Number(contentsView.printGuideHorizontalInset) || 1);
                            const rightInset = Math.max(1, Number(contentsView.printGuideHorizontalInset) || 1);
                            const topInset = Math.max(1, Number(contentsView.printGuideVerticalInset) || 1);
                            const bottomInset = Math.max(1, Number(contentsView.printGuideVerticalInset) || 1);
                            const left = leftInset;
                            const top = topInset;
                            const right = Math.max(left + 1, width - rightInset);
                            const bottom = Math.max(top + 1, height - bottomInset);
                            const segment = 6;
                            const gap = 4;

                            ctx.lineWidth = 1;
                            ctx.strokeStyle = "#66727D";
                            ctx.beginPath();

                            for (let x = left; x < right; x += segment + gap) {
                                const x2 = Math.min(right, x + segment);
                                ctx.moveTo(x, top);
                                ctx.lineTo(x2, top);
                                ctx.moveTo(x, bottom);
                                ctx.lineTo(x2, bottom);
                            }
                            for (let y = top; y < bottom; y += segment + gap) {
                                const y2 = Math.min(bottom, y + segment);
                                ctx.moveTo(left, y);
                                ctx.lineTo(left, y2);
                                ctx.moveTo(right, y);
                                ctx.lineTo(right, y2);
                            }
                            ctx.stroke();
                        }
                    }
                }
            }
        }
    }

    Flickable {
        id: structuredDocumentViewportItem

        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentHeight: Math.max(
                           height,
                           (Number(structuredDocumentFlowItem.y) || 0)
                           + (Number(structuredDocumentFlowItem.implicitHeight) || 0)
                           + contentsView.editorBottomInset)
        contentWidth: width
        flickableDirection: Flickable.VerticalFlick
        interactive: !contentsView.showPrintEditorLayout && contentHeight > height
        visible: contentsView.showStructuredDocumentFlow && !contentsView.showPrintEditorLayout
        z: 1

        TapHandler {
            acceptedButtons: Qt.LeftButton
            enabled: structuredDocumentViewportItem.visible

            onTapped: function (eventPoint) {
                const viewportTapX = eventPoint && eventPoint.position ? Number(eventPoint.position.x) || 0 : 0;
                const viewportTapY = eventPoint && eventPoint.position ? Number(eventPoint.position.y) || 0 : 0;
                const contentTapX = viewportTapX + (Number(structuredDocumentViewportItem.contentX) || 0);
                const contentTapY = viewportTapY + (Number(structuredDocumentViewportItem.contentY) || 0);
                const flowTapX = contentTapX - (Number(structuredDocumentFlowItem.x) || 0);
                const flowTapY = contentTapY - (Number(structuredDocumentFlowItem.y) || 0);
                if (structuredDocumentFlowItem
                        && structuredDocumentFlowItem.visible
                        && structuredDocumentFlowItem.hasBlockAtPoint !== undefined
                        && structuredDocumentFlowItem.hasBlockAtPoint(flowTapX, flowTapY)) {
                    return;
                }
                Qt.callLater(function () {
                    contentsView.requestStructuredDocumentEndEdit();
                });
            }
        }
    }

    ContentsStructuredDocumentFlow {
        id: structuredDocumentFlowItem
        objectName: "contentsDisplayStructuredDocumentFlow"

        agendaBackend: contentsAgendaBackend
        calloutBackend: contentsCalloutBackend
        documentBlocks: structuredBlockRenderer.renderedDocumentBlocks
        lineHeightHint: contentsView.editorLineHeight
        nativeTextInputPriority: contentsView.nativeTextInputPriority
        paperPaletteEnabled: contentsView.showPrintEditorLayout
        parent: contentsView.showPrintEditorLayout ? printDocumentSurface : structuredDocumentViewportItem.contentItem
        renderedResources: bodyResourceRenderer.renderedResources
        sourceText: contentsView.structuredFlowSourceText
        tagManagementShortcutKeyPressHandler: function (event) {
            return contentsView.handleTagManagementShortcutKeyPress(event);
        }
        viewportContentY: contentsView.showPrintEditorLayout ? 0 : (Number(structuredDocumentViewportItem.contentY) || 0)
        viewportHeight: contentsView.showPrintEditorLayout ? 0 : (Number(structuredDocumentViewportItem.height) || 0)
        visible: contentsView.showStructuredDocumentFlow
        width: contentsView.showPrintEditorLayout
               ? contentsView.printPaperTextWidth
               : Math.max(0, structuredDocumentViewportItem.width - contentsView.editorHorizontalInset * 2)
        x: contentsView.showPrintEditorLayout
           ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset
           : contentsView.editorHorizontalInset
        y: contentsView.showPrintEditorLayout
           ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset
           : contentsView.editorDocumentStartY
        z: contentsView.showPrintEditorLayout ? 2 : 1

        onSourceMutationRequested: function (nextSourceText, focusRequest) {
            contentsView.applyDocumentSourceMutation(nextSourceText, focusRequest);
        }
    }

    ContentsAgendaLayer {
        id: agendaBackgroundLayer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: contentsView.showPrintEditorLayout
                            ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset
                            : contentsView.editorHorizontalInset
        anchors.rightMargin: contentsView.showPrintEditorLayout
                             ? Math.max(
                                   0,
                                   (parent ? Number(parent.width) || 0 : 0)
                                   - ((Number(printPaperColumn.x) || 0)
                                      + contentsView.printGuideHorizontalInset
                                      + contentsView.printPaperTextWidth))
                             : contentsView.editorHorizontalInset
        anchors.topMargin: contentsView.showPrintEditorLayout
                           ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset
                           : contentsView.editorDocumentStartY
        blockFocusHandler: function (sourceOffset) {
            contentsView.focusStructuredBlockSourceOffset(sourceOffset);
        }
        enableCardFocus: false
        enableTaskToggle: false
        enabled: visible
        paperPaletteEnabled: contentsView.showPrintEditorLayout
        renderedAgendas: contentsView.showStructuredDocumentFlow ? [] : structuredBlockRenderer.renderedAgendas
        showTaskCheckbox: false
        showTaskText: false
        sourceOffsetYResolver: function (sourceOffset) {
            const logicalOffset = editorTypingController.logicalOffsetForSourceOffset(
                        Math.max(0, Math.floor(Number(sourceOffset) || 0)));
            return Math.max(0, Number(contentsView.documentYForOffset(logicalOffset)) || 0);
        }
        taskToggleHandler: function (taskOpenTagStart, taskOpenTagEnd, checked) {
            contentsView.setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked);
        }
        visible: contentsView.noteDocumentParseMounted
                 && !contentsView.showStructuredDocumentFlow
                 && !contentsView.showDedicatedResourceViewer
                 && !contentsView.showFormattedTextRenderer
                 && agendaBackgroundLayer.agendaCount > 0
        z: 0
    }

    ContentsCalloutLayer {
        id: calloutBackgroundLayer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: contentsView.showPrintEditorLayout
                            ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset
                            : contentsView.editorHorizontalInset
        anchors.rightMargin: contentsView.showPrintEditorLayout
                             ? Math.max(
                                   0,
                                   (parent ? Number(parent.width) || 0 : 0)
                                   - ((Number(printPaperColumn.x) || 0)
                                      + contentsView.printGuideHorizontalInset
                                      + contentsView.printPaperTextWidth))
                             : contentsView.editorHorizontalInset
        anchors.topMargin: contentsView.showPrintEditorLayout
                           ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset
                           : contentsView.editorDocumentStartY
        blockFocusHandler: function (sourceOffset) {
            contentsView.focusStructuredBlockSourceOffset(sourceOffset);
        }
        enableCardFocus: false
        enabled: visible
        paperPaletteEnabled: contentsView.showPrintEditorLayout
        renderedCallouts: contentsView.showStructuredDocumentFlow ? [] : structuredBlockRenderer.renderedCallouts
        showText: false
        sourceOffsetYResolver: function (sourceOffset) {
            const logicalOffset = editorTypingController.logicalOffsetForSourceOffset(
                        Math.max(0, Math.floor(Number(sourceOffset) || 0)));
            return Math.max(0, Number(contentsView.documentYForOffset(logicalOffset)) || 0);
        }
        visible: contentsView.noteDocumentParseMounted
                 && !contentsView.showStructuredDocumentFlow
                 && !contentsView.showDedicatedResourceViewer
                 && !contentsView.showFormattedTextRenderer
                 && calloutBackgroundLayer.calloutCount > 0
        z: 0
    }

    ContentsAgendaLayer {
        id: agendaRenderLayer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: contentsView.showPrintEditorLayout
                            ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset
                            : contentsView.editorHorizontalInset
        anchors.rightMargin: contentsView.showPrintEditorLayout
                             ? Math.max(
                                   0,
                                   (parent ? Number(parent.width) || 0 : 0)
                                   - ((Number(printPaperColumn.x) || 0)
                                      + contentsView.printGuideHorizontalInset
                                      + contentsView.printPaperTextWidth))
                             : contentsView.editorHorizontalInset
        anchors.topMargin: contentsView.showPrintEditorLayout
                           ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset
                           : contentsView.editorDocumentStartY
        blockFocusHandler: function (sourceOffset) {
            contentsView.focusStructuredBlockSourceOffset(sourceOffset);
        }
        enableCardFocus: false
        enabled: visible
        paperPaletteEnabled: contentsView.showPrintEditorLayout
        renderedAgendas: contentsView.showStructuredDocumentFlow ? [] : structuredBlockRenderer.renderedAgendas
        showFrame: false
        showHeader: false
        showTaskText: false
        sourceOffsetYResolver: function (sourceOffset) {
            const logicalOffset = editorTypingController.logicalOffsetForSourceOffset(
                        Math.max(0, Math.floor(Number(sourceOffset) || 0)));
            return Math.max(0, Number(contentsView.documentYForOffset(logicalOffset)) || 0);
        }
        taskToggleHandler: function (taskOpenTagStart, taskOpenTagEnd, checked) {
            contentsView.setAgendaTaskDone(taskOpenTagStart, taskOpenTagEnd, checked);
        }
        visible: contentsView.noteDocumentParseMounted
                 && !contentsView.showStructuredDocumentFlow
                 && !contentsView.showDedicatedResourceViewer
                 && !contentsView.showFormattedTextRenderer
                 && agendaRenderLayer.agendaCount > 0
        z: 3
    }

    Flickable {
        id: formattedPreviewViewport

        readonly property real bottomInset: contentsView.showPrintEditorLayout ? contentsView.printGuideVerticalInset : contentsView.editorBottomInset
        readonly property real horizontalInset: contentsView.showPrintEditorLayout ? (Number(printPaperColumn.x) || 0) + contentsView.printGuideHorizontalInset : contentsView.editorHorizontalInset
        readonly property real textWidth: {
            if (contentsView.showPrintEditorLayout) {
                const pageWidth = Number(printPaperColumn.width) || 0;
                return Math.max(0, pageWidth - contentsView.printGuideHorizontalInset * 2);
            }
            return Math.max(0, formattedPreviewViewport.width - contentsView.editorHorizontalInset * 2);
        }
        readonly property real topInset: contentsView.showPrintEditorLayout ? (Number(printPaperColumn.y) || 0) + contentsView.printGuideVerticalInset : contentsView.editorDocumentStartY

        anchors.fill: parent
        clip: true
        contentHeight: Math.max(height, Math.max(0, Number(formattedPreviewText.paintedHeight) || 0) + formattedPreviewViewport.topInset + formattedPreviewViewport.bottomInset)
        contentWidth: width
        interactive: contentHeight > height
        visible: contentsView.showFormattedTextRenderer
        z: 1

        Text {
            id: formattedPreviewText

            color: contentsView.showPrintEditorLayout ? contentsView.printPaperTextColor : LV.Theme.bodyColor
            font.family: LV.Theme.fontBody
            font.letterSpacing: 0
            font.pixelSize: contentsView.effectiveEditorFontPixelSize
            font.weight: contentsView.editorFontWeight
            text: editorProjection.renderedHtml
            textFormat: Text.RichText
            width: formattedPreviewViewport.textWidth
            wrapMode: Text.Wrap
            x: formattedPreviewViewport.horizontalInset
            y: formattedPreviewViewport.topInset

            onLinkActivated: function (link) {
                Qt.openUrlExternally(link);
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        border.color: LV.Theme.primary
        border.width: contentsView.resourceDropActive ? 1 : 0
        color: contentsView.resourceDropActive ? "#1A9DA0A8" : "transparent"
        radius: LV.Theme.radiusSm
        visible: contentsView.resourceDropActive
                 && !contentsView.showDedicatedResourceViewer
                 && !contentsView.showFormattedTextRenderer
        z: 4
    }

    DropArea {
        anchors.fill: parent
        enabled: !contentsView.showDedicatedResourceViewer && !contentsView.showFormattedTextRenderer
        z: 5

        onDropped: function (drop) {
            const dropUrls = resourceImportController.extractResourceDropUrls(drop);
            if (!resourceImportController.canAcceptResourceDropUrls(dropUrls)) {
                if (drop)
                    drop.accepted = false;
                resourceImportController.releaseResourceDropEditorSurfaceGuard(false);
                contentsView.resourceDropActive = false;
                return;
            }
            if (drop && drop.acceptProposedAction !== undefined)
                drop.acceptProposedAction();
            const inserted = resourceImportController.importUrlsAsResourcesWithPrompt(dropUrls);
            if (drop)
                drop.accepted = inserted;
        }
        onEntered: function (drag) {
            const dropUrls = resourceImportController.extractResourceDropUrls(drag);
            const accepted = resourceImportController.canAcceptResourceDropUrls(dropUrls);
            if (drag && accepted && drag.acceptProposedAction !== undefined)
                drag.acceptProposedAction();
            if (drag)
                drag.accepted = accepted;
            contentsView.resourceDropActive = accepted;
        }
        onExited: {
            contentsView.resourceDropActive = false;
        }
        onPositionChanged: function (drag) {
            const dropUrls = resourceImportController.extractResourceDropUrls(drag);
            const accepted = resourceImportController.canAcceptResourceDropUrls(dropUrls);
            if (drag && accepted && drag.acceptProposedAction !== undefined)
                drag.acceptProposedAction();
            if (drag)
                drag.accepted = accepted;
            contentsView.resourceDropActive = accepted;
        }
    }

    EditorDisplayModel.ContentsDisplayInputCommandSurface {
        id: inputCommandSurfaceItem

        contentsView: contentsView
        resourceImportController: resourceImportController
        z: 6
    }
}
