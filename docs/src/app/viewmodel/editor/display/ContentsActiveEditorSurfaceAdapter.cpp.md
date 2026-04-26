# `src/app/viewmodel/editor/display/ContentsActiveEditorSurfaceAdapter.cpp`

## Responsibility

Implements active editor surface routing for the unified display host.

## Boundary

- Prefers the structured document flow when it is mounted and active.
- Falls back to the inline editor only when the inline surface is mounted and active.
- Normalizes focus requests into structured `requestFocus(...)` calls or inline cursor/focus calls without exposing
  those concrete paths to selection/mount controllers.
- Does not mutate document source, selection snapshots, persistence state, or visual layout.
