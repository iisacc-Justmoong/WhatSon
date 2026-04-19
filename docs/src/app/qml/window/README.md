# `src/app/qml/window`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/window`
- Child directories: 1
- Child files: 8

## Child Directories
- `preference`

## Child Files
- `MacNativeMenuBar.qml`
- `IosInlineOnboardingSequence.qml`
- `Onboarding.qml`
- `OnboardingContent.qml`
- `Preference.qml`
- `ProfileControl.qml`
- `QuickNote.qml`
- `TrialStatus.qml`

## Current Notes
- Scene-graph visualization helpers were removed from the runtime window set. This directory now only contains
  user-facing application windows and onboarding/trial surfaces.
- `Onboarding.qml`, `OnboardingContent.qml`, and `TrialStatus.qml` now route visible window geometry through LVRS
  `gap`, `radius`, `stroke`, and `scaleMetric(...)` helpers instead of local pixel literals.
- `IosInlineOnboardingSequence.qml` now keeps the iOS inline onboarding presentation inside the root LVRS workspace
  page, avoiding the `/onboarding` route flip while reusing the shared onboarding content surface.
- Ordinary desktop/mobile startup onboarding now lives inside `Main.qml`; `Onboarding.qml` remains as the explicit
  standalone shell for `--onboarding-only`.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
