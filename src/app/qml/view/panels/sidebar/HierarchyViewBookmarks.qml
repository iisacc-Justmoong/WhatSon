import QtQuick
import LVRS 1.0 as LV

SidebarHierarchyView {
    id: hierarchyViewBookmarks

    signal viewHookRequested

    function requestViewHook() {
        viewHookRequested();
    }

    defaultToolbarIndex: 2
    frameName: "HierarchyView-Bookmarks"
    frameNodeId: "sidebar.hierarchy.bookmarks"
}
