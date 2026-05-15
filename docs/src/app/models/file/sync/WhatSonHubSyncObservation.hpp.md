# `src/app/models/file/sync/WhatSonHubSyncObservation.hpp`

## Role
Declares the value object shared by hub sync observation, watcher setup, and controller baseline comparison.

## Contract
- `signature` is the hash of the observed hub filesystem state.
- `directoryWatchPaths` is the normalized directory list that should be registered with the hub sync watcher.
- The struct is passive data only; it performs no filesystem access and owns no timers or watchers.
