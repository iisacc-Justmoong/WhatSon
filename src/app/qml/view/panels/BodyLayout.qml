import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: hStack

    function traceActiveBindings(reason) {
        console.log("[whatson:qml][BodyLayout][" + reason + "] activeHierarchyIndex=" + hStack.activeHierarchyIndex + " activeHierarchyController=" + hStack.activeHierarchyController + " activeNoteListModel=" + hStack.activeNoteListModel);
    }

    property var activeHierarchyBindingSnapshot: ({
            "index": 0,
            "controller": null
        })
    readonly property int activeHierarchyIndex: {
        const snapshot = hStack.activeHierarchyBindingSnapshot;
        const numericIndex = Number(snapshot && snapshot.index !== undefined ? snapshot.index : 0);
        return isFinite(numericIndex) ? Math.floor(numericIndex) : 0;
    }
    readonly property var activeHierarchyController: hStack.activeHierarchyBindingSnapshot ? hStack.activeHierarchyBindingSnapshot.controller : null
    readonly property var activeNoteListModel: hStack.sidebarHierarchyController ? hStack.sidebarHierarchyController.activeNoteListModel : null
    property color compactCanvasColor: LV.Theme.panelBackground01
    property bool compactMode: false
    property color contentsDisplayColor: "transparent"
    property var editorViewModeController: null
    readonly property int effectiveMinSidebarWidth: Math.max(minSidebarWidth, LV.Theme.gap20 * 7 + LV.Theme.gap12)
    property color gutterColor: LV.Theme.panelBackground02
    property bool isMobilePlatform: false
    property var libraryHierarchyController: null
    property color listViewColor: "transparent"
    property int listViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property bool listVisible: hStack.listViewWidth > 0
    property int minContentWidth: LV.Theme.dialogMaxWidth - LV.Theme.gap20 * 2
    property int minListViewWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    property int minRightPanelWidth: LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + LV.Theme.controlHeightMd + Math.round(LV.Theme.strokeThin)
    property int minSidebarWidth: LV.Theme.gap20 * 7 + LV.Theme.gap12
    property var noteDeletionController: null
    readonly property var resolvedNoteDeletionController: {
        const activeController = hStack.activeHierarchyController;
        if (activeController && (activeController.deleteNotesByIds !== undefined || activeController.deleteNoteById !== undefined || activeController.clearNoteFoldersByIds !== undefined || activeController.clearNoteFoldersById !== undefined)) {
            return activeController;
        }
        return hStack.noteDeletionController;
    }
    property var panelControllerRegistry: null
    readonly property var panelController: hStack.panelControllerRegistry ? hStack.panelControllerRegistry.panelController("BodyLayout") : null
    property var resourcesImportController: null
    property color rightPanelColor: "transparent"
    property int rightPanelWidth: LV.Theme.inputMinWidth + LV.Theme.gap14
    readonly property bool rightVisible: hStack.rightPanelWidth > 0
    property color sidebarColor: "transparent"
    required property var sidebarHierarchyController
    readonly property bool sidebarVisible: hStack.sidebarWidth > 0
    property int sidebarHorizontalInset: LV.Theme.gap2
    property int sidebarWidth: LV.Theme.gap24 * 9
    property color splitterColor: LV.Theme.panelBackground10
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarController: null
    property bool agendaOverlayVisible: false
    property var agendaController: null
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    property bool monthCalendarOverlayVisible: false
    property var monthCalendarController: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarController: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarController: null

    signal listViewWidthDragRequested(int value)
    signal noteActivated(int index, string noteId)
    signal rightPanelWidthDragRequested(int value)
    signal sidebarWidthDragRequested(int value)
    signal viewHookRequested
    signal dayCalendarOverlayDismissRequested
    signal agendaOverlayDismissRequested
    signal monthCalendarOverlayOpenRequested
    signal monthCalendarOverlayDismissRequested
    signal weekCalendarOverlayDismissRequested
    signal yearCalendarOverlayDismissRequested

    function clampListViewWidth(value) {
        const numericValue = Number(value);
        const resolvedValue = isFinite(numericValue) ? numericValue : hStack.listViewWidth;
        const occupiedWidth = hStack.totalSplitterWidth() + (hStack.sidebarVisible ? hStack.sidebarWidth : 0) + hStack.minContentWidth + (hStack.rightVisible ? hStack.minRightPanelWidth : 0);
        const maxWidth = Math.max(hStack.minListViewWidth, hStack.width - occupiedWidth);
        return Math.max(hStack.minListViewWidth, Math.min(maxWidth, Math.floor(resolvedValue)));
    }
    function syncActiveHierarchyBindings() {
        const sidebarController = hStack.sidebarHierarchyController;
        if (!sidebarController) {
            hStack.activeHierarchyBindingSnapshot = {
                "index": 0,
                "controller": null
            };
            return;
        }
        const numericIndex = Number(sidebarController.resolvedActiveHierarchyIndex);
        const resolvedIndex = isFinite(numericIndex) ? Math.floor(numericIndex) : 0;
        const resolvedHierarchyController = sidebarController.hierarchyControllerForIndex !== undefined ? sidebarController.hierarchyControllerForIndex(resolvedIndex) : sidebarController.resolvedHierarchyController;
        hStack.activeHierarchyBindingSnapshot = {
            "index": resolvedIndex,
            "controller": resolvedHierarchyController
        };
    }
    function clampRightPanelWidth(value) {
        const numericValue = Number(value);
        const resolvedValue = isFinite(numericValue) ? numericValue : hStack.rightPanelWidth;
        const occupiedWidth = hStack.totalSplitterWidth() + (hStack.sidebarVisible ? hStack.sidebarWidth : 0) + hStack.minContentWidth + (hStack.listVisible ? hStack.minListViewWidth : 0);
        const maxWidth = Math.max(hStack.minRightPanelWidth, hStack.width - occupiedWidth);
        return Math.max(hStack.minRightPanelWidth, Math.min(maxWidth, Math.floor(resolvedValue)));
    }
    function clampSidebarWidth(value) {
        const numericValue = Number(value);
        const resolvedValue = isFinite(numericValue) ? numericValue : hStack.sidebarWidth;
        const occupiedWidth = hStack.totalSplitterWidth() + hStack.minContentWidth + (hStack.listVisible ? hStack.minListViewWidth : 0) + (hStack.rightVisible ? hStack.minRightPanelWidth : 0);
        const maxWidth = Math.max(hStack.effectiveMinSidebarWidth, hStack.width - occupiedWidth);
        return Math.max(hStack.effectiveMinSidebarWidth, Math.min(maxWidth, Math.floor(resolvedValue)));
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested();
    }
    function totalSplitterWidth() {
        // CRITICAL REGRESSION GUARD:
        // This function MUST return a finite number. If return is removed or undefined/NaN leaks,
        // clamp math fails and panel edge drag-resizing becomes blocked (historically repeated issue).
        var width = hStack.sidebarVisible ? hStack.splitterThickness : 0;
        if (hStack.listVisible)
            width += hStack.splitterThickness;
        if (hStack.rightVisible)
            width += hStack.splitterThickness;
        return isFinite(width) ? width : 0;
    }

    Layout.fillHeight: true
    Layout.fillWidth: true
    clip: true
    Component.onCompleted: {
        hStack.syncActiveHierarchyBindings();
        hStack.traceActiveBindings("completed");
    }
    onSidebarHierarchyControllerChanged: hStack.syncActiveHierarchyBindings()
    onActiveHierarchyBindingSnapshotChanged: hStack.traceActiveBindings("activeHierarchyBindingSnapshotChanged")

    Connections {
        target: hStack.sidebarHierarchyController
        ignoreUnknownSignals: true

        function onActiveBindingsChanged() {
            hStack.syncActiveHierarchyBindings();
        }
    }

    Item {
        anchors.fill: parent
        visible: !hStack.compactMode

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            color: hStack.splitterColor
            height: Math.max(1, hStack.splitterThickness)
            z: 2
        }

        LV.HStack {
            anchors.fill: parent
            spacing: LV.Theme.gapNone

            HierarchySidebarLayout {
                id: sideBar

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.sidebarVisible ? hStack.effectiveMinSidebarWidth : 0
                Layout.preferredWidth: hStack.sidebarVisible ? hStack.sidebarWidth : 0
                horizontalInset: hStack.sidebarHorizontalInset
                panelColor: hStack.sidebarColor
                searchFieldVisible: true
                sidebarHierarchyController: hStack.sidebarHierarchyController
                toolbarIconNames: hStack.toolbarIconNames
                visible: hStack.sidebarVisible
            }
            PanelEdgeSplitter {
                id: sideBarSplitter

                clampSize: function (value) {
                    return hStack.clampSidebarWidth(value);
                }
                currentSize: hStack.sidebarWidth
                dragDirection: 1
                handleThickness: hStack.splitterHandleThickness
                splitterColor: hStack.splitterColor
                splitterThickness: hStack.splitterThickness
                visible: hStack.sidebarVisible

                onSizeDragRequested: function (value) {
                    hStack.sidebarWidthDragRequested(value);
                }
            }
            Rectangle {
                id: listBar

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.minListViewWidth
                Layout.preferredWidth: hStack.listVisible ? hStack.listViewWidth : 0
                color: LV.Theme.accentTransparent
                visible: hStack.listVisible

                ListBarLayout {
                    activeToolbarIndex: hStack.activeHierarchyIndex
                    anchors.fill: parent
                    hierarchyController: hStack.activeHierarchyController
                    isMobilePlatform: hStack.isMobilePlatform
                    noteListModel: hStack.activeNoteListModel
                    noteDeletionController: hStack.resolvedNoteDeletionController
                    noteDropTarget: sideBar.noteDropTargetView
                    panelColor: hStack.listViewColor
                    panelControllerRegistry: null

                    onNoteActivated: function (index, noteId) {
                        hStack.noteActivated(index, noteId);
                    }
                }
            }
            PanelEdgeSplitter {
                id: listSplitter

                clampSize: function (value) {
                    return hStack.clampListViewWidth(value);
                }
                currentSize: hStack.listViewWidth
                dragDirection: 1
                handleThickness: hStack.splitterHandleThickness
                splitterColor: hStack.splitterColor
                splitterThickness: hStack.splitterThickness
                visible: hStack.listVisible

                onSizeDragRequested: function (value) {
                    hStack.listViewWidthDragRequested(value);
                }
            }
            ContentViewLayout {
                id: contentsView

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumWidth: hStack.minContentWidth
                contentController: hStack.activeHierarchyController
                displayColor: hStack.contentsDisplayColor
                editorViewModeController: hStack.editorViewModeController
                gutterColor: hStack.gutterColor
                isMobilePlatform: hStack.isMobilePlatform
                libraryHierarchyController: hStack.libraryHierarchyController
                noteListModel: hStack.activeNoteListModel
                panelControllerRegistry: null
                resourcesImportController: hStack.resourcesImportController
                sidebarHierarchyController: hStack.sidebarHierarchyController
                dayCalendarOverlayVisible: hStack.dayCalendarOverlayVisible
                dayCalendarController: hStack.dayCalendarController
                agendaOverlayVisible: hStack.agendaOverlayVisible
                agendaController: hStack.agendaController
                monthCalendarOverlayVisible: hStack.monthCalendarOverlayVisible
                monthCalendarController: hStack.monthCalendarController
                weekCalendarOverlayVisible: hStack.weekCalendarOverlayVisible
                weekCalendarController: hStack.weekCalendarController
                yearCalendarOverlayVisible: hStack.yearCalendarOverlayVisible
                yearCalendarController: hStack.yearCalendarController

                onDayCalendarOverlayCloseRequested: hStack.dayCalendarOverlayDismissRequested()
                onAgendaOverlayCloseRequested: hStack.agendaOverlayDismissRequested()
                onMonthCalendarOverlayOpenRequested: hStack.monthCalendarOverlayOpenRequested()
                onMonthCalendarOverlayCloseRequested: hStack.monthCalendarOverlayDismissRequested()
                onWeekCalendarOverlayCloseRequested: hStack.weekCalendarOverlayDismissRequested()
                onYearCalendarOverlayCloseRequested: hStack.yearCalendarOverlayDismissRequested()
            }
            PanelEdgeSplitter {
                id: rightSplitter

                clampSize: function (value) {
                    return hStack.clampRightPanelWidth(value);
                }
                currentSize: hStack.rightPanelWidth
                dragDirection: -1
                handleThickness: hStack.splitterHandleThickness
                splitterColor: hStack.splitterColor
                splitterThickness: hStack.splitterThickness
                visible: hStack.rightVisible

                onSizeDragRequested: function (value) {
                    hStack.rightPanelWidthDragRequested(value);
                }
            }
            DetailPanelLayout {
                id: rightPanel

                Layout.fillHeight: true
                Layout.minimumWidth: hStack.rightVisible ? hStack.minRightPanelWidth : 0
                Layout.preferredWidth: hStack.rightVisible ? hStack.rightPanelWidth : 0
                panelColor: hStack.rightPanelColor
                visible: hStack.rightVisible
            }
        }
    }
    Item {
        anchors.fill: parent
        visible: hStack.compactMode

        Rectangle {
            anchors.fill: parent
            color: hStack.compactCanvasColor
        }
    }
}
