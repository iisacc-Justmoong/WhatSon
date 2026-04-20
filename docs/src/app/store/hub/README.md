# `src/app/store/hub`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/store/hub`
- Child directories: 0
- Child files: 2

## Child Directories
- No child directories.

## Child Files
- `SelectedHubStore.cpp`
- `SelectedHubStore.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- Automated C++ regression coverage now lives in `test/cpp/suites/*.cpp`, locking sandboxed
  `QSettings` persistence, normalization, clear behavior, and startup resolver policy around persisted selections
  versus onboarding fallback.
