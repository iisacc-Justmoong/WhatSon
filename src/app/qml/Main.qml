import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.ApplicationWindow {
    id: window

    readonly property string activeFolderId: {
        if (selectedFolderIndex < 0 || selectedFolderIndex >= folderItems.length)
            return "all";
        return folderItems[selectedFolderIndex].id;
    }
    readonly property var filteredNotes: {
        var results = [];
        var query = searchQuery.trim().toLowerCase();

        for (var i = 0; i < noteItems.length; ++i) {
            var note = noteItems[i];
            var folderMatch = activeFolderId === "all" || note.folder === activeFolderId;
            if (!folderMatch)
                continue;
            if (query.length > 0) {
                var inTitle = String(note.title).toLowerCase().indexOf(query) !== -1;
                var inPreview = String(note.preview).toLowerCase().indexOf(query) !== -1;
                if (!inTitle && !inPreview)
                    continue;
            }

            results.push(note);
        }

        return results;
    }
    readonly property var folderItems: [
        {
            "id": "all",
            "name": "All Notes",
            "count": 1842
        },
        {
            "id": "creative",
            "name": "Creative",
            "count": 286
        },
        {
            "id": "brand",
            "name": "Brand",
            "count": 194
        },
        {
            "id": "knowledge",
            "name": "Knowledge",
            "count": 1179
        },
        {
            "id": "archive",
            "name": "Archive",
            "count": 183
        }
    ]
    readonly property var noteItems: [
        {
            "folder": "creative",
            "title": "Spring 2026 Campaign Message Architecture",
            "preview": "Proposes reframing customer problems as information flow issues rather than feature gaps, and tying the narrative to brand trust.",
            "modified": "Today 09:18",
            "body": "Document Title: Spring 2026 Campaign Message Architecture\n\nCore Narrative:\nThe customer problem does not start from missing features, but from the absence of a trustworthy information flow. This document defines message principles that combine product value and brand trust in one context.\n\nExecution:\n1. Define customer problems as decision delay, not feature deficiency.\n2. Reframe product features from an information reliability perspective.\n3. Attach evidence documents to each message sentence."
        },
        {
            "folder": "brand",
            "title": "Brand Voice Guide v2",
            "preview": "Applies evidence-first sentence structure and removes exaggerated expressions as shared organizational rules.",
            "modified": "Yesterday",
            "body": "Brand Voice Guide v2\n\nRules:\n- Put evidence before claims.\n- Limit absolute promotional language.\n- Present technical description with business meaning."
        },
        {
            "folder": "knowledge",
            "title": "Market Research 2026 Q1",
            "preview": "For core customer groups, trust decisions respond more to information transparency than to price.",
            "modified": "2 days ago",
            "body": "Market Research 2026 Q1\n\nSummary:\n- Information clarity score showed strong correlation with conversion rate.\n- Visibility of comparison tables and evidence links was critical on product pages."
        },
        {
            "folder": "knowledge",
            "title": "AI Usage Policy",
            "preview": "Mandates source citation, reviewer assignment, and pre-release risk checks for automated document generation.",
            "modified": "Last week",
            "body": "AI Usage Policy\n\nMandatory Clauses:\n1. Record evidence sources for generated content.\n2. Assign a final reviewer for public documents.\n3. Run a pre-check for sensitive information."
        }
    ]
    property string searchQuery: ""
    property int selectedFolderIndex: 0
    readonly property var selectedNote: {
        if (filteredNotes.length === 0)
            return {
                "title": "New Note",
                "preview": "",
                "modified": "Now",
                "body": ""
            };

        var clampedIndex = Math.max(0, Math.min(selectedNoteIndex, filteredNotes.length - 1));
        return filteredNotes[clampedIndex];
    }
    property int selectedNoteIndex: 0

    autoAttachRuntimeEvents: false
    autoHookBackendUserEvents: false
    globalEventListenersEnabled: true
    height: 920
    navigationEnabled: false
    subtitle: "Business Creative, Branding, Information, Knowledge"
    title: "WhatSon Notes"
    visible: true
    width: 1520

    onSearchQueryChanged: selectedNoteIndex = 0
    onSelectedFolderIndexChanged: selectedNoteIndex = 0

    Item {
        anchors.fill: parent

        RowLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                id: sidebarPane

                Layout.fillHeight: true
                Layout.preferredWidth: 260
                color: LV.Theme.windowAlt

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.top: parent.top
                    color: LV.Theme.surfaceAlt
                    width: 1
                }
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LV.Theme.gap14
                    spacing: LV.Theme.gap10

                    LV.Label {
                        style: title2
                        text: "Folders"
                    }
                    LV.Label {
                        style: description
                        text: "iCloud"
                    }
                    Repeater {
                        model: window.folderItems

                        delegate: LV.ListItem {
                            required property int index
                            required property var modelData

                            detail: String(modelData.count)
                            label: modelData.name
                            selected: window.selectedFolderIndex === index
                            showChevron: false

                            onClicked: window.selectedFolderIndex = index
                        }
                    }
                    LV.Spacer {
                    }
                    LV.LabelButton {
                        text: "New Folder"
                        tone: LV.AbstractButton.Borderless
                    }
                }
            }
            Rectangle {
                id: notesListPane

                Layout.fillHeight: true
                Layout.preferredWidth: 380
                color: LV.Theme.surfaceSolid

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.top: parent.top
                    color: LV.Theme.surfaceAlt
                    width: 1
                }
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LV.Theme.gap14
                    spacing: LV.Theme.gap10

                    LV.Label {
                        style: title2
                        text: activeFolderId === "all" ? "All Notes" : folderItems[selectedFolderIndex].name
                    }
                    LV.InputField {
                        id: searchField

                        Layout.fillWidth: true
                        mode: searchMode
                        placeholderText: "Search notes"
                        text: window.searchQuery

                        onTextChanged: window.searchQuery = text
                    }
                    LV.Label {
                        style: caption
                        text: filteredNotes.length + " notes"
                    }
                    Flickable {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        clip: true
                        contentHeight: notesColumn.implicitHeight
                        contentWidth: width

                        Column {
                            id: notesColumn

                            spacing: LV.Theme.gap6
                            width: notesListPane.width - (LV.Theme.gap14 * 2)

                            Repeater {
                                model: window.filteredNotes

                                delegate: Rectangle {
                                    required property int index
                                    required property var modelData

                                    border.color: window.selectedNoteIndex === index ? LV.Theme.primary : LV.Theme.surfaceAlt
                                    border.width: 1
                                    color: window.selectedNoteIndex === index ? LV.Theme.accentOverlay : "transparent"
                                    implicitHeight: 114
                                    radius: LV.Theme.radiusMd
                                    width: notesColumn.width

                                    MouseArea {
                                        anchors.fill: parent

                                        onClicked: window.selectedNoteIndex = index
                                    }
                                    Column {
                                        anchors.fill: parent
                                        anchors.margins: LV.Theme.gap10
                                        spacing: LV.Theme.gap4

                                        LV.Label {
                                            elide: Text.ElideRight
                                            style: header2
                                            text: modelData.title
                                            width: parent.width
                                        }
                                        LV.Label {
                                            maximumLineCount: 2
                                            style: description
                                            text: modelData.preview
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        LV.Label {
                                            style: caption
                                            text: modelData.modified
                                            width: parent.width
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: editorPane

                Layout.fillHeight: true
                Layout.fillWidth: true
                color: LV.Theme.surfaceAlt

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: LV.Theme.gap16
                    spacing: LV.Theme.gap10

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: LV.Theme.gap8

                        LV.Label {
                            Layout.fillWidth: true
                            style: caption
                            text: selectedNote.modified
                        }
                        LV.LabelButton {
                            text: "Share"
                            tone: LV.AbstractButton.Default
                        }
                        LV.LabelButton {
                            text: "Add"
                            tone: LV.AbstractButton.Default
                        }
                    }
                    LV.InputField {
                        Layout.fillWidth: true
                        placeholderText: "Title"
                        text: selectedNote.title
                    }
                    LV.TextEditor {
                        id: editor

                        Layout.fillWidth: true
                        editorHeight: Math.max(420, editorPane.height - 220)
                        placeholderText: "Write your note"
                        showRenderedOutput: false
                        text: selectedNote.body
                    }
                }
            }
        }
    }
}
