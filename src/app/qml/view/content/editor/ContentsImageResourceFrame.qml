pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: imageFrame

    property string resourceTitle: "Image"
    property string fileNameText: "filename"
    property color borderColor: LV.Theme.panelBackground08
    property color dotColor: LV.Theme.captionColor
    property color labelColor: LV.Theme.captionColor
    property color panelColor: "transparent"
    property int frameCornerRadius: 12
    property bool menuButtonVisible: true
    property bool menuButtonEnabled: true
    property real mediaWidthHint: 338
    property real mediaHeightHint: 352
    property real maxMediaHeight: 352
    readonly property real mediaAspectRatio: {
        const normalizedWidth = Number(imageFrame.mediaWidthHint) || 0
        const normalizedHeight = Number(imageFrame.mediaHeightHint) || 0
        if (normalizedWidth <= 0 || normalizedHeight <= 0)
            return 1
        return normalizedWidth / normalizedHeight
    }
    readonly property real designFrameWidth: 480
    readonly property real designBarHeight: 19
    readonly property real horizontalPadding: 8
    readonly property real parentFrameWidth: parent
                                           ? (Number(parent.width) || Number(parent.implicitWidth) || 0)
                                           : 0
    readonly property real resolvedFrameWidth: Math.max(
                                                   120,
                                                   imageFrame.parentFrameWidth > 0
                                                   ? imageFrame.parentFrameWidth
                                                   : imageFrame.designFrameWidth)
    readonly property real availableMediaWidth: Math.max(
                                                    120,
                                                    imageFrame.resolvedFrameWidth - imageFrame.horizontalPadding * 2)
    readonly property real availableMediaHeight: Math.max(120, Number(imageFrame.maxMediaHeight) || 352)
    readonly property real naturalMediaWidth: Math.max(1, Number(imageFrame.mediaWidthHint) || 338)
    readonly property real naturalMediaHeight: Math.max(1, Number(imageFrame.mediaHeightHint) || 352)
    readonly property real resolvedMediaScale: {
        const widthScale = imageFrame.availableMediaWidth / imageFrame.naturalMediaWidth
        const heightScale = imageFrame.availableMediaHeight / imageFrame.naturalMediaHeight
        const fittedScale = Math.min(widthScale, heightScale)
        if (!isFinite(fittedScale) || fittedScale <= 0)
            return 1
        return Math.min(1, fittedScale)
    }
    readonly property real resolvedMediaWidth: Math.max(
                                                   1,
                                                   Math.round(imageFrame.naturalMediaWidth * imageFrame.resolvedMediaScale))
    readonly property real resolvedMediaHeight: Math.max(
                                                    1,
                                                    Math.round(imageFrame.naturalMediaHeight * imageFrame.resolvedMediaScale))
    default property alias contentData: mediaViewport.data

    signal menuRequested()

    border.color: imageFrame.borderColor
    border.width: Math.max(1, Math.round(LV.Theme.strokeThin))
    color: imageFrame.panelColor
    implicitWidth: imageFrame.designFrameWidth
    implicitHeight: imageFrame.designBarHeight * 2 + imageFrame.resolvedMediaHeight
    height: implicitHeight
    radius: imageFrame.frameCornerRadius
    width: imageFrame.resolvedFrameWidth

    Item {
        id: headerBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: imageFrame.designBarHeight

        LV.Label {
            anchors.left: parent.left
            anchors.leftMargin: imageFrame.horizontalPadding
            anchors.right: menuButton.visible ? menuButton.left : parent.right
            anchors.rightMargin: imageFrame.horizontalPadding
            anchors.verticalCenter: parent.verticalCenter
            color: imageFrame.labelColor
            elide: Text.ElideRight
            style: caption
            text: imageFrame.resourceTitle
        }

        Item {
            id: menuButton

            anchors.right: parent.right
            anchors.rightMargin: imageFrame.horizontalPadding
            anchors.verticalCenter: parent.verticalCenter
            height: 16
            visible: imageFrame.menuButtonVisible
            width: 16

            MouseArea {
                anchors.fill: parent
                cursorShape: imageFrame.menuButtonEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                enabled: imageFrame.menuButtonEnabled

                onClicked: imageFrame.menuRequested()
            }

            Row {
                anchors.centerIn: parent
                spacing: 3

                Repeater {
                    model: 3

                    Rectangle {
                        width: 2
                        height: 2
                        radius: 1
                        color: imageFrame.dotColor
                    }
                }
            }
        }
    }

    Item {
        id: mediaSlot

        anchors.left: parent.left
        anchors.leftMargin: imageFrame.horizontalPadding
        anchors.right: parent.right
        anchors.rightMargin: imageFrame.horizontalPadding
        anchors.top: headerBar.bottom
        height: imageFrame.resolvedMediaHeight

        Item {
            id: mediaViewport

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            clip: true
            height: imageFrame.resolvedMediaHeight
            width: imageFrame.resolvedMediaWidth
        }
    }

    Item {
        id: footerBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: mediaSlot.bottom
        anchors.bottom: parent.bottom

        LV.Label {
            anchors.left: parent.left
            anchors.leftMargin: imageFrame.horizontalPadding
            anchors.right: parent.right
            anchors.rightMargin: imageFrame.horizontalPadding
            anchors.verticalCenter: parent.verticalCenter
            color: imageFrame.labelColor
            elide: Text.ElideMiddle
            style: caption
            text: imageFrame.fileNameText
        }
    }
}
