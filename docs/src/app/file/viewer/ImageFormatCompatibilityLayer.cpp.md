# `src/app/file/viewer/ImageFormatCompatibilityLayer.cpp`

## Responsibility
Implements format normalization and runtime compatibility checks for bitmap preview.

## Key Behavior
- Accepts format probes from:
  - direct extension values (`.PNG`, `jpeg`)
  - MIME values (`image/jpeg`, `image/webp`)
  - local paths and URL strings (`/tmp/photo.png`, `file:///tmp/photo.png`)
- Canonicalizes known aliases (`.jpeg` -> `.jpg`, `.tif` -> `.tiff`).
- Builds a compatibility set from `QImageReader::supportedImageFormats()` and uses it as the
  single source of truth for in-app bitmap renderability.
- Returns a deterministic incompatibility message when preview cannot be rendered in-app.

