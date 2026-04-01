# `src/app/file/viewer/ResourceBitmapViewer.hpp`

## Responsibility
Declares the QML bridge that projects a resource-entry payload into bitmap-viewer state.

## Public Contract
- `resourceEntry`
  Raw resource payload from `ContentsBodyResourceRenderer`.
- `openTarget`
  Effective open target URL/path used by inline preview or external open actions.
- `viewerSource`
  Bitmap source URL used by `Image` when in-app preview is renderable.
- `normalizedFormat`
  Canonical bitmap format token resolved from entry metadata/path.
- `bitmapFormatCompatible`
  Whether the format is compatible with the current runtime bitmap decoders.
- `bitmapRenderable`
  True only when render mode is `image`, source exists, and format compatibility is confirmed.
- `incompatibilityReason`
  Failure reason used by fallback UI when bitmap preview cannot render inline.

