# `src/app/file/viewer/ResourceBitmapViewer.cpp`

## Responsibility
Implements derived bitmap-viewer state projection from resource-entry metadata.

## Key Behavior
- Accepts a generic resource-entry variant payload and normalizes it into:
  - effective `openTarget`
  - canonical `normalizedFormat`
  - `bitmapPreviewCandidate` flag
  - `bitmapFormatCompatible` flag
  - `bitmapRenderable` flag
  - user-facing `incompatibilityReason`
- Prefers `source` for open targets and falls back to `resolvedPath` when needed.
- Defers bitmap format decisions to `ImageFormatCompatibilityLayer` so QML does not duplicate
  extension/MIME compatibility logic.
- The bitmap path no longer trusts `renderMode` as the only authority.
  If the renderer payload forgot to label the resource as `image` but still resolved a bitmap file path/format, the
  viewer now promotes that entry back into the bitmap-preview path instead of leaving it on the empty metadata-card
  fallback.
- Emits `viewerStateChanged()` only when derived state actually changes.
