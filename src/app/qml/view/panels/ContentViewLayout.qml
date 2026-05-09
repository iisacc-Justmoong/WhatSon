pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV
import "../calendar" as CalendarView
import "../contents" as ContentsChrome

Item {
    id: contentViewLayout

    function traceNoteListModelBinding(reason) {
        console.log("[whatson:qml][ContentViewLayout][" + reason + "] noteListModel="
                    + contentViewLayout.noteListModel
                    + " resolvedNoteListModel=" + contentViewLayout.resolvedNoteListModel
                    + " contentController=" + contentViewLayout.resolvedContentController)
    }

    property var contentController: null
    property color displayColor: "transparent"
    property var editorViewModeController: null
    property int editorTopInsetOverride: -1
    property int frameHorizontalInsetOverride: -1
    property bool isMobilePlatform: false
    property int libraryHierarchyIndex: 0
    property var libraryHierarchyController: null
    property bool minimapVisible: true
    property var noteActiveState: null
    property var noteListModel: null
    property var panelControllerRegistry: null
    readonly property var panelController: contentViewLayout.panelControllerRegistry ? contentViewLayout.panelControllerRegistry.panelController("ContentViewLayout") : null
    property var resourcesImportController: null
    readonly property var resolvedContentController: contentViewLayout.contentController
    readonly property var resolvedNoteListModel: contentViewLayout.noteListModel
    readonly property bool resourceEditorVisible: editorSurfaceModeSupport.resourceEditorVisible
    readonly property var resolvedCurrentResourceEntry: editorSurfaceModeSupport.currentResourceEntry
    property var sidebarHierarchyController: null
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarController: null
    property bool agendaOverlayVisible: false
    property var agendaController: null
    property bool monthCalendarOverlayVisible: false
    property var monthCalendarController: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarController: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarController: null
    readonly property bool calendarOverlayVisible: contentViewLayout.agendaOverlayVisible
                                                 || contentViewLayout.dayCalendarOverlayVisible
                                                 || contentViewLayout.weekCalendarOverlayVisible
                                                 || contentViewLayout.monthCalendarOverlayVisible
                                                 || contentViewLayout.yearCalendarOverlayVisible
    readonly property bool showingAgendaOverlay: contentViewLayout.agendaOverlayVisible
    readonly property bool showingDayCalendarOverlay: !contentViewLayout.agendaOverlayVisible
                                                      && contentViewLayout.dayCalendarOverlayVisible
    readonly property bool showingWeekCalendarOverlay: !contentViewLayout.agendaOverlayVisible
                                                       && !contentViewLayout.dayCalendarOverlayVisible
                                                       && contentViewLayout.weekCalendarOverlayVisible
    readonly property bool showingMonthCalendarOverlay: !contentViewLayout.agendaOverlayVisible
                                                        && !contentViewLayout.dayCalendarOverlayVisible
                                                        && !contentViewLayout.weekCalendarOverlayVisible
                                                        && contentViewLayout.monthCalendarOverlayVisible
    readonly property bool showingYearCalendarOverlay: !contentViewLayout.agendaOverlayVisible
                                                       && !contentViewLayout.dayCalendarOverlayVisible
                                                       && !contentViewLayout.weekCalendarOverlayVisible
                                                       && !contentViewLayout.monthCalendarOverlayVisible
                                                       && contentViewLayout.yearCalendarOverlayVisible
    readonly property int activeSurfaceIndex: contentViewLayout.calendarOverlayVisible ? 1 : 0

    signal editorTextEdited(string text)
    signal dayCalendarOverlayCloseRequested
    signal agendaOverlayCloseRequested
    signal monthCalendarOverlayOpenRequested
    signal monthCalendarOverlayCloseRequested
    signal weekCalendarOverlayCloseRequested
    signal viewHookRequested
    signal yearCalendarOverlayCloseRequested

    ContentsEditorSurfaceModeSupport {
        id: editorSurfaceModeSupport

        noteListModel: contentViewLayout.resolvedNoteListModel
    }

    Component.onCompleted: contentViewLayout.traceNoteListModelBinding("completed")
    onNoteListModelChanged: contentViewLayout.traceNoteListModelBinding("noteListModelChanged")
    onContentControllerChanged: contentViewLayout.traceNoteListModelBinding("contentControllerChanged")

    onResourceEditorVisibleChanged: {
        if (contentViewLayout.resourceEditorVisible && contentViewLayout.calendarOverlayVisible)
            contentViewLayout.requestActiveCalendarOverlayClose();
    }

    function requestActiveCalendarOverlayClose() {
        if (contentViewLayout.showingAgendaOverlay) {
            contentViewLayout.agendaOverlayCloseRequested();
            return;
        }
        if (contentViewLayout.showingDayCalendarOverlay) {
            contentViewLayout.dayCalendarOverlayCloseRequested();
            return;
        }
        if (contentViewLayout.showingWeekCalendarOverlay) {
            contentViewLayout.weekCalendarOverlayCloseRequested();
            return;
        }
        if (contentViewLayout.showingMonthCalendarOverlay) {
            contentViewLayout.monthCalendarOverlayCloseRequested();
            return;
        }
        if (contentViewLayout.showingYearCalendarOverlay)
            contentViewLayout.yearCalendarOverlayCloseRequested();
    }
    function openMonthCalendarOverlayForSelection(year, month, selectedDateIso) {
        const targetYear = Math.floor(Number(year) || 0);
        const targetMonth = Math.floor(Number(month) || 0);
        const normalizedDateIso = selectedDateIso === undefined || selectedDateIso === null
                ? ""
                : String(selectedDateIso).trim();
        if (contentViewLayout.monthCalendarController) {
            if (contentViewLayout.yearCalendarController
                    && contentViewLayout.yearCalendarController.calendarSystem !== undefined
                    && contentViewLayout.monthCalendarController.setCalendarSystemByValue !== undefined) {
                contentViewLayout.monthCalendarController.setCalendarSystemByValue(
                            Number(contentViewLayout.yearCalendarController.calendarSystem));
            }
            if (isFinite(targetYear) && targetYear > 0 && contentViewLayout.monthCalendarController.setDisplayedYear !== undefined)
                contentViewLayout.monthCalendarController.setDisplayedYear(targetYear);
            if (isFinite(targetMonth) && targetMonth > 0 && contentViewLayout.monthCalendarController.setDisplayedMonth !== undefined)
                contentViewLayout.monthCalendarController.setDisplayedMonth(targetMonth);
            if (normalizedDateIso.length > 0 && contentViewLayout.monthCalendarController.setSelectedDateIso !== undefined)
                contentViewLayout.monthCalendarController.setSelectedDateIso(normalizedDateIso);
        }
        contentViewLayout.monthCalendarOverlayOpenRequested();
    }
    function requestOpenLibraryNote(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length === 0)
            return false;
        if (!contentViewLayout.libraryHierarchyController
                || contentViewLayout.libraryHierarchyController.activateNoteById === undefined
                || !contentViewLayout.sidebarHierarchyController
                || contentViewLayout.sidebarHierarchyController.setActiveHierarchyIndex === undefined) {
            return false;
        }

        const activated = Boolean(contentViewLayout.libraryHierarchyController.activateNoteById(normalizedNoteId));
        if (!activated)
            return false;

        contentViewLayout.sidebarHierarchyController.setActiveHierarchyIndex(contentViewLayout.libraryHierarchyIndex);
        contentViewLayout.requestActiveCalendarOverlayClose();
        return true;
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined && reason !== null ? String(reason) : "manual";
        if (contentViewLayout.panelController && contentViewLayout.panelController.requestControllerHook)
            contentViewLayout.panelController.requestControllerHook(hookReason);
        contentViewLayout.viewHookRequested();
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    StackLayout {
        id: contentSurfaceStack

        anchors.fill: parent
        currentIndex: contentViewLayout.activeSurfaceIndex

        Item {
            id: editorContentSurface

            enabled: visible
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: !contentViewLayout.calendarOverlayVisible

            Loader {
                anchors.fill: parent
                active: editorContentSurface.visible
                sourceComponent: contentViewLayout.resourceEditorVisible
                                 ? resourceEditorSurfaceComponent
                                 : editorSurfaceComponent
            }
        }
        Item {
            id: calendarContentSurface

            enabled: visible
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: contentViewLayout.calendarOverlayVisible

            Loader {
                id: calendarPageLoader

                anchors.fill: parent
                sourceComponent: contentViewLayout.showingAgendaOverlay
                                 ? agendaPageComponent
                                 : contentViewLayout.showingDayCalendarOverlay
                                   ? dayCalendarPageComponent
                                   : contentViewLayout.showingWeekCalendarOverlay
                                   ? weekCalendarPageComponent
                                   : contentViewLayout.showingMonthCalendarOverlay
                                     ? monthCalendarPageComponent
                                     : yearCalendarPageComponent
            }
        }
    }
    Connections {
        target: contentViewLayout.resolvedNoteListModel
        ignoreUnknownSignals: true

        function onCurrentNoteIdChanged() {
            const model = contentViewLayout.resolvedNoteListModel;
            const currentNoteId = model && model.currentNoteId !== undefined
                ? String(model.currentNoteId)
                : "";
            if (currentNoteId.length > 0 && contentViewLayout.calendarOverlayVisible)
                contentViewLayout.requestActiveCalendarOverlayClose();
        }
        function onCurrentIndexChanged() {
            if (contentViewLayout.calendarOverlayVisible)
                contentViewLayout.requestActiveCalendarOverlayClose();
        }
    }
    Component {
        id: editorSurfaceComponent

        Item {
            id: contentsEditorSurface

            anchors.fill: parent
            clip: true
            enabled: contentViewLayout.visible
            readonly property int contentVerticalPadding: LV.Theme.gap8
            readonly property int metricsLargeDocumentNativeSurfaceLengthThreshold: 32 * 1024
            readonly property string metricsSourceText: editorDisplayBackend.editorSession.editorText
            readonly property string metricsRenderedSurfaceHtml: structuredDocumentFlow.resolvedEditorSurfaceHtml
            readonly property string metricsDisplayGeometryText: contentsEditorSurface.metricsLargeDocumentNativeSurface
                    ? ""
                    : structuredDocumentFlow.resolvedDisplayGeometryText
            readonly property bool metricsLargeDocumentNativeSurface: String(contentsEditorSurface.metricsRenderedSurfaceHtml || "").length <= 0
                    && String(contentsEditorSurface.metricsSourceText || "").length >= contentsEditorSurface.metricsLargeDocumentNativeSurfaceLengthThreshold
                    && String(contentsEditorSurface.metricsSourceText || "").indexOf("<") < 0
            readonly property bool metricsRenderedOverlayVisible: String(contentsEditorSurface.metricsRenderedSurfaceHtml || "").length > 0

            function editorViewportScrollRange() {
                if (!editorDocumentViewport)
                    return 0;
                return Math.max(0, editorDocumentViewport.contentHeight - editorDocumentViewport.height);
            }

            function scrollEditorViewportByDelta(deltaY) {
                const scrollRange = contentsEditorSurface.editorViewportScrollRange();
                const nextContentY = editorDocumentViewport.contentY + (Number(deltaY) || 0);
                editorDocumentViewport.contentY = Math.max(0, Math.min(scrollRange, nextContentY));
                return true;
            }

            function metricsListLength(values) {
                return values && values.length !== undefined ? values.length : 0;
            }

            function resolvedMetricBlocks() {
                const documentBlocks = editorDisplayBackend.structuredBlockRenderer.renderedDocumentBlocks || [];
                if (contentsEditorSurface.metricsListLength(documentBlocks) > 0)
                    return documentBlocks;
                return editorDisplayBackend.presentationProjection.normalizedHtmlBlocks || [];
            }

            ContentsEditorDisplayBackend {
                id: editorDisplayBackend

                contentController: contentViewLayout.resolvedContentController
                editorCursorPosition: structuredDocumentFlow.editorCursorPosition
                libraryHierarchyController: contentViewLayout.libraryHierarchyController
                minimapVisible: contentViewLayout.minimapVisible
                noteActiveState: contentViewLayout.noteActiveState
                noteListModel: contentViewLayout.resolvedNoteListModel
                paperPaletteEnabled: pagePrintLayoutRenderer.showPrintEditorLayout
                structuredDocumentFlow: structuredDocumentFlow

                onEditorTextEdited: function (text) {
                    contentViewLayout.editorTextEdited(text);
                }
                onEditorViewportResetRequested: {
                    editorDocumentViewport.contentY = 0;
                }
            }

            ContentsPagePrintLayoutRenderer {
                id: pagePrintLayoutRenderer

                activeEditorViewMode: contentViewLayout.editorViewModeController
                                      && contentViewLayout.editorViewModeController.activeViewMode !== undefined
                                      ? Number(contentViewLayout.editorViewModeController.activeViewMode)
                                      : 0
                dedicatedResourceViewerVisible: contentViewLayout.resourceEditorVisible
                editorContentHeight: structuredDocumentFlow.editorContentHeight
                editorViewportHeight: editorDocumentViewport.height
                editorViewportWidth: editorDocumentViewport.width
                guideHorizontalInset: LV.Theme.gap24
                guideVerticalInset: LV.Theme.gap24
                hasSelectedNote: editorDisplayBackend.noteDocumentParseMounted
                paperHorizontalMargin: LV.Theme.gap12
                paperSeparatorThickness: LV.Theme.strokeThin
                paperShadowOffsetX: LV.Theme.strokeThin
                paperShadowOffsetY: LV.Theme.gap2
                paperVerticalMargin: LV.Theme.gap4
            }

            ContentsEditorVisualLineMetrics {
                id: contentsDisplayVisualLineMetrics

                objectName: "contentsDisplayVisualLineMetrics"
            }

            ContentsLineNumberRailMetrics {
                id: contentsDisplayLineNumberRailMetrics

                displayContentHeight: (contentsEditorSurface.metricsRenderedOverlayVisible
                                       ? contentsDisplayRenderedMetricProbe.contentHeight
                                       : contentsDisplayLogicalMetricProbe.contentHeight)
                                      + LV.Theme.gap16
                geometryWidth: structuredDocumentFlow.width
                normalizedHtmlBlocks: contentsEditorSurface.metricsLargeDocumentNativeSurface
                                      ? []
                                      : contentsEditorSurface.resolvedMetricBlocks()
                objectName: "contentsDisplayLineNumberRailMetrics"
                sourceText: contentsEditorSurface.metricsLargeDocumentNativeSurface
                            ? ""
                            : contentsEditorSurface.metricsSourceText
                textLineHeight: LV.Theme.textBodyLineHeight
            }

            ContentsEditorGeometryProvider {
                id: contentsDisplayMetricGeometryProvider

                fallbackLineHeight: LV.Theme.textBodyLineHeight
                fallbackWidth: structuredDocumentFlow.width
                lineNumberRanges: contentsEditorSurface.metricsLargeDocumentNativeSurface
                                  ? []
                                  : contentsDisplayLineNumberRailMetrics.logicalLineRanges
                logicalLength: contentsEditorSurface.metricsDisplayGeometryText.length
                objectName: "contentsDisplayMetricGeometryProvider"
                resourceVisualBlocks: structuredDocumentFlow.resourceVisualBlocks
                targetItem: contentsDisplayMetricProbeLayer
                textItem: contentsDisplayLogicalMetricProbe
                visualItem: contentsEditorSurface.metricsRenderedOverlayVisible
                            ? contentsDisplayRenderedMetricProbe
                            : contentsDisplayLogicalMetricProbe
                visualLineHeight: LV.Theme.textBodyLineHeight
                visualStrokeThin: LV.Theme.strokeThin
                visualTextContentHeight: contentsDisplayMetricGeometryProvider.visualItem !== null
                                         && contentsDisplayMetricGeometryProvider.visualItem !== undefined
                                         && contentsDisplayMetricGeometryProvider.visualItem.contentHeight !== undefined
                        ? contentsDisplayMetricGeometryProvider.visualItem.contentHeight
                        : LV.Theme.gapNone
                visualTextLineCount: contentsDisplayMetricGeometryProvider.visualItem !== null
                                     && contentsDisplayMetricGeometryProvider.visualItem !== undefined
                                     && contentsDisplayMetricGeometryProvider.visualItem.lineCount !== undefined
                        ? contentsDisplayMetricGeometryProvider.visualItem.lineCount
                        : LV.Theme.gapNone
                visualTextWidth: contentsDisplayMetricGeometryProvider.visualItem !== null
                                 && contentsDisplayMetricGeometryProvider.visualItem !== undefined
                                 && contentsDisplayMetricGeometryProvider.visualItem.width !== undefined
                        ? contentsDisplayMetricGeometryProvider.visualItem.width
                        : LV.Theme.gapNone
            }

            Binding {
                property: "measuredLineWidthRatios"
                target: contentsDisplayVisualLineMetrics
                value: contentsDisplayMetricGeometryProvider.visualLineWidthRatios
            }

            Binding {
                property: "measuredVisualLineCount"
                target: contentsDisplayVisualLineMetrics
                value: contentsDisplayMetricGeometryProvider.visualLineCount
            }

            Binding {
                property: "geometryRows"
                target: contentsDisplayLineNumberRailMetrics
                value: contentsDisplayMetricGeometryProvider.lineNumberGeometryRows
            }

            Binding {
                target: editorDisplayBackend.minimapLayoutMetrics
                property: "buttonMinWidth"
                value: LV.Theme.buttonMinWidth
            }
            Binding {
                target: editorDisplayBackend.minimapLayoutMetrics
                property: "gapNone"
                value: LV.Theme.gapNone
            }
            Binding {
                target: editorDisplayBackend.minimapLayoutMetrics
                property: "gap8"
                value: LV.Theme.gap8
            }
            Binding {
                target: editorDisplayBackend.minimapLayoutMetrics
                property: "gap12"
                value: LV.Theme.gap12
            }
            Binding {
                target: editorDisplayBackend.minimapLayoutMetrics
                property: "gap20"
                value: LV.Theme.gap20
            }
            Binding {
                target: editorDisplayBackend.minimapLayoutMetrics
                property: "gap24"
                value: LV.Theme.gap24
            }
            Binding {
                target: editorDisplayBackend.minimapLayoutMetrics
                property: "strokeThin"
                value: LV.Theme.strokeThin
            }
            Binding {
                target: editorDisplayBackend.minimapLayoutMetrics
                property: "visualLineCount"
                value: contentsDisplayVisualLineMetrics.visualLineCount
            }

            Rectangle {
                anchors.fill: parent
                color: pagePrintLayoutRenderer.showPrintEditorLayout
                       ? pagePrintLayoutRenderer.canvasColor
                       : contentViewLayout.displayColor
            }

            LV.HStack {
                anchors.fill: parent
                anchors.bottomMargin: contentsEditorSurface.contentVerticalPadding
                anchors.topMargin: contentsEditorSurface.contentVerticalPadding
                objectName: "contentsDisplayEditorChromeHStack"
                spacing: LV.Theme.gapNone

                Item {
                    id: editorDocumentSlot

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    clip: true
                    objectName: "contentsDisplayEditorDocumentSlot"

                    Flickable {
                        id: editorDocumentViewport

                        anchors.fill: parent
                        boundsBehavior: Flickable.StopAtBounds
                        boundsMovement: Flickable.StopAtBounds
                        clip: true
                        contentHeight: editorDocumentContent.height
                        contentWidth: Math.max(width, editorDocumentContent.width)
                        flickableDirection: Flickable.VerticalFlick
                        interactive: (!structuredDocumentFlow.editorRenderedOverlayVisible
                                      || pagePrintLayoutRenderer.showPrintEditorLayout)
                                     && contentHeight > height
                        objectName: "contentsDisplayEditorDocumentViewport"

                        Item {
                            id: editorDocumentContent

                            height: pagePrintLayoutRenderer.showPrintEditorLayout
                                    ? pagePrintLayoutRenderer.documentSurfaceHeight
                                    : Math.max(editorDocumentViewport.height, structuredDocumentFlow.editorContentHeight)
                            width: editorDocumentViewport.width

                            Item {
                                id: printPaperPreviewLayer

                                anchors.fill: parent
                                visible: pagePrintLayoutRenderer.showPrintEditorLayout

                                Rectangle {
                                    id: printPaperShadow

                                    color: pagePrintLayoutRenderer.paperShadowColor
                                    height: printPaperColumn.height
                                    radius: LV.Theme.radiusSm
                                    visible: printPaperColumn.width > 0 && printPaperColumn.height > 0
                                    width: printPaperColumn.width
                                    x: printPaperColumn.x + pagePrintLayoutRenderer.paperShadowOffsetX
                                    y: printPaperColumn.y + pagePrintLayoutRenderer.paperShadowOffsetY
                                }
                                Rectangle {
                                    id: printPaperColumn

                                    border.color: pagePrintLayoutRenderer.paperBorderColor
                                    border.width: LV.Theme.strokeThin
                                    color: pagePrintLayoutRenderer.paperColor
                                    height: pagePrintLayoutRenderer.paperDocumentHeight
                                    radius: LV.Theme.radiusSm
                                    width: pagePrintLayoutRenderer.paperResolvedWidth
                                    x: Math.max(0, (Number(parent ? parent.width : 0) - width) / 2)
                                    y: pagePrintLayoutRenderer.paperVerticalMargin
                                }
                                Repeater {
                                    model: pagePrintLayoutRenderer.documentPageCount

                                    delegate: Item {
                                        required property int index

                                        height: pagePrintLayoutRenderer.paperResolvedHeight
                                        width: printPaperColumn.width
                                        x: printPaperColumn.x
                                        y: printPaperColumn.y + index * pagePrintLayoutRenderer.paperTextHeight

                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.top: parent.top
                                            color: pagePrintLayoutRenderer.paperSeparatorColor
                                            height: pagePrintLayoutRenderer.paperSeparatorThickness
                                            visible: index > 0
                                        }
                                        Canvas {
                                            anchors.fill: parent
                                            visible: pagePrintLayoutRenderer.showPrintMarginGuides

                                            onHeightChanged: requestPaint()
                                            onVisibleChanged: requestPaint()
                                            onWidthChanged: requestPaint()

                                            onPaint: {
                                                const ctx = getContext("2d");
                                                ctx.clearRect(0, 0, width, height);
                                                if (!visible)
                                                    return;

                                                const left = Math.max(1, Number(pagePrintLayoutRenderer.guideHorizontalInset) || 1);
                                                const top = Math.max(1, Number(pagePrintLayoutRenderer.guideVerticalInset) || 1);
                                                const right = Math.max(left + 1, width - left);
                                                const bottom = Math.max(top + 1, height - top);
                                                const segment = Math.max(2, Number(LV.Theme.gap8) || 6);
                                                const gap = Math.max(1, Number(LV.Theme.gap4) || 4);

                                                ctx.lineWidth = Math.max(1, Number(LV.Theme.strokeThin) || 1);
                                                ctx.strokeStyle = "#66727D";
                                                ctx.beginPath();
                                                for (let xPosition = left; xPosition < right; xPosition += segment + gap) {
                                                    const xEnd = Math.min(right, xPosition + segment);
                                                    ctx.moveTo(xPosition, top);
                                                    ctx.lineTo(xEnd, top);
                                                    ctx.moveTo(xPosition, bottom);
                                                    ctx.lineTo(xEnd, bottom);
                                                }
                                                for (let yPosition = top; yPosition < bottom; yPosition += segment + gap) {
                                                    const yEnd = Math.min(bottom, yPosition + segment);
                                                    ctx.moveTo(left, yPosition);
                                                    ctx.lineTo(left, yEnd);
                                                    ctx.moveTo(right, yPosition);
                                                    ctx.lineTo(right, yEnd);
                                                }
                                                ctx.stroke();
                                            }
                                        }
                                    }
                                }
                            }

                            ContentsLineNumberRail {
                                id: contentsDisplayGutter

                                height: editorDocumentContent.height
                                objectName: "contentsDisplayGutter"
                                activeSelectionEnd: structuredDocumentFlow.editorSelectionEnd
                                activeSelectionStart: structuredDocumentFlow.editorSelectionStart
                                activeSourceCursorPosition: structuredDocumentFlow.editorCursorPosition
                                rows: contentsDisplayLineNumberRailMetrics.rows
                                visible: editorDisplayBackend.noteDocumentParseMounted
                                         && !pagePrintLayoutRenderer.showPrintEditorLayout
                                width: visible ? contentsDisplayGutter.preferredWidth : LV.Theme.gapNone
                                x: LV.Theme.gapNone
                                y: LV.Theme.gapNone
                            }

                            ContentsStructuredDocumentFlow {
                                id: structuredDocumentFlow

                                coordinateMapper: editorDisplayBackend.presentationProjection
                                documentBlocks: editorDisplayBackend.structuredBlockRenderer.renderedDocumentBlocks
                                editorSurfaceHtml: editorDisplayBackend.editorSurfaceHtmlWithResourceVisualBlocks(
                                                       editorDisplayBackend.presentationProjection.htmlTokens,
                                                       structuredDocumentFlow.resourceVisualBlocks,
                                                       editorDisplayBackend.presentationProjection.editorSurfaceHtml)
                                height: editorDocumentContent.height
                                logicalText: editorDisplayBackend.presentationProjection.logicalText
                                normalizedHtmlBlocks: editorDisplayBackend.presentationProjection.normalizedHtmlBlocks
                                objectName: "contentsDisplayStructuredDocumentFlow"
                                paperPaletteEnabled: pagePrintLayoutRenderer.showPrintEditorLayout
                                projectionSourceText: editorDisplayBackend.presentationProjection.sourceText
                                resourceVisualBlocks: editorDisplayBackend.inlineResourceVisualBlocks(
                                                          editorDisplayBackend.bodyResourceRenderer.renderedResources,
                                                          pagePrintLayoutRenderer.showPrintEditorLayout
                                                          ? Math.floor(pagePrintLayoutRenderer.paperTextWidth)
                                                          : structuredDocumentFlow.width - LV.Theme.gap16 * 2)
                                sourceText: editorDisplayBackend.editorSession.editorText
                                textColor: pagePrintLayoutRenderer.showPrintEditorLayout
                                           ? pagePrintLayoutRenderer.paperTextColor
                                           : LV.Theme.bodyColor
                                visible: editorDisplayBackend.noteDocumentParseMounted
                                width: pagePrintLayoutRenderer.showPrintEditorLayout
                                       ? pagePrintLayoutRenderer.paperTextWidth
                                       : Math.max(0, editorDocumentContent.width - contentsDisplayGutter.width)
                                x: pagePrintLayoutRenderer.showPrintEditorLayout
                                   ? printPaperColumn.x + pagePrintLayoutRenderer.guideHorizontalInset
                                   : contentsDisplayGutter.width
                                y: pagePrintLayoutRenderer.showPrintEditorLayout
                                   ? printPaperColumn.y + pagePrintLayoutRenderer.guideVerticalInset
                                   : LV.Theme.gapNone

                                onSourceTextEdited: function (text) {
                                    editorDisplayBackend.commitEditedSourceText(text);
                                }

                                onViewHookRequested: function (reason) {
                                    contentViewLayout.requestViewHook(reason);
                                }
                            }

                            Item {
                                id: contentsDisplayMetricProbeLayer

                                clip: true
                                enabled: false
                                height: structuredDocumentFlow.height
                                objectName: "contentsDisplayMetricProbeLayer"
                                opacity: 0
                                visible: editorDisplayBackend.noteDocumentParseMounted
                                width: structuredDocumentFlow.width
                                x: structuredDocumentFlow.x
                                y: structuredDocumentFlow.y
                                z: -10

                                TextEdit {
                                    id: contentsDisplayLogicalMetricProbe

                                    activeFocusOnPress: false
                                    anchors.fill: parent
                                    anchors.leftMargin: LV.Theme.gap16
                                    anchors.rightMargin: LV.Theme.gap16
                                    color: "transparent"
                                    enabled: false
                                    font.family: LV.Theme.fontBody
                                    font.pixelSize: LV.Theme.textBody
                                    objectName: "contentsDisplayLogicalMetricProbe"
                                    readOnly: true
                                    selectByKeyboard: false
                                    selectByMouse: false
                                    text: contentsEditorSurface.metricsDisplayGeometryText
                                    textFormat: TextEdit.PlainText
                                    textMargin: LV.Theme.gapNone
                                    wrapMode: TextEdit.Wrap
                                }

                                TextEdit {
                                    id: contentsDisplayRenderedMetricProbe

                                    activeFocusOnPress: false
                                    anchors.fill: contentsDisplayLogicalMetricProbe
                                    color: "transparent"
                                    enabled: false
                                    font.family: LV.Theme.fontBody
                                    font.pixelSize: LV.Theme.textBody
                                    objectName: "contentsDisplayRenderedMetricProbe"
                                    readOnly: true
                                    selectByKeyboard: false
                                    selectByMouse: false
                                    text: contentsEditorSurface.metricsRenderedOverlayVisible
                                          ? contentsEditorSurface.metricsRenderedSurfaceHtml
                                          : ""
                                    textFormat: TextEdit.RichText
                                    textMargin: LV.Theme.gapNone
                                    wrapMode: TextEdit.Wrap
                                }
                            }

                            Connections {
                                target: editorDisplayBackend.editorSession
                                ignoreUnknownSignals: true

                                function onEditorTextSynchronized() {
                                    structuredDocumentFlow.forceProjectionTextSync();
                                }
                            }
                        }
                    }

                    LV.WheelScrollGuard {
                        consumeInside: true
                        targetFlickable: editorDocumentViewport
                    }

                    Text {
                        anchors.centerIn: parent
                        color: LV.Theme.descriptionColor
                        font.family: LV.Theme.fontBody
                        font.pixelSize: LV.Theme.textBody
                        text: "No document opened"
                        visible: !editorDisplayBackend.noteDocumentParseMounted
                    }
                }

                ContentsChrome.Minimap {
                    id: contentsDisplayMinimap

                    Layout.fillHeight: true
                    Layout.preferredWidth: editorDisplayBackend.minimapLayoutMetrics.effectiveMinimapWidth
                    lineColor: LV.Theme.captionColor
                    objectName: "contentsDisplayMinimap"
                    rowCount: editorDisplayBackend.minimapLayoutMetrics.effectiveRowCount
                    rowWidthRatios: contentsDisplayVisualLineMetrics.visualLineWidthRatios
                    scrollDragEnabled: editorDocumentViewport.contentHeight > editorDocumentViewport.height
                    visible: contentViewLayout.minimapVisible

                    onScrollDeltaRequested: function (deltaY) {
                        contentsEditorSurface.scrollEditorViewportByDelta(deltaY);
                    }

                    onViewHookRequested: function (reason) {
                        contentViewLayout.requestViewHook(reason);
                    }
                }
            }
        }
    }
    Component {
        id: resourceEditorSurfaceComponent

        ContentsResourceEditorView {
            anchors.fill: parent
            displayColor: contentViewLayout.displayColor
            resourceEntry: contentViewLayout.resolvedCurrentResourceEntry

            onViewHookRequested: {
                contentViewLayout.viewHookRequested();
            }
        }
    }
    Component {
        id: dayCalendarPageComponent

        CalendarView.DayCalendarPage {
            anchors.fill: parent
            dayCalendarController: contentViewLayout.dayCalendarController

            onNoteOpenRequested: function (noteId) {
                contentViewLayout.requestOpenLibraryNote(noteId);
            }
            onViewHookRequested: function (reason) {
                contentViewLayout.viewHookRequested();
            }
        }
    }
    Component {
        id: agendaPageComponent

        CalendarView.AgendaPage {
            anchors.fill: parent
            agendaController: contentViewLayout.agendaController

            onNoteOpenRequested: function (noteId) {
                contentViewLayout.requestOpenLibraryNote(noteId);
            }
            onViewHookRequested: function (reason) {
                contentViewLayout.viewHookRequested();
            }
        }
    }
    Component {
        id: weekCalendarPageComponent

        CalendarView.WeekCalendarPage {
            anchors.fill: parent
            weekCalendarController: contentViewLayout.weekCalendarController

            onNoteOpenRequested: function (noteId) {
                contentViewLayout.requestOpenLibraryNote(noteId);
            }
            onViewHookRequested: function (reason) {
                contentViewLayout.viewHookRequested();
            }
        }
    }
    Component {
        id: monthCalendarPageComponent

        CalendarView.MonthCalendarPage {
            anchors.fill: parent
            monthCalendarController: contentViewLayout.monthCalendarController

            onNoteOpenRequested: function (noteId) {
                contentViewLayout.requestOpenLibraryNote(noteId);
            }
            onViewHookRequested: function (reason) {
                contentViewLayout.viewHookRequested();
            }
        }
    }
    Component {
        id: yearCalendarPageComponent

        CalendarView.YearCalendarPage {
            anchors.fill: parent
            yearCalendarController: contentViewLayout.yearCalendarController

            onMonthCalendarOpenRequested: function (year, month, selectedDateIso) {
                contentViewLayout.openMonthCalendarOverlayForSelection(year, month, selectedDateIso);
            }
            onViewHookRequested: function (reason) {
                contentViewLayout.viewHookRequested();
            }
        }
    }
}
