pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: drawerMenubar

    readonly property string figmaNodeId: "155:4565"
    readonly property string quickNoteModeName: "QuickNote"
    readonly property string itemBoxModeName: "ItemBox"
    readonly property string dataSearchModeName: "DataSearch"
    readonly property string graphViewModeName: "GraphView"
    property string activeDrawerMode: drawerMenubar.quickNoteModeName

    signal drawerConfigActionRequested(string actionName)
    signal drawerModeRequested(string modeName)
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        viewHookRequested(hookReason);
    }

    objectName: "DrawerMenubar"
    color: LV.Theme.panelBackground02
    implicitHeight: LV.Theme.gap24

    Item {
        anchors.fill: parent
        anchors.leftMargin: LV.Theme.gap4
        anchors.rightMargin: LV.Theme.gap4
        anchors.topMargin: LV.Theme.gap2
        anchors.bottomMargin: LV.Theme.gap2

        LV.HStack {
            anchors.fill: parent
            spacing: LV.Theme.gapNone

            LV.IconSegmentedControl {
                id: drawerModes

                readonly property string figmaNodeId: "155:4566"
                objectName: "DrawerModes"
                Layout.alignment: Qt.AlignVCenter
                backgroundColor: LV.Theme.panelBackground08
                borderColor: LV.Theme.panelBackground12
                borderWidth: LV.Theme.scaleMetric(2)
                cornerRadius: LV.Theme.radiusMd
                forceBorderlessTone: false
                horizontalPadding: LV.Theme.gap4
                spacing: LV.Theme.gap2
                verticalPadding: LV.Theme.gap4

                LV.IconButton {
                    id: quickNoteButton

                    readonly property string figmaNodeId: "I155:4566;206:4189"
                    objectName: "QuickNote"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "textToImage"
                    tone: drawerMenubar.activeDrawerMode === drawerMenubar.quickNoteModeName
                        ? LV.AbstractButton.Default
                        : LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        drawerMenubar.drawerModeRequested(drawerMenubar.quickNoteModeName);
                        drawerMenubar.requestViewHook("drawer-mode-quick-note");
                    }
                }
                LV.IconButton {
                    id: itemBoxButton

                    readonly property string figmaNodeId: "I155:4566;206:4190"
                    objectName: "ItemBox"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "swiftPackage"
                    tone: drawerMenubar.activeDrawerMode === drawerMenubar.itemBoxModeName
                        ? LV.AbstractButton.Default
                        : LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        drawerMenubar.drawerModeRequested(drawerMenubar.itemBoxModeName);
                        drawerMenubar.requestViewHook("drawer-mode-item-box");
                    }
                }
                LV.IconButton {
                    id: dataSearchButton

                    readonly property string figmaNodeId: "I155:4566;206:4194"
                    objectName: "DataSearch"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "shortcutFilter"
                    tone: drawerMenubar.activeDrawerMode === drawerMenubar.dataSearchModeName
                        ? LV.AbstractButton.Default
                        : LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        drawerMenubar.drawerModeRequested(drawerMenubar.dataSearchModeName);
                        drawerMenubar.requestViewHook("drawer-mode-data-search");
                    }
                }
                LV.IconButton {
                    id: graphViewButton

                    readonly property string figmaNodeId: "I155:4566;206:4231"
                    objectName: "GraphView"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "graphMachineLearning"
                    tone: drawerMenubar.activeDrawerMode === drawerMenubar.graphViewModeName
                        ? LV.AbstractButton.Default
                        : LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        drawerMenubar.drawerModeRequested(drawerMenubar.graphViewModeName);
                        drawerMenubar.requestViewHook("drawer-mode-graph-view");
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
            LV.HStack {
                id: drawerViewConfig

                readonly property string figmaNodeId: "155:4567"
                objectName: "DrawerViewConfig"
                Layout.alignment: Qt.AlignVCenter
                spacing: LV.Theme.gap2

                LV.IconButton {
                    id: textAlignButton

                    readonly property string figmaNodeId: "155:4568"
                    objectName: "TextAlign"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "leftAlign"
                    tone: LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        drawerMenubar.drawerConfigActionRequested("TextAlign");
                        drawerMenubar.requestViewHook("drawer-text-align");
                    }
                }
                LV.IconMenuButton {
                    id: viewOptionsButton

                    readonly property string figmaNodeId: "155:4569"
                    objectName: "ViewOptions"
                    bottomPadding: LV.Theme.gap2
                    iconName: "breakpointsbreakpointFieldUnsuspendentValid"
                    leftPadding: LV.Theme.gap2
                    rightPadding: LV.Theme.gap4
                    tone: LV.AbstractButton.Borderless
                    topPadding: LV.Theme.gap2

                    onClicked: {
                        drawerMenubar.drawerConfigActionRequested("ViewOptions");
                        drawerMenubar.requestViewHook("drawer-view-options");
                    }
                }
                LV.IconButton {
                    id: editorPreviewVertical

                    readonly property string figmaNodeId: "194:6693"
                    objectName: "editorPreviewVertical"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "editorPreviewVertical"
                    tone: LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        drawerMenubar.drawerConfigActionRequested("editorPreviewVertical");
                        drawerMenubar.requestViewHook("drawer-preview-vertical");
                    }
                }
            }
        }
    }
}
