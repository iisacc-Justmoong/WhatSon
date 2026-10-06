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
  page, avoiding the `/onboarding` route flip while reusing the shared onboarding content surface.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/qml/window``  ( `docs/src/app/qml/window/README.md` )
- Location:  `docs/src/app/qml/window`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
