pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

LV.Alert {
    id: conflictAlert

    required property var contentsView
    required property var resourceImportController

    parent: contentsView
    buttonCount: 3
    dismissOnBackground: false
    message: resourceImportController.resourceImportConflictAlertMessage()
    open: contentsView.resourceImportConflictAlertOpen
    primaryText: "Overwrite"
    secondaryText: "Keep Both"
    tertiaryText: "Cancel Import"
    title: "Duplicate Resource Import"

    onPrimaryClicked: {
        resourceImportController.executePendingResourceImportWithPolicy(
                    contentsView.resourceImportConflictPolicyOverwrite);
    }
    onSecondaryClicked: {
        resourceImportController.executePendingResourceImportWithPolicy(
                    contentsView.resourceImportConflictPolicyKeepBoth);
    }
    onTertiaryClicked: {
        resourceImportController.cancelPendingResourceImportConflict();
    }
    onDismissed: {
        resourceImportController.cancelPendingResourceImportConflict();
    }
}
