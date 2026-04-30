pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: agendaPage

    readonly property var allDayModels: agendaRuntimeController && agendaRuntimeController.allDayEvents ? agendaRuntimeController.allDayEvents : []
    readonly property var locationModel: agendaRuntimeController && agendaRuntimeController.location ? agendaRuntimeController.location : ({})
    readonly property var summaryModel: agendaRuntimeController && agendaRuntimeController.summary ? agendaRuntimeController.summary : ({})
    readonly property var agendaItemModels: agendaRuntimeController && agendaRuntimeController.agendaItems ? agendaRuntimeController.agendaItems : []
    readonly property var timedModels: agendaRuntimeController && agendaRuntimeController.timedEvents ? agendaRuntimeController.timedEvents : []
    readonly property var agendaRuntimeController: agendaController
    property var agendaController: null

    signal noteOpenRequested(string noteId)
    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (agendaRuntimeController && agendaRuntimeController.requestAgendaView)
            agendaRuntimeController.requestAgendaView(hookReason);
        viewHookRequested(hookReason);
    }
    function jumpToToday() {
        if (agendaRuntimeController && agendaRuntimeController.setDisplayedDateIso)
            agendaRuntimeController.setDisplayedDateIso(Qt.formatDateTime(new Date(), "yyyy-MM-dd"));
        agendaPage.requestViewHook("today");
    }
    function summaryCountValue(key) {
        if (!summaryModel || summaryModel[key] === undefined)
            return 0;
        return Number(summaryModel[key]);
    }
    function stringValue(value, fallback) {
        if (value === undefined || value === null)
            return fallback;
        const text = String(value).trim();
        return text.length > 0 ? text : fallback;
    }
    function toggleAgendaItem(agendaItemModel) {
        if (!agendaRuntimeController || !agendaRuntimeController.toggleAgendaItemCompleted || !agendaItemModel || agendaItemModel.id === undefined)
            return;
        agendaRuntimeController.toggleAgendaItemCompleted(String(agendaItemModel.id));
        agendaPage.requestViewHook("toggle-agenda-item");
    }
    function noteIdForEntry(entryModel) {
        if (!entryModel)
            return "";
        const sourceKind = entryModel.sourceKind !== undefined ? String(entryModel.sourceKind).trim() : "";
        if (sourceKind !== "note")
            return "";
        return entryModel.sourceId !== undefined && entryModel.sourceId !== null ? String(entryModel.sourceId).trim() : "";
    }
    function requestOpenNote(entryModel) {
        const noteId = agendaPage.noteIdForEntry(entryModel);
        if (noteId.length === 0)
            return;
        agendaPage.requestViewHook("open-note");
        agendaPage.noteOpenRequested(noteId);
    }

    color: LV.Theme.accentTransparent
    radius: LV.Theme.radiusMd

    Component.onCompleted: requestViewHook("page-open")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap12
        spacing: LV.Theme.gap8

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: headerColumn.implicitHeight + LV.Theme.gap6
            color: LV.Theme.panelBackground10
            radius: LV.Theme.radiusSm

            Column {
                id: headerColumn

                anchors.fill: parent
                anchors.margins: LV.Theme.gap4
                spacing: LV.Theme.gap3

                LV.HStack {
                    spacing: LV.Theme.gap4

                    CalendarTodayControl {
                        id: agendaTodayControl

                        Layout.alignment: Qt.AlignVCenter

                        onPreviousRequested: {
                            if (agendaPage.agendaRuntimeController && agendaPage.agendaRuntimeController.shiftDay)
                                agendaPage.agendaRuntimeController.shiftDay(-1);
                            agendaPage.requestViewHook("previous-day");
                        }
                        onTodayRequested: agendaPage.jumpToToday()
                        onNextRequested: {
                            if (agendaPage.agendaRuntimeController && agendaPage.agendaRuntimeController.shiftDay)
                                agendaPage.agendaRuntimeController.shiftDay(1);
                            agendaPage.requestViewHook("next-day");
                        }
                    }
                    LV.Label {
                        Layout.alignment: Qt.AlignVCenter
                        color: LV.Theme.titleHeaderColor
                        font.weight: Font.Medium
                        text: agendaPage.agendaRuntimeController && agendaPage.agendaRuntimeController.dateLabel ? String(agendaPage.agendaRuntimeController.dateLabel) : "Agenda"
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    LV.Label {
                        Layout.alignment: Qt.AlignVCenter
                        color: LV.Theme.descriptionColor
                        text: agendaPage.stringValue(agendaPage.locationModel.displayName, "")
                    }
                }
            }
        }
        Flickable {
            id: listViewport

            Layout.fillHeight: true
            Layout.fillWidth: true
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            contentHeight: contentColumn.implicitHeight
            contentWidth: width
            flickableDirection: Flickable.VerticalFlick

            Column {
                id: contentColumn

                width: listViewport.width
                spacing: LV.Theme.gap6

                Rectangle {
                    color: LV.Theme.panelBackground10
                    height: allDayColumn.implicitHeight + LV.Theme.gap6
                    radius: LV.Theme.radiusSm
                    width: parent.width

                    Column {
                        id: allDayColumn

                        anchors.fill: parent
                        anchors.margins: LV.Theme.gap4
                        spacing: LV.Theme.gap3

                        LV.HStack {
                            spacing: LV.Theme.gap3

                            LV.Label {
                                color: LV.Theme.titleHeaderColor
                                font.weight: Font.Medium
                                text: "All day"
                            }
                            LV.Label {
                                color: LV.Theme.descriptionColor
                                text: "(" + String(agendaPage.summaryCountValue("allDayEventCount")) + ")"
                            }
                        }
                        LV.Label {
                            color: LV.Theme.descriptionColor
                            text: "No all day events"
                            visible: agendaPage.allDayModels.length === 0
                        }
                        Repeater {
                            model: agendaPage.allDayModels

                            Rectangle {
                                id: allDayItem

                                required property var modelData
                                readonly property string noteId: agendaPage.noteIdForEntry(allDayItem.modelData)

                                color: LV.Theme.panelBackground11
                                height: allDayText.implicitHeight + LV.Theme.gap4
                                radius: LV.Theme.radiusSm
                                width: parent.width

                                TapHandler {
                                    enabled: allDayItem.noteId.length > 0

                                    onTapped: agendaPage.requestOpenNote(allDayItem.modelData)
                                }

                                LV.Label {
                                    id: allDayText

                                    anchors.fill: parent
                                    anchors.margins: LV.Theme.gap3
                                    color: LV.Theme.titleHeaderColor
                                    text: agendaPage.stringValue(allDayItem.modelData && allDayItem.modelData.title, "Untitled")
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
                Rectangle {
                    color: LV.Theme.panelBackground10
                    height: timedColumn.implicitHeight + LV.Theme.gap6
                    radius: LV.Theme.radiusSm
                    width: parent.width

                    Column {
                        id: timedColumn

                        anchors.fill: parent
                        anchors.margins: LV.Theme.gap4
                        spacing: LV.Theme.gap3

                        LV.HStack {
                            spacing: LV.Theme.gap3

                            LV.Label {
                                color: LV.Theme.titleHeaderColor
                                font.weight: Font.Medium
                                text: "Timed"
                            }
                            LV.Label {
                                color: LV.Theme.descriptionColor
                                text: "(" + String(agendaPage.summaryCountValue("timedEventCount")) + ")"
                            }
                        }
                        LV.Label {
                            color: LV.Theme.descriptionColor
                            text: "No timed events"
                            visible: agendaPage.timedModels.length === 0
                        }
                        Repeater {
                            model: agendaPage.timedModels

                            Rectangle {
                                id: timedItem

                                required property var modelData
                                readonly property string noteId: agendaPage.noteIdForEntry(timedItem.modelData)

                                color: LV.Theme.panelBackground11
                                height: timedRow.implicitHeight + LV.Theme.gap4
                                radius: LV.Theme.radiusSm
                                width: parent.width

                                TapHandler {
                                    enabled: timedItem.noteId.length > 0

                                    onTapped: agendaPage.requestOpenNote(timedItem.modelData)
                                }

                                LV.HStack {
                                    id: timedRow

                                    anchors.fill: parent
                                    anchors.margins: LV.Theme.gap3
                                    spacing: LV.Theme.gap3

                                    LV.Label {
                                        color: LV.Theme.descriptionColor
                                        text: agendaPage.stringValue(timedItem.modelData && timedItem.modelData.timeLabel, "")
                                        width: LV.Theme.gap24 * 2
                                    }
                                    LV.Label {
                                        color: LV.Theme.titleHeaderColor
                                        text: agendaPage.stringValue(timedItem.modelData && timedItem.modelData.title, "Untitled")
                                        wrapMode: Text.WordWrap
                                    }
                                }
                            }
                        }
                    }
                }
                Rectangle {
                    color: LV.Theme.panelBackground10
                    height: agendaItemColumn.implicitHeight + LV.Theme.gap6
                    radius: LV.Theme.radiusSm
                    width: parent.width

                    Column {
                        id: agendaItemColumn

                        anchors.fill: parent
                        anchors.margins: LV.Theme.gap4
                        spacing: LV.Theme.gap3

                        LV.HStack {
                            spacing: LV.Theme.gap3

                            LV.Label {
                                color: LV.Theme.titleHeaderColor
                                font.weight: Font.Medium
                                text: "Agenda"
                            }
                            LV.Label {
                                color: LV.Theme.descriptionColor
                                text: String(agendaPage.summaryCountValue("completedAgendaItemCount")) + "/" + String(agendaPage.summaryCountValue("agendaItemCount"))
                            }
                        }
                        LV.Label {
                            color: LV.Theme.descriptionColor
                            text: "No agenda items"
                            visible: agendaPage.agendaItemModels.length === 0
                        }
                        Repeater {
                            model: agendaPage.agendaItemModels

                            Rectangle {
                                id: agendaItem

                                required property var modelData
                                readonly property bool completed: agendaItem.modelData && agendaItem.modelData.completed === true

                                color: agendaItem.completed ? LV.Theme.panelBackground08 : LV.Theme.panelBackground11
                                height: agendaItemRow.implicitHeight + LV.Theme.gap4
                                radius: LV.Theme.radiusSm
                                width: parent.width

                                LV.HStack {
                                    id: agendaItemRow

                                    anchors.fill: parent
                                    anchors.margins: LV.Theme.gap3
                                    spacing: LV.Theme.gap3

                                    Rectangle {
                                        id: agendaItemToggle

                                        border.color: LV.Theme.descriptionColor
                                        border.width: Math.max(1, LV.Theme.strokeThin)
                                        color: agendaItem.completed ? LV.Theme.primary : "transparent"
                                        height: LV.Theme.gap8
                                        radius: height / 2
                                        width: LV.Theme.gap8

                                        LV.Label {
                                            anchors.centerIn: parent
                                            color: LV.Theme.panelBackground01
                                            font.pixelSize: LV.Theme.textCaption
                                            text: agendaItem.completed ? "\u2713" : ""
                                        }
                                        MouseArea {
                                            anchors.fill: parent

                                            onClicked: {
                                                agendaPage.toggleAgendaItem(agendaItem.modelData);
                                            }
                                        }
                                    }
                                    LV.VStack {
                                        spacing: LV.Theme.gapNone

                                        LV.Label {
                                            color: LV.Theme.titleHeaderColor
                                            text: agendaPage.stringValue(agendaItem.modelData && agendaItem.modelData.title, "Untitled")
                                            wrapMode: Text.WordWrap
                                        }
                                        LV.Label {
                                            color: LV.Theme.descriptionColor
                                            text: agendaPage.stringValue(agendaItem.modelData && agendaItem.modelData.time, "")
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
}
