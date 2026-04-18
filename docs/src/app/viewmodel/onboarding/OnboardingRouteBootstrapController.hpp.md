# `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.hpp`

## Role
`OnboardingRouteBootstrapController` drives embedded onboarding visibility and route commits.

## Interface Alignment
- The controller now stores `IOnboardingHubController` instead of `OnboardingHubController`.
- This keeps route logic bound only to transition hooks, not the full onboarding implementation.
- The same visibility state is now also reused by the iOS inline onboarding sequence in `Main.qml`, so the controller
  remains the single onboarding/workspace transition authority even when iOS no longer pushes the page stack through
  `/onboarding`.
