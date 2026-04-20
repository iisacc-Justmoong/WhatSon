# `src/app/runtime/bootstrap/WhatSonAppLaunchSupport.hpp`

## Role
Declares bootstrap-time launch-option helpers that `main.cpp` can use without re-encoding startup policy inline.

## Implementation Notes
- `startupWorkspaceReady(...)` now centralizes the rule that startup may skip onboarding only when a hub both mounts
  successfully and finishes the first runtime load.
- The helper keeps the startup fallback policy testable from the C++ regression suite without depending on `main.cpp`.
