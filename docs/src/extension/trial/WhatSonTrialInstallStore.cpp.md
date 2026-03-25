# `src/extension/trial/WhatSonTrialInstallStore.cpp`

## Role
Implements the `QSettings`-backed persistence path for the optional trial module.

## Behavior
- Stored values are normalized to ISO `YYYY-MM-DD` text.
- Invalid or malformed stored values are deleted on read so future evaluations can recreate a clean install date.
- `ensureInstallDate(...)` is the write-once path used by the activation policy on first evaluation.

## Failure Model
- Missing settings data is treated as a first-run condition, not as an error.
- Invalid settings data is treated as recoverable local corruption and is cleared automatically.
