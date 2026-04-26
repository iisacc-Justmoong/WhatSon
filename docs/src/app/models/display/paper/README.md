# `src/app/models/display/paper`

## Responsibility
Owns paper-surface helpers shared by page and print editor modes.

## Child Files
- `ContentsA4PaperBackground.cpp`
- `ContentsA4PaperBackground.hpp`
- `ContentsPaperSelection.cpp`
- `ContentsPaperSelection.hpp`

## Child Directories
- `print`

## Current Notes
- `ContentsA4PaperBackground` is the canonical A4 paper background object for shared paper geometry and palette tokens.
- `ContentsPaperSelection` is the shared enum-backed paper-choice object that tells the rest of the display layer
  which paper standard is currently selected.
- Inline text formatting moved to `src/app/models/editor/format`, so this directory only owns reusable paper-surface
  state and the `print` child shard owns print layout.
