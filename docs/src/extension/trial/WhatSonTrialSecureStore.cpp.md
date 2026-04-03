# `src/extension/trial/WhatSonTrialSecureStore.cpp`

## Role
Implements the default host secure-store backend for the optional trial module.

## Behavior
- macOS uses the system Keychain through `Security.framework` generic-password entries.
- Linux uses `secret-tool` when the host provides a Secret Service bridge.
- Unsupported platforms report the secure store as unavailable and let higher layers defer secret-backed verification or migration instead of trusting plain local settings.

## Integration Intent
- The secure store is deliberately kept behind a tiny interface so non-production flows can use an in-memory backend.
- Trial persistence helpers treat secure-store-backed identity material as authoritative whenever it exists.
- The install-date helper now uses the secure store only as a legacy migration source; the signed `QSettings` record is the long-term local source of truth for the date itself.
