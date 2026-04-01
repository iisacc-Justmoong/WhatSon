pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../calendar" as CalendarView

Item {
    id: contentViewLayout

    property var contentViewModel: null
    property color displayColor: "transparent"
    property bool drawerVisible: true
    property color drawerColor: "transparent"
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    property int editorTopInsetOverride: -1
    property int frameHorizontalInsetOverride: -1
    property color gutterColor: "transparent"
    property int gutterWidthOverride: -1
    property var libraryHierarchyViewModel: null
    property int lineNumberColumnLeftOverride: -1
    property int lineNumberColumnTextWidthOverride: -1
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    property bool minimapVisible: true
    property var noteListModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ContentViewLayout") : null
    property var resourcesImportViewModel: null
    readonly property var resolvedContentViewModel: contentViewLayout.contentViewModel
    readonly property var resolvedNoteListModel: contentViewLayout.noteListModel
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarViewModel: null
    property bool todoListOverlayVisible: false
    property var todoListViewModel: null
    property bool monthCalendarOverlayVisible: false
    property var monthCalendarViewModel: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarViewModel: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarViewModel: null
    readonly property bool calendarOverlayVisible: contentViewLayout.todoListOverlayVisible
                                                 || contentViewLayout.dayCalendarOverlayVisible
                                                 || contentViewLayout.weekCalendarOverlayVisible
                                                 || contentViewLayout.monthCalendarOverlayVisible
                                                 || contentViewLayout.yearCalendarOverlayVisible
    readonly property bool showingTodoListOverlay: contentViewLayout.todoListOverlayVisible
    readonly property bool showingDayCalendarOverlay: !contentViewLayout.todoListOverlayVisible
                                                      && contentViewLayout.dayCalendarOverlayVisible
    readonly property bool showingWeekCalendarOverlay: !contentViewLayout.todoListOverlayVisible
                                                       && !contentViewLayout.dayCalendarOverlayVisible
                                                       && contentViewLayout.weekCalendarOverlayVisible
    readonly property bool showingMonthCalendarOverlay: !contentViewLayout.todoListOverlayVisible
                                                        && !contentViewLayout.dayCalendarOverlayVisible
                                                        && !contentViewLayout.weekCalendarOverlayVisible
                                                        && contentViewLayout.monthCalendarOverlayVisible
    readonly property bool showingYearCalendarOverlay: !contentViewLayout.todoListOverlayVisible
                                                       && !contentViewLayout.dayCalendarOverlayVisible
                                                       && !contentViewLayout.weekCalendarOverlayVisible
                                                       && !contentViewLayout.monthCalendarOverlayVisible
                                                       && contentViewLayout.yearCalendarOverlayVisible
    readonly property int activeSurfaceIndex: contentViewLayout.calendarOverlayVisible ? 1 : 0

    signal drawerHeightDragRequested(int value)
    signal editorTextEdited(string text)
    signal dayCalendarOverlayCloseRequested
    signal todoListOverlayCloseRequested
    signal monthCalendarOverlayCloseRequested
    signal weekCalendarOverlayCloseRequested
    signal viewHookRequested
    signal yearCalendarOverlayCloseRequested

    function requestActiveCalendarOverlayClose() {
        if (contentViewLayout.showingTodoListOverlay) {
            contentViewLayout.todoListOverlayCloseRequested();
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

            ContentsDisplayView {
                anchors.fill: parent
                contentViewModel: contentViewLayout.resolvedContentViewModel
                displayColor: contentViewLayout.displayColor
                drawerVisible: contentViewLayout.drawerVisible
                drawerColor: contentViewLayout.drawerColor
                drawerHeight: contentViewLayout.drawerHeight
                enabled: contentViewLayout.visible
                editorTopInsetOverride: contentViewLayout.editorTopInsetOverride
                frameHorizontalInsetOverride: contentViewLayout.frameHorizontalInsetOverride
                gutterColor: contentViewLayout.gutterColor
                gutterWidthOverride: contentViewLayout.gutterWidthOverride
                libraryHierarchyViewModel: contentViewLayout.libraryHierarchyViewModel
                lineNumberColumnLeftOverride: contentViewLayout.lineNumberColumnLeftOverride
                lineNumberColumnTextWidthOverride: contentViewLayout.lineNumberColumnTextWidthOverride
                minDisplayHeight: contentViewLayout.minDisplayHeight
                minDrawerHeight: contentViewLayout.minDrawerHeight
                minimapVisible: contentViewLayout.minimapVisible
                noteListModel: contentViewLayout.resolvedNoteListModel
                panelViewModel: contentViewLayout.panelViewModel
                resourcesImportViewModel: contentViewLayout.resourcesImportViewModel
                splitterColor: contentViewLayout.splitterColor
                splitterHandleThickness: contentViewLayout.splitterHandleThickness
                splitterThickness: contentViewLayout.splitterThickness

                onDrawerHeightDragRequested: function (value) {
                    contentViewLayout.drawerHeightDragRequested(value);
                }
                onEditorTextEdited: function (text) {
                    contentViewLayout.editorTextEdited(text);
                }
                onViewHookRequested: {
                    contentViewLayout.viewHookRequested();
                }
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
                sourceComponent: contentViewLayout.showingTodoListOverlay
                                 ? todoListPageComponent
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
        id: dayCalendarPageComponent

        CalendarView.DayCalendarPage {
            anchors.fill: parent
            dayCalendarViewModel: contentViewLayout.dayCalendarViewModel

            onViewHookRequested: function (reason) {
                contentViewLayout.viewHookRequested();
            }
        }
    }
    Component {
        id: todoListPageComponent

        CalendarView.TodoListPage {
            anchors.fill: parent
            todoListViewModel: contentViewLayout.todoListViewModel

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

            onViewHookRequested: function (reason) {
                contentViewLayout.viewHookRequested();
            }
        }
    }
}
