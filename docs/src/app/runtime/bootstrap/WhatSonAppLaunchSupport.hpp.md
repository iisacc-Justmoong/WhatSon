# `src/app/runtime/bootstrap/WhatSonAppLaunchSupport.hpp`

## Role
Declares bootstrap-time launch-option helpers that `main.cpp` can use without re-encoding startup policy inline.

## Implementation Notes
- `startupWorkspaceReady(...)` now centralizes the rule that startup may skip onboarding as soon as a persisted hub
  mounts successfully.
- The first runtime load is scheduled after the workspace root reaches LVRS `AfterFirstIdle`, so domain I/O and
  snapshot application do not block initial window creation.
- The helper keeps the startup fallback policy testable from the C++ regression suite without depending on `main.cpp`.
