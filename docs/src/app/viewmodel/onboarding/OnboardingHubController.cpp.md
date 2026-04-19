# `src/app/viewmodel/onboarding/OnboardingHubController.cpp`

## Implementation Notes
- Constructor now initializes the `IOnboardingHubController` base.
- Existing-hub loading now tracks the original selection URL alongside the resolved mount path, so main-thread startup
  persistence can keep the provider-backed iOS pick alive with a matching security-scoped bookmark.
- Folder-based hub preparation now also preserves the original picker URL before package-path promotion/validation.
  That keeps iOS Files/cloud-provider selections restorable even when onboarding resolves the chosen folder down to a
  concrete `.wshub` package path later in the same operation.
- Folder-based hub creation no longer loops back through a synthesized child `.wshub` URL. The controller now starts
  access on the user-selected directory URL, computes the final package path inside that directory, and invokes the
  shared `WhatSonHubCreator` callback directly so the full hub scaffold is created under the original provider scope.
- iOS direct file-pick onboarding now supports selecting a file from inside a `.wshub` package. The controller derives
  how many path components it must strip from the picked provider URL, rewrites the stored selection URL to the
  enclosing package URL, and persists the bookmark against that ancestor scope instead of the leaf file.
- Newly created hubs on iOS now refresh their stored bookmark against the final `.wshub` package path rather than
  keeping a parent-directory bookmark from the pre-create picker location.
- The controller now stops after validating and normalizing a hub selection; it emits a selection-ready signal and lets
  `main.cpp` own the actual runtime mount/load decision.
- `hubLoaded(...)` is again reserved for the post-load success path, and is now emitted only when the host composition
  root confirms that the selected hub has been mounted into the workspace runtime successfully.
