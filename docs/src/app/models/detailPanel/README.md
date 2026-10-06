# `src/app/models/detailPanel`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/detailPanel`
- Child directories: 1
- Child files: 23

## Child Directories
- `session`

## Child Files
- `DetailContentSectionController.cpp`
- `DetailContentSectionController.hpp`
- `DetailPanelCurrentHierarchyBinder.cpp`
- `DetailPanelCurrentHierarchyBinder.hpp`
- `DetailCurrentNoteContextBridge.cpp`
- `DetailCurrentNoteContextBridge.hpp`
- `DetailFileStatController.cpp`
- `DetailFileStatController.hpp`
- `DetailHierarchySelectionController.cpp`
- `DetailHierarchySelectionController.hpp`
- `DetailNoteHeaderSelectionSourceController.cpp`
- `DetailNoteHeaderSelectionSourceController.hpp`
- `DetailPanelState.cpp`
- `DetailPanelState.hpp`
- `DetailPanelToolbarItemsFactory.cpp`
- `DetailPanelToolbarItemsFactory.hpp`
- `DetailPanelController.cpp`
- `DetailPanelController.hpp`
- `NoteDetailPanelController.hpp`
- `ResourceDetailPanelController.cpp`
- `ResourceDetailPanelController.hpp`
- `DetailPropertiesController.cpp`
- `DetailPropertiesController.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- `DetailPanelCurrentHierarchyBinder` now owns the composition-root binding between the active sidebar hierarchy
  context plus the concrete note/resource detail-panel controllers.
- The runtime now mounts a dedicated `NoteDetailPanelController` for note-backed hierarchies and a separate
  `ResourceDetailPanelController` for the resources hierarchy.
- `DetailPanelController` therefore remains the reusable note-detail implementation instead of acting as a shared
  one-size-fits-all detail-panel selector object.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/detailPanel``  ( `docs/src/app/models/detailPanel/README.md` )
- Location:  `docs/src/app/models/detailPanel`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
