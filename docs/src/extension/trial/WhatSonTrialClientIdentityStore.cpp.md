# `src/extension/trial/WhatSonTrialClientIdentityStore.cpp`

## Role
Implements the trial-only client identity persistence layer.

## Behavior
- Invalid stored UUID or key values are removed from `QSettings` during reads.
- Missing device UUID values are derived from `QSysInfo::machineUniqueId()` when available, with a random UUID fallback.
- Missing client keys are generated as 32-character alpha-numeric strings.
- `ensureIdentity()` persists the generated values so the same in-app key survives across the current app install.
