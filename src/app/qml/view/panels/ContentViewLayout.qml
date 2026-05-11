pragma ComponentBehavior: Bound

import QtQuick
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
                    contentsTextEditor.cursorPosition,
                    0,
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
                lineCount: contentViewLayout.editorParsedLineCount
                lineHeight: contentsTextEditor.lineHeight
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
            }

            ContentsView.Minimap {
                Layout.fillHeight: true
                Layout.preferredWidth: implicitWidth
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
    }
}
