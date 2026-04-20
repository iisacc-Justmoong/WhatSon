# `src/app/viewmodel/navigationbar`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/navigationbar`
- Child directories: 0
- Child files: 12

## Child Directories
- No child directories.

## Child Files
- `EditorViewModeViewModel.cpp`
- `EditorViewModeViewModel.hpp`
- `EditorViewSectionViewModel.cpp`
- `EditorViewSectionViewModel.hpp`
- `EditorViewState.cpp`
- `EditorViewState.hpp`
- `NavigationModeSectionViewModel.cpp`
- `NavigationModeSectionViewModel.hpp`
- `NavigationModeState.cpp`
- `NavigationModeState.hpp`
- `NavigationModeViewModel.cpp`
- `NavigationModeViewModel.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- Automated C++ regression coverage now lives in `test/cpp/suites/*.cpp`, locking state cycling,
  invalid-value rejection, and active-section synchronization for `NavigationModeViewModel` and
  `EditorViewModeViewModel`.
