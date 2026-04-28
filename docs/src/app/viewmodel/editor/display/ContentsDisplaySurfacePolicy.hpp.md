# `src/app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.hpp`

## Responsibility

Declares the C++ ViewModel policy that decides which document surface `ContentsDisplayView.qml` may mount.

## Contract

- `hasSelectedNote` is the primary input from the layout host.
- A selected note requests the structured document surface.
- Legacy whole-note inline editor mounting is no longer part of this policy contract at all.
  The policy exposes only the structured note host and the explicit non-editor display modes.
- Resource viewer and formatted-preview requests are explicit policy inputs, not hard-coded `ContentsDisplayView.qml`
  branches.
