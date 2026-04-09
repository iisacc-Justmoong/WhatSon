# `src/app/viewmodel/hierarchy/tags`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/hierarchy/tags`
- Child directories: 0
- Child files: 5

## Child Directories
- No child directories.

## Child Files
- `TagsHierarchyModel.cpp`
- `TagsHierarchyModel.hpp`
- `TagsHierarchyViewModel.cpp`
- `TagsHierarchyViewModel.hpp`
- `TagsHierarchyViewModelSupport.hpp`

## Recent Notes
- `TagsHierarchyViewModel` now owns both the hierarchy rows and the tag-projected `LibraryNoteListModel`,
  so the Tags sidebar domain can return notes directly.
- Detail-panel tag writes now re-synchronize the tags viewmodel cache through
  `reloadNoteMetadataForNoteId(...)`, avoiding stale tag note lists when the current active domain
  is not Tags.
- Tag hierarchy rows now also expose live subtree note counts derived from the indexed note cache,
  so sidebar badges change in lockstep with tag projection refreshes instead of staying pinned to `0`.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
