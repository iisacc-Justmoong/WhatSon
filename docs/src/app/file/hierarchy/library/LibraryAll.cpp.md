# `src/app/file/hierarchy/library/LibraryAll.cpp`

## Role
`LibraryAll.cpp` scans the library storage, resolves note records, and produces the canonical runtime `LibraryNoteRecord` list used by the Library hierarchy and note list.

## Body Extraction
- `extractBodyContentFromWsnbody(...)` is the load-time body decoder for indexed note records.
- It still resolves resource thumbnails from `<resource ...>` tags, but plain-text body extraction now delegates to `WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(...)`.
- That shared parser preserves blank paragraphs and whitespace-only paragraphs, so the runtime note model exposes the same logical body text that the editor will compare during a save.

## Why The Shared Parser Matters
The library runtime and the local note file store must agree on what the body text actually is. If one side trims blank lines and the other side preserves them, the editor will observe false body changes and the next save can rewrite `.wsnbody` unexpectedly.

## Regression Coverage
- `tests/app/test_library_hierarchy_view_model.cpp` now verifies that an unchanged save preserves both blank paragraphs and whitespace-only paragraphs in an existing `.wsnbody` document.
