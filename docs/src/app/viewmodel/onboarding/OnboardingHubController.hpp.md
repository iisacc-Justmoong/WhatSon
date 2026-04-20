# `src/app/viewmodel/onboarding/OnboardingHubController.hpp`

## Role
`OnboardingHubController` owns onboarding-state mutation, hub creation/loading callbacks, and workspace transition state.

## Interface Alignment
- Implements `IOnboardingHubController` for route-layer communication.
- Keeps the broader onboarding surface, properties, and signals available to QML.
- Internal hub mount validation is delegated to the shared hub validator in `src/app/file/hub`.
