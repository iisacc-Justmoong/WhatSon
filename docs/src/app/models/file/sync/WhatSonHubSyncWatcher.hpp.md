# `src/app/models/file/sync/WhatSonHubSyncWatcher.hpp`

## Role
Declares the filesystem watcher wrapper for mounted hub sync.

## Contract
- Applies normalized directory watch paths to an internal `QFileSystemWatcher`.
- Emits `watchedPathChanged(...)` when a watched directory changes.
- Remembers applied paths so unchanged path sets do not churn watcher registration.

## Boundary
- The watcher does not compute hub signatures and does not decide whether runtime reload is required.
