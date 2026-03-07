import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: noteListItem

    readonly property bool activeState: pressed
    property color bookmarkColor: LV.Theme.accentYellow
    property bool bookmarked: false
    readonly property color captionColor: LV.Theme.captionColor
    readonly property color cardColor: LV.Theme.accentBlueMuted
    property string displayDate: ""
    readonly property url folderIconSource: LV.Theme.iconPath("folder@14x14")
    readonly property color folderStrokeColor: LV.Theme.accentGrayLight
    property var folders: []
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
    readonly property color hoverCardColor: LV.Theme.panelBackground06
    readonly property bool hovered: noteHoverHandler.hovered
    property string noteId: ""
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("NoteListItem") : null
    property bool pressed: false
    property string primaryText: ""
    readonly property color primaryTextColor: LV.Theme.captionColor
    readonly property url tagIconSource: LV.Theme.iconPath("vcscurrentBranch")
    property var tags: []
    readonly property var visibleFolders: noteListItem.metadataPreview(noteListItem.folders)
    readonly property var visibleTags: noteListItem.metadataPreview(noteListItem.tags)

    signal viewHookRequested

    function metadataPreview(values) {
        if (values === undefined || values === null)
            return [];

        var items = [];
        var sourceValues = values;
        if (typeof sourceValues === "string")
            sourceValues = sourceValues.split(",");
        else if (!Array.isArray(sourceValues) && sourceValues.length !== undefined)
            sourceValues = Array.prototype.slice.call(sourceValues);

        for (var i = 0; i < sourceValues.length; ++i) {
            var entry = String(sourceValues[i]).trim();
            if (!entry.length || items.indexOf(entry) >= 0)
                continue;
            items.push(entry);
        }

        return items.slice(0, 2);
    }
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
        color: noteListItem.activeState ? noteListItem.cardColor : noteListItem.hovered ? noteListItem.hoverCardColor : LV.Theme.accentTransparent
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
            id: contentColumn

            anchors.fill: parent
            spacing: 8

            RowLayout {
                spacing: 10
                width: parent.width

                LV.Label {
                    Layout.fillWidth: true
                    clip: true
                    color: noteListItem.primaryTextColor
                    elide: Text.ElideRight
                    lineHeightMode: Text.FixedHeight
                    maximumLineCount: 2
                    style: description
                    text: noteListItem.primaryText
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.WordWrap
                }
                Item {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredHeight: noteListItem.bookmarked ? 16 : 0
                    Layout.preferredWidth: noteListItem.bookmarked ? 16 : 0
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
            Item {
                height: visible ? 12 : 0
                visible: noteListItem.displayDate.length > 0
                width: parent.width

                LV.Label {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    color: noteListItem.captionColor
                    style: caption
                    text: noteListItem.displayDate
                }
            }
            Column {
                spacing: 2
                visible: foldersRow.visible || tagsRow.visible
                width: parent.width

                Item {
                    id: foldersRow

                    height: visible ? 16 : 0
                    visible: noteListItem.visibleFolders.length > 0
                    width: parent.width

                    Item {
                        anchors.fill: parent
                        clip: true

                        Row {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            Repeater {
                                model: noteListItem.visibleFolders

                                delegate: Row {
                                    spacing: 8

                                    Item {
                                        height: 16
                                        width: 16

                                        Image {
                                            anchors.centerIn: parent
                                            fillMode: Image.PreserveAspectFit
                                            height: 14
                                            smooth: true
                                            noteListItem.folderIconSource
                                            sourceSize.height: 14
                                            sourceSize.width: 14
                                            width: 14
                                        }
                                    }
                                    Item {
                                        height: 16
                                        width: folderLabel.implicitWidth

                                        LV.Label {
                                            id: folderLabel

                                            anchors.verticalCenter: parent.verticalCenter
                                            color: noteListItem.captionColor
                                            style: caption
                                            text: modelData === undefined || modelData === null ? "" : String(modelData)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                Item {
                    id: tagsRow

                    height: visible ? 16 : 0
                    visible: noteListItem.visibleTags.length > 0
                    width: parent.width

                    Item {
                        anchors.fill: parent
                        clip: true

                        Row {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            Repeater {
                                model: noteListItem.visibleTags

                                delegate: Row {
                                    spacing: 8

                                    Item {
                                        height: 16
                                        width: 16

                                        Image {
                                            anchors.centerIn: parent
                                            fillMode: Image.PreserveAspectFit
                                            height: 16
                                            smooth: true
                                            noteListItem.tagIconSource
                                            sourceSize.height: 16
                                            sourceSize.width: 16
                                            width: 16
                                        }
                                    }
                                    Item {
                                        height: 16
                                        width: tagLabel.implicitWidth

                                        LV.Label {
                                            id: tagLabel

                                            anchors.verticalCenter: parent.verticalCenter
                                            color: noteListItem.captionColor
                                            style: caption
                                            text: modelData === undefined || modelData === null ? "" : String(modelData)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
