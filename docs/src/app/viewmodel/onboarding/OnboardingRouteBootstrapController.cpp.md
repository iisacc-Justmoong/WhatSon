# `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.cpp`

## Implementation Notes
- `setHubController(...)` now accepts `IOnboardingHubController`.
- Route handling still calls `beginWorkspaceTransition`, `completeWorkspaceTransition`, and `failWorkspaceTransition` at the same points.
- `embeddedOnboardingVisible` now serves two presentation strategies:
  - Android routed onboarding uses `routeSyncRequested(...)` to move between `/onboarding` and `/`.
  - iOS inline onboarding still keeps `/` mounted when the main window is already active, while launch-time missing-hub
    recovery is now handled earlier in `main.cpp` by the standalone onboarding-window flow.
