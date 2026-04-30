import QtQuick
import LVRS 1.0 as LV

Item {
    id: noteDetailPanel

    property var noteDetailPanelController: null
    readonly property var defaultToolbarItems: [
        {
            "figmaNodeId": "155:4576",
            "iconName": "config",
            "iconSource": LV.Theme.iconPath("configuration"),
            "objectName": "Properties",
            "selected": true,
            "stateValue": 0
        },
        {
            "figmaNodeId": "155:4577",
            "iconName": "chartBar",
            "objectName": "FileStat",
            "selected": false,
            "stateValue": 1
        },
        {
            "figmaNodeId": "155:4578",
            "iconName": "generaladd",
            "objectName": "Insert",
            "selected": false,
            "stateValue": 2
        },
        {
            "figmaNodeId": "155:4579",
            "iconName": "toolwindowdependencies",
            "objectName": "Layer",
            "selected": false,
            "stateValue": 4
        },
        {
            "figmaNodeId": "155:4580",
            "iconName": "toolWindowClock",
            "objectName": "FileHistory",
            "selected": false,
            "stateValue": 3
        },
        {
            "figmaNodeId": "155:4581",
            "iconName": "featureAnswer",
            "objectName": "Help",
            "selected": false,
            "stateValue": 5
        }
    ]
    readonly property int defaultPanelWidth: LV.Theme.inputMinWidth + LV.Theme.gap14
    readonly property int detailContentsHeight: Math.max(0, noteDetailPanel.height - noteDetailPanel.headerToolbarHeight - noteDetailPanel.panelSpacing)
    readonly property int detailContentsWidth: noteDetailPanel.width
    readonly property var detailPanelRuntime: noteDetailPanel.noteDetailPanelController
    property int panelSpacing: LV.Theme.gap10
    readonly property var resolvedActiveContentController: noteDetailPanel.resolveActiveContentController()
    readonly property var resolvedFileStatController: noteDetailPanel.resolveFileStatController()
    readonly property var resolvedProjectSelectionController: noteDetailPanel.resolveProjectSelectionController()
    readonly property var resolvedBookmarkSelectionController: noteDetailPanel.resolveBookmarkSelectionController()
    readonly property var resolvedProgressSelectionController: noteDetailPanel.resolveProgressSelectionController()
    readonly property string resolvedActiveStateName: noteDetailPanel.resolveActiveStateName()
    readonly property bool resolvedNoteContextLinked: noteDetailPanel.resolveNoteContextLinked()
    readonly property string resolvedPanelStateName: noteDetailPanel.resolvedNoteContextLinked ? "linked" : "detached"
    readonly property var resolvedToolbarItems: noteDetailPanel.resolveToolbarItems()
    readonly property int headerToolbarHeight: detailHeaderToolbar ? Math.max(0, Math.round(detailHeaderToolbar.implicitHeight || detailHeaderToolbar.height || 0)) : 0

    signal viewHookRequested

    function requestViewHook(reason) {
        viewHookRequested();
    }
    function toolbarMetadataForStateValue(stateValue) {
        switch (Number(stateValue)) {
        case 0:
            return noteDetailPanel.defaultToolbarItems[0];
        case 1:
            return noteDetailPanel.defaultToolbarItems[1];
        case 2:
            return noteDetailPanel.defaultToolbarItems[2];
        case 3:
            return noteDetailPanel.defaultToolbarItems[4];
        case 4:
            return noteDetailPanel.defaultToolbarItems[3];
        case 5:
            return noteDetailPanel.defaultToolbarItems[5];
        default:
            return noteDetailPanel.defaultToolbarItems[0];
        }
    }
    function resolveActiveContentController() {
        if (!noteDetailPanel.detailPanelRuntime || noteDetailPanel.detailPanelRuntime.activeContentController === undefined)
            return null;
        return noteDetailPanel.detailPanelRuntime.activeContentController;
    }
    function resolveFileStatController() {
        if (!noteDetailPanel.detailPanelRuntime || noteDetailPanel.detailPanelRuntime.fileStatController === undefined)
            return null;
        return noteDetailPanel.detailPanelRuntime.fileStatController;
    }
    function resolveProjectSelectionController() {
        if (!noteDetailPanel.detailPanelRuntime || noteDetailPanel.detailPanelRuntime.projectSelectionController === undefined)
            return null;
        return noteDetailPanel.detailPanelRuntime.projectSelectionController;
    }
    function resolveBookmarkSelectionController() {
        if (!noteDetailPanel.detailPanelRuntime || noteDetailPanel.detailPanelRuntime.bookmarkSelectionController === undefined)
            return null;
        return noteDetailPanel.detailPanelRuntime.bookmarkSelectionController;
    }
    function resolveProgressSelectionController() {
        if (!noteDetailPanel.detailPanelRuntime || noteDetailPanel.detailPanelRuntime.progressSelectionController === undefined)
            return null;
        return noteDetailPanel.detailPanelRuntime.progressSelectionController;
    }
    function resolveActiveStateName() {
        if (!noteDetailPanel.detailPanelRuntime || noteDetailPanel.detailPanelRuntime.activeStateName === undefined)
            return "properties";
        const stateName = String(noteDetailPanel.detailPanelRuntime.activeStateName).trim();
        return stateName.length > 0 ? stateName : "properties";
    }
    function resolveNoteContextLinked() {
        if (!noteDetailPanel.detailPanelRuntime || noteDetailPanel.detailPanelRuntime.noteContextLinked === undefined)
            return false;
        return noteDetailPanel.detailPanelRuntime.noteContextLinked === true;
    }
    function resolveToolbarItems() {
        if (!noteDetailPanel.detailPanelRuntime || noteDetailPanel.detailPanelRuntime.toolbarItems === undefined || noteDetailPanel.detailPanelRuntime.toolbarItems === null)
            return noteDetailPanel.defaultToolbarItems;
        const sourceItems = noteDetailPanel.detailPanelRuntime.toolbarItems;
        const normalizedItems = Array.isArray(sourceItems) ? sourceItems : sourceItems.length !== undefined ? Array.prototype.slice.call(sourceItems) : [];
        if (normalizedItems.length <= 0)
            return noteDetailPanel.defaultToolbarItems;
        const resolvedItems = [];
        for (var index = 0; index < normalizedItems.length; ++index) {
            const sourceItem = normalizedItems[index];
            const stateValue = sourceItem && sourceItem.stateValue !== undefined ? Number(sourceItem.stateValue) : NaN;
            const metadata = noteDetailPanel.toolbarMetadataForStateValue(stateValue);
            resolvedItems.push({
                "figmaNodeId": metadata.figmaNodeId,
                "iconName": metadata.iconName,
                "iconSource": metadata.iconSource,
                "objectName": metadata.objectName,
                "selected": sourceItem && sourceItem.selected === true,
                "stateValue": isFinite(stateValue) ? stateValue : metadata.stateValue
            });
        }
        return resolvedItems.length > 0 ? resolvedItems : noteDetailPanel.defaultToolbarItems;
    }

    objectName: "NoteDetailPanel"
    state: noteDetailPanel.resolvedPanelStateName
    implicitWidth: noteDetailPanel.defaultPanelWidth

    Column {
        anchors.fill: parent
        spacing: noteDetailPanel.panelSpacing
        visible: noteDetailPanel.resolvedNoteContextLinked

        Item {
            width: parent.width
            height: noteDetailPanel.headerToolbarHeight

            DetailPanelHeaderToolbar {
                id: detailHeaderToolbar

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                toolbarButtonSpecs: noteDetailPanel.resolvedToolbarItems

                onDetailStateChangeRequested: function (stateValue) {
                    noteDetailPanel.requestViewHook("detailStateChangeRequested.stateValue=" + stateValue);
                    if (noteDetailPanel.detailPanelRuntime)
                        noteDetailPanel.detailPanelRuntime.requestStateChange(stateValue);
                }
            }
        }
        DetailContents {
            activeContentController: noteDetailPanel.resolvedActiveContentController
            activeStateName: noteDetailPanel.resolvedActiveStateName
            bookmarkSelectionController: noteDetailPanel.resolvedBookmarkSelectionController
            detailPanelController: noteDetailPanel.detailPanelRuntime
            fileStatController: noteDetailPanel.resolvedFileStatController
            height: noteDetailPanel.detailContentsHeight
            noteContextLinked: noteDetailPanel.resolvedNoteContextLinked
            progressSelectionController: noteDetailPanel.resolvedProgressSelectionController
            projectSelectionController: noteDetailPanel.resolvedProjectSelectionController
            width: noteDetailPanel.detailContentsWidth
        }
    }

    Item {
        anchors.fill: parent
        visible: !noteDetailPanel.resolvedNoteContextLinked
    }

    states: [
        State {
            name: "linked"
        },
        State {
            name: "detached"
        }
    ]
}
