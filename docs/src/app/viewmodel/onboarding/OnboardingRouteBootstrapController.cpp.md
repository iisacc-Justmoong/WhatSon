# `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.cpp`

## Implementation Notes
- `setHubController(...)` now accepts `IOnboardingHubController`.
- Route handling still calls `beginWorkspaceTransition`, `completeWorkspaceTransition`, and `failWorkspaceTransition` at the same points.
