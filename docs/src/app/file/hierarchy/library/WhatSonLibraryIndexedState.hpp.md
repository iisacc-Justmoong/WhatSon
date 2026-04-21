# `src/app/models/file/hierarchy/library/WhatSonLibraryIndexedState.hpp`

## Responsibility

`WhatSonLibraryIndexedState` defines the backend state object that owns the canonical indexed
library note set plus the derived `draft` and `today` smart buckets.

## Public Contract

- `indexFromWshub(...)`: index the mounted hub once and rebuild all derived buckets.
- `applySnapshot(...)`: accept already computed runtime snapshot collections.
- `setIndexedNotes(...)`: replace the canonical note set and rebuild the derived buckets.
- `setSourceWshubPath(...)`: retarget the source hub path without touching the current note buckets.
- `upsertNote(...)` / `removeNoteById(...)`: update one canonical note and propagate that change into the derived
  `draft` / `today` buckets without forcing a full-state replacement.
- `noteById(...)`: expose single-note lookup for mutation and projection collaborators.
- `collectBookmarkedNotes(...)`: derive bookmark candidates from an existing library note vector
  without forcing another hub parse.

## Architectural Role

This header exists to move note-index bookkeeping out of `LibraryHierarchyViewModel` and to provide
the shared projection backend used by runtime loading, library viewmodels, and bookmarks viewmodels.
