# `src/app/models/editor/resource/ContentsInlineResourcePresentationController.hpp`

## Responsibility

Declares the C++ inline resource presentation controller consumed by editor view QML.

## Contract

- Exposes legacy resource-frame HTML rendering helpers through narrow invokables for compatibility with older
  placeholder projections.
- Exposes `inlineResourceVisualBlocks(...)` as the normal editor-surface contract. QML delegates consume those records
  directly and geometry reads their heights without parsing rendered HTML.
- Exposes `editorSurfaceHtmlWithResourceVisualBlocks(...)` for the RichText overlay flow. The method keeps resource
  height ownership in C++ by turning resource tokens into transparent block-flow spacers, while the actual image frame
  is still painted by the visual delegate.
- Keeps resource-entry normalization and frame-image generation in C++ instead of a QML helper.
- Does not mutate editor source or resource import state.
