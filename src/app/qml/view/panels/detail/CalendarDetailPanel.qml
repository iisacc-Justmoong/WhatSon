import QtQuick
import LVRS 1.0 as LV

Item {
    id: calendarDetailPanel

    signal viewHookRequested

    function requestViewHook(reason) {
        viewHookRequested();
    }

    objectName: "CalendarDetailPanel"
}
