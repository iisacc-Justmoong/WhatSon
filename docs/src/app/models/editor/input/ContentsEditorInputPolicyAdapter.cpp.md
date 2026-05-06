# `src/app/models/editor/input/ContentsEditorInputPolicyAdapter.cpp`

## Responsibility

Implements editor input-policy decisions that must not live in QML helper files.

## Current Behavior

- Normalizes text before programmatic source-sync comparisons.
- Defers host-driven text replacement during native composition or focused local text/selection interaction.
- Allows immediate programmatic sync only when the native editor is not protecting an active user input session.
- Computes focus-restoration eligibility for mutation paths.
