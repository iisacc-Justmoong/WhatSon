# `src/app/main.cpp`

## Runtime Wiring Notes
- `WhatSonRuntimeParallelLoader` is now instantiated in `main.cpp` and injected into `WhatSonStartupRuntimeCoordinator` through `setParallelLoader(...)`.
- Calendar and system calendar stores continue to be instantiated concretely here, but downstream collaborators now consume interface types.
- `main.cpp` now keeps the concrete `CalendarBoardStore` synchronized with the currently loaded hub path and refreshes
  calendar note projections from the live `LibraryHierarchyViewModel` snapshot after startup load, onboarding load,
  sync reload, and every `indexedNotesSnapshotChanged()` emission from the library runtime viewmodel.
- `main.cpp` also wires the library viewmodel's single-note mutation signals into `CalendarBoardStore`:
  `indexedNoteUpserted(...)` now updates one projected note mount in place and `noteDeleted(...)` removes that mount
  without forcing a full calendar note reindex.
- `main.cpp` also injects a live library-note provider into `CalendarBoardStore`, so calendar queries can lazily
  resolve projected note items even before an explicit projection reload has populated the cache.
- Bookmark/progress-originated note mutations still use the disk reindex fallback path so calendar projection remains
  aligned even when those domains mutate note metadata outside the library viewmodel's in-memory note list.
- Project-domain mutations now join the same `hubFilesystemMutated() -> acknowledgeLocalMutation()`
  wiring path as library/bookmark/progress mutations, so local project edits are not misclassified
  as foreign hub changes.
- Resource-domain mutations now join that same local-mutation wiring path as well, so deleting
  `.wsresource` packages from the resources list is acknowledged as a local hub edit immediately.
- Hub selection wiring now updates sync/import state directly without a background write-lease heartbeat timer.
- Startup hub resolution no longer synthesizes a `blueprint/*.wshub` fallback, so an unmounable persisted selection now
  leaves the composition root unmounted until onboarding opens a real workspace.
- The composition root now suppresses automatic startup onboarding on iOS specifically: it still configures
  `OnboardingRouteBootstrapController` for embedded presentation reuse, but if no startup hub is mounted it immediately
  dismisses the embedded onboarding visibility before the main window loads.
- Permission startup wiring now consumes `permissions/WhatSonPermissionBootstrapper.hpp` after consolidating
  permission bootstrap code under `src/app/permissions`.
- Application bootstrap no longer forces Qt scene-graph visualization environment variables and no longer auto-opens
  the scene debug companion window.
- This keeps the application bootstrap as the composition root for concrete object selection.
