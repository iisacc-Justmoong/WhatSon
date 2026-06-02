pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../calendar" as CalendarView
import "../contents" as ContentsView

Item {
    id: contentViewLayout

    property var contentController: null
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarController: null
    readonly property bool calendarOverlayActive: contentViewLayout.dayCalendarOverlayVisible
            || contentViewLayout.weekCalendarOverlayVisible
            || contentViewLayout.monthCalendarOverlayVisible
            || contentViewLayout.yearCalendarOverlayVisible
    property color displayColor: "transparent"
    property int editorTopInsetOverride: -1
    property int frameHorizontalInsetOverride: -1
    property bool gutterVisible: true
    property bool isMobilePlatform: false
    property int libraryHierarchyIndex: 0
    property var libraryHierarchyController: null
    property bool minimapVisible: true
    property bool monthCalendarOverlayVisible: false
    property var monthCalendarController: null
    property var noteActiveState: null
    property var noteListModel: null
    readonly property var currentResourceEntry: contentViewLayout.resourceListActive
            && contentViewLayout.noteListModel.currentResourceEntry !== undefined
            && contentViewLayout.noteListModel.currentResourceEntry !== null
            ? contentViewLayout.noteListModel.currentResourceEntry
            : ({})
    readonly property bool resourceListActive: contentViewLayout.noteListModel
            && contentViewLayout.noteListModel.currentResourceEntry !== undefined
    readonly property string currentResourceType: contentViewLayout.resourceEntryString("type").toLowerCase()
    readonly property string currentResourceFormat: contentViewLayout.resourceEntryString("format").toLowerCase()
    readonly property bool imageResourceSelected: contentViewLayout.resourceListActive
            && (contentViewLayout.currentResourceType === "image"
                || contentViewLayout.resourceFormatLooksLikeImage(contentViewLayout.currentResourceFormat))
            && (contentViewLayout.resourceEntryString("source").length > 0
                || contentViewLayout.resourceEntryString("resolvedPath").length > 0
                || contentViewLayout.resourceEntryString("resourcePath").length > 0)
    readonly property bool noteEditorSurfaceVisible: !contentViewLayout.imageResourceSelected
    property var panelControllerRegistry: null
    readonly property var panelController: contentViewLayout.panelControllerRegistry ? contentViewLayout.panelControllerRegistry.panelController("ContentViewLayout") : null
    property var inAppClipboard: null
    property var editorFontFamilyProvider: null
    property var sidebarHierarchyController: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarController: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarController: null

    signal dayCalendarOverlayCloseRequested
    signal monthCalendarOverlayCloseRequested
    signal monthCalendarOverlayOpenRequested
    signal viewHookRequested
    signal weekCalendarOverlayCloseRequested
    signal yearCalendarOverlayCloseRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined && reason !== null ? String(reason) : "manual";
        if (contentViewLayout.panelController && contentViewLayout.panelController.requestControllerHook)
            contentViewLayout.panelController.requestControllerHook(hookReason);
        contentViewLayout.viewHookRequested();
    }

    function closeCalendarOverlays() {
        if (contentViewLayout.dayCalendarOverlayVisible)
            contentViewLayout.dayCalendarOverlayCloseRequested();
        if (contentViewLayout.weekCalendarOverlayVisible)
            contentViewLayout.weekCalendarOverlayCloseRequested();
        if (contentViewLayout.monthCalendarOverlayVisible)
            contentViewLayout.monthCalendarOverlayCloseRequested();
        if (contentViewLayout.yearCalendarOverlayVisible)
            contentViewLayout.yearCalendarOverlayCloseRequested();
    }

    function openCalendarNote(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null
                ? ""
                : String(noteId).trim();
        if (normalizedNoteId.length === 0
                || !contentViewLayout.libraryHierarchyController
                || contentViewLayout.libraryHierarchyController.activateNoteById === undefined)
            return false;

        if (contentViewLayout.sidebarHierarchyController
                && contentViewLayout.sidebarHierarchyController.setActiveHierarchyIndex !== undefined)
            contentViewLayout.sidebarHierarchyController.setActiveHierarchyIndex(contentViewLayout.libraryHierarchyIndex);

        const activated = Boolean(contentViewLayout.libraryHierarchyController.activateNoteById(normalizedNoteId));
        if (activated)
            contentViewLayout.closeCalendarOverlays();
        return activated;
    }

    function openMonthCalendarFromYear(year, month, selectedDateIso) {
        const targetYear = Math.floor(Number(year));
        const targetMonth = Math.floor(Number(month));
        const normalizedSelectedDateIso = selectedDateIso === undefined || selectedDateIso === null
                ? ""
                : String(selectedDateIso).trim();

        if (contentViewLayout.monthCalendarController) {
            if (isFinite(targetYear) && contentViewLayout.monthCalendarController.setDisplayedYear !== undefined)
                contentViewLayout.monthCalendarController.setDisplayedYear(targetYear);
            if (isFinite(targetMonth) && contentViewLayout.monthCalendarController.setDisplayedMonth !== undefined)
                contentViewLayout.monthCalendarController.setDisplayedMonth(targetMonth);
            if (normalizedSelectedDateIso.length > 0
                    && contentViewLayout.monthCalendarController.setSelectedDateIso !== undefined)
                contentViewLayout.monthCalendarController.setSelectedDateIso(normalizedSelectedDateIso);
        }

        contentViewLayout.monthCalendarOverlayOpenRequested();
    }

    function resourceEntryString(key) {
        if (!contentViewLayout.currentResourceEntry || typeof contentViewLayout.currentResourceEntry !== "object")
            return "";
        const value = contentViewLayout.currentResourceEntry[key];
        if (value === undefined || value === null)
            return "";
        return String(value).trim();
    }

    function resourceFormatLooksLikeImage(format) {
        const normalizedFormat = format === undefined || format === null
                ? ""
                : String(format).trim().toLowerCase();
        const suffixedFormat = normalizedFormat.startsWith(".")
                ? normalizedFormat
                : "." + normalizedFormat;
        return suffixedFormat === ".avif"
                || suffixedFormat === ".bmp"
                || suffixedFormat === ".gif"
                || suffixedFormat === ".heic"
                || suffixedFormat === ".heif"
                || suffixedFormat === ".jpeg"
                || suffixedFormat === ".jpg"
                || suffixedFormat === ".png"
                || suffixedFormat === ".svg"
                || suffixedFormat === ".webp";
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle {
        anchors.fill: parent
        color: contentViewLayout.displayColor

        RowLayout {
            anchors.fill: parent
            enabled: !contentViewLayout.calendarOverlayActive
            spacing: LV.Theme.gapNone
            visible: !contentViewLayout.calendarOverlayActive

            ContentsView.Gutter {
                Layout.fillHeight: true
                Layout.preferredWidth: visible ? implicitWidth : 0
                contentY: contentsTextEditor.viewportContentY
                currentLineIndex: contentsTextEditor.editorCursorLineIndex
                fallbackLineHeight: contentsTextEditor.editorLogicalLineHeight
                lineCount: 0
                lineMetricProvider: contentsTextEditor.editorLogicalLineMetricFor
                lineMetricsRevision: contentsTextEditor.editorLineMetricsRevision
                parsedLineCount: 0
                selectedNoteDirectoryPath: ""
                selectedNoteId: ""
                sourceFilePath: ""
                visible: contentViewLayout.noteEditorSurfaceVisible && contentViewLayout.gutterVisible
            }

            ContentsView.ImageEditor {
                id: contentsImageEditor

                Layout.fillHeight: true
                Layout.fillWidth: true
                resourceEntry: contentViewLayout.currentResourceEntry
                visible: contentViewLayout.imageResourceSelected
            }

            Item {
                id: contentsTextEditorStack

                Layout.fillHeight: true
                Layout.fillWidth: true
                enabled: contentViewLayout.noteEditorSurfaceVisible
                visible: contentViewLayout.noteEditorSurfaceVisible

                ContentsView.TextEditor {
                    id: contentsTextEditor

                    anchors.fill: parent
                    editorReadOnly: true
                    noteBodyFilePath: ""
                    objectName: "contentsDisplayTextEditor"
                }
            }

            ContentsView.Minimap {
                Layout.fillHeight: true
                Layout.preferredWidth: visible ? implicitWidth : 0
                documentText: contentsTextEditor.editorDocumentText
                scrollTarget: contentsTextEditor.scrollEditorViewportTo
                sourceContentHeight: contentsTextEditor.editorViewportContentHeight
                sourceContentWidth: contentsTextEditor.editorViewportWidth
                sourceContentY: contentsTextEditor.viewportContentY
                sourceFontFamily: LV.Theme.fontBody
                sourceFontLetterSpacing: LV.Theme.textBodyLetterSpacing
                sourceFontPixelSize: LV.Theme.textBody
                sourceFontWeight: LV.Theme.textBodyWeight
                sourceViewportHeight: contentsTextEditor.editorViewportHeight
                visible: contentViewLayout.noteEditorSurfaceVisible && contentViewLayout.minimapVisible
            }
        }

        Item {
            id: calendarOverlayStack

            anchors.fill: parent
            enabled: contentViewLayout.calendarOverlayActive
            visible: contentViewLayout.calendarOverlayActive
            z: 4

            Rectangle {
                anchors.fill: parent
                color: contentViewLayout.displayColor
            }

            CalendarView.DayCalendarPage {
                anchors.fill: parent
                dayCalendarController: contentViewLayout.dayCalendarController
                visible: contentViewLayout.dayCalendarOverlayVisible

                onNoteOpenRequested: function(noteId) {
                    contentViewLayout.openCalendarNote(noteId);
                }
                onOverlayCloseRequested: contentViewLayout.dayCalendarOverlayCloseRequested()
            }

            CalendarView.MonthCalendarPage {
                anchors.fill: parent
                monthCalendarController: contentViewLayout.monthCalendarController
                visible: contentViewLayout.monthCalendarOverlayVisible

                onNoteOpenRequested: function(noteId) {
                    contentViewLayout.openCalendarNote(noteId);
                }
                onOverlayCloseRequested: contentViewLayout.monthCalendarOverlayCloseRequested()
            }

            CalendarView.WeekCalendarPage {
                anchors.fill: parent
                weekCalendarController: contentViewLayout.weekCalendarController
                visible: contentViewLayout.weekCalendarOverlayVisible

                onNoteOpenRequested: function(noteId) {
                    contentViewLayout.openCalendarNote(noteId);
                }
                onOverlayCloseRequested: contentViewLayout.weekCalendarOverlayCloseRequested()
            }

            CalendarView.YearCalendarPage {
                anchors.fill: parent
                yearCalendarController: contentViewLayout.yearCalendarController
                visible: contentViewLayout.yearCalendarOverlayVisible

                onMonthOpenRequested: function(year, month, selectedDateIso) {
                    contentViewLayout.openMonthCalendarFromYear(year, month, selectedDateIso);
                }
                onOverlayCloseRequested: contentViewLayout.yearCalendarOverlayCloseRequested()
            }
        }
    }
}
