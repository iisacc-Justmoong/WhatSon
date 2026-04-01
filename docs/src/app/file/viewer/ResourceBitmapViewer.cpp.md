# `src/app/file/viewer/ResourceBitmapViewer.cpp`

## Responsibility
Implements derived bitmap-viewer state projection from resource-entry metadata.

## Key Behavior
- Accepts a generic resource-entry variant payload and normalizes it into:
  - effective `openTarget`
  - canonical `normalizedFormat`
  - `bitmapFormatCompatible` flag
  - `bitmapRenderable` flag
  - user-facing `incompatibilityReason`
- Prefers `source` for open targets and falls back to `resolvedPath` when needed.
- Defers bitmap format decisions to `ImageFormatCompatibilityLayer` so QML does not duplicate
  extension/MIME compatibility logic.
- Emits `viewerStateChanged()` only when derived state actually changes.

