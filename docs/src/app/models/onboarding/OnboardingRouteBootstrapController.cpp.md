# `src/app/models/onboarding/OnboardingRouteBootstrapController.cpp`

## Implementation Notes
- `setHubController(...)` now accepts `IOnboardingHubController`.
- Route handling still calls `beginWorkspaceTransition`, `completeWorkspaceTransition`, and `failWorkspaceTransition` at the same points.
- Startup configuration now treats a hub as workspace-ready only after both mount validation and the first runtime load
  succeed, which keeps embedded onboarding visible for startup failures that happen after mount preflight.
