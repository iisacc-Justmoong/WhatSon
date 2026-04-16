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

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
