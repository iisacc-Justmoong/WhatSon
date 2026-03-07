import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: noteListItem

    readonly property bool activeState: hovered || pressed
    property color bookmarkColor: LV.Theme.accentYellow
    property bool bookmarked: false
    readonly property color captionColor: LV.Theme.captionColor
    readonly property color cardColor: LV.Theme.panelBackground08
    readonly property color folderStrokeColor: LV.Theme.accentGrayLight
    property var folders: ["FolderName1", "FolderName2"]
    readonly property string foldersText: {
        const value = noteListItem.folders;
        if (value === undefined || value === null)
            return "";
        if (typeof value === "string")
            return value;
        if (value.join !== undefined)
            return value.join(", ");
        return "";
    }
    readonly property bool hovered: noteHoverHandler.hovered
    property string noteId: ""
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("NoteListItem") : null
    property bool pressed: false
    property string primaryText: "NotePrimaryText"
    readonly property color primaryTextColor: LV.Theme.bodyColor

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    implicitHeight: 86
    implicitWidth: 194

    Rectangle {
        anchors.fill: parent
        color: noteListItem.activeState ? noteListItem.cardColor : LV.Theme.accentTransparent
        radius: 16
    }
    HoverHandler {
        id: noteHoverHandler

        enabled: true
    }
    Item {
        anchors.bottomMargin: 8
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 8

        Column {
            anchors.fill: parent
            spacing: 8

            RowLayout {
                spacing: 8
                width: parent.width

                LV.Label {
                    Layout.fillWidth: true
                    clip: true
                    color: noteListItem.primaryTextColor
                    elide: Text.ElideRight
                    style: body
                    maximumLineCount: 3
                    text: noteListItem.primaryText
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.WordWrap
                }
                Item {
                    Layout.preferredHeight: 16
                    Layout.preferredWidth: 16
                    visible: noteListItem.bookmarked

                    Canvas {
                        anchors.fill: parent
                        antialiasing: true

                        onPaint: {
                            const ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            ctx.beginPath();
                            ctx.moveTo(4, 1.5);
                            ctx.lineTo(12, 1.5);
                            ctx.lineTo(12, 14);
                            ctx.lineTo(8, 10.5);
                            ctx.lineTo(4, 14);
                            ctx.closePath();
                            ctx.fillStyle = noteListItem.bookmarkColor;
                            ctx.fill();
                        }
                    }
                }
            }
            RowLayout {
                width: parent.width

                Item {
                    Layout.preferredHeight: 14
                    Layout.preferredWidth: 14

                    Canvas {
                        anchors.fill: parent
                        antialiasing: true

                        onPaint: {
                            const ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            ctx.beginPath();
                            ctx.moveTo(1, 4);
                            ctx.lineTo(5.5, 4);
                            ctx.lineTo(6.7, 2.3);
                            ctx.lineTo(12.8, 2.3);
                            ctx.lineTo(12.8, 11.8);
                            ctx.lineTo(1, 11.8);
                            ctx.closePath();
                            ctx.lineWidth = 1.2;
                            ctx.strokeStyle = noteListItem.folderStrokeColor;
                            ctx.stroke();
                        }
                    }
                }
                LV.Label {
                    Layout.fillWidth: true
                    color: noteListItem.captionColor
                    elide: Text.ElideRight
                    style: caption
                    text: noteListItem.foldersText
                }
            }
        }
    }
}
