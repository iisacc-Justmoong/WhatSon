# `src/app`

## Role
`src/app` contains the main Qt Quick application. It is responsible for building the runtime object graph, loading a `.wshub` workspace, exposing long-lived services to QML, and composing the LVRS-based interface.

This directory is the architectural center of the repository. Most other modules are either dependencies of `src/app` or helpers used by it during startup, runtime synchronization, and UI rendering.

## Main Subsystems
- `models`: QML-adjacent QObject helpers such as editor block backends, calendar stores, and sensor-style hub inspectors.
- `file`: persistent storage, parsers, creators, validators, and hub-local mutations.
- `viewmodel`: QObject-facing state and behavior consumed by QML.
- `qml`: LVRS-based view composition and interaction surfaces.
- `runtime`: startup-time scheduling and background domain loading.
- `store`: lightweight shared state containers such as selected hub and sidebar selection.
- `file/sync`: runtime and editor synchronization controllers consolidated under the file domain.
- `policy`: layer rules and lock semantics for startup wiring.
- `permissions`, `platform`: host-integration points that sit beside the app domain model.

## Startup Shape
The app startup sequence lives in `main.cpp` and performs five broad phases.
1. Parse launch options and resolve a startup hub candidate.
2. Construct stores, services, viewmodels, and helper bridges on the C++ side.
3. Load critical workspace domain snapshots, usually through `WhatSonRuntimeParallelLoader`.
4. Defer low-priority hierarchy domains until the first post-show idle turns or the first sidebar activation that needs them.
5. Wire dedicated hierarchy viewmodels into `HierarchyViewModelProvider` and `SidebarHierarchyViewModel`.
6. Freeze mutable wiring through `ArchitecturePolicyLock`, then expose runtime objects to QML.

## Architectural Notes
- The repository follows dedicated hierarchy viewmodels per domain. Library, projects, bookmarks, tags, resources, progress, event, and preset each keep their own viewmodel instance.
- Read-oriented hierarchy access now flows through `IHierarchyViewModel`.
- Write-oriented hierarchy actions are split into capability interfaces and panel bridges.
- QML still receives bootstrap objects through context properties, but actual writable ownership is increasingly routed through the LVRS `ViewModels` registry.

## Hotspots
- `main.cpp` remains the most complex file in the application because it still owns startup orchestration and QML exposure.
- `qml` and `viewmodel` move together. Changes to one usually require documentation changes in the other.
- The sidebar and hierarchy domain are the current center of MVVM decomposition work.
