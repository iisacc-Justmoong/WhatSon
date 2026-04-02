# `src/app/sync/WhatSonHubSyncController.hpp`

## Role
`WhatSonHubSyncController` defines the app-level synchronization boundary between the mounted `.wshub` filesystem and
the in-memory runtime.

It is intentionally narrow: the class watches the hub, debounces change hints, and asks a caller-provided reload
callback to rebuild runtime state when the observed hub signature changes.

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
- `HubObservation` is the internal single-pass contract: one recursive walk produces both the signature payload and the
  watcher path coverage.
- The controller also remembers the last applied watch-path set so unchanged sync hints do not rebuild watcher
  registration.

## Collaborators
- `main.cpp`: creates the controller, injects the reload callback, and wires local mutation acknowledgements from the
  hierarchy viewmodels.
- `WhatSonHubPathUtils`: normalizes the mounted hub path.
- `QFileSystemWatcher`: provides recursive watch coverage once the controller enumerates relevant hub paths.
