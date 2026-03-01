import QtQuick
import QtQuick.Window
import LVRS 1.0 as LV

Window {
    id: root

    color: LV.Theme.panelBackground03
    height: LV.Theme.gap20 * 14
    minimumHeight: LV.Theme.gap20 * 8
    minimumWidth: LV.Theme.gap20 * 12
    title: "WhatSon Preference"
    visible: false
    width: LV.Theme.gap20 * 24

    Rectangle {
        anchors.fill: parent
        color: root.color
    }
    LV.Label {
        anchors.centerIn: parent
        text: "Preference"
    }
}
