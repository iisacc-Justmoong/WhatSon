pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: agendaLayer

    property string sourceText: ""
    property var agendaBackend: internalAgendaBackend
    property var taskToggleHandler: null
    readonly property color frameBorderColor: "#343536"
    readonly property int frameBorderWidth: 1
    readonly property color frameColor: "#262728"
    readonly property int framePadding: 8
    readonly property int frameRadius: 12
    readonly property color headerTextColor: "#80FFFFFF"
    readonly property int rowGap: 2
    readonly property var parsedAgendas: {
        if (!agendaLayer.agendaBackend || agendaLayer.agendaBackend.parseAgendas === undefined)
            return [];
        const parsed = agendaLayer.agendaBackend.parseAgendas(sourceText);
        return Array.isArray(parsed) ? parsed : [];
    }
    readonly property int agendaCount: Array.isArray(agendaLayer.parsedAgendas) ? agendaLayer.parsedAgendas.length : 0

    ContentsAgendaBackend {
        id: internalAgendaBackend
    }

    implicitHeight: agendaColumn.implicitHeight

    LV.VStack {
        id: agendaColumn

        anchors.left: parent.left
        anchors.right: parent.right
        spacing: agendaLayer.rowGap

        Repeater {
            model: agendaLayer.parsedAgendas

            delegate: Rectangle {
                id: agendaCard

                readonly property var agendaEntry: modelData && typeof modelData === "object" ? modelData : ({})
                readonly property var agendaTasks: Array.isArray(agendaEntry.tasks) ? agendaEntry.tasks : []
                readonly property string dateText: agendaEntry.date !== undefined ? String(agendaEntry.date) : "yyyy-mm-dd"

                border.color: agendaLayer.frameBorderColor
                border.width: agendaLayer.frameBorderWidth
                color: agendaLayer.frameColor
                implicitHeight: agendaCardLayout.implicitHeight + agendaLayer.framePadding * 2
                radius: agendaLayer.frameRadius
                width: parent ? parent.width : 0

                LV.VStack {
                    id: agendaCardLayout

                    anchors.fill: parent
                    anchors.margins: agendaLayer.framePadding
                    spacing: agendaLayer.rowGap

                    RowLayout {
                        Layout.fillWidth: true

                        LV.Label {
                            Layout.fillWidth: true
                            color: agendaLayer.headerTextColor
                            style: caption
                            text: "Agenda"
                        }
                        LV.Label {
                            color: agendaLayer.headerTextColor
                            style: caption
                            text: agendaCard.dateText
                        }
                    }
                    LV.VStack {
                        Layout.leftMargin: agendaLayer.framePadding
                        Layout.rightMargin: agendaLayer.framePadding
                        Layout.fillWidth: true
                        spacing: agendaLayer.rowGap

                        Repeater {
                            model: agendaCard.agendaTasks

                            delegate: LV.CheckBox {
                                readonly property var taskEntry: modelData && typeof modelData === "object" ? modelData : ({})
                                checked: !!taskEntry.done
                                text: taskEntry.text !== undefined ? String(taskEntry.text) : ""

                                onToggled: {
                                    if (agendaLayer.taskToggleHandler !== undefined
                                            && agendaLayer.taskToggleHandler !== null) {
                                        agendaLayer.taskToggleHandler(
                                                    Number(taskEntry.openTagStart) || 0,
                                                    Number(taskEntry.openTagEnd) || 0,
                                                    checked);
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
