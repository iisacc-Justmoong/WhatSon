# `src/app/models/detailPanel/session/WhatSonNoteHeaderSessionStore.cpp`

## Implementation Notes
- Constructor now initializes the `IWhatSonNoteHeaderSessionStore` base.
- Loading still owns `.wsnhead` parse/cache lifecycle for the active note context.
- `assignFolderBinding(...)` updates the in-memory header snapshot only; concrete note header persistence is disabled
  after note package deletion.
- `assignTag(...)` now appends a normalized tag value only when the active header does not already contain the same
  case-folded tag, then persists through the same shared cache entry.
- Indexed folder/tag removal mutates the cached header snapshot and marks the cache clean without file persistence.
