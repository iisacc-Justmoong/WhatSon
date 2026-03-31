# `src/app/runtime/bootstrap/WhatSonHubSyncWiring.cpp`

## Responsibility
Implements hub-sync wiring extracted from `main.cpp`.

## Behavior
- Builds one logging connection from `syncFailed` to warning output.
- Builds one local-mutation connection per provided source object using
  `hubFilesystemMutated() -> acknowledgeLocalMutation()`.
- Reports aggregate validity through `HubSyncWiringResult`.

## Test Coverage
- `tests/app/test_runtime_bootstrap_wiring.cpp` verifies wiring creation and connection validity.
