import QtQuick
import QtQuick.Window
import LVRS 1.0 as LV

Window {
    id: root

    property int defaultHeight: 480
    property int defaultWidth: 720
    property Window hostWindow: null
    property int minHeight: 360
    property int minWidth: 520
    property color panelColor: LV.Theme.panelBackground10

    signal createFileRequested
    signal selectFileRequested
    signal viewHookRequested

    function recenter() {
        if (!hostWindow)
            return;
        x = hostWindow.x + Math.round((hostWindow.width - width) / 2);
        y = hostWindow.y + Math.round((hostWindow.height - height) / 2);
    }

    color: panelColor
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
    height: defaultHeight
    minimumHeight: minHeight
    minimumWidth: minWidth
    modality: Qt.ApplicationModal
    title: "WhatSon Onboarding"
    transientParent: hostWindow
    visible: false
    width: defaultWidth

    onHeightChanged: {
        if (visible)
            recenter();
    }
    onHostWindowChanged: recenter()
    onVisibleChanged: {
        if (visible)
            recenter();
    }
    onWidthChanged: {
        if (visible)
            recenter();
    }

    Rectangle {
        anchors.fill: parent
        color: root.panelColor
    }
    LV.VStack {
        anchors.centerIn: parent
        spacing: LV.Theme.gap10

        LV.Label {
            style: title
            text: "Onboarding"
        }
        LV.Label {
            style: description
            text: "Create or select a hub."
        }
        LV.HStack {
            spacing: LV.Theme.gap8

            LV.LabelButton {
                text: "Create"
                tone: LV.AbstractButton.Primary

                onClicked: root.createFileRequested()
            }
            LV.LabelButton {
                text: "Select"
                tone: LV.AbstractButton.Default

                onClicked: root.selectFileRequested()
            }
            LV.LabelButton {
                text: "Hook"
                tone: LV.AbstractButton.Default

                onClicked: root.viewHookRequested()
            }
        }
    }
}
