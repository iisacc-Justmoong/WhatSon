# `src/app/viewmodel/detailPanel/session/WhatSonNoteHeaderSessionStore.hpp`

## Role
`WhatSonNoteHeaderSessionStore` is the concrete note-header session cache and persistence service.

## Interface Alignment
- Implements `IWhatSonNoteHeaderSessionStore`.
- Keeps internal cache storage private while exposing only the session contract to sibling viewmodels.
- The public mutation surface now covers both `assignFolderBinding(...)` and `assignTag(...)`, so the detail-panel
  add flows can share the same file-backed session cache without reaching into raw note-header persistence code.
