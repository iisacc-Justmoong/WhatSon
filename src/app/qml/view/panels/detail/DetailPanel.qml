import QtQuick
import LVRS 1.0 as LV

Item {
    id: detailPanel

    readonly property var registeredViewModelKeys: LV.ViewModels.keys
    readonly property var noteDetailPanelVm: {
        const _ = detailPanel.registeredViewModelKeys;
        return LV.ViewModels.get("noteDetailPanelViewModel");
    }
    readonly property var resourceDetailPanelVm: {
        const _ = detailPanel.registeredViewModelKeys;
        return LV.ViewModels.get("resourceDetailPanelViewModel");
    }
    readonly property var resourcesHierarchyVm: {
        const _ = detailPanel.registeredViewModelKeys;
        return LV.ViewModels.get("resourcesHierarchyViewModel");
    }
    readonly property var sidebarHierarchyVm: {
        const _ = detailPanel.registeredViewModelKeys;
        return LV.ViewModels.get("sidebarHierarchyViewModel");
    }
    readonly property bool resourceHierarchyActive: sidebarHierarchyVm
                                                    && resourcesHierarchyVm
                                                    && sidebarHierarchyVm.activeHierarchyViewModel === resourcesHierarchyVm

    signal viewHookRequested

    function requestViewHook(reason) {
        viewHookRequested();
    }

    objectName: "DetailPanel"

    NoteDetailPanel {
        anchors.fill: parent
        noteDetailPanelViewModel: detailPanel.noteDetailPanelVm
        visible: !detailPanel.resourceHierarchyActive
    }

    ResourceDetailPanel {
        anchors.fill: parent
        resourceDetailPanelViewModel: detailPanel.resourceDetailPanelVm
        visible: detailPanel.resourceHierarchyActive
    }
}
