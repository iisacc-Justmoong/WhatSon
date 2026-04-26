pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

Item {
    id: commandSurface

    property var contentsView: null
    property var resourceImportController: null
    readonly property alias selectionContextMenu: editorSelectionContextMenu

    anchors.fill: parent

    MouseArea {
        property real lastPressX: 0
        property real lastPressY: 0

        acceptedButtons: Qt.RightButton
        anchors.fill: parent
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentContextMenuSurfaceEnabled
        hoverEnabled: false
        preventStealing: false
        propagateComposedEvents: true
        z: 6

        onClicked: function (mouse) {
            if (mouse.button !== Qt.RightButton)
                return;
            commandSurface.contentsView.requestEditorSelectionContextMenuFromPointer(lastPressX, lastPressY, "rightClick");
        }

        onPressed: function (mouse) {
            if (mouse.button !== Qt.RightButton)
                return;
            lastPressX = mouse.x;
            lastPressY = mouse.y;
            commandSurface.contentsView.primeEditorSelectionContextMenuSnapshot();
        }
    }

    TapHandler {
        id: editorLongPressContextMenuTapHandler

        acceptedButtons: Qt.LeftButton
        acceptedDevices: PointerDevice.TouchScreen | PointerDevice.Stylus
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentContextMenuSurfaceEnabled
                 && commandSurface.contentsView.contextMenuLongPressEnabled
        gesturePolicy: TapHandler.DragThreshold
        grabPermissions: PointerHandler.ApprovesTakeOverByAnything
        target: null

        onLongPressed: {
            const pressPoint = editorLongPressContextMenuTapHandler.point
                    && editorLongPressContextMenuTapHandler.point.position !== undefined
                    ? editorLongPressContextMenuTapHandler.point.position
                    : Qt.point(0, 0);
            commandSurface.contentsView.requestEditorSelectionContextMenuFromPointer(
                        pressPoint.x,
                        pressPoint.y,
                        "longPress");
        }
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
                 && commandSurface.contentsView.resourcesImportViewModel
                 && (!commandSurface.contentsView.resourcesImportViewModel.busy)
                 && commandSurface.contentsView.resourcesImportViewModel.clipboardImageAvailable
        sequence: StandardKey.Paste

        onActivated: commandSurface.resourceImportController.pasteClipboardImageAsResource()
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Meta+B"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("bold")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Ctrl+B"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("bold")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Meta+I"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("italic")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Ctrl+I"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("italic")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Meta+U"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("underline")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Ctrl+U"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("underline")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Meta+Shift+X"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("strikethrough")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Ctrl+Shift+X"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("strikethrough")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Meta+Shift+E"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("highlight")
    }

    Shortcut {
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Ctrl+Shift+E"

        onActivated: commandSurface.contentsView.queueInlineFormatWrap("highlight")
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Meta+Alt+T"

        onActivated: commandSurface.contentsView.queueAgendaShortcutInsertion()
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Ctrl+Alt+T"

        onActivated: commandSurface.contentsView.queueAgendaShortcutInsertion()
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Meta+Alt+C"

        onActivated: commandSurface.contentsView.queueCalloutShortcutInsertion()
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Ctrl+Alt+C"

        onActivated: commandSurface.contentsView.queueCalloutShortcutInsertion()
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Meta+Shift+H"

        onActivated: commandSurface.contentsView.queueBreakShortcutInsertion()
    }

    Shortcut {
        autoRepeat: false
        context: Qt.WindowShortcut
        enabled: commandSurface.contentsView
                 && commandSurface.contentsView.noteDocumentTagManagementShortcutSurfaceEnabled
        sequence: "Ctrl+Shift+H"

        onActivated: commandSurface.contentsView.queueBreakShortcutInsertion()
    }

    LV.ContextMenu {
        id: editorSelectionContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        items: commandSurface.contentsView ? commandSurface.contentsView.editorSelectionContextMenuItems : []
        modal: false
        parent: Controls.Overlay.overlay

        onItemEventTriggered: function (eventName, payload, index, item) {
            commandSurface.contentsView.handleSelectionContextMenuEvent(eventName);
        }
    }
}
