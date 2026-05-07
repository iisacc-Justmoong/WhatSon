# `src/app/models/editor/display/ContentsEditorDisplayBackend.hpp`

## Responsibility

Declares the C++ backend object that owns live note-editor display wiring for `ContentViewLayout.qml`.

## Contract

- Exposes the editor session, presentation projection, structured renderer, body-resource renderer, resource-tag
  controller, inline-resource presenter, and minimap metrics as QObject properties for view binding.
- Accepts note-list, active-note, content-controller, library-controller, cursor, and minimap inputs
  from the view.
- Publishes note readiness and edit signals without making QML own note-session synchronization.
- Provides invokable RAW mutation and XML-attribute helper hooks for existing editor controllers that call back through
  a generic QObject surface.
- Provides structured inline resource visual block helpers for QML. The editor surface delegates render those blocks
  directly, and the geometry adapter reads their heights instead of parsing rendered HTML.
- Provides `editorSurfaceHtmlWithResourceVisualBlocks(...)` so the live RichText overlay receives C++-built resource
  flow spacers derived from `htmlTokens` and `resourceVisualBlocks`. QML displays the resulting HTML instead of
  rebuilding block-flow markup itself.
- Does not own `requestViewHook(...)` or a panel-controller relay. `ContentViewLayout.qml` owns that view-local hook
  dispatch directly.
