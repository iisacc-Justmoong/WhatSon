import QtQuick
import LVRS 1.0 as LV

Item {
    id: detailPanel

    readonly property var noteDetailPanelRuntime: typeof noteDetailPanelController !== "undefined" ? noteDetailPanelController : null
    readonly property var resourceDetailPanelRuntime: typeof resourceDetailPanelController !== "undefined" ? resourceDetailPanelController : null
    readonly property var resourcesHierarchyControllerObject: typeof resourcesHierarchyController !== "undefined" ? resourcesHierarchyController : null
    readonly property var sidebarHierarchyControllerObject: typeof sidebarHierarchyController !== "undefined" ? sidebarHierarchyController : null
    readonly property bool resourceHierarchyActive: sidebarHierarchyControllerObject && resourcesHierarchyControllerObject && sidebarHierarchyControllerObject.activeHierarchyController === resourcesHierarchyControllerObject

    signal viewHookRequested

    function requestViewHook(reason) {
        viewHookRequested();
    }

    objectName: "DetailPanel"

    NoteDetailPanel {
        anchors.fill: parent
        noteDetailPanelController: detailPanel.noteDetailPanelRuntime
        visible: !detailPanel.resourceHierarchyActive
    }

    ResourceDetailPanel {
        anchors.fill: parent
        resourceDetailPanelController: detailPanel.resourceDetailPanelRuntime
        visible: detailPanel.resourceHierarchyActive
    }
}
