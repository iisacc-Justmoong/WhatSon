# `src/app/viewmodel/onboarding/OnboardingHubController.cpp`

## Implementation Notes
- Constructor now initializes the `IOnboardingHubController` base.
- Hub selection, creation, and transition behavior now delegate mount/access + structure validation to
  `WhatSonHubMountValidator` before invoking the runtime load callback.
- The change isolates route orchestration from the full concrete controller surface.
