# `src/app/runtime/startup/WhatSonStartupHubResolver.cpp`

## Implementation Notes
- Startup selection resolution now reads persisted hub state through `ISelectedHubStore`.
- Individual hub mount/access preflight is delegated to `WhatSonHubMountValidator`.
- If no persisted startup hub exists, the resolver returns an empty selection and startup stays on onboarding.
- If a persisted startup hub exists but cannot be mounted, the resolver preserves that failure instead of silently
  switching to any fallback hub. The onboarding surface can then show the real startup error.
- Mount and validation behavior for an individual hub candidate now comes from the shared hub validator used by
  onboarding as well.
