# `src/app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.hpp`

## Responsibility

Declares the C++ ViewModel policy that decides which document surface `ContentsDisplayView.qml` may mount.

## Contract

- `hasSelectedNote` is the primary input from the layout host.
- A selected note requests the structured document surface.
- Legacy whole-note inline editor mounting remains disabled through `inlineDocumentSurfaceRequested == false`.
- Resource viewer and formatted-preview requests are explicit policy inputs, not hard-coded `ContentsDisplayView.qml`
  branches.
