# `src/app/viewmodel/detailPanel/session/WhatSonNoteHeaderSessionStore.cpp`

## Implementation Notes
- Constructor now initializes the `IWhatSonNoteHeaderSessionStore` base.
- Loading, persistence, and mutation logic are unchanged.
- The change separates selection-source consumers from the concrete session implementation.
