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
- That same derived state now feeds both inline resource cards and the dedicated resource editor surface, so image
  preview eligibility stays identical regardless of whether the resource appears inside a note body or as the active
  center-surface selection.
- Defers bitmap format decisions to `ImageFormatCompatibilityLayer` so QML does not duplicate
  extension/MIME compatibility logic.
- The bitmap path no longer trusts `renderMode` as the only authority.
  If the renderer payload forgot to label the resource as `image` but still resolved a bitmap file path/format, the
  viewer now promotes that entry back into the bitmap-preview path instead of leaving it on the empty metadata-card
  fallback.
- Emits `viewerStateChanged()` only when derived state actually changes.

## Tests

- Automated regression coverage now verifies this bridge in `test/cpp/whatson_cpp_regression_tests.cpp`.
- Regression checklist:
  - a compatible image resource entry must produce a local-file `openTarget`, normalized lowercase format, and a
    renderable bitmap state
  - an unsupported image-like entry must still identify itself as a bitmap preview candidate while keeping
    `bitmapRenderable == false`
  - unsupported formats must expose a non-empty incompatibility reason for the dedicated resource editor fallback
