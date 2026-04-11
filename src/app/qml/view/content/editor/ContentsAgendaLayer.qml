pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: agendaLayer

    property string sourceText: ""
    property var agendaBackend: internalAgendaBackend
    property var sourceOffsetYResolver: null
    property var taskToggleHandler: null
    readonly property color frameBorderColor: "#343536"
    readonly property int frameBorderWidth: 1
    readonly property color frameColor: "#262728"
    readonly property int framePadding: 8
    readonly property int frameRadius: 12
    readonly property color headerTextColor: "#80FFFFFF"
    readonly property int rowGap: 2
    function normalizedList(value) {
        if (value === undefined || value === null)
            return [];
        if (Array.isArray(value))
            return value;
        const length = Number(value.length);
        if (isFinite(length) && length >= 0) {
            const normalized = [];
            for (let index = 0; index < Math.floor(length); ++index)
                normalized.push(value[index]);
            return normalized;
        }
        return [];
    }
    readonly property var parsedAgendas: {
        if (!agendaLayer.agendaBackend || agendaLayer.agendaBackend.parseAgendas === undefined)
            return [];
        const parsed = agendaLayer.agendaBackend.parseAgendas(sourceText);
        return agendaLayer.normalizedList(parsed);
    }
    readonly property int agendaCount: agendaLayer.parsedAgendas.length

    ContentsAgendaBackend {
        id: internalAgendaBackend
    }

    function fallbackAgendaY(index) {
        return Math.max(0, index) * Math.max(1, LV.Theme.gap20);
    }
    function agendaYForEntry(agendaEntry, index) {
        if (!agendaLayer.sourceOffsetYResolver || typeof agendaLayer.sourceOffsetYResolver !== "function")
            return agendaLayer.fallbackAgendaY(index);
        const safeEntry = agendaEntry && typeof agendaEntry === "object" ? agendaEntry : ({});
        const sourceStart = Number(safeEntry.sourceStart);
        const resolvedY = Number(agendaLayer.sourceOffsetYResolver(isFinite(sourceStart) ? sourceStart : 0));
        if (!isFinite(resolvedY))
            return agendaLayer.fallbackAgendaY(index);
        return Math.max(0, resolvedY);
    }
    readonly property real contentBottomY: {
        let maxBottom = 0;
        if (!agendaRepeater)
            return maxBottom;
        for (let index = 0; index < agendaRepeater.count; ++index) {
            const item = agendaRepeater.itemAt(index);
            if (!item)
                continue;
            const itemY = Number(item.y) || 0;
            const itemHeight = Math.max(0, Number(item.implicitHeight) || Number(item.height) || 0);
            maxBottom = Math.max(maxBottom, itemY + itemHeight);
        }
        return maxBottom;
    }

    implicitHeight: contentBottomY

    Repeater {
        id: agendaRepeater

        model: agendaLayer.parsedAgendas

        delegate: Rectangle {
            id: agendaCard

            readonly property var agendaEntry: modelData && typeof modelData === "object" ? modelData : ({})
            readonly property var agendaTasks: agendaLayer.normalizedList(agendaEntry.tasks)
            readonly property string dateText: agendaEntry.date !== undefined ? String(agendaEntry.date) : "yyyy-mm-dd"

            border.color: agendaLayer.frameBorderColor
            border.width: agendaLayer.frameBorderWidth
            color: agendaLayer.frameColor
            implicitHeight: agendaCardLayout.implicitHeight + agendaLayer.framePadding * 2
            radius: agendaLayer.frameRadius
            width: agendaLayer.width
            y: agendaLayer.agendaYForEntry(agendaCard.agendaEntry, index)
            z: 1

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
