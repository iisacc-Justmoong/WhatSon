# `src/app/models/editor/resource/ContentsInlineResourcePresentationController.hpp`

## Responsibility

Declares the C++ inline resource presentation controller consumed by editor view QML.

## Contract

- Exposes resource-frame HTML rendering helpers through narrow invokables.
- Exposes `inlineResourceVisualHeights(...)` so editor geometry can consume explicit resource block heights without
  parsing the rendered HTML projection.
- Keeps inline resource placeholder replacement in C++ instead of a QML helper.
- Does not mutate editor source or resource import state.
