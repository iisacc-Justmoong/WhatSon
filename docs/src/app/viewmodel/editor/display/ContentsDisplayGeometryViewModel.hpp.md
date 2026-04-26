# `src/app/viewmodel/editor/display/ContentsDisplayGeometryViewModel.hpp`

## Responsibility

Publishes the editor display geometry command surface.

## Contract

- Owns only minimap, gutter, logical-line, and viewport-correction hooks.
- Delegates implementation to the model-side display geometry controller.
- Remains C++ and slot-based so QML view code cannot become the ViewModel layer.
