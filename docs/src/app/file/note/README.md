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
- `ContentsNoteManagementCoordinator.cpp`
- `ContentsNoteManagementCoordinator.hpp`
- `WhatSonHubNoteCreationService.cpp`
- `WhatSonHubNoteCreationService.hpp`
- `WhatSonHubNoteDeletionService.cpp`
- `WhatSonHubNoteDeletionService.hpp`
- `WhatSonHubNoteFolderClearService.cpp`
- `WhatSonHubNoteFolderClearService.hpp`
- `WhatSonHubNoteMutationSupport.cpp`
- `WhatSonHubNoteMutationSupport.hpp`
- `WhatSonLocalNoteDocument.hpp`
- `WhatSonLocalNoteFileStore.cpp`
- `WhatSonLocalNoteFileStore.hpp`
- `WhatSonNoteBodyCreator.cpp`
- `WhatSonNoteBodyCreator.hpp`
- `WhatSonNoteBodyPersistence.cpp`
- `WhatSonNoteBodyPersistence.hpp`
- `WhatSonNoteBodySemanticTagSupport.cpp`
- `WhatSonNoteBodySemanticTagSupport.hpp`
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
- `file/sync/ContentsEditorIdleSyncController` now owns the editor-side buffered fetch clock and best-effort
  lifecycle flush requests. This `file/note` directory only owns the downstream note-package management queue once a
  snapshot is already selected for async persistence.
- `ContentsNoteManagementCoordinator` now owns editor-adjacent note-management orchestration:
  - direct `.wsnote` persistence serialization
  - header-only `openCount` updates
  - tracked-stat refresh follow-up
  - post-persist metadata resync back into the bound content view-model
- Shared derived-statistic helpers now live under `src/app/file/statistic/WhatSonNoteFileStatSupport.*` rather than in
  this note-package directory.
- `.wsnhead` now carries a dedicated `fileStat` block for numeric detail-panel metadata.
- Note creation, note update, and editor note selection all participate in keeping that block
  synchronized with the current body/header state.
- Editor note selection now uses a header-only `openCount` rewrite path, so switching notes no longer forces a
  hub-wide `.wsnbody` backlink rescan.
- The `fileStat` schema is tracked as a documented package contract; this repository no longer maintains a dedicated
  scripted test for it.
- Empty-note body parsing now strips formatting-only `<body>` indentation so a newly created note opens on the first
  editable line instead of showing a phantom leading blank line.
- Body-save normalization now also owns inline hashtag promotion:
  - editor-visible `#label` source persists into `.wsnbody` as `<tag>label</tag>`
  - the same save transaction unions that tag into `.wsnhead`
  - new tags are inserted into `Tags.wstags` so the tags hierarchy can reload them as first-class entries
- Body-save normalization now also preserves proprietary inline formatting across paragraph boundaries by reopening any
  still-active style tags at the next serialized `<paragraph>`.
- `.wsnbody` semantic tag classification is now owned by `WhatSonNoteBodySemanticTagSupport.*` so the note-body save
  path and the editor HTML read paths resolve legacy body tags through the same registry.
- `fileStat.modifiedCount` is now the local commit counter for note package history.
  - whenever it advances, `.wsnversion` appends a snapshot with the matching `commitModifiedCount`
  - snapshot/diff persistence is delegated to `src/app/file/diff/WhatSonLocalNoteVersionStore.*`


## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
