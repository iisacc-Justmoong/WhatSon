pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../contents" as ContentsView

Item {
    id: contentViewLayout

    property bool agendaOverlayVisible: false
    property var agendaController: null
    property var contentController: null
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarController: null
    property color displayColor: "transparent"
    property int editorTopInsetOverride: -1
    property int frameHorizontalInsetOverride: -1
    property bool isMobilePlatform: false
    property int libraryHierarchyIndex: 0
    property var libraryHierarchyController: null
    property bool minimapVisible: true
    property bool monthCalendarOverlayVisible: false
    property var monthCalendarController: null
    property var noteActiveState: null
    property var noteEditorSession: null
    readonly property string editorSourceFilePath: contentViewLayout.noteEditorSession
            && contentViewLayout.noteEditorSession.editorFilePath !== undefined
            ? String(contentViewLayout.noteEditorSession.editorFilePath).trim()
            : ""
    readonly property string editorActiveNoteId: contentViewLayout.noteEditorSession
            && contentViewLayout.noteEditorSession.activeNoteId !== undefined
            ? String(contentViewLayout.noteEditorSession.activeNoteId).trim()
            : ""
    readonly property string editorActiveNoteDirectoryPath: contentViewLayout.noteEditorSession
            && contentViewLayout.noteEditorSession.activeNoteDirectoryPath !== undefined
            ? String(contentViewLayout.noteEditorSession.activeNoteDirectoryPath).trim()
            : ""
    readonly property int editorParsedLineCount: contentViewLayout.noteEditorSession
            && contentViewLayout.noteEditorSession.parsedLineCount !== undefined
            ? Math.max(0, Number(contentViewLayout.noteEditorSession.parsedLineCount) || 0)
            : 0
    readonly property bool editorReadOnly: !contentViewLayout.noteEditorSession
            || contentViewLayout.noteEditorSession.readOnly === undefined
            || Boolean(contentViewLayout.noteEditorSession.readOnly)
    readonly property bool editorFormatContextMenuAvailable: !contentViewLayout.editorReadOnly
            && contentsTextEditor
            && contentsTextEditor.editorSelectionLength > 0
    readonly property var editorFormatContextMenuItems: [
        {
            "label": "Bold",
            "shortcut": "Cmd+B",
            "keyVisible": true,
            "enabled": contentViewLayout.editorFormatContextMenuAvailable,
            "eventName": "editor.format.bold"
        },
        {
            "label": "Italic",
            "shortcut": "Cmd+I",
            "keyVisible": true,
            "enabled": contentViewLayout.editorFormatContextMenuAvailable,
            "eventName": "editor.format.italic"
        },
        {
            "label": "Underline",
            "shortcut": "Cmd+U",
            "keyVisible": true,
            "enabled": contentViewLayout.editorFormatContextMenuAvailable,
            "eventName": "editor.format.underline"
        },
        {
            "label": "Strikethrough",
            "shortcut": "Cmd+Shift+X",
            "keyVisible": true,
            "enabled": contentViewLayout.editorFormatContextMenuAvailable,
            "eventName": "editor.format.strikethrough"
        },
        {
            "label": "Highlight",
            "shortcut": "Cmd+Shift+E",
            "keyVisible": true,
            "enabled": contentViewLayout.editorFormatContextMenuAvailable,
            "eventName": "editor.format.highlight"
        }
    ]
    property var noteListModel: null
    property var panelControllerRegistry: null
    readonly property var panelController: contentViewLayout.panelControllerRegistry ? contentViewLayout.panelControllerRegistry.panelController("ContentViewLayout") : null
    property var resourcesImportController: null
    property var sidebarHierarchyController: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarController: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarController: null

    signal agendaOverlayCloseRequested
    signal dayCalendarOverlayCloseRequested
    signal monthCalendarOverlayCloseRequested
    signal monthCalendarOverlayOpenRequested
    signal viewHookRequested
    signal weekCalendarOverlayCloseRequested
    signal yearCalendarOverlayCloseRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined && reason !== null ? String(reason) : "manual";
        if (contentViewLayout.panelController && contentViewLayout.panelController.requestControllerHook)
            contentViewLayout.panelController.requestControllerHook(hookReason);
        contentViewLayout.viewHookRequested();
    }
    function listLikeCount(value) {
        if (value === undefined || value === null)
            return 0;
        if (value.length !== undefined)
            return Math.max(0, Number(value.length) || 0);
        if (value.count !== undefined)
            return Math.max(0, Number(value.count) || 0);
        return 0;
    }
    function pasteClipboardImageIntoEditor() {
        if (!contentViewLayout.resourcesImportController
                || !contentViewLayout.noteEditorSession
                || contentViewLayout.editorReadOnly
                || contentViewLayout.resourcesImportController.importClipboardImageForEditor === undefined
                || contentViewLayout.noteEditorSession.insertImportedResourcesIntoSource === undefined)
            return false;

        const importedEntries = contentViewLayout.resourcesImportController.importClipboardImageForEditor();
        if (contentViewLayout.listLikeCount(importedEntries) <= 0)
            return false;

        const insertion = contentViewLayout.noteEditorSession.insertImportedResourcesIntoSource(
                    contentsTextEditor.editorDocumentText,
                    contentsTextEditor.editorSelectionStart,
                    contentsTextEditor.editorSelectionLength,
                    importedEntries);
        if (!insertion || !Boolean(insertion.valid))
            return false;

        const editorDocumentText = insertion.editorDocumentText !== undefined
                && insertion.editorDocumentText !== null
                ? String(insertion.editorDocumentText)
                : String(insertion.bodySourceText);
        if (!contentsTextEditor.replaceEditorDocumentText(
                    editorDocumentText,
                    Number(insertion.cursorPosition) || 0))
            return false;

        if (contentViewLayout.resourcesImportController.reloadImportedResources !== undefined)
            return Boolean(contentViewLayout.resourcesImportController.reloadImportedResources());
        return true;
    }
    function handleEditorPasteShortcut() {
        if (!contentViewLayout.resourcesImportController
                || contentViewLayout.resourcesImportController.refreshClipboardImageAvailabilitySnapshot === undefined
                || !contentViewLayout.resourcesImportController.refreshClipboardImageAvailabilitySnapshot()) {
            contentsTextEditor.pasteNativeClipboardText();
            return;
        }

        contentViewLayout.pasteClipboardImageIntoEditor();
    }
    function applyEditorFormatTag(tagName) {
        if (!contentViewLayout.noteEditorSession
                || contentViewLayout.editorReadOnly
                || contentViewLayout.noteEditorSession.insertFormatTagIntoSource === undefined)
            return false;

        const formatResult = contentViewLayout.noteEditorSession.insertFormatTagIntoSource(
                    tagName,
                    contentsTextEditor.editorDocumentText,
                    contentsTextEditor.editorSelectionStart,
                    contentsTextEditor.editorSelectionLength);
        if (!formatResult || !Boolean(formatResult.valid))
            return false;

        const editorDocumentText = formatResult.editorDocumentText !== undefined
                && formatResult.editorDocumentText !== null
                ? String(formatResult.editorDocumentText)
                : String(formatResult.bodySourceText);
        return contentsTextEditor.replaceEditorDocumentText(
                    editorDocumentText,
                    Number(formatResult.cursorPosition) || 0);
    }
    function editorFormatContextMenuPointerTriggerAccepted(triggerKind) {
        const normalizedTrigger = triggerKind === undefined || triggerKind === null
                ? ""
                : String(triggerKind).trim().toLowerCase();
        return normalizedTrigger === "rightclick"
                || normalizedTrigger === "right-click"
                || normalizedTrigger === "contextmenu"
                || normalizedTrigger === "context-menu";
    }
    function openEditorFormatContextMenuFromPointer(referenceItem, localX, localY, triggerKind) {
        if (!contentViewLayout.editorFormatContextMenuPointerTriggerAccepted(triggerKind)
                || !contentViewLayout.editorFormatContextMenuAvailable) {
            if (editorFormatContextMenu.opened)
                editorFormatContextMenu.close();
            return false;
        }

        editorFormatContextMenu.openFor(
                    referenceItem ? referenceItem : contentViewLayout,
                    Number(localX) || 0,
                    Number(localY) || 0);
        return true;
    }
    function handleEditorFormatContextMenuTrigger(eventName) {
        const normalizedEventName = eventName === undefined || eventName === null
                ? ""
                : String(eventName).trim();
        if (normalizedEventName === "editor.format.bold")
            return contentViewLayout.applyEditorFormatTag("bold");
        if (normalizedEventName === "editor.format.italic")
            return contentViewLayout.applyEditorFormatTag("italic");
        if (normalizedEventName === "editor.format.underline")
            return contentViewLayout.applyEditorFormatTag("underline");
        if (normalizedEventName === "editor.format.strikethrough")
            return contentViewLayout.applyEditorFormatTag("strikethrough");
        if (normalizedEventName === "editor.format.highlight")
            return contentViewLayout.applyEditorFormatTag("highlight");
        return false;
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle {
        anchors.fill: parent
        color: contentViewLayout.displayColor

        RowLayout {
            anchors.fill: parent
            spacing: LV.Theme.gapNone

            ContentsView.Gutter {
                Layout.fillHeight: true
                Layout.preferredWidth: implicitWidth
                contentY: contentsTextEditor.viewportContentY
                currentLineIndex: contentsTextEditor.editorCursorLineIndex
                fallbackLineHeight: contentsTextEditor.editorLogicalLineHeight
                lineCount: contentViewLayout.editorParsedLineCount
                lineMetricProvider: contentsTextEditor.editorLogicalLineMetricFor
                lineMetricsRevision: contentsTextEditor.editorLineMetricsRevision
                parsedLineCount: contentViewLayout.editorParsedLineCount
                selectedNoteDirectoryPath: contentViewLayout.editorActiveNoteDirectoryPath
                selectedNoteId: contentViewLayout.editorActiveNoteId
                sourceFilePath: contentViewLayout.editorSourceFilePath
            }

            ContentsView.TextEditor {
                id: contentsTextEditor

                Layout.fillHeight: true
                Layout.fillWidth: true
                editorReadOnly: contentViewLayout.editorReadOnly
                noteBodyFilePath: contentViewLayout.editorSourceFilePath
                objectName: "contentsDisplayTextEditor"

                onSyncFinished: function(path) {
                    if (contentViewLayout.noteEditorSession
                            && contentViewLayout.noteEditorSession.persistEditorFile !== undefined) {
                        contentViewLayout.noteEditorSession.persistEditorFile(path);
                    }
                }

                TapHandler {
                    id: editorFormatContextMenuTapHandler

                    acceptedButtons: Qt.RightButton
                    acceptedModifiers: Qt.KeyboardModifierMask
                    enabled: contentViewLayout.editorFormatContextMenuAvailable
                    gesturePolicy: TapHandler.DragThreshold
                    grabPermissions: PointerHandler.ApprovesTakeOverByAnything

                    onTapped: function (eventPoint, button) {
                        if (button !== Qt.RightButton)
                            return;
                        contentViewLayout.openEditorFormatContextMenuFromPointer(
                                    contentsTextEditor,
                                    eventPoint && eventPoint.position !== undefined ? eventPoint.position.x : 0,
                                    eventPoint && eventPoint.position !== undefined ? eventPoint.position.y : 0,
                                    "rightClick");
                    }
                }
            }

            ContentsView.Minimap {
                Layout.fillHeight: true
                Layout.preferredWidth: implicitWidth
                documentText: contentsTextEditor.editorDocumentText
                scrollTarget: contentsTextEditor.scrollEditorViewportTo
                sourceContentHeight: contentsTextEditor.editorViewportContentHeight
                sourceContentWidth: contentsTextEditor.editorViewportWidth
                sourceContentY: contentsTextEditor.viewportContentY
                sourceFontFamily: LV.Theme.fontBody
                sourceFontLetterSpacing: LV.Theme.textBodyLetterSpacing
                sourceFontPixelSize: LV.Theme.textBody
                sourceFontWeight: LV.Theme.textBodyWeight
                sourceViewportHeight: contentsTextEditor.editorViewportHeight
                visible: contentViewLayout.minimapVisible
            }
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: StandardKey.Paste

            onActivated: contentViewLayout.handleEditorPasteShortcut()
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: StandardKey.Bold

            onActivated: contentViewLayout.applyEditorFormatTag("bold")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: StandardKey.Italic

            onActivated: contentViewLayout.applyEditorFormatTag("italic")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: StandardKey.Underline

            onActivated: contentViewLayout.applyEditorFormatTag("underline")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: "Ctrl+Shift+X"

            onActivated: contentViewLayout.applyEditorFormatTag("strikethrough")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: "Meta+Shift+X"

            onActivated: contentViewLayout.applyEditorFormatTag("strikethrough")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: "Ctrl+Shift+E"

            onActivated: contentViewLayout.applyEditorFormatTag("highlight")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: "Meta+Shift+E"

            onActivated: contentViewLayout.applyEditorFormatTag("highlight")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: "Ctrl+Shift+B"

            onActivated: contentViewLayout.applyEditorFormatTag("break")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentsTextEditor.activeFocus && !contentViewLayout.editorReadOnly
            sequence: "Meta+Shift+B"

            onActivated: contentViewLayout.applyEditorFormatTag("break")
        }

        LV.ContextMenu {
            id: editorFormatContextMenu

            autoCloseOnTrigger: true
            closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
            items: contentViewLayout.editorFormatContextMenuItems
            modal: false
            parent: Controls.Overlay.overlay

            onItemEventTriggered: function (eventName, payload, index, item) {
                contentViewLayout.handleEditorFormatContextMenuTrigger(eventName);
            }
        }
    }
}
