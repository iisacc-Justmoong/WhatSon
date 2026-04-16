# `src/app/viewmodel/detailPanel`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/detailPanel`
- Child directories: 1
- Child files: 20

## Child Directories
- `session`

## Child Files
- `DetailContentSectionViewModel.cpp`
- `DetailContentSectionViewModel.hpp`
- `DetailPanelCurrentHierarchyBinder.cpp`
- `DetailPanelCurrentHierarchyBinder.hpp`
- `DetailCurrentNoteContextBridge.cpp`
- `DetailCurrentNoteContextBridge.hpp`
- `DetailFileStatViewModel.cpp`
- `DetailFileStatViewModel.hpp`
- `DetailHierarchySelectionViewModel.cpp`
- `DetailHierarchySelectionViewModel.hpp`
- `DetailNoteHeaderSelectionSourceViewModel.cpp`
- `DetailNoteHeaderSelectionSourceViewModel.hpp`
- `DetailPanelState.cpp`
- `DetailPanelState.hpp`
- `DetailPanelToolbarItemsFactory.cpp`
- `DetailPanelToolbarItemsFactory.hpp`
- `DetailPanelViewModel.cpp`
- `DetailPanelViewModel.hpp`
- `DetailPropertiesViewModel.cpp`
- `DetailPropertiesViewModel.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- `DetailPanelCurrentHierarchyBinder` now owns the composition-root binding between the active sidebar hierarchy
  context and `DetailPanelViewModel`.
- `DetailPanelViewModel` therefore no longer depends on `main.cpp` lambda wiring to keep its current note-list model
  and current hierarchy directory resolver synchronized.
