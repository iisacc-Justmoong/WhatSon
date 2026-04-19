# `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.hpp`

## Role
`OnboardingRouteBootstrapController` drives embedded onboarding visibility and route commits.

## Interface Alignment
- The controller now stores `IOnboardingHubController` instead of `OnboardingHubController`.
- This keeps route logic bound only to transition hooks, not the full onboarding implementation.
- The same visibility state is now also reused by the iOS inline onboarding sequence in `Main.qml`, so the controller
  remains the single onboarding/workspace transition authority for Android and iOS even when iOS no longer pushes the
  page stack through `/onboarding`.
- Desktop still injects the controller for explicit reopen hooks, but ordinary startup presentation is owned by the
  separate `Onboarding.qml` window instead of the embedded route stack.
