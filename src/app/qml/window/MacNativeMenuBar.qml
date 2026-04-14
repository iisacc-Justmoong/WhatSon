pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import Qt.labs.platform as Platform
import LVRS 1.0 as LV

Platform.MenuBar {
    id: root

    readonly property int resourceImportConflictPolicyAbort: 0
    readonly property int resourceImportConflictPolicyOverwrite: 1
    readonly property int resourceImportConflictPolicyKeepBoth: 2
    property var hostWindow: null
    property var pendingDuplicateImportConflict: ({})
    property var pendingDuplicateImportUrls: []
    property bool duplicateImportAlertOpen: false
    property var resourcesImportViewModel: null
    property string importFailureText: ""

    window: root.hostWindow

    function selectedImportUrls() {
        const urls = [];

        const selectedFilesValue = importResourcesDialog.selectedFiles;
        if (selectedFilesValue !== undefined && selectedFilesValue !== null) {
            if (Array.isArray(selectedFilesValue))
                urls.push(...selectedFilesValue);
            else if (selectedFilesValue.length !== undefined)
                urls.push(...Array.from(selectedFilesValue));
            else
                urls.push(selectedFilesValue);
        }

        const selectedFileValue = importResourcesDialog.selectedFile;
        if (selectedFileValue !== undefined
                && selectedFileValue !== null
                && String(selectedFileValue).trim().length > 0
                && !urls.includes(selectedFileValue))
            urls.push(selectedFileValue);

        return urls;
    }
    function clearPendingDuplicateImport() {
        root.pendingDuplicateImportConflict = ({});
        root.pendingDuplicateImportUrls = [];
        root.duplicateImportAlertOpen = false;
    }
    function normalizedDuplicateImportConflict(conflict) {
        return conflict && typeof conflict === "object" ? conflict : ({});
    }
    function duplicateImportAlertMessage() {
        const conflict = root.normalizedDuplicateImportConflict(root.pendingDuplicateImportConflict);
        const fileName = conflict.sourceFileName !== undefined ? String(conflict.sourceFileName).trim() : "";
        const resourcePath = conflict.existingResourcePath !== undefined ? String(conflict.existingResourcePath).trim() : "";
        if (fileName.length === 0)
            return "A resource with the same name already exists. Choose how to continue.";
        if (resourcePath.length === 0)
            return "A resource named \"" + fileName + "\" already exists. Choose whether to overwrite it, keep both copies, or cancel the import.";
        return "A resource named \"" + fileName + "\" already exists at \"" + resourcePath + "\". Choose whether to overwrite it, keep both copies, or cancel the import.";
    }
    function openImportFailureDialog() {
        const failureText = root.resourcesImportViewModel && root.resourcesImportViewModel.lastError !== undefined
            ? String(root.resourcesImportViewModel.lastError).trim()
            : "";
        root.importFailureText = failureText.length > 0
            ? failureText
            : qsTr("WhatSon could not import the selected files.");
        importFailureDialog.open();
    }
    function importSelectedFilesWithConflictPolicy(selectedFiles, conflictPolicy) {
        if (!root.resourcesImportViewModel
                || root.resourcesImportViewModel.importUrlsWithConflictPolicy === undefined)
            return false;
        const succeeded = root.resourcesImportViewModel.importUrlsWithConflictPolicy(selectedFiles, conflictPolicy);
        if (!succeeded)
            root.openImportFailureDialog();
        return succeeded;
    }
    function handleSelectedImportUrls(selectedFiles) {
        if (!root.resourcesImportViewModel)
            return;
        const conflict = root.resourcesImportViewModel.inspectImportConflictForUrls !== undefined
            ? root.resourcesImportViewModel.inspectImportConflictForUrls(selectedFiles)
            : ({});
        if (conflict && conflict.conflict) {
            root.pendingDuplicateImportUrls = Array.isArray(selectedFiles) ? selectedFiles.slice(0) : [];
            root.pendingDuplicateImportConflict = root.normalizedDuplicateImportConflict(conflict);
            root.duplicateImportAlertOpen = true;
            return;
        }
        root.importSelectedFilesWithConflictPolicy(selectedFiles, root.resourceImportConflictPolicyAbort);
    }
    function resolvePendingDuplicateImport(conflictPolicy) {
        const selectedFiles = Array.isArray(root.pendingDuplicateImportUrls) ? root.pendingDuplicateImportUrls.slice(0) : [];
        root.duplicateImportAlertOpen = false;
        root.importSelectedFilesWithConflictPolicy(selectedFiles, conflictPolicy);
        root.clearPendingDuplicateImport();
    }

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

            const selectedFiles = root.selectedImportUrls();
            root.handleSelectedImportUrls(selectedFiles);
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
    LV.Alert {
        id: duplicateImportAlert

        parent: root.hostWindow ? root.hostWindow.contentItem : null
        buttonCount: 3
        dismissOnBackground: false
        message: root.duplicateImportAlertMessage()
        open: root.duplicateImportAlertOpen
        primaryText: "Overwrite"
        secondaryText: "Keep Both"
        tertiaryText: "Cancel Import"
        title: "Duplicate Resource Import"

        onPrimaryClicked: {
            root.resolvePendingDuplicateImport(root.resourceImportConflictPolicyOverwrite);
        }
        onSecondaryClicked: {
            root.resolvePendingDuplicateImport(root.resourceImportConflictPolicyKeepBoth);
        }
        onTertiaryClicked: {
            root.clearPendingDuplicateImport();
        }
        onDismissed: {
            root.clearPendingDuplicateImport();
        }
    }
}
