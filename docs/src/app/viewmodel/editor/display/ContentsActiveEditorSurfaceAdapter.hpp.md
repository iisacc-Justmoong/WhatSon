# `src/app/viewmodel/editor/display/ContentsActiveEditorSurfaceAdapter.hpp`

## Responsibility

Publishes the active editor surface contract used by the display host.

## Contract

- Accepts the currently mounted structured and inline editor surfaces as QObject dependencies.
- Exposes one C++ slot surface for focus requests, logical cursor movement, focus-state queries, and native-input
  support checks.
- Keeps selection/mount orchestration dependent on an active-surface contract instead of concrete QML editor item names.
- Emits surface-change signals whenever the mounted editor or active mode flags change.
