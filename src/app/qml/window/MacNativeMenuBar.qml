pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import Qt.labs.platform as Platform

Platform.MenuBar {
    id: root

    property var hostWindow: null
    property var resourcesImportViewModel: null
    property string importFailureText: ""

    window: root.hostWindow

    MessageDialog {
        id: importFailureDialog

        buttons: MessageDialog.Ok
        text: root.importFailureText
        title: qsTr("Import Failed")
    }
    FileDialog {
        id: importResourcesDialog

        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("All Files (*)")]
        title: qsTr("Import Resource Files")

        onAccepted: {
            if (!root.resourcesImportViewModel)
                return;

            const succeeded = root.resourcesImportViewModel.importUrls(selectedFiles);
            if (!succeeded) {
                const failureText = root.resourcesImportViewModel.lastError !== undefined
                    ? String(root.resourcesImportViewModel.lastError).trim()
                    : "";
                root.importFailureText = failureText.length > 0
                    ? failureText
                    : qsTr("WhatSon could not import the selected files.");
                importFailureDialog.open();
            }
        }
    }
    Platform.Menu {
        title: qsTr("File")

        Platform.MenuItem {
            enabled: root.resourcesImportViewModel
                     && !root.resourcesImportViewModel.busy
                     && root.resourcesImportViewModel.currentHubPath !== undefined
                     && String(root.resourcesImportViewModel.currentHubPath).trim().length > 0
            text: qsTr("Import File...")

            onTriggered: importResourcesDialog.open()
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
