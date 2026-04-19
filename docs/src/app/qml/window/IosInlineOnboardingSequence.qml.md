# `src/app/qml/window/IosInlineOnboardingSequence.qml`

## Role
`IosInlineOnboardingSequence.qml` hosts the shared onboarding surface inside the main LVRS workspace page on iOS for
both startup missing-hub recovery and explicit reopen flows.

## Responsibilities
- Keep iOS onboarding pinned to the workspace route instead of pushing the LVRS page stack through `/onboarding`.
- Reuse `OnboardingContent.qml` so iOS onboarding behavior stays aligned with the existing desktop/mobile onboarding
  business rules.
- Paint a full-window canvas behind the shared onboarding content when the inline sequence is shown.

## Invariants
- `autoCompleteOnHubLoaded` stays `false`; `Main.qml` and `OnboardingRouteBootstrapController` remain responsible for
  completing the workspace transition.
- The component must remain a pure in-window host. It must not create another top-level `Window` or bypass
  `LV.ApplicationWindow`.
