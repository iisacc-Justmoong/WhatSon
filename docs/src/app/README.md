# `src/app`

## Role
`src/app` contains the main Qt Quick application. It is responsible for building the runtime object graph, loading a
`.wshub` workspace, exposing long-lived services to QML, and composing the LVRS-based interface.

This directory is the architectural center of the repository. Most other modules are either dependencies of `src/app`
or helpers used by it during startup, runtime synchronization, and UI rendering.

## Main Subsystems
- `models`: QObject-based runtime and storage domains, including content, editor, file, calendar, navigation, panel,
  sidebar, detail-panel, onboarding, display, and sensor helpers.
- `qml`: LVRS-based view composition and interaction surfaces.
- `runtime`: startup-time scheduling and background domain loading.
- `store`: lightweight shared state containers such as selected hub and sidebar selection.
- `models/file/sync`: runtime and editor synchronization controllers consolidated under the file domain.
- `policy`: layer rules and lock semantics for startup wiring.
- `permissions`, `platform`: host-integration points that sit beside the app domain model.

## Startup Shape
The app startup sequence lives in `main.cpp` and performs five broad phases.
1. Parse launch options and resolve a startup hub candidate through the shared hub mount validator.
   If a persisted hub selection exists but cannot be mounted, startup now leaves the app in onboarding with the
   original failure visible instead of silently opening any fallback hub. If that candidate mounts but the first
   workspace runtime load still fails, startup also returns to onboarding and keeps the runtime error visible.
2. Construct stores, services, controllers, and helper bridges on the C++ side.
3. Load critical workspace domain snapshots, usually through `WhatSonRuntimeParallelLoader`.
4. Defer low-priority hierarchy domains until the first post-show idle turns or the first sidebar activation that
   needs them.
5. Wire dedicated hierarchy controllers into `HierarchyControllerProvider` and `SidebarHierarchyController`.
6. Freeze mutable wiring through `ArchitecturePolicyLock`, then expose runtime objects to QML.

## Architectural Notes
- The repository follows dedicated hierarchy controllers per domain. Library, projects, bookmarks, tags, resources,
  progress, event, and preset each keep their own controller instance.
- Read-oriented hierarchy access now flows through `IHierarchyController`.
- Write-oriented hierarchy actions are split into capability interfaces and panel bridges.
- QML receives bootstrap objects through LVRS context-object bindings. MVVM and a separate view-model directory are no
  longer part of the app architecture.

## Hotspots
- `main.cpp` remains the most complex file in the application because it still owns startup orchestration and QML
  exposure.
- `qml` and `models` move together. Changes to one usually require documentation changes in the other.
- The sidebar and hierarchy domain are the current center of model-domain controller decomposition work.
