# `src/app/file/note`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/note`
- Child directories: 0
- Child files: 33

## Child Directories
- No child directories.

## Child Files
- `WhatSonBookmarkColorPalette.hpp`
- `WhatSonHubNoteCreationService.cpp`
- `WhatSonHubNoteCreationService.hpp`
- `WhatSonHubNoteDeletionService.cpp`
- `WhatSonHubNoteDeletionService.hpp`
- `WhatSonHubNoteFolderClearService.cpp`
- `WhatSonHubNoteFolderClearService.hpp`
- `WhatSonNoteFileStatSupport.cpp`
- `WhatSonNoteFileStatSupport.hpp`
- `WhatSonHubNoteMutationSupport.cpp`
- `WhatSonHubNoteMutationSupport.hpp`
- `WhatSonLocalNoteDocument.hpp`
- `WhatSonLocalNoteFileStore.cpp`
- `WhatSonLocalNoteFileStore.hpp`
- `WhatSonLocalNoteVersionStore.cpp`
- `WhatSonLocalNoteVersionStore.hpp`
- `WhatSonNoteBodyCreator.cpp`
- `WhatSonNoteBodyCreator.hpp`
- `WhatSonNoteBodyPersistence.cpp`
- `WhatSonNoteBodyPersistence.hpp`
- `WhatSonNoteCreator.cpp`
- `WhatSonNoteCreator.hpp`
- `WhatSonNoteFolderBindingRepository.cpp`
- `WhatSonNoteFolderBindingRepository.hpp`
- `WhatSonNoteFolderBindingService.cpp`
- `WhatSonNoteFolderBindingService.hpp`
- `WhatSonNoteFolderSemantics.hpp`
- `WhatSonNoteHeaderCreator.cpp`
- `WhatSonNoteHeaderCreator.hpp`
- `WhatSonNoteHeaderParser.cpp`
- `WhatSonNoteHeaderParser.hpp`
- `WhatSonNoteHeaderStore.cpp`
- `WhatSonNoteHeaderStore.hpp`

## Current Focus Areas
- `.wsnhead` now carries a dedicated `fileStat` block for numeric detail-panel metadata.
- Note creation, note update, and editor note selection all participate in keeping that block
  synchronized with the current body/header state.
- The repository now also ships a static schema regression script:
  `scripts/test_wsnhead_file_stat_schema.py`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
