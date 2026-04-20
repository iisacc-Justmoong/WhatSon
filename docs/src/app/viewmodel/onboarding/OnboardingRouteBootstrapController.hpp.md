# `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.hpp`

## Role
`OnboardingRouteBootstrapController` drives embedded onboarding visibility and route commits.

## Interface Alignment
- The controller now stores `IOnboardingHubController` instead of `OnboardingHubController`.
- This keeps route logic bound only to transition hooks, not the full onboarding implementation.
- `configure(...)` is keyed by startup workspace readiness, so mobile onboarding stays mounted whenever startup mount
  validation or the first runtime load leaves the app without a usable workspace.
