# `src/app/models/editor/display/ContentsDisplayTraceFormatter.cpp`

## Responsibility

Formats compact trace strings for `ContentsDisplayView.qml` and related display coordinators.

## Key Behavior

- Formats selection-sync option payloads for `scheduleSelectionModelSync(...)`.
- Formats selection/mount/poll/reconcile plans from the editor display pipeline.
- Selection-plan formatting now includes both snapshot-driving keys and mount-driving keys:
  - `noteId`
  - `selectedNoteId`
  - `selectedNoteBodyNoteId`
  - `selectedNoteBodyResolved`
  - `allowSnapshotRefresh`
  - `attemptReconcile`
  - mount/sync fallback and visual-refresh flags
- This keeps one shared trace formatter usable across mount plans, selection-sync flush plans, and snapshot poll/reconcile
  plans without QML-side string duplication.
