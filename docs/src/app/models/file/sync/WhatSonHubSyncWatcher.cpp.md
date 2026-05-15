# `src/app/models/file/sync/WhatSonHubSyncWatcher.cpp`

## Role
Implements watcher path registration for mounted hub sync.

## Behavior
- Normalizes, deduplicates, and sorts requested directory paths.
- Computes incremental add/remove sets against the currently applied watcher paths.
- Removes stale paths and adds new paths on the internal `QFileSystemWatcher`.
- Clears file and directory watcher paths when the mounted hub is unset.

## Boundary
- The watcher only forwards filesystem change hints. Debounce, polling, signature comparison, and reload decisions live
  in sibling sync objects.
