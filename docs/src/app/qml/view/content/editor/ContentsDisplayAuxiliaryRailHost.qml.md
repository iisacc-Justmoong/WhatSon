# `src/app/qml/view/content/editor/ContentsDisplayAuxiliaryRailHost.qml`

## Responsibility

`ContentsDisplayAuxiliaryRailHost.qml` assembles the editor body rail around the center surface.

It composes `ContentsDisplayGutterHost.qml`, `ContentsDisplaySurfaceHost.qml`, the reserved minimap spacer in the
editor row, and the visible minimap rail overlay.
It also owns the whole-editor activation surface. That surface covers the full `ContentsDisplayView` body while a note
is selected, excludes visible gutter/minimap rails, and forwards remaining non-block clicks/touches to the center
surface as document-end edit requests. The activation surface stays behind the editor row so text editors and block
delegates receive direct input first; it is only the fallback for otherwise empty body space.

## Boundary

The host only places already-derived geometry and visual collaborators. Gutter and minimap measurements still come from
the display geometry controller/coordinators and the parsed structured-flow layout cache.
The activation surface is intentionally independent from `noteDocumentParseMounted`; parse completion decides which
blocks are visible, not whether the editor can accept the first blank-area click.

It exposes aliases for the mounted center surface and minimap layer so `ContentsDisplayView.qml` can stay as the
controller assembler without re-owning the auxiliary visual tree.
