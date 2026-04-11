pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: agendaLayer

    property var renderedAgendas: []
    property var sourceOffsetYResolver: null
    property var blockFocusHandler: null
    property var taskToggleHandler: null
    property bool enableCardFocus: true
    property bool enableTaskToggle: true
    property bool showFrame: true
    property bool showHeader: true
    property bool showTaskCheckbox: true
    property bool showTaskText: true
    readonly property int cardSpacing: 8
    readonly property color frameBorderColor: "#343536"
    readonly property int frameBorderWidth: 1
    readonly property color frameColor: "#262728"
    readonly property int framePadding: 8
    readonly property int frameRadius: 12
    readonly property color headerTextColor: "#80FFFFFF"
    readonly property color taskBoxColor: "#CCFFFFFF"
    readonly property int taskBoxSize: 17
    readonly property real taskBoxRadius: 3.5
    readonly property int taskListInset: 8
    readonly property int rowGap: 2
    readonly property int taskSpacing: 6
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
    readonly property var agendaEntries: agendaLayer.normalizedList(renderedAgendas)
    readonly property int agendaCount: agendaLayer.agendaEntries.length

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
    height: implicitHeight

    Repeater {
        id: agendaRepeater

        model: agendaLayer.agendaEntries

        delegate: Rectangle {
            id: agendaCard

            readonly property var agendaEntry: modelData && typeof modelData === "object" ? modelData : ({})
            readonly property var agendaTasks: agendaLayer.normalizedList(agendaEntry.tasks)
            readonly property string dateText: agendaEntry.date !== undefined ? String(agendaEntry.date) : "yyyy-mm-dd"
            readonly property int focusSourceOffset: Math.max(0, Number(agendaEntry.focusSourceOffset) || 0)

            border.color: agendaLayer.showFrame ? agendaLayer.frameBorderColor : "transparent"
            border.width: agendaLayer.showFrame ? agendaLayer.frameBorderWidth : 0
            color: agendaLayer.showFrame ? agendaLayer.frameColor : "transparent"
            implicitHeight: agendaCardLayout.implicitHeight + agendaLayer.framePadding * 2
            implicitWidth: agendaLayer.width
            radius: agendaLayer.frameRadius
            width: implicitWidth
            height: implicitHeight
            y: agendaLayer.agendaYForEntry(agendaCard.agendaEntry, index)
            z: 1

            LV.VStack {
                id: agendaCardLayout

                anchors.fill: parent
                anchors.margins: agendaLayer.framePadding
                spacing: agendaLayer.cardSpacing

                RowLayout {
                    Layout.fillWidth: true

                    LV.Label {
                        Layout.fillWidth: true
                        color: agendaLayer.headerTextColor
                        font.family: "Pretendard"
                        font.pixelSize: 11
                        font.weight: Font.Normal
                        opacity: agendaLayer.showHeader ? 1 : 0
                        style: caption
                        text: "Agenda"
                    }
                    LV.Label {
                        color: agendaLayer.headerTextColor
                        font.family: "Pretendard"
                        font.pixelSize: 11
                        font.weight: Font.Normal
                        opacity: agendaLayer.showHeader ? 1 : 0
                        style: caption
                        text: agendaCard.dateText
                    }
                }
                LV.VStack {
                    Layout.leftMargin: agendaLayer.taskListInset
                    Layout.rightMargin: agendaLayer.taskListInset
                    Layout.fillWidth: true
                    spacing: agendaLayer.rowGap

                    Repeater {
                        model: agendaCard.agendaTasks

                        delegate: RowLayout {
                            readonly property var taskEntry: modelData && typeof modelData === "object" ? modelData : ({})
                            readonly property bool hasSourceTag: taskEntry.hasSourceTag === undefined ? true : !!taskEntry.hasSourceTag
                            Layout.fillWidth: true
                            spacing: agendaLayer.taskSpacing

                            LV.CheckBox {
                                id: taskToggle

                                Layout.alignment: Qt.AlignTop
                                boxSize: agendaLayer.taskBoxSize
                                boxRadius: agendaLayer.taskBoxRadius
                                boxBorderColorCheckedEnabled: agendaLayer.taskBoxColor
                                boxBorderColorCheckedDisabled: agendaLayer.taskBoxColor
                                boxBorderColorUncheckedEnabled: agendaLayer.taskBoxColor
                                boxBorderColorUncheckedDisabled: agendaLayer.taskBoxColor
                                boxBorderWidthCheckedEnabled: 0.5
                                boxBorderWidthCheckedDisabled: 0.5
                                checkColor: agendaCard.color
                                checkMarkColorDisabled: agendaCard.color
                                checkedColor: agendaLayer.taskBoxColor
                                checked: !!taskEntry.done
                                disabledCheckedColor: agendaLayer.taskBoxColor
                                disabledUncheckedColor: agendaLayer.taskBoxColor
                                enabled: agendaLayer.enableTaskToggle
                                         && agendaLayer.showTaskCheckbox
                                         && hasSourceTag
                                opacity: agendaLayer.showTaskCheckbox ? 1 : 0
                                text: ""
                                uncheckedColor: agendaLayer.taskBoxColor

                                onToggled: {
                                    if (!hasSourceTag)
                                        return;
                                    if (agendaLayer.taskToggleHandler !== undefined
                                            && agendaLayer.taskToggleHandler !== null) {
                                        agendaLayer.taskToggleHandler(
                                                    Number(taskEntry.openTagStart) || 0,
                                                    Number(taskEntry.openTagEnd) || 0,
                                                    checked);
                                    }
                                }
                            }

                            LV.Label {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignTop
                                color: "#FFFFFF"
                                font.family: "Pretendard"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                opacity: agendaLayer.showTaskText ? 1 : 0
                                style: body
                                text: taskEntry.text !== undefined ? String(taskEntry.text) : ""
                                textFormat: Text.PlainText
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton

                onTapped: {
                    if (agendaLayer.enableCardFocus
                            && agendaLayer.blockFocusHandler !== undefined
                            && agendaLayer.blockFocusHandler !== null) {
                        agendaLayer.blockFocusHandler(agendaCard.focusSourceOffset);
                    }
                }
            }
        }
    }
}
