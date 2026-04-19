# `src/app/viewmodel/onboarding/OnboardingHubController.hpp`

## Role
`OnboardingHubController` owns onboarding-state mutation, hub creation/loading callbacks, and workspace transition state.

## Interface Alignment
- Implements `IOnboardingHubController` for route-layer communication.
- Keeps the broader onboarding surface, properties, and signals available to QML.
- Also exposes the current hub bookmark and persisted selection URL to the composition root, so startup persistence can
  store both the resolved hub path and the original iOS picker URL together.
