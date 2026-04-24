pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

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
        breakBlock.forceActiveFocus()
        breakBlock.activated()
    }

    function applyFocusRequest(request) {
        const safeRequest = request && typeof request === "object" ? request : ({})
        const sourceOffset = Number(safeRequest.sourceOffset)
        if (!isFinite(sourceOffset))
            return false
        if (sourceOffset < breakBlock.sourceStart || sourceOffset > breakBlock.sourceEnd)
            return false
        breakBlock.selectBreakBlock()
        return true
    }

    function visiblePlainText() {
        return ""
    }

    function representativeCharCount(lineText) {
        const normalizedLineText = lineText === undefined || lineText === null ? "" : String(lineText)
        if (normalizedLineText.length > 0)
            return normalizedLineText.length
        return 8
    }

    function logicalLineLayoutEntries() {
        const mappedOrigin = divider.mapToItem !== undefined
                ? divider.mapToItem(breakBlock, 0, 0)
                : ({ "x": 0, "y": Math.max(0, (breakBlock.height - divider.height) / 2) })
        return [{
                    "contentHeight": Math.max(1, Number(divider.height) || 1),
                    "contentY": Math.max(0, Number(mappedOrigin.y) || 0)
                }]
    }

    function currentCursorRowRect() {
        const entries = breakBlock.logicalLineLayoutEntries()
        if (entries.length > 0)
            return entries[0]
        return ({
                    "contentHeight": 1,
                    "contentY": 0
                })
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
        if (!event)
            return
        if (breakBlock.tagManagementShortcutKeyPressHandler
                && typeof breakBlock.tagManagementShortcutKeyPressHandler === "function") {
            const shortcutHandled = !!breakBlock.tagManagementShortcutKeyPressHandler(event)
            if (shortcutHandled || event.accepted)
                return
        }
        const modifiers = Number(event.modifiers) || 0
        const moveUp = event.key === Qt.Key_Up
        const moveDown = event.key === Qt.Key_Down
        const macModifierVerticalNavigation = Qt.platform.os === "osx"
                && (moveUp || moveDown)
                && (modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) === 0
                && (modifiers & (Qt.AltModifier | Qt.MetaModifier)) !== 0
        if (macModifierVerticalNavigation) {
            if ((modifiers & Qt.MetaModifier) !== 0)
                breakBlock.boundaryNavigationRequested("document", moveUp ? "before" : "after")
            else
                breakBlock.boundaryNavigationRequested("vertical", moveUp ? "before" : "after")
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
            breakBlock.blockDeletionRequested()
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Left) {
            breakBlock.boundaryNavigationRequested("horizontal", "before")
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Right) {
            breakBlock.boundaryNavigationRequested("horizontal", "after")
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Up) {
            breakBlock.boundaryNavigationRequested("vertical", "before")
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Down) {
            breakBlock.boundaryNavigationRequested("vertical", "after")
            event.accepted = true
            return
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: function () {
            if (tapCount >= 2) {
                breakBlock.documentEndEditRequested()
                return
            }
            breakBlock.selectBreakBlock()
        }
    }
}
