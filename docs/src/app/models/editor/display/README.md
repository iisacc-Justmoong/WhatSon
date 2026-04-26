# `src/app/models/editor/display`

## Responsibility

This directory holds editor-domain display helpers that are performance-sensitive or safety-sensitive enough to keep out
of QML script bodies.

## Current Modules

- `ContentsDisplayContextMenuCoordinator.*`
  Normalizes structured editor context-menu selection snapshots before inline style commands consume them.
- `ContentsDisplayDocumentSourceResolver.*`
  Resolves which RAW note source the editor should present while note selection, snapshot refresh, and editor-session
  binding are converging.
- `ContentsDisplayEditOperationCoordinator.*`
  Coordinates high-level edit operation state that QML surfaces need to apply against the current note body.
- `ContentsDisplayGutterCoordinator.*`
  Keeps gutter state derived from the same editor document projection as the visible body.
- `ContentsDisplayMinimapCoordinator.*`
  Owns minimap snapshot reuse and rebuild decisions for structured editor geometry.
- `ContentsDisplayNoteBodyMountCoordinator.*`
  Drives note-body mount, retry, and exception plans while the selected RAW body is converging.
- `ContentsDisplayPresentationRefreshController.*` and `ContentsDisplayRefreshCoordinator.*`
  Keep presentation refresh requests explicit instead of burying them in QML timers.
- `ContentsDisplaySelectionSyncCoordinator.*`
  Coordinates selected-note snapshot reconciliation and editor selection sync.
- `ContentsDisplayStructuredFlowCoordinator.*`
  Publishes structured-flow visibility and source convergence plans.
- `ContentsDisplayTraceFormatter.*`
  Centralizes trace payload formatting shared by the display coordinators.
- `ContentsDisplayViewportCoordinator.*`
  Owns line-offset lookup, minimap track math, viewport correction plans, and structured gutter geometry summaries used
  by `ContentsDisplayView.qml`.
- `ContentsDisplayHostModePolicy.qml`
  Owns desktop/mobile presentation deltas for the unified editor host without keeping policy objects in the view
  directory.
- `ContentsEditorSurfaceModeSupport.js`
  Resolves whether `ContentViewLayout.qml` should mount the note editor surface or direct resource editor surface.
- `ContentsMinimapSnapshotSupport.js`
  Normalizes changed-line ranges and minimap snapshot rows for editor display refreshes.

## Boundary

- These helpers stay in the editor domain because they operate on editor-owned RAW source, logical-line offsets, and
  note-session state.
- Mobile hierarchy navigation remains in `src/app/models/content/mobile`; only editor-body display coordination belongs
  here.
- They are intentionally QML-facing `QObject` bridges, but the calculations themselves live in C++ so the view layer
  stays declarative.
