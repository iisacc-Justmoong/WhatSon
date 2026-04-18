# `src/app/file/viewer`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/viewer`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
- `ContentsBodyResourceRenderer.cpp`
- `ContentsBodyResourceRenderer.hpp`
- `ImageFormatCompatibilityLayer.cpp`
- `ImageFormatCompatibilityLayer.hpp`
- `ResourceBitmapViewer.cpp`
- `ResourceBitmapViewer.hpp`

## Current Notes
- `ContentsBodyResourceRenderer` now emits verbose editor trace events for content-view-model rebinding, note-id/path
  changes, filesystem mutation callbacks, render refresh passes, and resolved inline-resource counts so resource block
  materialization can be correlated with note-open and editor RAW mutations.
- `ResourceBitmapViewer` now also acts as the shared image-preview projection bridge for the dedicated resource editor.
  QML routing can hand a direct resource entry to one C++ bridge and receive stable bitmap renderability, normalized
  format, and viewer-target state without duplicating path/extension logic in the view layer.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
