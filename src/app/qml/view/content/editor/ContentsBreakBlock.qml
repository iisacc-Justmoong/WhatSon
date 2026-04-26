pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "../../../../models/editor/input" as EditorInputModel

FocusScope {
    id: breakBlock

    required property var blockData
    property var tagManagementShortcutKeyPressHandler: null

    signal activated()
    signal blockDeletionRequested()
    signal boundaryNavigationRequested(string axis, string side)
    signal documentEndEditRequested()

    readonly property var normalizedBlock: blockData && typeof blockData === "object" ? blockData : ({})
    readonly property bool focused: breakBlock.activeFocus
    readonly property int currentLogicalLineNumber: 1
    readonly property bool textEditable: false
    readonly property bool atomicBlock: true
    readonly property bool gutterCollapsed: true
    readonly property string minimapVisualKind: "text"
    readonly property int minimapRepresentativeCharCount: 8
    readonly property int sourceStart: Math.max(0, Number(normalizedBlock.sourceStart) || 0)
    readonly property int sourceEnd: Math.max(sourceStart, Number(normalizedBlock.sourceEnd) || sourceStart)

    implicitHeight: Math.max(
                        Math.round(LV.Theme.scaleMetric(18)),
                        divider.height + Math.round(LV.Theme.scaleMetric(10)) * 2)
    width: parent ? parent.width : implicitWidth

    function selectBreakBlock() {
        breakBlockController.selectBreakBlock()
    }

    function applyFocusRequest(request) {
        return breakBlockController.applyFocusRequest(request)
    }

    function visiblePlainText() {
        return breakBlockController.visiblePlainText()
    }

    function representativeCharCount(lineText) {
        return breakBlockController.representativeCharCount(lineText)
    }

    function logicalLineLayoutEntries() {
        return breakBlockController.logicalLineLayoutEntries()
    }

    function currentCursorRowRect() {
        return breakBlockController.currentCursorRowRect()
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: LV.Theme.panelBackground10
        border.width: breakBlock.focused ? Math.max(1, Math.round(LV.Theme.strokeThin)) : 0
        radius: Math.max(0, Math.round(LV.Theme.scaleMetric(8)))
    }

    Rectangle {
        id: divider

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        color: "#43464B"
        height: 1
        radius: 1
    }

    Keys.onPressed: function (event) {
        breakBlockController.handleKeyPress(event)
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: function () {
            breakBlockController.handleTap(tapCount)
        }
    }

    EditorInputModel.ContentsBreakBlockController {
        id: breakBlockController

        breakBlock: breakBlock
        divider: divider
    }
}
