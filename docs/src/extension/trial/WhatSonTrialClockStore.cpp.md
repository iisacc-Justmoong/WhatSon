# `src/extension/trial/WhatSonTrialClockStore.cpp`

## Role
Implements the timestamp-audit persistence path for the optional trial module.

## Behavior
- Exit timestamps are normalized to UTC and persisted as ISO-8601 text with milliseconds.
- The store keeps both the raw last-exit timestamp and a monotonic high-water mark.
- When the current UTC time is earlier than the high-water mark, rollback is reported through `WhatSonTrialClockCheck`.

## Why Two Keys Exist
- `lastExitTimestampUtc` preserves the raw time recorded for the most recent shutdown.
- `lastSeenTimestampUtc` preserves evidence even if a later shutdown happens under a rolled-back system clock.
