# `src/app/viewmodel/onboarding/OnboardingHubController.cpp`

## Implementation Notes
- Constructor now initializes the `IOnboardingHubController` base.
- Hub selection, creation, and transition behavior are unchanged.
- The change isolates route orchestration from the full concrete controller surface.
