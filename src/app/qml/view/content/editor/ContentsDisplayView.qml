pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: contentsDisplayView

    property var contentController: null
    property color displayColor: "transparent"
    property var editorViewModeController: null
    readonly property bool editorCustomTextInputEnabled: false
    readonly property bool editorTagManagementInputEnabled: true
    property int editorTopInsetOverride: -1
    property int frameHorizontalInsetOverride: -1
    property color gutterColor: "transparent"
    property int gutterWidthOverride: -1
    property var htmlTokens: editorPresentationProjection.htmlTokens
    property var libraryHierarchyController: null
    property int lineNumberColumnLeftOverride: -1
    property int lineNumberColumnTextWidthOverride: -1
    property bool minimapVisible: true
    property bool mobileHost: false
    property var normalizedHtmlBlocks: editorPresentationProjection.normalizedHtmlBlocks
    property var noteListModel: null
    property var panelController: null
    property bool paperPaletteEnabled: false
    property var resourcesImportController: null
    property var sidebarHierarchyController: null
    readonly property var currentNoteEntry: contentsDisplayView.noteEntryFromModel()
    readonly property string currentNoteId: contentsDisplayView.stringValue(
                                                contentsDisplayView.currentNoteEntry,
                                                "noteId",
                                                contentsDisplayView.modelStringProperty("currentNoteId"))
    readonly property string currentNoteDirectoryPath: contentsDisplayView.stringValue(
                                                           contentsDisplayView.currentNoteEntry,
                                                           "noteDirectoryPath",
                                                           contentsDisplayView.modelStringProperty("currentNoteDirectoryPath"))
    readonly property string currentRawBodyText: contentsDisplayView.stringValue(
                                                    contentsDisplayView.currentNoteEntry,
                                                    "bodyText",
                                                    contentsDisplayView.modelStringProperty("currentBodyText"))
    readonly property bool noteDocumentParseMounted: contentsDisplayView.currentNoteId.length > 0
    readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: contentsDisplayView.noteDocumentParseMounted
    readonly property bool structuredBlockBackgroundRefreshEnabled: contentsDisplayView.noteDocumentParseMounted

    signal editorTextEdited(string text)
    signal viewHookRequested

    function commitEditedSourceText(text) {
        const nextText = text === undefined || text === null ? "" : String(text);
        const changed = editorSession.commitRawEditorTextMutation(nextText);
        if (changed) {
            if (contentsDisplayView.contentController
                    && contentsDisplayView.contentController.saveCurrentBodyText !== undefined) {
                contentsDisplayView.contentController.saveCurrentBodyText(nextText);
            }
            contentsDisplayView.editorTextEdited(nextText);
        }
    }

    function applyDocumentSourceMutation(payload) {
        if (payload === null || payload === undefined || typeof payload !== "object")
            return false;
        if (payload.nextSourceText === undefined || payload.nextSourceText === null)
            return false;
        contentsDisplayView.commitEditedSourceText(String(payload.nextSourceText));
        return true;
    }

    function commitDocumentPresentationRefresh() {
        editorPresentationProjection.sourceText = editorSession.editorText;
    }

    function modelStringProperty(propertyName) {
        if (!contentsDisplayView.noteListModel
                || contentsDisplayView.noteListModel[propertyName] === undefined
                || contentsDisplayView.noteListModel[propertyName] === null) {
            return "";
        }
        return String(contentsDisplayView.noteListModel[propertyName]);
    }

    function noteEntryFromModel() {
        if (contentsDisplayView.noteListModel
                && contentsDisplayView.noteListModel.currentNoteEntry !== undefined
                && contentsDisplayView.noteListModel.currentNoteEntry !== null
                && typeof contentsDisplayView.noteListModel.currentNoteEntry === "object") {
            return contentsDisplayView.noteListModel.currentNoteEntry;
        }
        return ({});
    }

    function requestViewHook(reason) {
        if (contentsDisplayView.panelController && contentsDisplayView.panelController.requestControllerHook)
            contentsDisplayView.panelController.requestControllerHook(reason !== undefined ? String(reason) : "manual");
        contentsDisplayView.viewHookRequested();
    }

    function requestEditorSelectionContextMenuFromPointer(pointerKind) {
        const normalizedKind = pointerKind === undefined || pointerKind === null ? "" : String(pointerKind);
        return normalizedKind === "right-click" || normalizedKind === "long-press";
    }

    function scheduleStructuredDocumentOpenLayoutRefresh() {
        contentsDisplayView.commitDocumentPresentationRefresh();
    }

    function stringValue(payload, key, fallback) {
        if (payload !== null
                && payload !== undefined
                && typeof payload === "object"
                && payload[key] !== undefined
                && payload[key] !== null) {
            return String(payload[key]);
        }
        return fallback === undefined || fallback === null ? "" : String(fallback);
    }

    function terminalBodyClickSourceOffset() {
        return editorSession.editorText.length;
    }

    function syncSessionFromCurrentNote() {
        if (contentsDisplayView.currentNoteId.length === 0)
            return;
        editorSession.requestSyncEditorTextFromSelection(
                    contentsDisplayView.currentNoteId,
                    contentsDisplayView.currentRawBodyText,
                    contentsDisplayView.currentNoteId,
                    contentsDisplayView.currentNoteDirectoryPath);
    }

    clip: true

    Component.onCompleted: contentsDisplayView.syncSessionFromCurrentNote()
    onCurrentNoteDirectoryPathChanged: contentsDisplayView.syncSessionFromCurrentNote()
    onCurrentNoteIdChanged: contentsDisplayView.syncSessionFromCurrentNote()
    onCurrentRawBodyTextChanged: contentsDisplayView.syncSessionFromCurrentNote()
    onNoteListModelChanged: contentsDisplayView.syncSessionFromCurrentNote()

    Connections {
        target: contentsDisplayView.noteListModel
        ignoreUnknownSignals: true

        function onCurrentBodyTextChanged() {
            contentsDisplayView.syncSessionFromCurrentNote();
        }

        function onCurrentIndexChanged() {
            contentsDisplayView.syncSessionFromCurrentNote();
        }

        function onCurrentNoteDirectoryPathChanged() {
            contentsDisplayView.syncSessionFromCurrentNote();
        }

        function onCurrentNoteEntryChanged() {
            contentsDisplayView.syncSessionFromCurrentNote();
        }

        function onCurrentNoteIdChanged() {
            contentsDisplayView.syncSessionFromCurrentNote();
        }
    }

    ContentsEditorSessionController {
        id: editorSession

        objectName: "contentsDisplayEditorSession"
    }

    ContentsEditorPresentationProjection {
        id: editorPresentationProjection

        objectName: "contentsDisplayPresentationProjection"
        paperPaletteEnabled: contentsDisplayView.paperPaletteEnabled
        sourceText: editorSession.editorText
    }

    Rectangle {
        anchors.fill: parent
        color: contentsDisplayView.displayColor
    }

    ContentsStructuredDocumentFlow {
        id: structuredDocumentFlow

        anchors.fill: parent
        editorSurfaceHtml: editorPresentationProjection.editorSurfaceHtml
        htmlTokens: editorPresentationProjection.htmlTokens
        normalizedHtmlBlocks: editorPresentationProjection.normalizedHtmlBlocks
        objectName: "contentsDisplayStructuredDocumentFlow"
        paperPaletteEnabled: contentsDisplayView.paperPaletteEnabled
        sourceText: editorSession.editorText
        textColor: LV.Theme.bodyColor
        visible: contentsDisplayView.noteDocumentParseMounted

        onSourceTextEdited: function (text) {
            contentsDisplayView.commitEditedSourceText(text);
        }

        onViewHookRequested: {
            contentsDisplayView.viewHookRequested();
        }
    }

    Text {
        anchors.centerIn: parent
        color: LV.Theme.descriptionColor
        font.family: LV.Theme.fontBody
        font.pixelSize: LV.Theme.textBody
        text: "No document opened"
        visible: !contentsDisplayView.noteDocumentParseMounted
    }
}
