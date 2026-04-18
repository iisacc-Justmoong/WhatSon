# `src/app/models/display/paper/ContentsPaperSelection.cpp`

## Responsibility

Implements paper-selection enum normalization and the currently chosen paper state.

## Key Behavior

- Maps stable enum values to user-visible paper labels:
  - `A4`
  - `Letter`
  - `Legal`
  - `A5`
  - `B5`
  - fallback `Unknown`
- Keeps the current paper choice inside one QObject so QML and future viewmodels can observe the selected paper
  without inventing duplicate selection state.
- Normalizes arbitrary integer input through `normalizePaperKind(...)` so unsupported values collapse to `Unknown`
  instead of becoming unchecked enum garbage.
- Starts from `A4` by default because the current paper background implementation still anchors the live page/print
  experience to A4.

## Regression Checks

- The selection object must keep `A4` as its default chosen paper.
- Valid enum values must round-trip to the expected user-visible paper label.
- Unsupported integer values must normalize to `Unknown`.
