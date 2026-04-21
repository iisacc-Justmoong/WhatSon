# `src/app/models/file/viewer/ImageFormatCompatibilityLayer.hpp`

## Responsibility
Declares the image-format compatibility layer used by in-app bitmap viewers.

## Public Contract
- `normalizedBitmapFormat(...)`
  Normalizes format probes (extension, MIME string, local path, or URL) into a canonical
  lowercase extension token (for example `.jpg`, `.png`).
- `isBitmapFormatCompatible(...)`
  Checks whether the normalized format is supported by the current Qt image-reader runtime.
- `unsupportedBitmapFormatMessage(...)`
  Produces a stable user-facing message for unsupported or missing bitmap format metadata.

