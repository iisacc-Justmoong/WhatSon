# `src/app/models/file/note/folder`

## Scope
- Owns note-to-folder binding semantics, repository persistence, and folder binding service operations.
- Keeps raw folder block inspection separate from body serialization and hierarchy controller code.

## Files
- `WhatSonNoteFolderBindingRepository.*`
- `WhatSonNoteFolderBindingService.*`
- `WhatSonNoteFolderSemantics.hpp`

## Boundary
- May read and update note package metadata through the note file store.
- Must not own hierarchy controller state, note body formatting, or hub-level mutation orchestration.
