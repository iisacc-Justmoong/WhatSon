# `src/app/viewmodel/editor/display/ContentsDisplayMutationViewModel.hpp`

## Responsibility

Publishes the editor display mutation command surface.

## Contract

- Owns only RAW source mutation, inline formatting, structured insertion, agenda task rewrite, and persistence hooks.
- Delegates implementation to the model-side mutation controller.
- Does not render text, own timers, or persist alternate editor-surface state.
