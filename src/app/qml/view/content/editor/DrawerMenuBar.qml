pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: DrawerMenubar

    readonly property string figmaNodeId: "155:4565"
    readonly property string quickNoteModeName: "QuickNote"
    readonly property string itemBoxModeName: "ItemBox"
    readonly property string dataSearchModeName: "DataSearch"
    readonly property string graphViewModeName: "GraphView"
    property string activeDrawerMode: DrawerMenubar.quickNoteModeName

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
                id: DrawerModes

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
                    id: QuickNote

                    readonly property string figmaNodeId: "I155:4566;206:4189"
                    objectName: "QuickNote"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "textToImage"
                    tone: DrawerMenubar.activeDrawerMode === DrawerMenubar.quickNoteModeName
                        ? LV.AbstractButton.Default
                        : LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        DrawerMenubar.drawerModeRequested(DrawerMenubar.quickNoteModeName);
                        DrawerMenubar.requestViewHook("drawer-mode-quick-note");
                    }
                }
                LV.IconButton {
                    id: ItemBox

                    readonly property string figmaNodeId: "I155:4566;206:4190"
                    objectName: "ItemBox"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "swiftPackage"
                    tone: DrawerMenubar.activeDrawerMode === DrawerMenubar.itemBoxModeName
                        ? LV.AbstractButton.Default
                        : LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        DrawerMenubar.drawerModeRequested(DrawerMenubar.itemBoxModeName);
                        DrawerMenubar.requestViewHook("drawer-mode-item-box");
                    }
                }
                LV.IconButton {
                    id: DataSearch

                    readonly property string figmaNodeId: "I155:4566;206:4194"
                    objectName: "DataSearch"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "shortcutFilter"
                    tone: DrawerMenubar.activeDrawerMode === DrawerMenubar.dataSearchModeName
                        ? LV.AbstractButton.Default
                        : LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        DrawerMenubar.drawerModeRequested(DrawerMenubar.dataSearchModeName);
                        DrawerMenubar.requestViewHook("drawer-mode-data-search");
                    }
                }
                LV.IconButton {
                    id: GraphView

                    readonly property string figmaNodeId: "I155:4566;206:4231"
                    objectName: "GraphView"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "graphMachineLearning"
                    tone: DrawerMenubar.activeDrawerMode === DrawerMenubar.graphViewModeName
                        ? LV.AbstractButton.Default
                        : LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        DrawerMenubar.drawerModeRequested(DrawerMenubar.graphViewModeName);
                        DrawerMenubar.requestViewHook("drawer-mode-graph-view");
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
            LV.HStack {
                id: DrawerViewConfig

                readonly property string figmaNodeId: "155:4567"
                objectName: "DrawerViewConfig"
                Layout.alignment: Qt.AlignVCenter
                spacing: LV.Theme.gap2

                LV.IconButton {
                    id: TextAlign

                    readonly property string figmaNodeId: "155:4568"
                    objectName: "TextAlign"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "leftAlign"
                    tone: LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        DrawerMenubar.drawerConfigActionRequested("TextAlign");
                        DrawerMenubar.requestViewHook("drawer-text-align");
                    }
                }
                LV.IconMenuButton {
                    id: ViewOptions

                    readonly property string figmaNodeId: "155:4569"
                    objectName: "ViewOptions"
                    bottomPadding: LV.Theme.gap2
                    iconName: "breakpointsbreakpointFieldUnsuspendentValid"
                    leftPadding: LV.Theme.gap2
                    rightPadding: LV.Theme.gap4
                    tone: LV.AbstractButton.Borderless
                    topPadding: LV.Theme.gap2

                    onClicked: {
                        DrawerMenubar.drawerConfigActionRequested("ViewOptions");
                        DrawerMenubar.requestViewHook("drawer-view-options");
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
                        DrawerMenubar.drawerConfigActionRequested("editorPreviewVertical");
                        DrawerMenubar.requestViewHook("drawer-preview-vertical");
                    }
                }
            }
        }
    }
}
