import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: noteListItem

    readonly property bool activeState: hovered
    property color bookmarkColor: "#F2C55C"
    property bool bookmarked: true
    readonly property color captionColor: Qt.rgba(1, 1, 1, 0.5)
    readonly property color cardColor: LV.Theme.panelBackground08
    property string desc: "Note Contents Thumbnail... is can has 2 lines..."
    readonly property color folderStrokeColor: "#CED0D6"
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
    property string title: "NoteTitle"
    readonly property color titleColor: LV.Theme.bodyColor

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
                width: parent.width

                Text {
                    Layout.fillWidth: true
                    color: noteListItem.titleColor
                    font.family: "Pretendard"
                    font.pixelSize: 12
                    font.weight: 500
                    lineHeight: 12
                    lineHeightMode: Text.FixedHeight
                    text: noteListItem.title
                    verticalAlignment: Text.AlignVCenter
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
            Text {
                color: noteListItem.captionColor
                font.family: "Pretendard"
                font.pixelSize: 11
                font.weight: 400
                lineHeight: 11
                lineHeightMode: Text.FixedHeight
                maximumLineCount: 2
                text: noteListItem.desc
                width: parent.width
                wrapMode: Text.WordWrap
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
                Text {
                    Layout.fillWidth: true
                    color: noteListItem.captionColor
                    elide: Text.ElideRight
                    font.family: "Pretendard"
                    font.pixelSize: 11
                    font.weight: 400
                    lineHeight: 11
                    lineHeightMode: Text.FixedHeight
                    text: noteListItem.foldersText
                }
            }
        }
    }
}
