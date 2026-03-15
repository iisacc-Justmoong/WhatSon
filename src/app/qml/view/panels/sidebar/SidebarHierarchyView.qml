import QtQuick
import LVRS 1.0 as LV

Item {
    id: sidebarHierarchyView

    property int activeToolbarIndex: defaultToolbarIndex
    property int defaultToolbarIndex: 0
    property string frameName: ""
    property string frameNodeId: ""
    property var hierarchyViewModel: null
    property int horizontalInset: 2
    property color panelColor: LV.Theme.panelBackground04
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("sidebar.SidebarHierarchyView") : null
    readonly property int selectedFolderIndex: hierarchyViewModel && hierarchyViewModel.selectedIndex !== undefined ? hierarchyViewModel.selectedIndex : -1
    readonly property var standardHierarchyModel: hierarchyViewModel && hierarchyViewModel.hierarchyModel !== undefined ? hierarchyViewModel.hierarchyModel : []
    property var toolbarIconNames: ["nodeslibraryFolder", "generalprojectStructure", "bookmarksbookmarksList", "vcscurrentBranch", "imageToImage", "chartBar", "dataView", "dataFile"]
    readonly property var toolbarItems: {
        if (sidebarHierarchyView.hierarchyViewModel && sidebarHierarchyView.hierarchyViewModel.toolbarItems !== undefined)
            return sidebarHierarchyView.hierarchyViewModel.toolbarItems;
        var items = [];
        for (var i = 0; i < toolbarIconNames.length; ++i) {
            items.push({
                "id": i,
                "iconName": toolbarIconNames[i],
                "selected": i === activeToolbarIndex
            });
        }
        return items;
    }
    readonly property int verticalInset: 2

    signal toolbarIndexChangeRequested(int index)
    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function syncSelectedHierarchyItem(focusView) {
        if (selectedFolderIndex < 0)
            return;
        hierarchyTree.activateListItemById(selectedFolderIndex);
        if (focusView)
            sidebarHierarchyView.forceActiveFocus();
    }

    clip: true
    focus: true

    onHierarchyViewModelChanged: Qt.callLater(function () {
        sidebarHierarchyView.syncSelectedHierarchyItem(false);
    })
    onSelectedFolderIndexChanged: syncSelectedHierarchyItem(true)

    Connections {
        function onHierarchyModelChanged() {
            Qt.callLater(function () {
                sidebarHierarchyView.syncSelectedHierarchyItem(false);
            });
        }

        ignoreUnknownSignals: true
        target: sidebarHierarchyView.hierarchyViewModel
    }
    LV.Hierarchy {
        id: hierarchyTree

        activeToolbarIndex: sidebarHierarchyView.activeToolbarIndex
        anchors.bottomMargin: sidebarHierarchyView.verticalInset
        anchors.fill: parent
        anchors.leftMargin: sidebarHierarchyView.horizontalInset
        anchors.rightMargin: sidebarHierarchyView.horizontalInset
        anchors.topMargin: sidebarHierarchyView.verticalInset
        keyboardListNavigationEnabled: false
        model: sidebarHierarchyView.standardHierarchyModel
        panelColor: sidebarHierarchyView.panelColor
        toolbarItems: sidebarHierarchyView.toolbarItems

        onListItemActivated: function (item, itemId, index) {
            if (!sidebarHierarchyView.hierarchyViewModel || sidebarHierarchyView.hierarchyViewModel.setSelectedIndex === undefined)
                return;
            sidebarHierarchyView.hierarchyViewModel.setSelectedIndex(itemId);
        }
        onToolbarActivated: function (button, buttonId, index) {
            if (index < 0 || index === sidebarHierarchyView.activeToolbarIndex)
                return;
            sidebarHierarchyView.toolbarIndexChangeRequested(index);
        }
    }
}
