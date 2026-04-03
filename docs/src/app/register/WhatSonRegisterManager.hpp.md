# `src/app/register/WhatSonRegisterManager.hpp`

## Role
Declares the app-core registration manager that owns the persisted authentication-complete state.

## Public API
- `authenticated()`: returns the cached authentication-complete flag.
- `setAuthenticated(...)`: updates the flag, persists it, and emits `authenticatedChanged()` when the value changes.
- `reload()`: reloads the persisted state from `QSettings`, verifying the signed trial-build record before accepting it.
- `clearAuthentication()`: convenience slot that resets the flag to `false`.

## Integration
- Optional modules such as `src/extension/trial` can depend on this manager without moving trial-only code into the mandatory app build graph.
