# `src/app/main.cpp`

## Role
`main.cpp` is the application composition root. It is not a domain object and it is not supposed to hold business policy. Its job is to assemble long-lived runtime objects, run startup-time workspace loading, register QML bridge types, and expose the final object graph to the LVRS application shell.

## What It Constructs
- Dedicated hierarchy viewmodels for each hierarchy domain.
- `LibraryNoteMutationViewModel` as a narrower write facade for note creation and deletion.
- Navigation, detail-panel, onboarding, sidebar, calendar, and panel-registry viewmodels.
- In trial builds, a trial activation policy and a dedicated desktop trial-status window bootstrap path.
- Runtime services such as async scheduling, hub sync, permission bootstrap, write lease management, and startup hub selection persistence.
- QML bridge types such as `HierarchyDragDropBridge`, `HierarchyInteractionBridge`, and editor helper bridges.

## Startup Flow
1. Parse launch options such as onboarding-only mode.
2. Resolve a candidate startup workspace by checking persisted hub selection and then falling back to the first `blueprint/*.wshub`.
3. Create runtime stores and viewmodels before any QML is loaded.
4. If a hub can be mounted, load the critical startup domain snapshots through `WhatSonRuntimeParallelLoader` and apply them to the runtime objects before the first workspace frame.
5. Defer only low-priority hierarchy domains until either the post-show idle turns or the first sidebar activation that needs them.
6. Populate `HierarchyViewModelProvider`, then connect it to `SidebarHierarchyViewModel`.
7. Call `ArchitecturePolicyLock::lock()` after mutable dependency injection is complete.
8. Export root runtime objects to the `QQmlContext`, then load the LVRS root window module.
9. In trial builds, load `TrialStatus.qml` as a second desktop window and inject the refreshed `WhatSonTrialActivationPolicy`.

## Important Wiring Decisions
- `HierarchyViewModelProvider` is the single runtime map from sidebar domain index to dedicated hierarchy viewmodel.
- `SidebarHierarchyViewModel` must receive its selection store and provider before the architecture lock is enabled.
- `LibraryNoteMutationViewModel` wraps `LibraryHierarchyViewModel` so note mutation shortcuts no longer need the full library hierarchy surface.
- `DetailPanelViewModel` owns dedicated selector-copy viewmodels for Projects, Bookmarks, and Progress. `main.cpp` injects the canonical hierarchy viewmodels only as read-only selector sources so detail-panel combo state stays decoupled from sidebar selection.
- The detail panel current-note bridge must follow `SidebarHierarchyViewModel::activeNoteListModel` and `activeHierarchyViewModel`, not the library hierarchy unconditionally, because `.wsnhead` reads and writes must target the note identified by the current workspace view.
- `qmlRegisterType(...)` is used for bridge-like objects that should be instantiated from QML rather than pushed as singletons.
- Trial builds keep the trial-status window out of the main workspace route graph. The composition root injects the activation policy through dedicated initial properties instead of exposing another global workspace context object.

## QML Exposure
This file exports the root runtime objects through `engine.rootContext()->setContextProperty(...)`. `Main.qml` immediately re-registers the view-facing objects into `LV.ViewModels` and binds write ownership per view ID.

The practical split is this.
- C++ owns object lifetime and initial graph assembly.
- `Main.qml` owns view-facing registration and writable ownership claims.

## Failure and Recovery Behavior
- If no startup hub can be resolved, the app still starts and logs the condition.
- If initial hub loading fails, startup continues with warnings rather than crashing.
- Deferred low-priority startup domain loads are retried on first sidebar activation while the partial-bootstrap window stays active.
- If QML object creation fails, the process exits through the engine failure callback.
- Permission bootstrap starts after the foreground UI has been brought up, which keeps startup responsive.

## Why This File Is Still a Hotspot
- It mixes application bootstrap, runtime loading, platform integration, and QML exposure in one translation unit.
- The file is sensitive to architectural regressions because the policy lock, QML registration, and context-property export order all matter.
- It is the first place to read when debugging missing runtime objects in QML, incorrect startup hub selection, or broken initialization order.
