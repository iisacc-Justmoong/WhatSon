# `src/app/runtime/bootstrap/WhatSonHubSyncWiring.hpp`

## Responsibility
Declares bootstrap wiring helpers for `WhatSonHubSyncController`.

## Public Contract
- `HubSyncWiringResult`: captures generated connection handles and aggregate wiring status.
- `wireHubSyncController(...)`: connects:
  - controller `syncFailed` logging path
  - each mutation source `hubFilesystemMutated()` signal to controller
    `acknowledgeLocalMutation()`.

## Design Intent
`main.cpp` should not duplicate repetitive signal wiring details.
This helper centralizes connection setup while keeping runtime behavior unchanged.
