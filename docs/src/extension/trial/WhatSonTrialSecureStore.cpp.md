# `src/extension/trial/WhatSonTrialSecureStore.cpp`

## Role
Implements the default host secure-store backend for the optional trial module.

## Behavior
- macOS uses the system Keychain through `Security.framework` generic-password entries.
- Linux uses `secret-tool` when the host provides a Secret Service bridge.
- Unsupported platforms report the secure store as unavailable and let higher layers fall back to the mirrored `QSettings` copy.

## Integration Intent
- The secure store is deliberately kept behind a tiny interface so tests can use an in-memory backend.
- Trial persistence helpers treat the secure-store copy as authoritative whenever it exists, which lets the optional trial kit survive reinstall-driven `QSettings` resets.
