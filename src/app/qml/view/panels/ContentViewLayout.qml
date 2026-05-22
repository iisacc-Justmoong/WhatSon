pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../calendar" as CalendarView
import "../contents" as ContentsView

Item {
    id: contentViewLayout

    property var contentController: null
    property bool dayCalendarOverlayVisible: false
    property var dayCalendarController: null
    readonly property bool calendarOverlayActive: contentViewLayout.dayCalendarOverlayVisible
            || contentViewLayout.weekCalendarOverlayVisible
            || contentViewLayout.monthCalendarOverlayVisible
            || contentViewLayout.yearCalendarOverlayVisible
    property color displayColor: "transparent"
    property int editorTopInsetOverride: -1
    property int frameHorizontalInsetOverride: -1
    property bool gutterVisible: true
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
    readonly property bool editorFormatContextMenuAvailable: contentViewLayout.noteEditorSurfaceVisible
            && !contentViewLayout.editorReadOnly
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
    property var editorFormatSelectionSnapshot: ({
            "documentText": "",
            "selectionLength": 0,
            "selectionStart": 0,
            "selectedText": "",
            "valid": false
        })
    property bool editorFormatContextMenuPointerActive: false
    property bool editorApplyingPulledDocumentText: false
    property var noteListModel: null
    readonly property var currentResourceEntry: contentViewLayout.resourceListActive
            && contentViewLayout.noteListModel.currentResourceEntry !== undefined
            && contentViewLayout.noteListModel.currentResourceEntry !== null
            ? contentViewLayout.noteListModel.currentResourceEntry
            : ({})
    readonly property bool resourceListActive: contentViewLayout.noteListModel
            && contentViewLayout.noteListModel.currentResourceEntry !== undefined
    readonly property string currentResourceType: contentViewLayout.resourceEntryString("type").toLowerCase()
    readonly property string currentResourceFormat: contentViewLayout.resourceEntryString("format").toLowerCase()
    readonly property bool imageResourceSelected: contentViewLayout.resourceListActive
            && (contentViewLayout.currentResourceType === "image"
                || contentViewLayout.resourceFormatLooksLikeImage(contentViewLayout.currentResourceFormat))
            && (contentViewLayout.resourceEntryString("source").length > 0
                || contentViewLayout.resourceEntryString("resolvedPath").length > 0
                || contentViewLayout.resourceEntryString("resourcePath").length > 0)
    readonly property bool noteEditorSurfaceVisible: !contentViewLayout.imageResourceSelected
    property var panelControllerRegistry: null
    readonly property var panelController: contentViewLayout.panelControllerRegistry ? contentViewLayout.panelControllerRegistry.panelController("ContentViewLayout") : null
    property var inAppClipboard: null
    property var clipboardEditorPaste: null
    property var editorInputCommandFilter: null
    property var sidebarHierarchyController: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarController: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarController: null

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
    function closeCalendarOverlays() {
        if (contentViewLayout.dayCalendarOverlayVisible)
            contentViewLayout.dayCalendarOverlayCloseRequested();
        if (contentViewLayout.weekCalendarOverlayVisible)
            contentViewLayout.weekCalendarOverlayCloseRequested();
        if (contentViewLayout.monthCalendarOverlayVisible)
            contentViewLayout.monthCalendarOverlayCloseRequested();
        if (contentViewLayout.yearCalendarOverlayVisible)
            contentViewLayout.yearCalendarOverlayCloseRequested();
    }
    function openCalendarNote(noteId) {
        const normalizedNoteId = noteId === undefined || noteId === null
                ? ""
                : String(noteId).trim();
        if (normalizedNoteId.length === 0
                || !contentViewLayout.libraryHierarchyController
                || contentViewLayout.libraryHierarchyController.activateNoteById === undefined)
            return false;

        if (contentViewLayout.sidebarHierarchyController
                && contentViewLayout.sidebarHierarchyController.setActiveHierarchyIndex !== undefined)
            contentViewLayout.sidebarHierarchyController.setActiveHierarchyIndex(contentViewLayout.libraryHierarchyIndex);

        const activated = Boolean(contentViewLayout.libraryHierarchyController.activateNoteById(normalizedNoteId));
        if (activated)
            contentViewLayout.closeCalendarOverlays();
        return activated;
    }
    function openMonthCalendarFromYear(year, month, selectedDateIso) {
        const targetYear = Math.floor(Number(year));
        const targetMonth = Math.floor(Number(month));
        const normalizedSelectedDateIso = selectedDateIso === undefined || selectedDateIso === null
                ? ""
                : String(selectedDateIso).trim();

        if (contentViewLayout.monthCalendarController) {
            if (isFinite(targetYear) && contentViewLayout.monthCalendarController.setDisplayedYear !== undefined)
                contentViewLayout.monthCalendarController.setDisplayedYear(targetYear);
            if (isFinite(targetMonth) && contentViewLayout.monthCalendarController.setDisplayedMonth !== undefined)
                contentViewLayout.monthCalendarController.setDisplayedMonth(targetMonth);
            if (normalizedSelectedDateIso.length > 0
                    && contentViewLayout.monthCalendarController.setSelectedDateIso !== undefined)
                contentViewLayout.monthCalendarController.setSelectedDateIso(normalizedSelectedDateIso);
        }

        contentViewLayout.monthCalendarOverlayOpenRequested();
    }
    function editorCommandShortcutEnabled() {
        if (!contentViewLayout.noteEditorSurfaceVisible)
            return false;
        if (contentViewLayout.editorReadOnly || !contentsTextEditor)
            return false;
        if (contentsTextEditor.activeFocus)
            return true;
        if (contentsTextEditor.focused !== undefined && Boolean(contentsTextEditor.focused))
            return true;

        const editorItem = contentsTextEditor.editorItem !== undefined
                ? contentsTextEditor.editorItem
                : null;
        return !!(editorItem
                  && editorItem.activeFocus !== undefined
                  && Boolean(editorItem.activeFocus));
    }
    function resourceEntryString(key) {
        if (!contentViewLayout.currentResourceEntry || typeof contentViewLayout.currentResourceEntry !== "object")
            return "";
        const value = contentViewLayout.currentResourceEntry[key];
        if (value === undefined || value === null)
            return "";
        return String(value).trim();
    }
    function resourceFormatLooksLikeImage(format) {
        const normalizedFormat = format === undefined || format === null
                ? ""
                : String(format).trim().toLowerCase();
        const suffixedFormat = normalizedFormat.startsWith(".")
                ? normalizedFormat
                : "." + normalizedFormat;
        return suffixedFormat === ".avif"
                || suffixedFormat === ".bmp"
                || suffixedFormat === ".gif"
                || suffixedFormat === ".heic"
                || suffixedFormat === ".heif"
                || suffixedFormat === ".jpeg"
                || suffixedFormat === ".jpg"
                || suffixedFormat === ".png"
                || suffixedFormat === ".svg"
                || suffixedFormat === ".webp";
    }
    function listLikeCount(value) {
        if (!value)
            return 0;
        if (value.length !== undefined)
            return Math.max(0, Math.floor(Number(value.length) || 0));
        if (value.count !== undefined)
            return Math.max(0, Math.floor(Number(value.count) || 0));
        return 0;
    }
    function applyEditorFormatTag(tagName, allowSelectionSnapshot) {
        if (!contentViewLayout.noteEditorSession
                || contentViewLayout.editorReadOnly
                || contentViewLayout.noteEditorSession.insertFormatTagIntoSource === undefined)
            return false;

        const selectionState = contentViewLayout.editorFormatSelectionForCommand(
                    Boolean(allowSelectionSnapshot));
        const formatResult = contentViewLayout.noteEditorSession.insertFormatTagIntoSource(
                    tagName,
                    selectionState.documentText,
                    selectionState.selectionStart,
                    selectionState.selectionLength,
                    selectionState.selectedText);
        if (!formatResult || !Boolean(formatResult.valid))
            return false;

        const editorDocumentText = formatResult.editorDocumentText !== undefined
                && formatResult.editorDocumentText !== null
                ? String(formatResult.editorDocumentText)
                : String(formatResult.bodySourceText);
        const replaced = contentsTextEditor.replaceEditorDocumentText(
                    editorDocumentText,
                    Number(formatResult.cursorPosition) || 0);
        if (replaced) {
            contentViewLayout.clearEditorFormatSelectionSnapshot();
        }
        return replaced;
    }
    function rememberEditorFormatSelectionSnapshot() {
        if (contentViewLayout.editorFormatContextMenuPointerActive || editorFormatContextMenu.opened)
            return false;
        const selectionState = contentViewLayout.liveEditorFormatSelectionState();
        if (selectionState.valid)
            contentViewLayout.editorFormatSelectionSnapshot = selectionState;
        return selectionState.valid;
    }
    function clearEditorFormatSelectionSnapshot() {
        contentViewLayout.editorFormatSelectionSnapshot = ({
                "documentText": "",
                "selectionLength": 0,
                "selectionStart": 0,
                "selectedText": "",
                "valid": false
            });
    }
    function applyPulledEditorDocumentText(noteId, editorDocumentText) {
        if (!contentsTextEditor
                || String(noteId).trim() !== contentViewLayout.editorActiveNoteId)
            return false;

        contentViewLayout.editorApplyingPulledDocumentText = true;
        const replaced = contentsTextEditor.replaceEditorDocumentText(
                    editorDocumentText === undefined || editorDocumentText === null
                    ? ""
                    : String(editorDocumentText),
                    contentsTextEditor.cursorPosition);
        Qt.callLater(function () {
            contentViewLayout.editorApplyingPulledDocumentText = false;
        });
        return replaced;
    }
    function markEditorSessionFileReadyForRawPush(path) {
        const readPath = path === undefined || path === null ? "" : String(path).trim();
        if (!contentViewLayout.noteEditorSurfaceVisible
                || contentViewLayout.editorReadOnly
                || readPath.length === 0
                || readPath !== contentViewLayout.editorSourceFilePath
                || contentViewLayout.editorSourceFilePath.length === 0
                || !contentViewLayout.noteEditorSession
                || contentViewLayout.noteEditorSession.markEditorSessionFileReadyForRawPush === undefined)
            return false;

        return contentViewLayout.noteEditorSession.markEditorSessionFileReadyForRawPush(
                    readPath);
    }
    function requestEditorModifiedRawPush(editorDocumentText) {
        if (contentViewLayout.noteEditorSurfaceVisible
                && !contentViewLayout.editorApplyingPulledDocumentText
                && contentViewLayout.noteEditorSession
                && contentViewLayout.noteEditorSession.requestEditorModifiedCountRawPush !== undefined
                && contentViewLayout.editorSourceFilePath.length > 0) {
            contentViewLayout.noteEditorSession.requestEditorModifiedCountRawPush(
                        contentViewLayout.editorSourceFilePath,
                        contentsTextEditor.editorPlainTextRevision + 1,
                        editorDocumentText === undefined || editorDocumentText === null
                        ? contentsTextEditor.editorDocumentText
                        : String(editorDocumentText));
        }
    }
    function editorFormatSelectionForCommand(allowSelectionSnapshot) {
        if (Boolean(allowSelectionSnapshot)
                && contentViewLayout.editorFormatSelectionSnapshot
                && Boolean(contentViewLayout.editorFormatSelectionSnapshot.valid))
            return contentViewLayout.editorFormatSelectionSnapshot;
        return contentViewLayout.liveEditorFormatSelectionState();
    }
    function liveEditorFormatSelectionState() {
        const selectionLength = Math.max(0, Math.floor(Number(contentsTextEditor.editorSelectionLength) || 0));
        return {
            "documentText": contentsTextEditor.editorDocumentText,
            "selectionLength": selectionLength,
            "selectionStart": Math.max(0, Math.floor(Number(contentsTextEditor.editorSelectionStart) || 0)),
            "selectedText": contentsTextEditor.editorSelectedText,
            "valid": selectionLength > 0
        };
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
            contentViewLayout.editorFormatContextMenuPointerActive = false;
            return false;
        }

        if (!contentViewLayout.editorFormatSelectionSnapshot
                || !Boolean(contentViewLayout.editorFormatSelectionSnapshot.valid))
            contentViewLayout.rememberEditorFormatSelectionSnapshot();
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
            return contentViewLayout.applyEditorFormatTag("bold", true);
        if (normalizedEventName === "editor.format.italic")
            return contentViewLayout.applyEditorFormatTag("italic", true);
        if (normalizedEventName === "editor.format.underline")
            return contentViewLayout.applyEditorFormatTag("underline", true);
        if (normalizedEventName === "editor.format.strikethrough")
            return contentViewLayout.applyEditorFormatTag("strikethrough", true);
        if (normalizedEventName === "editor.format.highlight")
            return contentViewLayout.applyEditorFormatTag("highlight", true);
        return false;
    }

    Layout.fillHeight: true
    Layout.fillWidth: true

    Rectangle {
        anchors.fill: parent
        color: contentViewLayout.displayColor

        RowLayout {
            anchors.fill: parent
            enabled: !contentViewLayout.calendarOverlayActive
            spacing: LV.Theme.gapNone
            visible: !contentViewLayout.calendarOverlayActive

            ContentsView.Gutter {
                Layout.fillHeight: true
                Layout.preferredWidth: visible ? implicitWidth : 0
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
                visible: contentViewLayout.noteEditorSurfaceVisible && contentViewLayout.gutterVisible
            }

            ContentsView.ImageEditor {
                id: contentsImageEditor

                Layout.fillHeight: true
                Layout.fillWidth: true
                resourceEntry: contentViewLayout.currentResourceEntry
                visible: contentViewLayout.imageResourceSelected
            }

            Item {
                id: contentsTextEditorStack
                Layout.fillHeight: true
                Layout.fillWidth: true
                enabled: contentViewLayout.noteEditorSurfaceVisible
                visible: contentViewLayout.noteEditorSurfaceVisible

                ContentsView.TextEditor {
                    id: contentsTextEditor

                    anchors.fill: parent
                    clipboardEditorPaste: contentViewLayout.clipboardEditorPaste
                    editorInputCommandFilter: contentViewLayout.editorInputCommandFilter
                    enabled: contentViewLayout.noteEditorSurfaceVisible
                    editorReadOnly: contentViewLayout.editorReadOnly
                    inAppClipboard: contentViewLayout.inAppClipboard
                    noteBodyFilePath: contentViewLayout.editorSourceFilePath
                    noteEditorSession: contentViewLayout.noteEditorSession
                    objectName: "contentsDisplayTextEditor"
                    visible: contentViewLayout.noteEditorSurfaceVisible

                    onReadFinished: function(path) {
                        contentViewLayout.markEditorSessionFileReadyForRawPush(path);
                    }
                    onTextEdited: function() {
                        contentViewLayout.requestEditorModifiedRawPush(contentsTextEditor.editorDocumentText);
                    }
                    onSyncFinished: function(path) {
                        const syncedPath = path === undefined || path === null ? "" : String(path).trim();
                        if (syncedPath.length === 0
                                || syncedPath !== contentViewLayout.editorSourceFilePath) {
                            return;
                        }
                        if (contentViewLayout.noteEditorSurfaceVisible
                                && contentViewLayout.noteEditorSession
                                && contentViewLayout.noteEditorSession.requestEditorIdleRawPush !== undefined) {
                            contentViewLayout.noteEditorSession.requestEditorIdleRawPush(
                                        syncedPath,
                                        contentsTextEditor.editorDocumentText);
                        }
                    }
                    onEditorPlainTextRevisionChanged: {
                        contentViewLayout.clearEditorFormatSelectionSnapshot();
                    }
                    onEditorDocumentTextChanged: {
                        contentViewLayout.clearEditorFormatSelectionSnapshot();
                    }
                    onEditorSelectedTextChanged: contentViewLayout.rememberEditorFormatSelectionSnapshot()
                    onEditorSelectionLengthChanged: contentViewLayout.rememberEditorFormatSelectionSnapshot()
                    onEditorSelectionStartChanged: contentViewLayout.rememberEditorFormatSelectionSnapshot()

                    TapHandler {
                        id: editorFormatContextMenuTapHandler

                        acceptedButtons: Qt.RightButton
                        acceptedModifiers: Qt.KeyboardModifierMask
                        enabled: contentViewLayout.editorFormatContextMenuAvailable
                        gesturePolicy: TapHandler.DragThreshold
                        grabPermissions: PointerHandler.ApprovesTakeOverByAnything

                        onPressedChanged: {
                            if (pressed) {
                                contentViewLayout.editorFormatContextMenuPointerActive = true;
                            } else {
                                Qt.callLater(function () {
                                    if (!editorFormatContextMenu.opened)
                                        contentViewLayout.editorFormatContextMenuPointerActive = false;
                                });
                            }
                        }

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

            }

            ContentsView.Minimap {
                Layout.fillHeight: true
                Layout.preferredWidth: visible ? implicitWidth : 0
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
                visible: contentViewLayout.noteEditorSurfaceVisible && contentViewLayout.minimapVisible
            }
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: StandardKey.Bold

            onActivated: contentViewLayout.applyEditorFormatTag("bold")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: StandardKey.Italic

            onActivated: contentViewLayout.applyEditorFormatTag("italic")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: StandardKey.Underline

            onActivated: contentViewLayout.applyEditorFormatTag("underline")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: "Ctrl+Shift+X"

            onActivated: contentViewLayout.applyEditorFormatTag("strikethrough")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: "Meta+Shift+X"

            onActivated: contentViewLayout.applyEditorFormatTag("strikethrough")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: "Ctrl+Shift+E"

            onActivated: contentViewLayout.applyEditorFormatTag("highlight")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: "Meta+Shift+E"

            onActivated: contentViewLayout.applyEditorFormatTag("highlight")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: "Ctrl+Shift+B"

            onActivated: contentViewLayout.applyEditorFormatTag("break")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: "Meta+Shift+B"

            onActivated: contentViewLayout.applyEditorFormatTag("break")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: "Ctrl+Shift+C"

            onActivated: contentViewLayout.applyEditorFormatTag("callout")
        }

        Shortcut {
            autoRepeat: false
            context: Qt.WindowShortcut
            enabled: contentViewLayout.editorCommandShortcutEnabled()
            sequence: "Meta+Shift+C"

            onActivated: contentViewLayout.applyEditorFormatTag("callout")
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
            onClosed: {
                contentViewLayout.editorFormatContextMenuPointerActive = false;
                contentViewLayout.clearEditorFormatSelectionSnapshot();
            }
        }

        Connections {
            target: contentViewLayout.noteEditorSession

            function onEditorDocumentTextPulled(noteId, editorDocumentText) {
                contentViewLayout.applyPulledEditorDocumentText(noteId, editorDocumentText);
            }
        }

        Item {
            id: calendarOverlayStack

            anchors.fill: parent
            enabled: contentViewLayout.calendarOverlayActive
            visible: contentViewLayout.calendarOverlayActive
            z: 4

            Rectangle {
                anchors.fill: parent
                color: contentViewLayout.displayColor
            }

            CalendarView.DayCalendarPage {
                anchors.fill: parent
                dayCalendarController: contentViewLayout.dayCalendarController
                visible: contentViewLayout.dayCalendarOverlayVisible

                onNoteOpenRequested: function (noteId) {
                    contentViewLayout.openCalendarNote(noteId);
                }
                onViewHookRequested: function (reason) {
                    contentViewLayout.requestViewHook("calendar.day." + reason);
                }
            }
            CalendarView.WeekCalendarPage {
                anchors.fill: parent
                weekCalendarController: contentViewLayout.weekCalendarController
                visible: contentViewLayout.weekCalendarOverlayVisible

                onNoteOpenRequested: function (noteId) {
                    contentViewLayout.openCalendarNote(noteId);
                }
                onViewHookRequested: function (reason) {
                    contentViewLayout.requestViewHook("calendar.week." + reason);
                }
            }
            CalendarView.MonthCalendarPage {
                anchors.fill: parent
                monthCalendarController: contentViewLayout.monthCalendarController
                visible: contentViewLayout.monthCalendarOverlayVisible

                onNoteOpenRequested: function (noteId) {
                    contentViewLayout.openCalendarNote(noteId);
                }
                onViewHookRequested: function (reason) {
                    contentViewLayout.requestViewHook("calendar.month." + reason);
                }
            }
            CalendarView.YearCalendarPage {
                anchors.fill: parent
                visible: contentViewLayout.yearCalendarOverlayVisible
                yearCalendarController: contentViewLayout.yearCalendarController

                onMonthCalendarOpenRequested: function (year, month, selectedDateIso) {
                    contentViewLayout.openMonthCalendarFromYear(year, month, selectedDateIso);
                }
                onViewHookRequested: function (reason) {
                    contentViewLayout.requestViewHook("calendar.year." + reason);
                }
            }
        }
    }
}
