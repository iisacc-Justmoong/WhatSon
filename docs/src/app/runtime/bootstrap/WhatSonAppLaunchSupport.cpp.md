# `src/app/runtime/bootstrap/WhatSonAppLaunchSupport.cpp`

## Role
Owns command-line parsing for bootstrap-only launch behaviors.

## Implementation Notes
- `parseLaunchOptions(...)` still recognizes the dedicated onboarding-only launch path.
- Workspace-versus-onboarding startup readiness is intentionally defined in the header helper so both `main.cpp` and
  C++ regression tests evaluate the same rule.
