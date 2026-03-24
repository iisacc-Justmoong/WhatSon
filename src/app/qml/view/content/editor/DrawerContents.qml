pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: DrawerContents

    readonly property string figmaNodeId: "174:6352"
    readonly property string quickNoteModeName: "QuickNote"
    property string activeDrawerMode: quickNoteModeName
    property alias quickNoteText: quickNoteEditor.text
    property string quickNotePlaceholderText: "Quick note"

    signal quickNoteSubmitted(string text)
    signal quickNoteTextEdited(string text)
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        viewHookRequested(hookReason);
    }

    objectName: "DrawerContents"
    clip: true

    Item {
        anchors.fill: parent
        anchors.leftMargin: LV.Theme.gap16
        anchors.rightMargin: LV.Theme.gap16
        anchors.topMargin: LV.Theme.gap8
        anchors.bottomMargin: LV.Theme.gap8

        Item {
            id: QuickNotePage

            readonly property string figmaNodeId: "174:6350"
            objectName: "QuickNotePage"
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            visible: DrawerContents.activeDrawerMode === DrawerContents.quickNoteModeName
            width: Math.max(0, Math.min(511, parent.width))

            LV.TextEditor {
                id: quickNoteEditor

                anchors.fill: parent
                activeFocusOnPress: true
                autoFocusOnPress: true
                backgroundColor: "transparent"
                backgroundColorDisabled: "transparent"
                backgroundColorFocused: "transparent"
                backgroundColorHover: "transparent"
                backgroundColorPressed: "transparent"
                fieldMinHeight: QuickNotePage.height
                fontWeight: Font.Medium
                insetHorizontal: 0
                insetVertical: 0
                outputBackgroundColor: "transparent"
                outputMinHeight: 0
                outputSpacing: 0
                placeholderText: DrawerContents.quickNotePlaceholderText
                readOnly: false
                showRenderedOutput: false
                showScrollBar: false
                textColor: LV.Theme.bodyColor
                textLineHeight: LV.Theme.textBodyLineHeight

                onSubmitted: function (text) {
                    DrawerContents.quickNoteSubmitted(text);
                    DrawerContents.requestViewHook("drawer-quick-note-submitted");
                }
                onTextEdited: function (text) {
                    DrawerContents.quickNoteTextEdited(text);
                }
            }
            Binding {
                property: "topPadding"
                target: quickNoteEditor.editorItem
                value: 0
            }
            Binding {
                property: "bottomPadding"
                target: quickNoteEditor.editorItem
                value: 0
            }
        }
    }
}
