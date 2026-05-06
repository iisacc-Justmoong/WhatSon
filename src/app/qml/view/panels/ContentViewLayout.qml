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

            ContentsEditorDisplayBackend {
                id: editorDisplayBackend

                contentController: contentViewLayout.resolvedContentController
                editorCursorPosition: structuredDocumentFlow.editorCursorPosition
                libraryHierarchyController: contentViewLayout.libraryHierarchyController
                minimapVisible: contentViewLayout.minimapVisible
                noteActiveState: contentViewLayout.noteActiveState
                noteListModel: contentViewLayout.resolvedNoteListModel
                panelController: contentViewLayout.panelController
                structuredDocumentFlow: structuredDocumentFlow

                onEditorTextEdited: function (text) {
                    contentViewLayout.editorTextEdited(text);
                }
                onEditorViewportResetRequested: {
                    editorDocumentViewport.contentY = 0;
                }
                onViewHookRequested: function (reason) {
                    contentViewLayout.viewHookRequested();
                }
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
                value: structuredDocumentFlow.editorVisualLineCount
            }

            Rectangle {
                anchors.fill: parent
                color: contentViewLayout.displayColor
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
                        interactive: contentHeight > height
                        objectName: "contentsDisplayEditorDocumentViewport"

                        Item {
                            id: editorDocumentContent

                            height: Math.max(editorDocumentViewport.height, structuredDocumentFlow.editorContentHeight)
                            width: editorDocumentViewport.width

                            ContentsLineNumberRail {
                                id: contentsDisplayGutter

                                height: editorDocumentContent.height
                                objectName: "contentsDisplayGutter"
                                activeSelectionEnd: structuredDocumentFlow.editorSelectionEnd
                                activeSelectionStart: structuredDocumentFlow.editorSelectionStart
                                activeSourceCursorPosition: structuredDocumentFlow.editorCursorPosition
                                rows: structuredDocumentFlow.editorLogicalGutterRows
                                visible: editorDisplayBackend.noteDocumentParseMounted
                                width: contentsDisplayGutter.preferredWidth
                                x: LV.Theme.gapNone
                                y: LV.Theme.gapNone
                            }

                            ContentsStructuredDocumentFlow {
                                id: structuredDocumentFlow

                                coordinateMapper: editorDisplayBackend.presentationProjection
                                editorSurfaceHtml: editorDisplayBackend.renderInlineResourceEditorSurfaceHtml(
                                                       editorDisplayBackend.presentationProjection.editorSurfaceHtml,
                                                       editorDisplayBackend.bodyResourceRenderer.renderedResources,
                                                       structuredDocumentFlow.width - LV.Theme.gap16 * 2)
                                height: editorDocumentContent.height
                                htmlTokens: editorDisplayBackend.presentationProjection.htmlTokens
                                logicalCursorPosition: editorDisplayBackend.presentationProjection.logicalCursorPosition
                                logicalText: editorDisplayBackend.presentationProjection.logicalText
                                normalizedHtmlBlocks: editorDisplayBackend.presentationProjection.normalizedHtmlBlocks
                                objectName: "contentsDisplayStructuredDocumentFlow"
                                paperPaletteEnabled: false
                                sourceText: editorDisplayBackend.editorSession.editorText
                                textColor: LV.Theme.bodyColor
                                visible: editorDisplayBackend.noteDocumentParseMounted
                                width: Math.max(0, editorDocumentContent.width - contentsDisplayGutter.width)
                                x: contentsDisplayGutter.width
                                y: LV.Theme.gapNone

                                onSourceTextEdited: function (text) {
                                    editorDisplayBackend.commitEditedSourceText(text);
                                }

                                onViewHookRequested: function (reason) {
                                    editorDisplayBackend.requestViewHook(reason);
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
                    rowWidthRatios: structuredDocumentFlow.editorVisualLineWidthRatios
                    scrollDragEnabled: editorDocumentViewport.contentHeight > editorDocumentViewport.height
                    visible: contentViewLayout.minimapVisible

                    onScrollDeltaRequested: function (deltaY) {
                        contentsEditorSurface.scrollEditorViewportByDelta(deltaY);
                    }

                    onViewHookRequested: function (reason) {
                        editorDisplayBackend.requestViewHook(reason);
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
