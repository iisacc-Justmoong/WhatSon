# `src/app/file/note/WhatSonNoteFolderBindingRepository.hpp`

## Responsibility

This header declares the note-folder persistence gateway. It wraps `WhatSonLocalNoteFileStore` so
folder-management code no longer performs ad-hoc note header reads and writes inline.

## Public Contract

- `readDocument(...)` loads the note document needed for folder mutation flows.
- `writeDocument(...)` persists a prepared header-only update.
- `writeFolderBindings(...)` is the folder-specific convenience API that applies bindings while
  preserving existing header timestamp fields.

## Architectural Role

The repository is intentionally narrow: it does not decide which folders a note should have. That
decision belongs to `WhatSonNoteFolderBindingService`.
