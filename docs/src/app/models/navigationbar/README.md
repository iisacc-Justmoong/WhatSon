# `src/app/models/navigationbar`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/navigationbar`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
- `NavigationModeSectionController.cpp`
- `NavigationModeSectionController.hpp`
- `NavigationModeState.cpp`
- `NavigationModeState.hpp`
- `NavigationModeController.cpp`
- `NavigationModeController.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- Automated C++ regression coverage now lives in `test/cpp/suites/*.cpp`, locking state cycling,
  invalid-value rejection, and active-section synchronization for `NavigationModeController`.
- The editor view-mode controller family is removed. Navigation mode state remains focused on application-level
  sections only.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/navigationbar``  ( `docs/src/app/models/navigationbar/README.md` )
- Location:  `docs/src/app/models/navigationbar`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Current editor view-mode controller family is in deleted state and navigation mode handles only application-level sections.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
