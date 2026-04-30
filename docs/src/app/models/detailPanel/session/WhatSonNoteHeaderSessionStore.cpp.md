# `src/app/models/detailPanel/session/WhatSonNoteHeaderSessionStore.cpp`

## Implementation Notes
- Constructor now initializes the `IWhatSonNoteHeaderSessionStore` base.
- Loading still owns `.wsnhead` parse/cache lifecycle for the active note context.
- `assignFolderBinding(...)` keeps folder-path/folder-uuid persistence in one place through
  `WhatSonNoteFolderBindingService`.
- `assignTag(...)` now appends a normalized tag value only when the active header does not already contain the same
  case-folded tag, then persists through the same shared cache entry.
- Indexed folder/tag removal still mutates the cached header snapshot first and then writes the file once.
