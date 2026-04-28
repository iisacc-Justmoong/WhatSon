pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: state

    property bool resourceDropActive: false

    readonly property int resourceImportConflictPolicyAbort: 0
    readonly property int resourceImportConflictPolicyOverwrite: 1
    readonly property int resourceImportConflictPolicyKeepBoth: 2
    readonly property int resourceImportModeNone: 0
    readonly property int resourceImportModeUrls: 1
    readonly property int resourceImportModeClipboard: 2

    readonly property color resourceRenderBorderColor: "#334E5157"
    readonly property color resourceRenderCardColor: "#E61A1D22"
    readonly property int resourceRenderDisplayLimit: 0
    readonly property int resourceEditorPlaceholderLineCount: 1
}
