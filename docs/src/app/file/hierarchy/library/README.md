# `src/app/models/file/hierarchy/library`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/hierarchy/library`
- Child directories: 0
- Child files: 18

## Child Directories
- No child directories.

## Child Files
- `LibraryAll.cpp`
- `LibraryAll.hpp`
- `LibraryDraft.cpp`
- `LibraryDraft.hpp`
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
