# `src/app/models/editor/tags/ContentsResourceTagController.hpp`

## Responsibility

Declares the C++ controller that builds RAW resource-tag insertion mutation payloads.

## Contract

- Uses `ContentsResourceTagTextGenerator` for canonical `<resource ... />` source text.
- Resolves insertion offsets and selection payloads without QML-side mutation policy.
- Keeps resource tag operations in the editor tag model layer.
