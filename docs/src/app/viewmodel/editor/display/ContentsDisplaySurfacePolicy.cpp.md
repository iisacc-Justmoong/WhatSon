# `src/app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.cpp`

## Responsibility

Implements the active document-surface policy for the note display host.

## Behavior

- `activeSurfaceKind` is `structured` when a note is selected and no specialized surface request is active.
- `inlineDocumentSurfaceRequested`, `inlineDocumentSurfaceReady`, and `inlineDocumentSurfaceLoading` stay false so
  `ContentsDisplayView.qml` cannot silently mount the unreachable whole-note inline editor path.
- `documentPresentationProjectionEnabled` is only true for formatted-preview mode.
