# `src/app/viewmodel/editor/display/ContentsDisplaySelectionMountViewModel.hpp`

## Responsibility

Publishes the editor display selection and mount command surface.

## Contract

- Owns only selected-note snapshot polling, reconcile, editor-session delivery, focus scheduling, and mount hooks.
- Delegates implementation to the model-side selection/mount controller.
- Does not own visual layout or document rendering.
