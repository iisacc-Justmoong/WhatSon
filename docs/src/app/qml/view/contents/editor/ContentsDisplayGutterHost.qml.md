# `src/app/qml/view/contents/editor/ContentsDisplayGutterHost.qml`

## Responsibility

Composes the note-display gutter rail for `ContentsDisplayView.qml`.

## Boundary

- Owns only gutter view bindings into `ContentsGutterLayer`.
- Consumes the display host's published line, marker, and visibility state.
- Does not compute line geometry, minimap state, document source, or editor focus.
