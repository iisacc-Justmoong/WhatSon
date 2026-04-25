# `src/app/main.cpp`

## Runtime Wiring Notes
- `WhatSonRuntimeParallelLoader` is now instantiated in `main.cpp` and injected into `WhatSonStartupRuntimeCoordinator`
  through `setParallelLoader(...)`; the concrete loader delegates requested domain fan-out to LVRS `BootstrapParallel`.
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
- Regular startup now distinguishes between:
  - a persisted selection that merely resolved to a candidate `.wshub` path, and
  - a hub connection that actually loaded into runtime successfully.
  Startup onboarding is now driven by that successful runtime connection state, not just by the earlier resolver step.
- If startup hub mounting/resolution appears to succeed but `loadStartupHubIntoRuntime(...)` still fails, `main.cpp`
  now reports that failure through `OnboardingHubController::failHubLoad(...)` and immediately falls back to the same
  onboarding presentation used for an unmounted startup.
- Regular startup now forks desktop and mobile onboarding presentation again:
  - desktop loads `Main.qml` plus a dedicated onboarding window when no startup hub is connected successfully,
  - Android keeps using the embedded `/onboarding` route inside `Main.qml` when startup connection fails,
  - iOS still keeps the LVRS stack pinned to `/` and swaps the inline onboarding sequence inside the workspace page
    when startup connection fails.
- The dedicated `Onboarding.qml` shell is therefore used both for the explicit `--onboarding-only` launch path and for
  ordinary desktop startup when no persisted hub can be mounted.
- QML root loading now flows through `WhatSonQmlLaunchSupport`, which delegates initial-property setup and window
  activation to LVRS `loadQmlRootObjects(...)` instead of duplicating `QQmlApplicationEngine::loadFromModule(...)`
  and manual `show()/raise()/requestActivate()` logic in `main.cpp`.
- Workspace context binding now flows through `WhatSonQmlContextBinder` and LVRS `QmlContextBindPlan`, so C++
  registers root ViewModels into `LV.ViewModels` before `Main.qml` loads.
- Internal QML bridge type registration now flows through `WhatSonQmlInternalTypeRegistrar` and LVRS
  `QmlTypeRegistrar`; startup fails early if the internal type manifest cannot be registered.
- Foreground scheduler and permission startup now run through LVRS `ForegroundServiceGate`; `main.cpp` builds a
  lifecycle context from the loaded workspace root and starts those services only after a visible workspace window is
  present.
- Deferred startup hierarchy prefetch now runs as a non-fatal LVRS `QmlBootstrapTask` at
  `QmlAppLifecycleStage::AfterFirstIdle`, so Event/Preset follow-up loads are attached to the app lifecycle instead of
  a local `QTimer` chain in `main.cpp`.
- Permission startup wiring now consumes `permissions/WhatSonPermissionBootstrapper.hpp` after consolidating
  permission bootstrap code under `src/app/permissions`.
- Application bootstrap no longer forces Qt scene-graph visualization environment variables and no longer auto-opens
  the scene debug companion window.
- This keeps the application bootstrap as the composition root for concrete object selection.
