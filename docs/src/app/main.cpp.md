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
- Onboarding now submits only a validated hub selection ticket back into `main.cpp`; the composition root is the only
  place that mounts the runtime, persists the chosen hub selection, and fans the loaded hub path into sync/import and
  calendar projection state.
- Persisted hub selection wiring now stores the loaded hub path together with the original selection URL and any
  security-scoped bookmark, so iOS startup can re-resolve provider-backed `.wshub` picks even when the local path
  changes between launches.
- Startup hub resolution no longer synthesizes a `blueprint/*.wshub` fallback, so an unmounable persisted selection now
  leaves the composition root unmounted until onboarding opens a real workspace.
- Regular startup no longer forks desktop and mobile onboarding into different top-level window flows. `main.cpp` now
  loads `Main.qml` for ordinary startup on every platform and lets `OnboardingRouteBootstrapController` decide whether
  the root window should begin in onboarding or workspace presentation.
- The dedicated `Onboarding.qml` shell remains only for the explicit `--onboarding-only` launch path.
- Permission startup wiring now consumes `permissions/WhatSonPermissionBootstrapper.hpp` after consolidating
  permission bootstrap code under `src/app/permissions`.
- Application bootstrap no longer forces Qt scene-graph visualization environment variables and no longer auto-opens
  the scene debug companion window.
- This keeps the application bootstrap as the composition root for concrete object selection.
