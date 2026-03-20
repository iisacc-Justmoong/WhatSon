pragma ComponentBehavior: Bound

import QtQuick
import Qt.labs.platform as Platform

Platform.MenuBar {
    id: root

    property var hostWindow: null

    window: root.hostWindow

    Platform.Menu {
        title: qsTr("File")

        Platform.MenuItem {
            enabled: false
            text: root.hostWindow ? root.hostWindow.nativeMenuPlaceholderText() : " "
        }
    }
    Platform.Menu {
        title: qsTr("Edit")

        Platform.MenuItem {
            enabled: false
            text: root.hostWindow ? root.hostWindow.nativeMenuPlaceholderText() : " "
        }
    }
    Platform.Menu {
        title: qsTr("View")

        Platform.MenuItem {
            enabled: false
            text: root.hostWindow ? root.hostWindow.nativeMenuPlaceholderText() : " "
        }
    }
    Platform.Menu {
        title: qsTr("Window")

        Platform.MenuItem {
            text: qsTr("Onboarding")

            onTriggered: {
                if (root.hostWindow)
                    root.hostWindow.showOnboardingWindow();
            }
        }
    }
    Platform.Menu {
        title: qsTr("Help")

        Platform.MenuItem {
            enabled: false
            text: root.hostWindow ? root.hostWindow.nativeMenuPlaceholderText() : " "
        }
    }
}
