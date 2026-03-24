pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: drawerToolbar

    readonly property string figmaNodeId: "155:4570"

    signal newDraftRequested
    signal showQuickNoteWindowRequested
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        viewHookRequested(hookReason);
    }

    objectName: "DrawerToolbar"
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

            Item {
                Layout.fillWidth: true
            }
            LV.HStack {
                id: sumit

                readonly property string figmaNodeId: "155:4571"
                objectName: "Sumit"
                Layout.alignment: Qt.AlignVCenter
                spacing: LV.Theme.gap2

                LV.IconButton {
                    id: showQuickNoteWindow

                    readonly property string figmaNodeId: "155:4572"
                    objectName: "ShowQuickNoteWindow"
                    horizontalPadding: LV.Theme.gap2
                    iconName: "jpaConsoleToolWindow"
                    tone: LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        DrawerToolbar.showQuickNoteWindowRequested();
                        DrawerToolbar.requestViewHook("drawer-show-quick-note-window");
                    }
                }
                LV.LabelButton {
                    id: newDraft

                    readonly property string figmaNodeId: "155:4573"
                    objectName: "NewDraft"
                    horizontalPadding: LV.Theme.gap8
                    text: "New Draft"
                    tone: LV.AbstractButton.Default
                    verticalPadding: LV.Theme.gap4

                    onClicked: {
                        DrawerToolbar.newDraftRequested();
                        DrawerToolbar.requestViewHook("drawer-new-draft");
                    }
                }
            }
        }
    }
}
