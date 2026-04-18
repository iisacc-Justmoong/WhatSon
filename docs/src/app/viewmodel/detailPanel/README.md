# `src/app/viewmodel/detailPanel`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/detailPanel`
- Child directories: 1
- Child files: 23

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
- `NoteDetailPanelViewModel.hpp`
- `ResourceDetailPanelViewModel.cpp`
- `ResourceDetailPanelViewModel.hpp`
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
  context plus the concrete note/resource detail-panel viewmodels.
- The runtime now mounts a dedicated `NoteDetailPanelViewModel` for note-backed hierarchies and a separate
  `ResourceDetailPanelViewModel` for the resources hierarchy.
- `DetailPanelViewModel` therefore remains the reusable note-detail implementation instead of acting as a shared
  one-size-fits-all detail-panel selector object.
