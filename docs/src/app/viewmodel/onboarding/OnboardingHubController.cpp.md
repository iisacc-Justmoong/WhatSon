# `src/app/viewmodel/onboarding/OnboardingHubController.cpp`

## Implementation Notes
- Constructor now initializes the `IOnboardingHubController` base.
- Existing-hub loading now tracks the original selection URL alongside the resolved mount path, so main-thread startup
  persistence can keep the provider-backed iOS pick alive with a matching security-scoped bookmark.
- iOS direct `.wshub` file picks now flow through the same controller path as desktop file picks instead of requiring a
  folder-only indirection step.
- Newly created hubs on iOS now refresh their stored bookmark against the final `.wshub` package path rather than
  keeping a parent-directory bookmark from the pre-create picker location.
- The controller now stops after validating and normalizing a hub selection; it emits a selection-ready signal and lets
  `main.cpp` own the actual runtime mount/load decision.
- `hubLoaded(...)` is again reserved for the post-load success path, and is now emitted only when the host composition
  root confirms that the selected hub has been mounted into the workspace runtime successfully.
