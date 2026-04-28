# `src/app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.cpp`

## Responsibility

Implements the active document-surface policy for the note display host.

## Behavior

- `activeSurfaceKind` is `structured` when a note is selected and no specialized surface request is active.
- The policy no longer publishes legacy inline-surface readiness or loading flags.
  `ContentsDisplayView.qml` mounts the parser-backed structured document host directly when a note is selected.
- `documentPresentationProjectionEnabled` is only true for formatted-preview mode.
