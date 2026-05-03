# `src/app/qml/view/contents/editor/ContentsDisplayOverlayHost.qml`

## Responsibility

`ContentsDisplayOverlayHost.qml` composes root-level editor overlays for the note display host.

It currently mounts `ContentsDisplayExceptionOverlay.qml` and `ContentsDisplayResourceImportConflictAlert.qml`.

## Boundary

The host owns overlay placement only. Exception state comes from the note-body mount coordinator through
`ContentsDisplayView.qml`, and duplicate-resource decisions still execute through the resource import controller.
