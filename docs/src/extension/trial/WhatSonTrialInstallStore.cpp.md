# `src/extension/trial/WhatSonTrialInstallStore.cpp`

## Role
Implements the install-date persistence path for the optional trial module.

## Behavior
- Stored values are normalized to ISO `YYYY-MM-DD` text.
- The same value is mirrored into the OS secure store through `WhatSonTrialSecureStore`.
- When the secure-store copy exists, the install store rewrites `QSettings` from that secure value.
- Invalid or malformed stored values are deleted on read so future evaluations can recreate a clean install date.
- `ensureInstallDate(...)` is the write-once path used by the activation policy on first evaluation.

## Failure Model
- Missing settings data is treated as a first-run condition, not as an error.
- Invalid settings data is treated as recoverable local corruption and is cleared automatically.
- If the secure-store backend is unavailable, the helper still falls back to the mirrored `QSettings` value.
