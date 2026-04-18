# `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.cpp`

## Implementation Notes
- `setHubController(...)` now accepts `IOnboardingHubController`.
- Route handling still calls `beginWorkspaceTransition`, `completeWorkspaceTransition`, and `failWorkspaceTransition` at the same points.
- `embeddedOnboardingVisible` now serves two presentation strategies:
  - Android routed onboarding uses `routeSyncRequested(...)` to move between `/onboarding` and `/`.
  - iOS inline onboarding keeps `/` mounted and lets `Main.qml` complete the same transition in place once the
    workspace shell is ready.
