pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: noteListItem

    property bool active: false
    readonly property bool activeState: noteListItem.active
    property color bookmarkColor: LV.Theme.accentYellow
    property bool bookmarked: false
    property var systemCalendarStore: null
    readonly property var calendarStore: noteListItem.systemCalendarStore
    readonly property color captionColor: LV.Theme.captionColor
    readonly property color cardColor: LV.Theme.accentBlueMuted
    property string displayDate: ""
    readonly property string displayDatePlaceholder: calendarStore && calendarStore.shortDatePlaceholderText !== undefined ? String(calendarStore.shortDatePlaceholderText) : qsTr("Date")
    readonly property url folderIconSource: LV.Theme.iconPath("folder@14x14")
    readonly property color folderLabelColor: LV.Theme.captionColor
    property var folders: []
    readonly property int horizontalPadding: LV.Theme.gap12
    readonly property color hoverCardColor: LV.Theme.panelBackground08
    readonly property color pressedCardColor: noteListItem.hoverCardColor
    readonly property bool hovered: noteHoverHandler.hovered
    property bool image: false
    readonly property color imageBoxPlaceholderColor: "#D9D9D9"
    readonly property int imagePreviewSize: Math.max(0, Math.round(LV.Theme.scaleMetric(48)))
    property url imageSource: ""
    readonly property int metadataGroupSpacing: LV.Theme.gap2
    readonly property int metadataIconFrameSize: Math.max(0, Math.round(LV.Theme.scaleMetric(16)))
    readonly property int metadataIconSpacing: LV.Theme.gap8
    readonly property int metadataIconSize: Math.max(0, Math.round(LV.Theme.scaleMetric(14)))
    readonly property int metadataTextLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(11)))
    readonly property int metadataTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(11)))
    property string noteId: ""
    property var panelViewModelRegistry: null
    readonly property var panelViewModel: noteListItem.panelViewModelRegistry ? noteListItem.panelViewModelRegistry.panelViewModel("NoteListItem") : null
    property bool pressed: false
    property string primaryText: ""
    readonly property color primaryTextColor: LV.Theme.captionColor
    readonly property int primaryRowSpacing: Math.max(0, Math.round(LV.Theme.scaleMetric(10)))
    readonly property int primarySectionSpacing: LV.Theme.gap8
    readonly property int primaryTextBlockHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(24)))
    readonly property int primaryTextLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property int primaryTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property string resolvedDisplayDate: {
        const value = noteListItem.displayDate === undefined || noteListItem.displayDate === null ? "" : String(noteListItem.displayDate).trim();
        return value.length > 0 ? value : noteListItem.displayDatePlaceholder;
    }
    readonly property url tagIconSource: LV.Theme.iconPath("vcscurrentBranch")
    readonly property color tagLabelColor: LV.Theme.captionColor
    property var tags: []
    readonly property int secondaryTextLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property int verticalPadding: LV.Theme.gap8
    readonly property var visibleFolders: metadataPreview(noteListItem.folders, true)
    readonly property var visibleTags: metadataPreview(noteListItem.tags, false)

    signal viewHookRequested

    function leafFolderLabel(value) {
        const text = value === undefined || value === null ? "" : String(value).trim();
        if (!text.length)
            return "";

        const normalized = text.replace(/\\/g, "/");
        const segments = normalized.split("/");
        for (var index = segments.length - 1; index >= 0; --index) {
            const segment = String(segments[index]).trim();
            if (segment.length > 0)
                return segment;
        }

        return "";
    }
    function metadataPreview(values, leafOnly) {
        if (values === undefined || values === null)
            return [];

        var sourceValues = values;
        if (typeof sourceValues === "string")
            sourceValues = sourceValues.split(",");
        else if (!Array.isArray(sourceValues) && sourceValues.length !== undefined)
            sourceValues = Array.prototype.slice.call(sourceValues);

        var items = [];
        for (var i = 0; i < sourceValues.length; ++i) {
            var entry = leafOnly ? leafFolderLabel(sourceValues[i]) : String(sourceValues[i]).trim();
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
    implicitHeight: noteListItem.image
                    ? Math.max(0, Math.round(LV.Theme.scaleMetric(126)))
                    : Math.max(0, Math.round(LV.Theme.scaleMetric(102)))
    implicitWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(194)))

    Rectangle {
        anchors.fill: parent
        color: noteListItem.activeState
            ? noteListItem.cardColor
            : noteListItem.pressed
                ? noteListItem.pressedCardColor
                : noteListItem.hovered
                    ? noteListItem.hoverCardColor
                    : LV.Theme.accentTransparent
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
            spacing: noteListItem.primarySectionSpacing

            RowLayout {
                spacing: noteListItem.primaryRowSpacing
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
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    Layout.preferredHeight: noteListItem.primaryTextBlockHeight
                    clip: true
                    color: noteListItem.primaryTextColor
                    elide: Text.ElideRight
                    font.pixelSize: noteListItem.primaryTextSize
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignLeft
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
                    Layout.preferredHeight: noteListItem.bookmarked ? noteListItem.metadataIconFrameSize : 0
                    Layout.preferredWidth: noteListItem.bookmarked ? noteListItem.metadataIconFrameSize : 0
                    visible: noteListItem.bookmarked

                    Canvas {
                        anchors.fill: parent
                        antialiasing: true

                        onPaint: {
                            const ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            const leftX = width * 0.25;
                            const rightX = width * 0.75;
                            const midX = width * 0.5;
                            const topY = height * 0.09375;
                            const notchY = height * 0.65625;
                            const bottomY = height * 0.875;
                            ctx.beginPath();
                            ctx.moveTo(leftX, topY);
                            ctx.lineTo(rightX, topY);
                            ctx.lineTo(rightX, bottomY);
                            ctx.lineTo(midX, notchY);
                            ctx.lineTo(leftX, bottomY);
                            ctx.closePath();
                            ctx.fillStyle = noteListItem.bookmarkColor;
                            ctx.fill();
                        }
                    }
                }
            }
            Item {
                height: noteListItem.secondaryTextLineHeight
                width: parent.width

                LV.Label {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    color: noteListItem.captionColor
                    font.pixelSize: noteListItem.secondaryTextLineHeight
                    font.weight: Font.DemiBold
                    lineHeight: noteListItem.secondaryTextLineHeight
                    lineHeightMode: Text.FixedHeight
                    style: body
                    text: noteListItem.resolvedDisplayDate
                }
            }
            Column {
                spacing: noteListItem.metadataGroupSpacing
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
                        spacing: noteListItem.metadataIconSpacing

                        Repeater {
                            model: noteListItem.visibleFolders

                            delegate: Row {
                                id: folderLabelRow

                                required property var modelData
                                spacing: noteListItem.metadataIconSpacing

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
                                    text: folderLabelRow.modelData === undefined || folderLabelRow.modelData === null
                                          ? ""
                                          : String(folderLabelRow.modelData)
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
                        spacing: noteListItem.metadataIconSpacing

                        Repeater {
                            model: noteListItem.visibleTags

                            delegate: Row {
                                id: tagLabelRow

                                required property var modelData
                                spacing: noteListItem.metadataIconSpacing

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
                                    text: tagLabelRow.modelData === undefined || tagLabelRow.modelData === null
                                          ? ""
                                          : String(tagLabelRow.modelData)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
