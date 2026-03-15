import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: noteListItem

    readonly property bool activeState: pressed
    property color bookmarkColor: LV.Theme.accentYellow
    property bool bookmarked: false
    readonly property var calendarStore: typeof systemCalendarStore !== "undefined" ? systemCalendarStore : null
    readonly property color captionColor: LV.Theme.captionColor
    readonly property color cardColor: LV.Theme.accentBlueMuted
    property string displayDate: ""
    readonly property string displayDatePlaceholder: calendarStore && calendarStore.shortDatePlaceholderText !== undefined ? String(calendarStore.shortDatePlaceholderText) : qsTr("Date")
    readonly property url folderIconSource: LV.Theme.iconPath("folder@14x14")
    readonly property color folderLabelColor: LV.Theme.captionColor
    property var folders: []
    readonly property int horizontalPadding: 12
    readonly property color hoverCardColor: LV.Theme.panelBackground06
    readonly property bool hovered: noteHoverHandler.hovered
    property bool image: false
    readonly property color imageBoxPlaceholderColor: "#D9D9D9"
    readonly property int imagePreviewSize: 24
    property url imageSource: ""
    readonly property int metadataIconFrameSize: 16
    readonly property int metadataIconSize: 14
    readonly property int metadataTextLineHeight: 11
    readonly property int metadataTextSize: 11
    property string noteId: ""
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("NoteListItem") : null
    property bool pressed: false
    property string primaryText: ""
    readonly property color primaryTextColor: LV.Theme.captionColor
    readonly property int primaryTextLineHeight: 12
    readonly property int primaryTextSize: 12
    readonly property string resolvedDisplayDate: {
        const value = noteListItem.displayDate === undefined || noteListItem.displayDate === null ? "" : String(noteListItem.displayDate).trim();
        return value.length > 0 ? value : noteListItem.displayDatePlaceholder;
    }
    readonly property url tagIconSource: LV.Theme.iconPath("vcscurrentBranch")
    readonly property color tagLabelColor: LV.Theme.captionColor
    property var tags: []
    readonly property int verticalPadding: 8
    readonly property var visibleFolders: metadataPreview(noteListItem.folders)
    readonly property var visibleTags: metadataPreview(noteListItem.tags)

    signal viewHookRequested

    function metadataPreview(values) {
        if (values === undefined || values === null)
            return [];

        var sourceValues = values;
        if (typeof sourceValues === "string")
            sourceValues = sourceValues.split(",");
        else if (!Array.isArray(sourceValues) && sourceValues.length !== undefined)
            sourceValues = Array.prototype.slice.call(sourceValues);

        var items = [];
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

    clip: true
    implicitHeight: 102
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
        anchors.bottomMargin: noteListItem.verticalPadding
        anchors.fill: parent
        anchors.leftMargin: noteListItem.horizontalPadding
        anchors.rightMargin: noteListItem.horizontalPadding
        anchors.topMargin: noteListItem.verticalPadding

        Column {
            anchors.fill: parent
            spacing: 8

            RowLayout {
                spacing: 10
                width: parent.width

                Rectangle {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredHeight: noteListItem.image ? noteListItem.imagePreviewSize : 0
                    Layout.preferredWidth: noteListItem.image ? noteListItem.imagePreviewSize : 0
                    clip: true
                    color: noteListItem.imageBoxPlaceholderColor
                    visible: noteListItem.image

                    Image {
                        anchors.fill: parent
                        asynchronous: true
                        fillMode: Image.PreserveAspectCrop
                        source: noteListItem.imageSource
                        sourceSize.height: noteListItem.imagePreviewSize
                        sourceSize.width: noteListItem.imagePreviewSize
                        visible: noteListItem.imageSource.toString().length > 0
                    }
                }
                LV.Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24
                    clip: true
                    color: noteListItem.primaryTextColor
                    elide: Text.ElideRight
                    font.pixelSize: noteListItem.primaryTextSize
                    font.weight: Font.DemiBold
                    lineHeight: noteListItem.primaryTextLineHeight
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
                height: 12
                width: parent.width

                LV.Label {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    color: noteListItem.captionColor
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    lineHeight: 12
                    lineHeightMode: Text.FixedHeight
                    style: body
                    text: noteListItem.resolvedDisplayDate
                }
            }
            Column {
                spacing: 2
                visible: foldersRow.visible || tagsRow.visible
                width: parent.width

                Item {
                    id: foldersRow

                    clip: true
                    height: visible ? noteListItem.metadataIconFrameSize : 0
                    visible: noteListItem.visibleFolders.length > 0
                    width: parent.width

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        Repeater {
                            model: noteListItem.visibleFolders

                            delegate: Row {
                                spacing: 8

                                Item {
                                    height: noteListItem.metadataIconFrameSize
                                    width: noteListItem.metadataIconFrameSize

                                    Image {
                                        anchors.centerIn: parent
                                        fillMode: Image.PreserveAspectFit
                                        height: noteListItem.metadataIconSize
                                        source: noteListItem.folderIconSource
                                        sourceSize.height: noteListItem.metadataIconSize
                                        sourceSize.width: noteListItem.metadataIconSize
                                        width: noteListItem.metadataIconSize
                                    }
                                }
                                LV.Label {
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: noteListItem.folderLabelColor
                                    font.pixelSize: noteListItem.metadataTextSize
                                    font.weight: Font.Normal
                                    lineHeight: noteListItem.metadataTextLineHeight
                                    lineHeightMode: Text.FixedHeight
                                    style: caption
                                    text: modelData === undefined || modelData === null ? "" : String(modelData)
                                }
                            }
                        }
                    }
                }
                Item {
                    id: tagsRow

                    clip: true
                    height: visible ? noteListItem.metadataIconFrameSize : 0
                    visible: noteListItem.visibleTags.length > 0
                    width: parent.width

                    Row {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        Repeater {
                            model: noteListItem.visibleTags

                            delegate: Row {
                                spacing: 8

                                Item {
                                    height: noteListItem.metadataIconFrameSize
                                    width: noteListItem.metadataIconFrameSize

                                    Image {
                                        anchors.centerIn: parent
                                        fillMode: Image.PreserveAspectFit
                                        height: noteListItem.metadataIconFrameSize
                                        source: noteListItem.tagIconSource
                                        sourceSize.height: noteListItem.metadataIconFrameSize
                                        sourceSize.width: noteListItem.metadataIconFrameSize
                                        width: noteListItem.metadataIconFrameSize
                                    }
                                }
                                LV.Label {
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: noteListItem.tagLabelColor
                                    font.pixelSize: noteListItem.metadataTextSize
                                    font.weight: Font.Normal
                                    lineHeight: noteListItem.metadataTextLineHeight
                                    lineHeightMode: Text.FixedHeight
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
