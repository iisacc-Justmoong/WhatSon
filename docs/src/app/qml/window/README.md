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
- `DebugConsole.qml`
- `MacNativeMenuBar.qml`
- `Onboarding.qml`
- `OnboardingContent.qml`
- `Preference.qml`
- `ProfileControl.qml`
- `QuickNote.qml`
- `TrialStatus.qml`

## Current Notes
- `Onboarding.qml`, `OnboardingContent.qml`, and `TrialStatus.qml` now route visible window geometry through LVRS
  `gap`, `radius`, `stroke`, and `scaleMetric(...)` helpers instead of local pixel literals.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
