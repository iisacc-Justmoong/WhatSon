# `src/app/models/onboarding/IOnboardingHubController.hpp`

## Role
`IOnboardingHubController` defines the workspace-transition hooks needed by onboarding route orchestration.

## Contract
- `beginWorkspaceTransition()`
- `completeWorkspaceTransition()`
- `failWorkspaceTransition(const QString&)`

## Notes
- `OnboardingRouteBootstrapController` now targets this interface instead of the concrete onboarding controller.
