# `src/extension/trial/WhatSonTrialInstallStore.cpp`

## Role
Implements the install-date persistence path for the optional trial module.

## Behavior
- Stored values are normalized to ISO `YYYY-MM-DD` text.
- The persisted value is a signed JSON record in `QSettings`, not a plain ISO string.
- The signature is derived from the secure-store-backed register-integrity secret, so local settings edits cannot extend the trial window silently.
- Legacy plain `QSettings` dates and legacy secure-store mirrors are migrated into the signed record format when verification material is available.
- Invalid or malformed unsigned values are deleted on read so future evaluations can recreate a clean install date.
- `ensureInstallDate(...)` is the write-once path used by the activation policy on first evaluation.

## Failure Model
- Missing settings data is treated as a first-run condition, not as an error.
- Invalid settings data is treated as recoverable local corruption and is cleared automatically.
- If the signing secret cannot be loaded or persisted, the helper fails closed and returns an invalid install date instead of trusting an unsigned fallback.
