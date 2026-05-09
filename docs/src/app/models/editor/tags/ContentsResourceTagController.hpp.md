# `src/app/models/editor/tags/ContentsResourceTagController.hpp`

## Responsibility

Declares the C++ controller that builds RAW resource-tag insertion mutation payloads.

## Contract

- Uses `ContentsResourceTagTextGenerator` for canonical `<resource ... />` source text.
- Resolves insertion offsets and selection payloads without QML-side mutation policy.
- Treats `editorSession.editorText` as the fallback source authority, so large plain editor sessions do not need to copy
  their full RAW source into resource-tag compatibility properties on each text edit.
- Keeps resource tag operations in the editor tag model layer.
