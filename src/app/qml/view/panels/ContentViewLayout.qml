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
    readonly property var resolvedContentViewModel: contentViewLayout.contentViewModel
    readonly property var resolvedNoteListModel: contentViewLayout.noteListModel
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarViewModel: null

    signal drawerHeightDragRequested(int value)
    signal editorTextEdited(string text)
    signal viewHookRequested
    signal yearCalendarOverlayCloseRequested

    Layout.fillHeight: true
    Layout.fillWidth: true

    ContentsDisplayView {
        anchors.fill: parent
        contentViewModel: contentViewLayout.resolvedContentViewModel
        displayColor: contentViewLayout.displayColor
        drawerVisible: contentViewLayout.drawerVisible
        drawerColor: contentViewLayout.drawerColor
        drawerHeight: contentViewLayout.drawerHeight
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
    Item {
        id: yearCalendarOverlay

        anchors.fill: parent
        visible: contentViewLayout.yearCalendarOverlayVisible
        z: 40

        Rectangle {
            anchors.fill: parent
            color: "#6610151F"
        }
        MouseArea {
            anchors.fill: parent

            onClicked: contentViewLayout.yearCalendarOverlayCloseRequested()
        }
        Item {
            id: yearCalendarHost

            anchors.fill: parent
            anchors.margins: LV.Theme.gap8
            z: 1

            CalendarView.YearCalendarPage {
                id: yearCalendarPage

                anchors.fill: parent
                yearCalendarViewModel: contentViewLayout.yearCalendarViewModel

                onViewHookRequested: function (reason) {
                    contentViewLayout.viewHookRequested();
                }
            }
            LV.LabelButton {
                anchors.right: parent.right
                anchors.top: parent.top
                text: "Close"

                onClicked: contentViewLayout.yearCalendarOverlayCloseRequested()
            }
        }
    }
}
