# `src/app/models/file/note/folder`

## Scope
- Owns raw folder block inspection semantics only.
- Keeps raw folder block inspection separate from body serialization and hierarchy controller code.

## Files
- `WhatSonNoteFolderSemantics.hpp`

## Boundary
- Must not read or update note package metadata.
- Must not own hierarchy controller state, note body formatting, or hub-level mutation orchestration.
