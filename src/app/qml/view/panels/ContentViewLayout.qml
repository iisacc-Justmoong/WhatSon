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
    property bool editorRawPushDeferredForInputComposition: false
    property string editorRawPushDeferredReason: ""
    property string editorRawPushDeferredSourceFilePath: ""
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
    property var editorFontFamilyProvider: null
    property var sidebarHierarchyController: null
    property bool weekCalendarOverlayVisible: false
    property var weekCalendarController: null
    property bool yearCalendarOverlayVisible: false
    property var yearCalendarController: null
    readonly property int editorToolbarStyleContextRefreshInterval: 48
    property bool editorToolbarStyleContextRefreshQueued: false
    property bool editorToolbarStyleContextRefreshArmQueued: false

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
            contentViewLayout.requestEditorToolbarStyleContextRefresh(true);
        }
        return replaced;
    }
    function applyEditorStyleTagStyle(styleValue, allowSelectionSnapshot) {
        if (!contentViewLayout.noteEditorSession
                || contentViewLayout.editorReadOnly
                || contentViewLayout.noteEditorSession.insertStyleTagIntoSource === undefined)
            return false;

        const selectionState = contentViewLayout.editorFormatSelectionForCommand(
                    Boolean(allowSelectionSnapshot));
        const styleResult = contentViewLayout.noteEditorSession.insertStyleTagIntoSource(
                    styleValue,
                    selectionState.documentText,
                    selectionState.selectionStart,
                    selectionState.selectionLength,
                    selectionState.selectedText);
        if (!styleResult || !Boolean(styleResult.valid))
            return false;

        const editorDocumentText = styleResult.editorDocumentText !== undefined
                && styleResult.editorDocumentText !== null
                ? String(styleResult.editorDocumentText)
                : String(styleResult.bodySourceText);
        const replaced = contentsTextEditor.replaceEditorDocumentText(
                    editorDocumentText,
                    Number(styleResult.cursorPosition) || 0);
        if (replaced) {
            contentViewLayout.clearEditorFormatSelectionSnapshot();
            contentViewLayout.requestEditorToolbarStyleContextRefresh(true);
            contentViewLayout.requestViewHook("editor.toolbar.style." + String(styleValue));
        }
        return replaced;
    }
    function applyEditorStyleTagFont(fontFamily, allowSelectionSnapshot) {
        if (!contentViewLayout.noteEditorSession
                || contentViewLayout.editorReadOnly
                || contentViewLayout.noteEditorSession.insertStyleFontTagIntoSource === undefined)
            return false;

        const selectionState = contentViewLayout.editorFormatSelectionForCommand(
                    Boolean(allowSelectionSnapshot));
        const styleResult = contentViewLayout.noteEditorSession.insertStyleFontTagIntoSource(
                    fontFamily,
                    selectionState.documentText,
                    selectionState.selectionStart,
                    selectionState.selectionLength,
                    selectionState.selectedText);
        if (!styleResult || !Boolean(styleResult.valid))
            return false;

        const editorDocumentText = styleResult.editorDocumentText !== undefined
                && styleResult.editorDocumentText !== null
                ? String(styleResult.editorDocumentText)
                : String(styleResult.bodySourceText);
        const replaced = contentsTextEditor.replaceEditorDocumentText(
                    editorDocumentText,
                    Number(styleResult.cursorPosition) || 0);
        if (replaced) {
            contentViewLayout.clearEditorFormatSelectionSnapshot();
            contentViewLayout.requestEditorToolbarStyleContextRefresh(true);
            contentViewLayout.requestViewHook("editor.toolbar.font." + String(fontFamily));
        }
        return replaced;
    }
    function applyEditorStyleTagFontSize(fontSize, allowSelectionSnapshot) {
        if (!contentViewLayout.noteEditorSession
                || contentViewLayout.editorReadOnly
                || contentViewLayout.noteEditorSession.insertStyleFontSizeTagIntoSource === undefined)
            return false;

        const selectionState = contentViewLayout.editorFormatSelectionForCommand(
                    Boolean(allowSelectionSnapshot));
        const styleResult = contentViewLayout.noteEditorSession.insertStyleFontSizeTagIntoSource(
                    fontSize,
                    selectionState.documentText,
                    selectionState.selectionStart,
                    selectionState.selectionLength,
                    selectionState.selectedText);
        if (!styleResult || !Boolean(styleResult.valid))
            return false;

        const editorDocumentText = styleResult.editorDocumentText !== undefined
                && styleResult.editorDocumentText !== null
                ? String(styleResult.editorDocumentText)
                : String(styleResult.bodySourceText);
        const replaced = contentsTextEditor.replaceEditorDocumentText(
                    editorDocumentText,
                    Number(styleResult.cursorPosition) || 0);
        if (replaced) {
            contentViewLayout.clearEditorFormatSelectionSnapshot();
            contentViewLayout.requestEditorToolbarStyleContextRefresh(true);
            contentViewLayout.requestViewHook("editor.toolbar.font-size." + String(fontSize));
        }
        return replaced;
    }
    function applyEditorStyleTagBackground(backgroundColor, allowSelectionSnapshot) {
        if (!contentViewLayout.noteEditorSession
                || contentViewLayout.editorReadOnly
                || contentViewLayout.noteEditorSession.insertStyleBackgroundTagIntoSource === undefined)
            return false;

        const selectionState = contentViewLayout.editorFormatSelectionForCommand(
                    Boolean(allowSelectionSnapshot));
        const styleResult = contentViewLayout.noteEditorSession.insertStyleBackgroundTagIntoSource(
                    backgroundColor,
                    selectionState.documentText,
                    selectionState.selectionStart,
                    selectionState.selectionLength,
                    selectionState.selectedText);
        if (!styleResult || !Boolean(styleResult.valid))
            return false;

        const editorDocumentText = styleResult.editorDocumentText !== undefined
                && styleResult.editorDocumentText !== null
                ? String(styleResult.editorDocumentText)
                : String(styleResult.bodySourceText);
        const replaced = contentsTextEditor.replaceEditorDocumentText(
                    editorDocumentText,
                    Number(styleResult.cursorPosition) || 0);
        if (replaced) {
            contentViewLayout.clearEditorFormatSelectionSnapshot();
            contentViewLayout.requestEditorToolbarStyleContextRefresh(true);
            contentViewLayout.requestViewHook("editor.toolbar.highlight-color." + String(backgroundColor));
        }
        return replaced;
    }
    function refreshEditorToolbarStyleContext() {
        contentViewLayout.editorToolbarStyleContextRefreshQueued = false;
        contentViewLayout.editorToolbarStyleContextRefreshArmQueued = false;
        editorToolbarStyleContextRefreshTimer.stop();
        if (!contentEditorToolbar || contentEditorToolbar.applyStyleContext === undefined)
            return false;

        if (!contentViewLayout.noteEditorSurfaceVisible
                || !contentsTextEditor
                || !contentViewLayout.noteEditorSession
                || contentViewLayout.noteEditorSession.toolbarStyleContextAtCursor === undefined) {
            contentEditorToolbar.applyStyleContext({});
            return false;
        }

        const styleContext = contentViewLayout.noteEditorSession.toolbarStyleContextAtCursor(
                    contentsTextEditor.editorDocumentText,
                    contentsTextEditor.cursorPosition,
                    contentsTextEditor.editorSelectionLength);
        if (!styleContext || !Boolean(styleContext.valid)) {
            contentEditorToolbar.applyStyleContext({});
            return false;
        }

        contentEditorToolbar.applyStyleContext(styleContext);
        return true;
    }
    function requestEditorToolbarStyleContextRefresh(immediate) {
        if (Boolean(immediate))
            return contentViewLayout.refreshEditorToolbarStyleContext();

        if (contentViewLayout.editorToolbarStyleContextRefreshQueued)
            return true;

        contentViewLayout.editorToolbarStyleContextRefreshQueued = true;
        if (!contentViewLayout.editorToolbarStyleContextRefreshArmQueued) {
            contentViewLayout.editorToolbarStyleContextRefreshArmQueued = true;
            Qt.callLater(contentViewLayout.armEditorToolbarStyleContextRefresh);
        }
        return true;
    }
    function armEditorToolbarStyleContextRefresh() {
        contentViewLayout.editorToolbarStyleContextRefreshArmQueued = false;
        if (!contentViewLayout.editorToolbarStyleContextRefreshQueued)
            return false;

        editorToolbarStyleContextRefreshTimer.restart();
        return true;
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
    function editorInputCompositionActive() {
        return !!(contentsTextEditor
                  && contentsTextEditor.inputMethodComposing !== undefined
                  && Boolean(contentsTextEditor.inputMethodComposing));
    }
    function deferEditorRawPushForInputComposition(reason, editorFilePath) {
        const normalizedPath = editorFilePath === undefined || editorFilePath === null
                ? contentViewLayout.editorSourceFilePath
                : String(editorFilePath).trim();
        if (normalizedPath.length === 0)
            return false;

        contentViewLayout.editorRawPushDeferredForInputComposition = true;
        contentViewLayout.editorRawPushDeferredSourceFilePath = normalizedPath;
        const normalizedReason = reason === undefined || reason === null
                ? ""
                : String(reason).trim();
        if (normalizedReason === "modified-count"
                || contentViewLayout.editorRawPushDeferredReason.length === 0)
            contentViewLayout.editorRawPushDeferredReason = normalizedReason.length > 0
                    ? normalizedReason
                    : "idle";
        return true;
    }
    function flushDeferredEditorRawPushAfterInputComposition() {
        if (!contentViewLayout.editorRawPushDeferredForInputComposition
                || contentViewLayout.editorInputCompositionActive())
            return false;

        const deferredPath = contentViewLayout.editorRawPushDeferredSourceFilePath;
        const deferredReason = contentViewLayout.editorRawPushDeferredReason;
        contentViewLayout.editorRawPushDeferredForInputComposition = false;
        contentViewLayout.editorRawPushDeferredReason = "";
        contentViewLayout.editorRawPushDeferredSourceFilePath = "";

        if (deferredPath.length === 0
                || deferredPath !== contentViewLayout.editorSourceFilePath)
            return false;
        if (deferredReason === "modified-count")
            return contentViewLayout.requestEditorModifiedRawPush(
                        contentsTextEditor.editorDocumentText,
                        contentsTextEditor.editorPlainTextRevision + 1);
        return contentViewLayout.requestEditorIdleRawPush(deferredPath);
    }
    function requestEditorModifiedRawPush(editorDocumentText, editorDocumentRevision) {
        const modifiedRevision = Math.max(0, Math.floor(Number(editorDocumentRevision) || 0));
        if (contentViewLayout.editorInputCompositionActive())
            return contentViewLayout.deferEditorRawPushForInputComposition("modified-count", contentViewLayout.editorSourceFilePath);
        if (contentViewLayout.noteEditorSurfaceVisible
                && !contentViewLayout.editorApplyingPulledDocumentText
                && !contentsTextEditor.editorProgrammaticDocumentApplying
                && !contentsTextEditor.editorFrameViewportRefreshApplying
                && contentViewLayout.noteEditorSession
                && contentViewLayout.noteEditorSession.requestEditorModifiedCountRawPush !== undefined
                && contentViewLayout.editorSourceFilePath.length > 0) {
            contentViewLayout.noteEditorSession.requestEditorModifiedCountRawPush(
                        contentViewLayout.editorSourceFilePath,
                        modifiedRevision > 0 ? modifiedRevision : contentsTextEditor.editorPlainTextRevision + 1,
                        editorDocumentText === undefined || editorDocumentText === null
                        ? contentsTextEditor.editorDocumentText
                        : String(editorDocumentText));
            return true;
        }
        return false;
    }
    function requestEditorIdleRawPush(editorFilePath) {
        const syncedPath = editorFilePath === undefined || editorFilePath === null
                ? ""
                : String(editorFilePath).trim();
        if (syncedPath.length === 0
                || syncedPath !== contentViewLayout.editorSourceFilePath)
            return false;
        if (contentViewLayout.editorInputCompositionActive())
            return contentViewLayout.deferEditorRawPushForInputComposition("idle", syncedPath);
        if (contentViewLayout.noteEditorSurfaceVisible
                && !contentViewLayout.editorApplyingPulledDocumentText
                && !contentsTextEditor.editorProgrammaticDocumentApplying
                && !contentsTextEditor.editorFrameViewportRefreshApplying
                && contentViewLayout.noteEditorSession
                && contentViewLayout.noteEditorSession.requestEditorIdleRawPush !== undefined) {
            contentViewLayout.noteEditorSession.requestEditorIdleRawPush(
                        syncedPath,
                        contentsTextEditor.editorDocumentText);
            return true;
        }
        return false;
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

    Timer {
        id: editorToolbarStyleContextRefreshTimer

        interval: contentViewLayout.editorToolbarStyleContextRefreshInterval
        repeat: false

        onTriggered: contentViewLayout.refreshEditorToolbarStyleContext()
    }

    Rectangle {
        anchors.fill: parent
        color: contentViewLayout.displayColor

        LV.VStack {
            id: contentViewStack

            anchors.fill: parent
            enabled: !contentViewLayout.calendarOverlayActive
            spacing: LV.Theme.gapNone
            visible: !contentViewLayout.calendarOverlayActive

            ContentEditorToolbar {
                id: contentEditorToolbar

                Layout.fillWidth: true
                Layout.preferredHeight: visible ? implicitHeight : 0
                editorReadOnly: contentViewLayout.editorReadOnly
                fontFamilyProvider: contentViewLayout.editorFontFamilyProvider
                highlightColorProvider: contentViewLayout.noteEditorSession
                visible: contentViewLayout.noteEditorSurfaceVisible

                onFormatTagRequested: function(tagName) {
                    contentViewLayout.applyEditorFormatTag(tagName);
                }
                onStyleTagStyleRequested: function(styleValue) {
                    contentViewLayout.applyEditorStyleTagStyle(styleValue);
                }
                onFontFamilyRequested: function(fontFamily) {
                    contentViewLayout.applyEditorStyleTagFont(fontFamily);
                }
                onFontSizeRequested: function(fontSize) {
                    contentViewLayout.applyEditorStyleTagFontSize(fontSize);
                }
                onHighlightColorRequested: function(colorName, colorHex) {
                    contentViewLayout.applyEditorStyleTagBackground(colorHex);
                }
                onToolbarActionRequested: function(actionName) {
                    contentViewLayout.requestViewHook(actionName);
                }
            }

            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: LV.Theme.gapNone

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
                            contentViewLayout.requestEditorToolbarStyleContextRefresh(true);
                        }
                        onTextEdited: function() {
                            contentViewLayout.requestEditorToolbarStyleContextRefresh(false);
                        }
                        onEditorDocumentEdited: function(documentText, documentRevision) {
                            contentViewLayout.requestEditorModifiedRawPush(documentText, documentRevision);
                            contentViewLayout.requestEditorToolbarStyleContextRefresh(false);
                        }
                        onSyncFinished: function(path) {
                            contentViewLayout.requestEditorIdleRawPush(path);
                        }
                        onEditorPlainTextRevisionChanged: {
                            contentViewLayout.clearEditorFormatSelectionSnapshot();
                            contentViewLayout.requestEditorToolbarStyleContextRefresh(false);
                        }
                        onEditorDocumentTextChanged: {
                            contentViewLayout.clearEditorFormatSelectionSnapshot();
                            contentViewLayout.requestEditorToolbarStyleContextRefresh(false);
                            if (contentViewLayout.editorRawPushDeferredForInputComposition
                                    && !contentsTextEditor.inputMethodComposing)
                                Qt.callLater(contentViewLayout.flushDeferredEditorRawPushAfterInputComposition);
                        }
                        onInputMethodComposingChanged: {
                            if (!contentsTextEditor.inputMethodComposing)
                                Qt.callLater(contentViewLayout.flushDeferredEditorRawPushAfterInputComposition);
                        }
                        onCursorPositionChanged: contentViewLayout.requestEditorToolbarStyleContextRefresh(false)
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
