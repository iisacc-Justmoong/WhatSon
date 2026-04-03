# `src/app/sync/WhatSonHubSyncController.cpp`

## Implementation Summary
The implementation turns hub synchronization into three explicit phases:

1. inspect the mounted `.wshub` once and build a combined observation payload
2. watch and debounce filesystem change hints
3. decide whether to refresh baseline only or invoke the injected runtime reload callback

## Observation Model
`inspectHub(...)` walks the mounted hub recursively, collects normalized file/directory records for hashing, and also
records the watcher paths that must be registered with `QFileSystemWatcher`.

The observed signature intentionally ignores:
- `.whatson`

This keeps app-private bookkeeping churn out of runtime reload policy.

This keeps signature hashing and watcher coverage on a single recursive observation pass.

## Watcher Model
The collected watcher paths are fed into `QFileSystemWatcher`.

Watcher registration now short-circuits when the normalized watch-path set is unchanged, so a sync
hint that observes the same hub topology no longer tears down and re-adds every watched path.

The watcher is paired with:
- a periodic timer for eventual consistency
- a debounce timer for burst suppression

The controller therefore remains responsive to external edits without turning every individual filesystem callback into
an immediate runtime rebuild.

## Local Mutation Handling
`acknowledgeLocalMutation()` marks the next observed signature change as app-owned.

When `onDebounceTimeout()` sees that app-owned change, it refreshes the baseline and rebuilds watcher coverage, but it
does not call the runtime reload callback. This is the key separation that keeps user-initiated note/folder writes
from bouncing the live editor session through a self-reload.

## External Mutation Handling
When the signature changes without a local-mutation acknowledgement:
- the injected reload callback is invoked
- a failed reload emits `syncFailed(...)`
- a successful reload reuses the same observation payload to refresh the baseline and emits `syncReloaded(...)`

## Architectural Note
This implementation no longer reacts to pointer, touch, or application activation events. Hub synchronization is
driven by watcher/timer hints and explicit local-write acknowledgements only, which keeps filesystem sync policy
independent from UI navigation and gesture handling.
