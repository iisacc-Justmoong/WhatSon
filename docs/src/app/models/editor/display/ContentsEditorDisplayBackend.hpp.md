# `src/app/models/editor/display/ContentsEditorDisplayBackend.hpp`

## Responsibility

Declares the C++ backend object that owns live note-editor display wiring for `ContentViewLayout.qml`.

## Contract

- Exposes the editor session, presentation projection, structured renderer, body-resource renderer, resource-tag
  controller, inline-resource presenter, and minimap metrics as QObject properties for view binding.
- Accepts note-list, active-note, content-controller, library-controller, panel-controller, cursor, and minimap inputs
  from the view.
- Publishes note readiness and edit/view-hook signals without making QML own note-session synchronization.
- Provides invokable RAW mutation and XML-attribute helper hooks for existing editor controllers that call back through
  a generic QObject surface.
