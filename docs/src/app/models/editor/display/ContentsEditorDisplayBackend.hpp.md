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
- Provides inline resource presentation helpers for QML, including explicit resource visual heights used by the
  geometry adapter instead of rendered HTML parsing.
- Does not own `requestViewHook(...)` or a panel-controller relay. `ContentViewLayout.qml` owns that view-local hook
  dispatch directly.
