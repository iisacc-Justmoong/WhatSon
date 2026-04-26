# `src/app/qml/view/content/editor/ContentsDisplayAuxiliaryRailHost.qml`

## Responsibility

`ContentsDisplayAuxiliaryRailHost.qml` assembles the editor body rail around the center surface.

It composes `ContentsDisplayGutterHost.qml`, `ContentsDisplaySurfaceHost.qml`, the reserved minimap spacer in the
editor row, and the visible minimap rail overlay.

## Boundary

The host only places already-derived geometry and visual collaborators. Gutter and minimap measurements still come from
the display geometry controller/coordinators and the parsed structured-flow layout cache.

It exposes aliases for the mounted center surface and minimap layer so `ContentsDisplayView.qml` can stay as the
controller assembler without re-owning the auxiliary visual tree.
