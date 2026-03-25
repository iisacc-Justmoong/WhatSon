# `src/extension/trial/WhatSonTrialClientIdentityStore.cpp`

## Role
Implements the trial-only client identity persistence layer.

## Behavior
- Invalid stored UUID, key, or register-secret values are removed during reads.
- The device UUID, client key, and register-integrity secret are mirrored into `QSettings` and the trial secure store.
- When secure-store values exist, they repopulate `QSettings` so reinstalling the app does not silently reset the trial identity.
- Missing device UUID values are derived from `QSysInfo::machineUniqueId()` when available, with a random UUID fallback.
- Missing client keys are generated as 32-character alpha-numeric strings.
- Missing register-integrity secrets are generated as 64-character lowercase hexadecimal strings.
- `ensureIdentity()` persists the generated values so the same in-app key and signing secret survive across the current app install.
