# `src/app/qml/view/content/editor/ContentsDisplayExceptionOverlay.qml`

## Responsibility

Renders note-document mount and parse exception messaging over the editor surface.

## Boundary

- Owns only centered exception-title and exception-message presentation.
- Reads already-derived exception state from `ContentsDisplayView.qml`.
- Does not schedule selection sync, mount retries, document parsing, or focus restoration.
