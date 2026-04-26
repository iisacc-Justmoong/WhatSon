# `src/app/viewmodel/editor/display/ContentsDisplayPresentationViewModel.hpp`

## Responsibility

Publishes the editor display presentation command surface.

## Contract

- Owns only presentation refresh, creation trace, inline resource refresh, and view-hook dispatch hooks.
- Delegates implementation to the model-side presentation controller.
- Does not perform layout or RAW source mutation.
