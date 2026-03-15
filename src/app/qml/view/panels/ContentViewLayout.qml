pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: contentViewLayout

    property var contentViewModel: null
    property color displayColor: LV.Theme.panelBackground06
    property color drawerColor: LV.Theme.panelBackground08
    property int drawerHeight: LV.Theme.controlHeightMd * 7 + LV.Theme.gap3
    property int minDisplayHeight: LV.Theme.gap20 * 8
    property int minDrawerHeight: LV.Theme.gap20 * 6
    property var noteListModel: null
    property color panelColor: LV.Theme.panelBackground07
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("ContentViewLayout") : null
    readonly property var resolvedContentViewModel: contentViewLayout.contentViewModel
    readonly property var resolvedNoteListModel: contentViewLayout.noteListModel
    property color splitterColor: "transparent"
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone

    signal drawerHeightDragRequested(int value)
    signal editorTextEdited(string text)
    signal viewHookRequested

    Layout.fillHeight: true
    Layout.fillWidth: true

    ContentsDisplayView {
        anchors.fill: parent
        contentViewModel: contentViewLayout.resolvedContentViewModel
        displayColor: contentViewLayout.displayColor
        drawerColor: contentViewLayout.drawerColor
        drawerHeight: contentViewLayout.drawerHeight
        minDisplayHeight: contentViewLayout.minDisplayHeight
        minDrawerHeight: contentViewLayout.minDrawerHeight
        noteListModel: contentViewLayout.resolvedNoteListModel
        panelColor: contentViewLayout.panelColor
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
}
