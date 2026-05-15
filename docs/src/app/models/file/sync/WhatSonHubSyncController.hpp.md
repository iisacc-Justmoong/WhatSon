# `src/app/models/file/sync/WhatSonHubSyncController.hpp`

## Role
`WhatSonHubSyncController` defines the app-level synchronization boundary between the mounted `.wshub` filesystem and
the in-memory runtime.

It is intentionally narrow: the class watches the hub, debounces change hints, and asks a caller-provided reload
callback to rebuild runtime state when the observed hub signature changes.

The implementation is split into collaborators. The controller now keeps the public signal/slot boundary and owns the
reload decision flow, while observation, watcher registration, and timer scheduling live in dedicated objects.

## Public API
- `setReloadCallback(...)`: injects the runtime reload function used after an observed external change.
- `setCurrentHubPath(...)`: switches the mounted hub path, rebuilds the signature baseline, and reconfigures watcher
  coverage.
- `setPeriodicIntervalMs(...)` / `setDebounceIntervalMs(...)`: tune polling/debounce policy for runtime diagnostics or
  platform adjustments.
- `requestSyncHint()`: schedules a debounced sync check.
- `acknowledgeLocalMutation()`: marks an app-owned write so the next signature change refreshes baseline instead of
  reloading the runtime.

## Signals
- `syncReloaded(hubPath)`: emitted after the reload callback succeeds and the baseline is refreshed.
- `syncFailed(errorMessage)`: emitted when the reload callback reports failure.

## Architectural Constraints
- The controller is filesystem-oriented. It no longer exposes application-event attachment, event filtering, or app
  activation hooks.
- UI navigation, gestures, and generic input events are outside this class's responsibility.
- Local app writes are treated differently from external writes through `acknowledgeLocalMutation()`, which keeps the
  current session from reloading itself after its own mutation path.
- `WhatSonHubSyncObservationBuilder` owns the single-pass observation contract: one recursive walk produces both the
  signature payload and watcher path coverage.
- `WhatSonHubSyncWatcher` remembers the last applied watch-path set so unchanged sync hints do not rebuild watcher
  registration.
- `WhatSonHubSyncScheduler` owns periodic polling and debounce suppression.

## Collaborators
- `main.cpp`: creates the controller, injects the reload callback, and wires local mutation acknowledgements from the
  hierarchy controllers.
- `WhatSonHubPathUtils`: normalizes the mounted hub path.
- `WhatSonHubSyncObservationBuilder`: inspects the mounted hub and builds the observation signature.
- `WhatSonHubSyncWatcher`: wraps recursive `QFileSystemWatcher` coverage.
- `WhatSonHubSyncScheduler`: wraps periodic/debounce timers.

## Tests
- `test/cpp/suites/hub_sync_controller_tests.cpp` covers the responsibility split and the `.whatson` observation
  ignore contract.
- Regression checklist:
  - Include consumers must resolve this controller through `file/sync/WhatSonHubSyncController.hpp`.
  - Moving this header into `file/sync` must not alter the public API surface or signal/slot contract.
