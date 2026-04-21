# `src/app/runtime/bootstrap/WhatSonHubSyncWiring.cpp`

## Responsibility
Implements hub-sync wiring extracted from `main.cpp`.

## Behavior
- Builds one logging connection from `syncFailed` to warning output.
- Builds one local-mutation connection per provided source object using
  `hubFilesystemMutated() -> acknowledgeLocalMutation()`.
- Reports aggregate validity through `HubSyncWiringResult`.
- Includes `WhatSonHubSyncController` from `src/app/models/file/sync` after sync-domain consolidation.
- `main.cpp` now supplies the projects hierarchy viewmodel in that source list as well, so project
  hierarchy writes are treated as local mutations instead of triggering a foreign-change reload path.

## Test Coverage

Automated test files were removed from this repository; verify wiring validity through startup/runtime smoke runs.
