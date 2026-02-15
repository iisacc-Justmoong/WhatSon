import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.AppCard {
    id: root

    property int currentIndex: 0
    property var items: []

    signal activated(int index)

    implicitWidth: 248
    subtitle: "LVRS Runtime"
    title: "WhatSon"

    LV.VStack {
        anchors.fill: parent
        spacing: LV.Theme.gap8

        Repeater {
            model: root.items

            delegate: LV.LabelButton {
                required property int index
                required property var modelData

                text: modelData.title !== undefined ? modelData.title : String(modelData)
                tone: root.currentIndex === index ? LV.AbstractButton.Primary : LV.AbstractButton.Default

                onClicked: root.activated(index)
            }
        }
        LV.Spacer {
        }
        LV.Label {
            style: description
            text: "LVRS 1.0"
            wrapMode: Text.WordWrap
        }
    }
}
