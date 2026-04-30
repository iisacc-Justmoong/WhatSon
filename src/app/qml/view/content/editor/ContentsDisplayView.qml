pragma ComponentBehavior: Bound
import QtQuick
import LVRS 1.0 as LV

Item {
    id: contentsView
    objectName: contentsView.mobileHost ? "mobileContentsDisplayView" : "contentsDisplayView"

    property var contentViewModel: null
    property bool mobileHost: false
    property color displayColor: "transparent"
    property color gutterColor: "transparent"
    property var libraryHierarchyViewModel: null
    property var editorViewModeViewModel: null
    property string editorText: ""
    property string selectedNoteId: ""
    property string selectedNoteDirectoryPath: ""
    property bool hasSelectedNote: selectedNoteId.length > 0
    property bool formattedPreviewRequested: false
    property bool resourceViewerRequested: false
    property bool showDedicatedResourceViewer: false
    property bool showFormattedTextRenderer: false
    property bool showStructuredDocumentFlow: false
    property bool showEditorGutter: false
    property bool showPrintEditorLayout: false
    property bool showPrintMarginGuides: false
    property bool showPrintModeActive: false
    property bool noteDocumentMountFailureVisible: false
    property string noteDocumentMountFailureReason: ""
    property string noteDocumentMountFailureMessage: ""
    property bool noteDocumentExceptionVisible: false
    property string noteDocumentExceptionReason: ""
    property string noteDocumentExceptionTitle: ""
    property string noteDocumentExceptionMessage: ""

    signal editorTextEdited(string text)
    signal viewHookRequested

    Rectangle {
        anchors.fill: parent
        color: contentsView.displayColor
    }

    Text {
        anchors.centerIn: parent
        visible: false
        text: "ContentsDisplayView shell"
        color: LV.Theme.descriptionColor
    }
}
