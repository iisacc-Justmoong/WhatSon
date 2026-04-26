pragma ComponentBehavior: Bound

import QtQuick

ContentsMinimapLayer {
    id: minimapHost

    required property var contentsView
    required property var viewportCoordinator

    editorFlickable: contentsView.editorFlickable
    minimapBarWidthResolver: function (characterCount) {
        return viewportCoordinator.minimapBarWidth(
                    Number(characterCount) || 0,
                    contentsView.minimapResolvedTrackWidth);
    }
    minimapCurrentLineColor: contentsView.minimapCurrentLineColor
    minimapCurrentLineHeight: contentsView.minimapResolvedCurrentLineHeight
    minimapCurrentLineWidth: contentsView.minimapResolvedCurrentLineWidth
    minimapCurrentLineY: contentsView.minimapResolvedCurrentLineY
    minimapLineColor: contentsView.minimapLineColor
    minimapScrollable: contentsView.minimapScrollable
    minimapSilhouetteHeight: contentsView.minimapResolvedSilhouetteHeight
    minimapTrackInset: contentsView.minimapTrackInset
    minimapTrackWidth: contentsView.minimapTrackWidth
    minimapViewportFillColor: contentsView.minimapViewportFillColor
    minimapViewportHeight: contentsView.minimapResolvedViewportHeight
    minimapViewportY: contentsView.minimapResolvedViewportY
    minimapVisualRowPaintHeightResolver: function (row) {
        return contentsView.minimapVisualRowPaintHeight(row);
    }
    minimapVisualRowPaintYResolver: function (row) {
        return contentsView.minimapVisualRowPaintY(row);
    }
    minimapVisualRows: contentsView.minimapVisualRows
    scrollToMinimapPositionHandler: function (localY) {
        contentsView.scrollEditorViewportToMinimapPosition(localY);
    }
}
