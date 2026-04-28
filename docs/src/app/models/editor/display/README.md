# `src/app/models/editor/display`

## Responsibility

This directory holds editor-domain display helpers. Under the current QML boundary, QML must be used for view
construction only; model-side QML controller files in this directory are transitional migration targets and must not
gain new session, persistence, parsing, mutation, scheduling, or command-surface responsibility.

## Current Modules

- `ContentsDisplayContextMenuCoordinator.*`
  Normalizes structured editor context-menu selection snapshots before inline style commands consume them.
- `ContentsDisplayDocumentSourceResolver.*`
  Resolves which RAW note source the editor should present while note selection, snapshot refresh, and editor-session
  binding are converging.
- `ContentsDisplayEditOperationCoordinator.*`
  Coordinates high-level focus and shortcut operation state that QML surfaces need to apply against the current note body.
- `ContentsDisplayGutterCoordinator.*`
  Keeps gutter state derived from the same editor document projection as the visible body.
- `ContentsDisplayMinimapCoordinator.*`
  Owns minimap snapshot reuse and rebuild decisions.
  Its structured path now consumes parser-normalized block entries directly, producing one minimap row per block/tag
  and reserving block-like silhouettes only for visual entries such as resources.
- `ContentsDisplayNoteBodyMountCoordinator.*`
  Drives note-body mount, retry, and exception plans while the selected RAW body is converging.
  The coordinator is now split into narrow implementation units so public QObject state plumbing, mount-plan emission,
  and derived mount/exception status no longer live in one giant translation unit.
  It no longer publishes separate surface-ready or surface-interactive state; parse-mounted `.wsnbody` is the only
  readiness authority exposed upward.
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
- `ContentsDisplayEventPump.qml`
  Owns display-host timers and signal connections for note mount, selection sync, projection, cursor, and geometry
  refresh events.
- `ContentsDisplayInputCommandSurface.qml`
  Owns editor context-menu pointer triggers and tag-management shortcuts.
- `ContentsDisplayGeometryController.qml`
  Owns QML-runtime geometry scheduling and delegates public calls through the C++ geometry ViewModel.
- `ContentsDisplayGeometrySnapshotModel.qml`
  Owns QML-side geometry snapshot normalization, structured minimap snapshot planning, visible gutter entry building,
  and compatibility wrappers that let `ContentsDisplayView.qml` shed large helper bodies without breaking existing host
  call sites.
- `ContentsDisplayViewportModel.qml`
  Owns QML-side document/viewport line math such as logical-line Y lookup, gutter Y lookup, marker placement,
  minimap row positioning, and documentY-to-line resolution.
- `ContentsDisplayMutationController.qml`
  Owns QML-runtime RAW source writes delegated through the C++ mutation ViewModel. It applies already-built RAW
  `.wsnbody` text directly, then lets parser and renderer projections observe the changed source.
- `ContentsDisplayPresentationController.qml`
  Owns QML-runtime presentation refresh orchestration delegated through the C++ presentation ViewModel.
- `ContentsDisplaySelectionMountController.qml`
  Owns QML-runtime selection and mount orchestration delegated through the C++ selection/mount ViewModel.
- `ContentsDisplayHostModePolicy.qml`
  Owns desktop/mobile presentation deltas for the unified editor host without keeping policy objects in the view
  directory.
- `ContentsEditorSurfaceModeSupport.js`
  Resolves whether `ContentViewLayout.qml` should mount the note editor surface or direct resource editor surface.
- `ContentsMinimapSnapshotSupport.js`
  Normalizes changed-line ranges and minimap snapshot rows for editor display refreshes while preserving whichever row
  width metadata the owning minimap path already chose, including parser-derived block silhouettes.

## Boundary

- These helpers stay in the editor domain because they operate on editor-owned RAW source, logical-line offsets, and
  note-session state.
- Mobile hierarchy navigation remains in `src/app/models/content/mobile`; only editor-body display coordination belongs
  here.
- ViewModels remain C++ under `src/app/viewmodel`.
- New editor display orchestration should be C++ model or ViewModel code, with QML limited to view construction under
  `src/app/qml/view`.
