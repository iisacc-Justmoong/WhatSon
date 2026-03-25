# `src/extension/trial/WhatSonTrialClockStore.hpp`

## Role
Declares the local timestamp-audit store used to detect whether the system clock moved backwards between app sessions.

## Public API
- `loadLastExitTimestampUtc()`: returns the most recently stamped app-exit UTC timestamp.
- `loadLastSeenTimestampUtc()`: returns the monotonic high-water mark of all exit timestamps observed so far.
- `inspect(...)`: compares a supplied UTC time against the high-water mark and reports rollback metadata.
- `stampExitTimestamp(...)`: writes the current exit timestamp and preserves the larger of the existing high-water mark and the new value.
- `clear()`: removes both persisted timestamp keys.

## Persistence Keys
- `extension/trial/lastExitTimestampUtc`
- `extension/trial/lastSeenTimestampUtc`
