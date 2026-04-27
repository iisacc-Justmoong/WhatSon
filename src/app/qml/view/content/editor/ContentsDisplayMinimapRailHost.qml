pragma ComponentBehavior: Bound

import QtQuick

ContentsMinimapLayer {
    id: minimapHost

    required property var contentsView
    required property var viewportCoordinator

    editorFlickable: contentsView.editorFlickable
    minimapBarWidthResolver: function (row) {
        return viewportCoordinator.minimapLineBarWidth(
                    Number(row && row.contentWidth !== undefined ? row.contentWidth : 0) || 0,
                    Number(row && row.contentAvailableWidth !== undefined ? row.contentAvailableWidth : 0) || 0,
                    Number(row && row.charCount !== undefined ? row.charCount : 0) || 0,
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
