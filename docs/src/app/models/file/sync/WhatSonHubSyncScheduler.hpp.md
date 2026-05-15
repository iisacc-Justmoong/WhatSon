# `src/app/models/file/sync/WhatSonHubSyncScheduler.hpp`

## Role
Declares the timer scheduler for hub sync checks.

## Contract
- Owns periodic polling and debounce suppression.
- Emits `syncCheckDue()` when a debounced sync check should run.
- Provides interval setters used by `WhatSonHubSyncController` without exposing `QTimer` ownership there.
