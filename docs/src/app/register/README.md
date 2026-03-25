# `src/app/register`

## Role
This directory contains the app-core registration state objects that are shared with optional entitlement modules such as the trial extension.

## Files
- `WhatSonRegisterManager.hpp` / `WhatSonRegisterManager.cpp`: persist the app authentication-complete flag and expose it as a small `QObject` manager.

## Contract
- `authenticated == true` means the app has completed the product authentication flow.
- Trial-only modules may read this manager and disable trial restrictions entirely when the flag is `true`.
- The flag is persisted in `QSettings` so the authenticated state survives app restarts.
