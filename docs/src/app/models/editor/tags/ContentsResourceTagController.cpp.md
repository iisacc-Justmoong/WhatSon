# `src/app/models/editor/tags/ContentsResourceTagController.cpp`

## Responsibility

Implements RAW resource-tag insertion and tag-loss detection.

## Current Behavior

- Normalizes imported resource entries before tag generation.
- Builds structured-document mutation payloads for resource insertion.
- Counts canonical resource tags before and after mutation so resource insertion cannot silently drop existing tags.
- Resolves the current RAW editor source lazily from the bound `ContentsEditorSessionController` when the display backend
  has intentionally left the legacy string properties empty for a large plain document fast path.
