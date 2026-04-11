pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: calloutLayer

    property string sourceText: ""
    property var calloutBackend: internalCalloutBackend
    readonly property color calloutColor: "#262728"
    readonly property color dividerColor: "#D9D9D9"
    readonly property color textColor: "#FFFFFF"
    readonly property int dividerMinimumHeight: 14
    readonly property int dividerWidth: 1
    readonly property int framePadding: 4
    readonly property int frameSpacing: 12
    readonly property var parsedCallouts: {
        if (!calloutLayer.calloutBackend || calloutLayer.calloutBackend.parseCallouts === undefined)
            return [];
        const parsed = calloutLayer.calloutBackend.parseCallouts(sourceText);
        return Array.isArray(parsed) ? parsed : [];
    }
    readonly property int calloutCount: Array.isArray(calloutLayer.parsedCallouts) ? calloutLayer.parsedCallouts.length : 0

    ContentsCalloutBackend {
        id: internalCalloutBackend
    }

    implicitHeight: calloutColumn.implicitHeight

    LV.VStack {
        id: calloutColumn

        anchors.left: parent.left
        anchors.right: parent.right
        spacing: LV.Theme.gap2

        Repeater {
            model: calloutLayer.parsedCallouts

            delegate: Rectangle {
                id: calloutFrame

                readonly property var calloutEntry: modelData && typeof modelData === "object" ? modelData : ({})
                readonly property string calloutText: calloutEntry.text !== undefined ? String(calloutEntry.text) : ""

                color: calloutLayer.calloutColor
                implicitHeight: Math.max(calloutLayer.dividerMinimumHeight, calloutTextLabel.implicitHeight)
                                + calloutLayer.framePadding * 2
                width: parent ? parent.width : 0

                Rectangle {
                    id: calloutDivider

                    anchors.left: parent.left
                    anchors.leftMargin: calloutLayer.framePadding
                    anchors.top: parent.top
                    anchors.topMargin: calloutLayer.framePadding
                    color: calloutLayer.dividerColor
                    height: Math.max(calloutLayer.dividerMinimumHeight, calloutTextLabel.implicitHeight)
                    radius: 1
                    width: calloutLayer.dividerWidth
                }
                LV.Label {
                    id: calloutTextLabel

                    anchors.left: calloutDivider.right
                    anchors.leftMargin: calloutLayer.frameSpacing
                    anchors.right: parent.right
                    anchors.rightMargin: calloutLayer.framePadding
                    anchors.top: parent.top
                    anchors.topMargin: calloutLayer.framePadding
                    color: calloutLayer.textColor
                    font.family: "Pretendard"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    lineHeight: 1.0
                    style: body
                    text: calloutFrame.calloutText
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}
