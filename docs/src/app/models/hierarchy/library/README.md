# `src/app/models/hierarchy/library`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/hierarchy/library`
- Child directories: 0
- Child files: 30

## Child Directories
- No child directories.

## Child Files
- `LibraryAll.cpp`
- `LibraryAll.hpp`
- `LibraryDraft.cpp`
- `LibraryDraft.hpp`
- `LibraryHierarchyController.cpp`
- `LibraryHierarchyController.hpp`
- `LibraryHierarchyControllerSupport.hpp`
- `LibraryHierarchyModel.hpp`
- `LibraryNoteListModel.cpp`
- `LibraryNoteListModel.hpp`
- `LibraryNoteMutationController.cpp`
- `LibraryNoteMutationController.hpp`
- `LibraryNoteRecord.hpp`
- `LibraryNotePreviewText.hpp`
- `LibraryToday.cpp`
- `LibraryToday.hpp`
- `WhatSonLibraryIndexedState.cpp`
- `WhatSonLibraryIndexedState.hpp`
- `WhatSonLibraryFolderHierarchyMutationService.cpp`
- `WhatSonLibraryFolderHierarchyMutationService.hpp`
- `WhatSonLibraryHierarchyCreator.cpp`
- `WhatSonLibraryHierarchyCreator.hpp`
- `WhatSonLibraryHierarchyParser.cpp`
- `WhatSonLibraryHierarchyParser.hpp`
- `WhatSonLibraryHierarchyStore.cpp`
- `WhatSonLibraryHierarchyStore.hpp`
- `WhatSonLibraryNoteListProjection.cpp`
- `WhatSonLibraryNoteListProjection.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Notes
- `LibraryNoteRecord` now exposes structural equality so incremental mutation layers can suppress no-op updates before
  they fan out into note-list/calendar rebuilds.
- `LibraryAll`, `LibraryDraft`, and `LibraryToday` now all support single-note upsert/remove operations, and
  `WhatSonLibraryIndexedState` uses those paths to keep canonical and derived buckets synchronized without replacing
  the whole note snapshot on every local edit.
- `LibraryNotePreviewText.hpp` is the shared preview-text authority for library note cards and calendar note chips, so
  compact note renderers do not drift into separate headline rules.
- `LibraryHierarchyController` owns the hub-independent in-app scaffold (`All Library`, `Drafts`, `Today`) and must keep
  it visible during construction, empty depth refreshes, runtime snapshot refreshes, and load-failure recovery.
- `LibraryHierarchyModel.hpp` is now an item-struct/icon-helper header. The runtime item model exposed to QML is the
  shared `WhatSonHierarchyModel` owned by the controller.
- `WhatSonLibraryNoteListProjection` mirrors that scaffold label contract by using `Drafts` for notes that have no
  explicit hub-authored folder chips.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/hierarchy/library``  ( `docs/src/app/models/hierarchy/library/README.md` )
- Location:  `docs/src/app/models/hierarchy/library`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Current rule: `LibraryHierarchyController` always maintains independent `All Library`, `Drafts`, `Today` in-app scaffolds from the hub. The display item model is not a domain-specific model but a common `WhatSonHierarchyModel`.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
