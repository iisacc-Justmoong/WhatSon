pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../calendar" as CalendarView
import "../../../models/editor/display/ContentsEditorSurfaceModeSupport.js" as EditorSurfaceModeSupport

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
    property color gutterColor: LV.Theme.panelBackground02
    property int gutterWidthOverride: -1
    property bool isMobilePlatform: false
    property int libraryHierarchyIndex: 0
    property var libraryHierarchyController: null
    property int lineNumberColumnLeftOverride: -1
    property int lineNumberColumnTextWidthOverride: -1
    property bool minimapVisible: true
    property var noteListModel: null
    property var panelControllerRegistry: null
    readonly property var panelController: contentViewLayout.panelControllerRegistry ? contentViewLayout.panelControllerRegistry.panelController("ContentViewLayout") : null
    property var resourcesImportController: null
    readonly property var resolvedContentController: contentViewLayout.contentController
    readonly property var resolvedNoteListModel: contentViewLayout.noteListModel
    readonly property bool resourceEditorVisible: EditorSurfaceModeSupport.resourceEditorVisible(
                                                      contentViewLayout.resolvedNoteListModel)
    readonly property var resolvedCurrentResourceEntry: EditorSurfaceModeSupport.currentResourceEntry(
                                                            contentViewLayout.resolvedNoteListModel)
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

        ContentsDisplayView {
            anchors.fill: parent
            contentController: contentViewLayout.resolvedContentController
            displayColor: contentViewLayout.displayColor
            editorViewModeController: contentViewLayout.editorViewModeController
            enabled: contentViewLayout.visible
            editorTopInsetOverride: contentViewLayout.editorTopInsetOverride
            frameHorizontalInsetOverride: contentViewLayout.frameHorizontalInsetOverride
            gutterColor: contentViewLayout.gutterColor
            gutterWidthOverride: contentViewLayout.gutterWidthOverride
            libraryHierarchyController: contentViewLayout.libraryHierarchyController
            lineNumberColumnLeftOverride: contentViewLayout.lineNumberColumnLeftOverride
            lineNumberColumnTextWidthOverride: contentViewLayout.lineNumberColumnTextWidthOverride
            minimapVisible: contentViewLayout.minimapVisible
            mobileHost: contentViewLayout.isMobilePlatform
            noteListModel: contentViewLayout.resolvedNoteListModel
            panelController: contentViewLayout.panelController
            resourcesImportController: contentViewLayout.resourcesImportController
            sidebarHierarchyController: contentViewLayout.sidebarHierarchyController

            onEditorTextEdited: function (text) {
                contentViewLayout.editorTextEdited(text);
            }
            onViewHookRequested: {
                contentViewLayout.viewHookRequested();
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
