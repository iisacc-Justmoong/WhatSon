# `src/app/viewmodel/editor/display`

## Responsibility

Contains C++ ViewModels for the unified note display host. These classes keep `ContentsDisplayView.qml` layout-focused
while preserving the existing host API shape that QML collaborators call.

## Current Modules

- `ContentsDisplayControllerBridgeViewModel.*`
  Owns the shared `controller` property and signal/slot bridge used by the display ViewModels.
- `ContentsDisplayMutationViewModel.*`
  Publishes RAW source mutation commands, inline style wrapping, structured tag insertion, agenda task rewrites, and
  focus restoration hooks.
- `ContentsDisplayPresentationViewModel.*`
  Publishes editor creation tracing, presentation refresh, rendered overlay synchronization, and `requestViewHook(...)`
  hooks.
- `ContentsDisplaySelectionMountViewModel.*`
  Publishes selected-note snapshot polling, reconcile, editor-session delivery, and note-body mount scheduling hooks.
- `ContentsDisplayGeometryViewModel.*`
  Publishes minimap/gutter cache invalidation and viewport correction hooks.
- `ContentsActiveEditorSurfaceAdapter.*`
  Publishes the active editor surface contract used by selection/mount orchestration for focus restoration and logical
  cursor movement, without exposing concrete QML editor item names to that orchestration.
- `ContentsDisplaySurfacePolicy.*`
  Publishes the active editor surface decision. The canonical selected-note surface is the structured document flow;
  the legacy whole-note inline loader is disabled at the policy boundary.

## Boundary

- This directory must not contain QML.
- Each ViewModel is a C++ single-responsibility command surface with a signal/slot contract.
- Non-view editor orchestration must move into C++ model or ViewModel objects. QML is limited to view construction and
  must not be used as a ViewModel, session, persistence, scheduling, or command-surface boundary.
- `ContentsDisplayView.qml` instantiates these ViewModels and binds layout-only surfaces to their public hooks.
- The view may keep compatibility wrapper functions only when existing editor-domain collaborators still call the host
  API directly; those wrappers must delegate into the responsible C++ ViewModel.
