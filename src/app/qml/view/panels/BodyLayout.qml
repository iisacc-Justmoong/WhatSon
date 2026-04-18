import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: hStack

    property var activeHierarchyBindingSnapshot: ({
        "index": 0,
        "viewModel": null
    })
    readonly property int activeHierarchyIndex: {
        const snapshot = hStack.activeHierarchyBindingSnapshot;
        const numericIndex = Number(snapshot && snapshot.index !== undefined ? snapshot.index : 0);
        return isFinite(numericIndex) ? Math.floor(numericIndex) : 0;
    }
    readonly property var activeHierarchyViewModel: hStack.activeHierarchyBindingSnapshot
                                                  ? hStack.activeHierarchyBindingSnapshot.viewModel
                                                  : null
    readonly property var activeNoteListModel: activeNoteListModelResolver.noteListModel
    property color compactCanvasColor: LV.Theme.panelBackground01
    property bool compactMode: false
    property color contentsDisplayColor: "transparent"
    property var editorViewModeViewModel: null
    readonly property int effectiveMinSidebarWidth: Math.max(minSidebarWidth, LV.Theme.gap20 * 7 + LV.Theme.gap12)
    property color gutterColor: "transparent"
    property bool isMobilePlatform: false
    property var libraryHierarchyViewModel: null
    property color listViewColor: "transparent"
    property int listViewWidth: LV.Theme.inputWidthMd - LV.Theme.gap8
    readonly property bool listVisible: hStack.listViewWidth > 0
    property int minContentWidth: LV.Theme.dialogMaxWidth - LV.Theme.gap20 * 2
    property int minListViewWidth: LV.Theme.inputMinWidth - LV.Theme.gap24 * 2
    property int minRightPanelWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(145)))
    property int minSidebarWidth: LV.Theme.gap20 * 7 + LV.Theme.gap12
    property var noteDeletionViewModel: null
    readonly property var resolvedNoteDeletionViewModel: {
        const activeViewModel = hStack.activeHierarchyViewModel;
        if (activeViewModel
                && (activeViewModel.deleteNotesByIds !== undefined
                    || activeViewModel.deleteNoteById !== undefined
                    || activeViewModel.clearNoteFoldersByIds !== undefined
                    || activeViewModel.clearNoteFoldersById !== undefined)) {
            return activeViewModel;
        }
        return hStack.noteDeletionViewModel;
    }
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("BodyLayout") : null
    property var resourcesImportViewModel: null
    property color rightPanelColor: "transparent"
    property int rightPanelWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(194)))
    readonly property bool rightVisible: hStack.rightPanelWidth > 0
    property color sidebarColor: "transparent"
    required property var sidebarHierarchyViewModel
    readonly property bool sidebarVisible: hStack.sidebarWidth > 0
    property int sidebarHorizontalInset: LV.Theme.gap2
    property int sidebarWidth: LV.Theme.gap24 * 9
    property color splitterColor: LV.Theme.panelBackground10
    property int splitterHandleThickness: LV.Theme.gap12
    property int splitterThickness: LV.Theme.gapNone
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarViewModel: null
    property bool agendaOverlayVisible: false
    property var agendaViewModel: null
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    property bool monthCalendarOverlayVisible: false
    property var monthCalendarViewModel: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarViewModel: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarViewModel: null

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
        const sidebarViewModel = hStack.sidebarHierarchyViewModel;
        if (!sidebarViewModel) {
            hStack.activeHierarchyBindingSnapshot = {
                "index": 0,
                "viewModel": null
            };
            return;
        }
        const numericIndex = Number(sidebarViewModel.resolvedActiveHierarchyIndex);
        const resolvedIndex = isFinite(numericIndex) ? Math.floor(numericIndex) : 0;
        const resolvedHierarchyViewModel = sidebarViewModel.hierarchyViewModelForIndex !== undefined
                ? sidebarViewModel.hierarchyViewModelForIndex(resolvedIndex)
                : sidebarViewModel.resolvedHierarchyViewModel;
        hStack.activeHierarchyBindingSnapshot = {
            "index": resolvedIndex,
            "viewModel": resolvedHierarchyViewModel
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
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
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
    Component.onCompleted: hStack.syncActiveHierarchyBindings()
    onSidebarHierarchyViewModelChanged: hStack.syncActiveHierarchyBindings()

    NoteListModelContractBridge {
        id: activeNoteListModelResolver

        hierarchyViewModel: hStack.activeHierarchyViewModel
    }

    Connections {
        target: hStack.sidebarHierarchyViewModel
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
                sidebarHierarchyViewModel: hStack.sidebarHierarchyViewModel
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
                    hierarchyViewModel: hStack.activeHierarchyViewModel
                    noteListModel: hStack.activeNoteListModel
                    noteDeletionViewModel: hStack.resolvedNoteDeletionViewModel
                    noteDropTarget: sideBar.noteDropTargetView
                    panelColor: hStack.listViewColor

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
                contentViewModel: hStack.activeHierarchyViewModel
                displayColor: hStack.contentsDisplayColor
                editorViewModeViewModel: hStack.editorViewModeViewModel
                gutterColor: hStack.gutterColor
                isMobilePlatform: hStack.isMobilePlatform
                libraryHierarchyViewModel: hStack.libraryHierarchyViewModel
                noteListModel: hStack.activeNoteListModel
                resourcesImportViewModel: hStack.resourcesImportViewModel
                sidebarHierarchyViewModel: hStack.sidebarHierarchyViewModel
                dayCalendarOverlayVisible: hStack.dayCalendarOverlayVisible
                dayCalendarViewModel: hStack.dayCalendarViewModel
                agendaOverlayVisible: hStack.agendaOverlayVisible
                agendaViewModel: hStack.agendaViewModel
                monthCalendarOverlayVisible: hStack.monthCalendarOverlayVisible
                monthCalendarViewModel: hStack.monthCalendarViewModel
                weekCalendarOverlayVisible: hStack.weekCalendarOverlayVisible
                weekCalendarViewModel: hStack.weekCalendarViewModel
                yearCalendarOverlayVisible: hStack.yearCalendarOverlayVisible
                yearCalendarViewModel: hStack.yearCalendarViewModel

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
