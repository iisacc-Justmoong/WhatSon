pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: resourceListItem

    property bool active: false
    readonly property color activeCardColor: "#25324D"
    readonly property bool hovered: resourceHoverHandler.hovered
    readonly property color hoverCardColor: LV.Theme.panelBackground06
    readonly property int framePadding: LV.Theme.gap8
    property bool pressed: false
    readonly property color pressedCardColor: resourceListItem.hoverCardColor
    property url previewSource: ""
    readonly property int rowSpacing: Math.max(0, Math.round(LV.Theme.scaleMetric(10)))
    readonly property int thumbnailSize: Math.max(0, Math.round(LV.Theme.scaleMetric(48)))
    readonly property color thumbnailPlaceholderColor: "#D9D9D9"
    property string titleText: ""
    readonly property color titleColor: LV.Theme.captionColor
    readonly property int titleLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property int titlePixelSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))

    clip: true
    implicitHeight: resourceListItem.thumbnailSize + resourceListItem.framePadding * 2
    implicitWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(194)))

    Rectangle {
        anchors.fill: parent
        color: resourceListItem.active
            ? resourceListItem.activeCardColor
            : resourceListItem.pressed
                ? resourceListItem.pressedCardColor
                : resourceListItem.hovered
                    ? resourceListItem.hoverCardColor
                    : LV.Theme.accentTransparent
    }
    HoverHandler {
        id: resourceHoverHandler

        enabled: true
    }
    Item {
        anchors.bottomMargin: resourceListItem.framePadding
        anchors.fill: parent
        anchors.leftMargin: resourceListItem.framePadding
        anchors.rightMargin: resourceListItem.framePadding
        anchors.topMargin: resourceListItem.framePadding

        RowLayout {
            anchors.fill: parent
            spacing: resourceListItem.rowSpacing

            Rectangle {
                Layout.alignment: Qt.AlignTop
                Layout.preferredHeight: resourceListItem.thumbnailSize
                Layout.preferredWidth: resourceListItem.thumbnailSize
                clip: true
                color: resourceListItem.thumbnailPlaceholderColor

                Image {
                    anchors.fill: parent
                    asynchronous: true
                    fillMode: Image.PreserveAspectCrop
                    source: resourceListItem.previewSource
                    sourceSize.height: resourceListItem.thumbnailSize
                    sourceSize.width: resourceListItem.thumbnailSize
                    visible: resourceListItem.previewSource.toString().length > 0
                }
            }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true

                LV.Label {
                    anchors.fill: parent
                    color: resourceListItem.titleColor
                    elide: Text.ElideRight
                    font.pixelSize: resourceListItem.titlePixelSize
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignLeft
                    lineHeight: resourceListItem.titleLineHeight
                    lineHeightMode: Text.FixedHeight
                    maximumLineCount: 2
                    style: description
                    text: resourceListItem.titleText
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
