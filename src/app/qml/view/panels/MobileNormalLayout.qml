import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: mobileNormalLayout

    property color canvasColor: LV.Theme.panelBackground01
    property color controlSurfaceColor: LV.Theme.panelBackground10
    readonly property int contentInset: LV.Theme.gap16
    readonly property int sectionSpacing: LV.Theme.gap24
    property var editorViewModeViewModel: null
    property string hierarchySearchText: ""
    property var navigationModeViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("MobileNormalLayout") : null
    required property var sidebarHierarchyViewModel
    property string statusPlaceholderText: ""
    property string statusSearchText: ""
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    property var windowInteractions: null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function requestCreateFolder() {
        mobileNormalLayout.requestViewHook("create-folder");
        if (hierarchySidebar && hierarchySidebar.requestCreateFolder !== undefined)
            hierarchySidebar.requestCreateFolder();
    }
    function requestCreateNote() {
        mobileNormalLayout.requestViewHook("create-note");
        if (mobileNormalLayout.windowInteractions && mobileNormalLayout.windowInteractions.createNoteFromShortcut !== undefined)
            mobileNormalLayout.windowInteractions.createNoteFromShortcut();
    }

    Rectangle {
        anchors.fill: parent
        color: mobileNormalLayout.canvasColor
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: mobileNormalLayout.contentInset
        spacing: mobileNormalLayout.sectionSpacing

        NavigationBarLayout {
            id: compactNavigationBar

            compactAddFolderEnabled: hierarchySidebar.createFolderEnabled
            compactAddFolderVisible: true
            compactMode: true
            compactSurfaceColor: mobileNormalLayout.controlSurfaceColor
            editorViewModeViewModel: mobileNormalLayout.editorViewModeViewModel
            navigationModeViewModel: mobileNormalLayout.navigationModeViewModel

            onCompactAddFolderRequested: mobileNormalLayout.requestCreateFolder()
        }
        HierarchySidebarLayout {
            id: hierarchySidebar

            Layout.fillHeight: true
            Layout.fillWidth: true
            footerVisible: false
            horizontalInset: LV.Theme.gapNone
            panelColor: mobileNormalLayout.canvasColor
            searchFieldVisible: true
            searchHeaderTopGap: LV.Theme.gap2
            searchListGap: LV.Theme.gap2
            searchText: mobileNormalLayout.hierarchySearchText
            sidebarHierarchyViewModel: mobileNormalLayout.sidebarHierarchyViewModel
            toolbarFrameWidth: width
            toolbarIconNames: mobileNormalLayout.toolbarIconNames
            verticalInset: LV.Theme.gapNone

            onSearchSubmitted: function (text) {
                mobileNormalLayout.hierarchySearchText = text;
            }
            onSearchTextEdited: function (text) {
                mobileNormalLayout.hierarchySearchText = text;
            }
        }
        StatusBarLayout {
            compactFieldColor: mobileNormalLayout.controlSurfaceColor
            compactMode: true
            compactToolbarText: mobileNormalLayout.statusPlaceholderText
            searchText: mobileNormalLayout.statusSearchText

            onCreateNoteRequested: mobileNormalLayout.requestCreateNote()
            onSearchSubmitted: function (text) {
                mobileNormalLayout.statusSearchText = text;
            }
            onSearchTextEdited: function (text) {
                mobileNormalLayout.statusSearchText = text;
            }
        }
    }
}
