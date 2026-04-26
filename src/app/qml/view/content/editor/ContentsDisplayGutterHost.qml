pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

ContentsGutterLayer {
    id: gutterHost

    required property var contentsView

    Layout.fillHeight: true
    Layout.maximumWidth: contentsView.effectiveGutterWidth
    Layout.minimumWidth: contentsView.effectiveGutterWidth
    Layout.preferredWidth: contentsView.effectiveGutterWidth
    activeLineNumberColor: contentsView.activeLineNumberColor
    currentCursorLineNumber: contentsView.currentCursorLineNumber
    editorLineHeight: contentsView.editorLineHeight
    effectiveGutterMarkers: contentsView.effectiveGutterMarkers
    gutterColor: contentsView.gutterColor
    gutterCommentMarkerOffset: contentsView.gutterCommentMarkerOffset
    gutterCommentRailLeft: contentsView.gutterCommentRailLeft
    gutterIconRailLeft: contentsView.gutterIconRailLeft
    gutterIconRailWidth: contentsView.gutterIconRailWidth
    lineNumberColor: contentsView.lineNumberColor
    lineNumberColumnLeft: contentsView.effectiveLineNumberColumnLeft
    lineNumberColumnTextWidth: contentsView.effectiveLineNumberColumnTextWidth
    lineYResolver: function (lineNumber) {
        return contentsView.gutterLineY(lineNumber);
    }
    markerHeightResolver: function (markerSpec) {
        return contentsView.markerHeight(markerSpec);
    }
    markerYResolver: function (markerSpec) {
        return contentsView.markerY(markerSpec);
    }
    visible: contentsView.showEditorGutter && contentsView.noteDocumentParseMounted
    visibleLineNumbersModel: contentsView.visibleGutterLineEntries
}
