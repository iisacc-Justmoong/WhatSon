# `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.cpp`

## Implementation Notes
- `setHubController(...)` now accepts `IOnboardingHubController`.
- Route handling still calls `beginWorkspaceTransition`, `completeWorkspaceTransition`, and `failWorkspaceTransition` at the same points.
- `embeddedOnboardingVisible` now serves two presentation strategies:
  - Android routed onboarding uses `routeSyncRequested(...)` to move between `/onboarding` and `/` inside the same
    main window session.
  - iOS inline onboarding still keeps `/` mounted and swaps the workspace page loader instead of pushing the route
    stack through `/onboarding`.
- Desktop ordinary startup does not rely on this route controller for first-run presentation anymore; `Main.qml` reopens
  the separate `Onboarding.qml` window when no hub is mounted.
