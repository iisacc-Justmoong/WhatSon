import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 12

    NavigationCalendarBar {
        Layout.alignment: Qt.AlignVCenter
    }
    NavigationAddNewBar {
        Layout.alignment: Qt.AlignVCenter
    }
    NavigationAppControlBar {
        Layout.alignment: Qt.AlignVCenter
    }
    NavigationExportBar {
        Layout.alignment: Qt.AlignVCenter
    }
    NavigationPreferenceBar {
        Layout.alignment: Qt.AlignVCenter
    }
}
