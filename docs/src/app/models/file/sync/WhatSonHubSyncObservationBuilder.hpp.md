# `src/app/models/file/sync/WhatSonHubSyncObservationBuilder.hpp`

## Role
Declares the recursive hub observation builder.

## Contract
- `inspectHub(...)` returns one `WhatSonHubSyncObservation`.
- The builder owns mounted hub traversal, signature payload construction, and directory watch-path discovery.
- It does not decide whether to reload runtime state and does not register `QFileSystemWatcher` paths directly.
