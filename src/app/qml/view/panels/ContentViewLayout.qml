pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../calendar" as CalendarView
import "../content/editor/ContentsEditorSurfaceModeSupport.js" as EditorSurfaceModeSupport

Item {
    id: contentViewLayout

    function traceNoteListModelBinding(reason) {
        console.log("[whatson:qml][ContentViewLayout][" + reason + "] noteListModel="
                    + contentViewLayout.noteListModel
                    + " resolvedNoteListModel=" + contentViewLayout.resolvedNoteListModel
                    + " contentViewModel=" + contentViewLayout.resolvedContentViewModel)
    }

    property var contentViewModel: null
    property color displayColor: "transparent"
    property var editorViewModeViewModel: null
    property int editorTopInsetOverride: -1
    property int frameHorizontalInsetOverride: -1
    property color gutterColor: "transparent"
    property int gutterWidthOverride: -1
    property bool isMobilePlatform: false
    property int libraryHierarchyIndex: 0
    property var libraryHierarchyViewModel: null
    property int lineNumberColumnLeftOverride: -1
    property int lineNumberColumnTextWidthOverride: -1
    property bool minimapVisible: true
    property var noteListModel: null
    property var panelViewModelRegistry: null
    readonly property var panelViewModel: contentViewLayout.panelViewModelRegistry ? contentViewLayout.panelViewModelRegistry.panelViewModel("ContentViewLayout") : null
    property var resourcesImportViewModel: null
    readonly property var resolvedContentViewModel: contentViewLayout.contentViewModel
    readonly property var resolvedNoteListModel: contentViewLayout.noteListModel
    readonly property bool resourceEditorVisible: EditorSurfaceModeSupport.resourceEditorVisible(
                                                      contentViewLayout.resolvedNoteListModel)
    readonly property var resolvedCurrentResourceEntry: EditorSurfaceModeSupport.currentResourceEntry(
                                                            contentViewLayout.resolvedNoteListModel)
    property var sidebarHierarchyViewModel: null
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarViewModel: null
    property bool agendaOverlayVisible: false
    property var agendaViewModel: null
    property bool monthCalendarOverlayVisible: false
    property var monthCalendarViewModel: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarViewModel: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarViewModel: null
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
    onContentViewModelChanged: contentViewLayout.traceNoteListModelBinding("contentViewModelChanged")

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
        if (contentViewLayout.monthCalendarViewModel) {
            if (contentViewLayout.yearCalendarViewModel
                    && contentViewLayout.yearCalendarViewModel.calendarSystem !== undefined
                    && contentViewLayout.monthCalendarViewModel.setCalendarSystemByValue !== undefined) {
                contentViewLayout.monthCalendarViewModel.setCalendarSystemByValue(
                            Number(contentViewLayout.yearCalendarViewModel.calendarSystem));
            }
            if (isFinite(targetYear) && targetYear > 0 && contentViewLayout.monthCalendarViewModel.setDisplayedYear !== undefined)
                contentViewLayout.monthCalendarViewModel.setDisplayedYear(targetYear);
            if (isFinite(targetMonth) && targetMonth > 0 && contentViewLayout.monthCalendarViewModel.setDisplayedMonth !== undefined)
                contentViewLayout.monthCalendarViewModel.setDisplayedMonth(targetMonth);
            if (normalizedDateIso.length > 0 && contentViewLayout.monthCalendarViewModel.setSelectedDateIso !== undefined)
                contentViewLayout.monthCalendarViewModel.setSelectedDateIso(normalizedDateIso);
        }
        contentViewLayout.monthCalendarOverlayOpenRequested();
    }
    function requestOpenLibraryNote(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null ? "" : String(noteId).trim();
        if (normalizedNoteId.length === 0)
            return false;
        if (!contentViewLayout.libraryHierarchyViewModel
                || contentViewLayout.libraryHierarchyViewModel.activateNoteById === undefined
                || !contentViewLayout.sidebarHierarchyViewModel
                || contentViewLayout.sidebarHierarchyViewModel.setActiveHierarchyIndex === undefined) {
            return false;
        }

        const activated = Boolean(contentViewLayout.libraryHierarchyViewModel.activateNoteById(normalizedNoteId));
        if (!activated)
            return false;

        contentViewLayout.sidebarHierarchyViewModel.setActiveHierarchyIndex(contentViewLayout.libraryHierarchyIndex);
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
            contentViewModel: contentViewLayout.resolvedContentViewModel
            displayColor: contentViewLayout.displayColor
            editorViewModeViewModel: contentViewLayout.editorViewModeViewModel
            enabled: contentViewLayout.visible
            editorTopInsetOverride: contentViewLayout.editorTopInsetOverride
            frameHorizontalInsetOverride: contentViewLayout.frameHorizontalInsetOverride
            gutterColor: contentViewLayout.gutterColor
            gutterWidthOverride: contentViewLayout.gutterWidthOverride
            libraryHierarchyViewModel: contentViewLayout.libraryHierarchyViewModel
            lineNumberColumnLeftOverride: contentViewLayout.lineNumberColumnLeftOverride
            lineNumberColumnTextWidthOverride: contentViewLayout.lineNumberColumnTextWidthOverride
            minimapVisible: contentViewLayout.minimapVisible
            mobileHost: contentViewLayout.isMobilePlatform
            noteListModel: contentViewLayout.resolvedNoteListModel
            panelViewModel: contentViewLayout.panelViewModel
            resourcesImportViewModel: contentViewLayout.resourcesImportViewModel
            sidebarHierarchyViewModel: contentViewLayout.sidebarHierarchyViewModel

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
            dayCalendarViewModel: contentViewLayout.dayCalendarViewModel

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
            agendaViewModel: contentViewLayout.agendaViewModel

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
            weekCalendarViewModel: contentViewLayout.weekCalendarViewModel

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
            monthCalendarViewModel: contentViewLayout.monthCalendarViewModel

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
            yearCalendarViewModel: contentViewLayout.yearCalendarViewModel

            onMonthCalendarOpenRequested: function (year, month, selectedDateIso) {
                contentViewLayout.openMonthCalendarOverlayForSelection(year, month, selectedDateIso);
            }
            onViewHookRequested: function (reason) {
                contentViewLayout.viewHookRequested();
            }
        }
    }
}
