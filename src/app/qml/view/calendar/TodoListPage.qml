pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: todoListPage

    readonly property var allDayModels: todoVm && todoVm.allDayEvents ? todoVm.allDayEvents : []
    readonly property var locationModel: todoVm && todoVm.location ? todoVm.location : ({})
    readonly property var summaryModel: todoVm && todoVm.summary ? todoVm.summary : ({})
    readonly property var taskModels: todoVm && todoVm.tasks ? todoVm.tasks : []
    readonly property var timedModels: todoVm && todoVm.timedEvents ? todoVm.timedEvents : []
    readonly property var todoVm: todoListViewModel
    readonly property var weatherModel: todoVm && todoVm.weather ? todoVm.weather : ({})
    property var todoListViewModel: null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (todoVm && todoVm.requestTodoListView)
            todoVm.requestTodoListView(hookReason);
        viewHookRequested(hookReason);
    }
    function jumpToToday() {
        if (todoVm && todoVm.setDisplayedDateIso)
            todoVm.setDisplayedDateIso(Qt.formatDateTime(new Date(), "yyyy-MM-dd"));
        todoListPage.requestViewHook("today");
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
    function toggleTask(taskModel) {
        if (!todoVm || !todoVm.toggleTaskCompleted || !taskModel || taskModel.id === undefined)
            return;
        todoVm.toggleTaskCompleted(String(taskModel.id));
        todoListPage.requestViewHook("toggle-task");
    }

    color: "transparent"
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
                        id: todoTodayControl

                        Layout.alignment: Qt.AlignVCenter

                        onPreviousRequested: {
                            if (todoVm && todoVm.shiftDay)
                                todoVm.shiftDay(-1);
                            todoListPage.requestViewHook("previous-day");
                        }
                        onTodayRequested: todoListPage.jumpToToday()
                        onNextRequested: {
                            if (todoVm && todoVm.shiftDay)
                                todoVm.shiftDay(1);
                            todoListPage.requestViewHook("next-day");
                        }
                    }
                    LV.Label {
                        Layout.alignment: Qt.AlignVCenter
                        color: LV.Theme.titleHeaderColor
                        font.weight: Font.Medium
                        text: todoVm && todoVm.dateLabel ? String(todoVm.dateLabel) : "Todo"
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    LV.Label {
                        Layout.alignment: Qt.AlignVCenter
                        color: LV.Theme.descriptionColor
                        text: todoListPage.stringValue(todoListPage.locationModel.displayName, "")
                    }
                }
                Rectangle {
                    color: LV.Theme.panelBackground11
                    height: weatherRow.implicitHeight + LV.Theme.gap4
                    radius: LV.Theme.radiusSm
                    width: parent.width

                    LV.HStack {
                        id: weatherRow

                        anchors.fill: parent
                        anchors.margins: LV.Theme.gap3
                        spacing: LV.Theme.gap4

                        LV.Label {
                            color: LV.Theme.titleHeaderColor
                            font.pixelSize: 20
                            font.weight: Font.Medium
                            text: todoListPage.stringValue(todoListPage.weatherModel.temperatureText, "--C")
                        }
                        LV.VStack {
                            spacing: LV.Theme.gapNone

                            LV.Label {
                                color: LV.Theme.descriptionColor
                                text: todoListPage.stringValue(todoListPage.weatherModel.conditionText, "Weather")
                            }
                            LV.Label {
                                color: LV.Theme.descriptionColor
                                text: todoListPage.stringValue(todoListPage.weatherModel.highLowText, "")
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        LV.Label {
                            color: LV.Theme.descriptionColor
                            text: "Rain " + todoListPage.stringValue(todoListPage.weatherModel.precipitationText, "0%")
                        }
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
                                text: "(" + String(todoListPage.summaryCountValue("allDayEventCount")) + ")"
                            }
                        }
                        LV.Label {
                            color: LV.Theme.descriptionColor
                            text: "No all day events"
                            visible: todoListPage.allDayModels.length === 0
                        }
                        Repeater {
                            model: todoListPage.allDayModels

                            Rectangle {
                                id: allDayItem

                                required property var modelData

                                color: LV.Theme.panelBackground11
                                height: allDayText.implicitHeight + LV.Theme.gap4
                                radius: LV.Theme.radiusSm
                                width: parent.width

                                LV.Label {
                                    id: allDayText

                                    anchors.fill: parent
                                    anchors.margins: LV.Theme.gap3
                                    color: LV.Theme.titleHeaderColor
                                    text: todoListPage.stringValue(
                                              allDayItem.modelData && allDayItem.modelData.title,
                                              "Untitled")
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
                                text: "(" + String(todoListPage.summaryCountValue("timedEventCount")) + ")"
                            }
                        }
                        LV.Label {
                            color: LV.Theme.descriptionColor
                            text: "No timed events"
                            visible: todoListPage.timedModels.length === 0
                        }
                        Repeater {
                            model: todoListPage.timedModels

                            Rectangle {
                                id: timedItem

                                required property var modelData

                                color: LV.Theme.panelBackground11
                                height: timedRow.implicitHeight + LV.Theme.gap4
                                radius: LV.Theme.radiusSm
                                width: parent.width

                                LV.HStack {
                                    id: timedRow

                                    anchors.fill: parent
                                    anchors.margins: LV.Theme.gap3
                                    spacing: LV.Theme.gap3

                                    LV.Label {
                                        color: LV.Theme.descriptionColor
                                        text: todoListPage.stringValue(
                                                  timedItem.modelData && timedItem.modelData.timeLabel,
                                                  "")
                                        width: LV.Theme.gap24 * 2
                                    }
                                    LV.Label {
                                        color: LV.Theme.titleHeaderColor
                                        text: todoListPage.stringValue(
                                                  timedItem.modelData && timedItem.modelData.title,
                                                  "Untitled")
                                        wrapMode: Text.WordWrap
                                    }
                                }
                            }
                        }
                    }
                }
                Rectangle {
                    color: LV.Theme.panelBackground10
                    height: taskColumn.implicitHeight + LV.Theme.gap6
                    radius: LV.Theme.radiusSm
                    width: parent.width

                    Column {
                        id: taskColumn

                        anchors.fill: parent
                        anchors.margins: LV.Theme.gap4
                        spacing: LV.Theme.gap3

                        LV.HStack {
                            spacing: LV.Theme.gap3

                            LV.Label {
                                color: LV.Theme.titleHeaderColor
                                font.weight: Font.Medium
                                text: "Tasks"
                            }
                            LV.Label {
                                color: LV.Theme.descriptionColor
                                text: String(todoListPage.summaryCountValue("completedTaskCount"))
                                      + "/"
                                      + String(todoListPage.summaryCountValue("taskCount"))
                            }
                        }
                        LV.Label {
                            color: LV.Theme.descriptionColor
                            text: "No tasks"
                            visible: todoListPage.taskModels.length === 0
                        }
                        Repeater {
                            model: todoListPage.taskModels

                            Rectangle {
                                id: taskItem

                                required property var modelData
                                readonly property bool completed: taskItem.modelData
                                                                 && taskItem.modelData.completed === true

                                color: taskItem.completed ? LV.Theme.panelBackground08 : LV.Theme.panelBackground11
                                height: taskRow.implicitHeight + LV.Theme.gap4
                                radius: LV.Theme.radiusSm
                                width: parent.width

                                LV.HStack {
                                    id: taskRow

                                    anchors.fill: parent
                                    anchors.margins: LV.Theme.gap3
                                    spacing: LV.Theme.gap3

                                    Rectangle {
                                        id: taskToggle

                                        border.color: LV.Theme.descriptionColor
                                        border.width: Math.max(1, LV.Theme.strokeThin)
                                        color: taskItem.completed ? LV.Theme.primary : "transparent"
                                        height: LV.Theme.gap8
                                        radius: height / 2
                                        width: LV.Theme.gap8

                                        LV.Label {
                                            anchors.centerIn: parent
                                            color: LV.Theme.panelBackground01
                                            font.pixelSize: 10
                                            text: taskItem.completed ? "\u2713" : ""
                                        }
                                        MouseArea {
                                            anchors.fill: parent

                                            onClicked: {
                                                todoListPage.toggleTask(taskItem.modelData);
                                            }
                                        }
                                    }
                                    LV.VStack {
                                        spacing: LV.Theme.gapNone

                                        LV.Label {
                                            color: LV.Theme.titleHeaderColor
                                            text: todoListPage.stringValue(
                                                      taskItem.modelData && taskItem.modelData.title,
                                                      "Untitled")
                                            wrapMode: Text.WordWrap
                                        }
                                        LV.Label {
                                            color: LV.Theme.descriptionColor
                                            text: todoListPage.stringValue(
                                                      taskItem.modelData && taskItem.modelData.time,
                                                      "")
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
