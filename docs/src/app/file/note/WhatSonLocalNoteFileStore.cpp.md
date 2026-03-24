# `src/app/file/note/WhatSonLocalNoteFileStore.cpp`

## Role
`WhatSonLocalNoteFileStore` is the concrete filesystem adapter for `.wsnhead`, `.wsnbody`, `.wsnhistory`, and `.wsnversion`.

It creates notes, reads materialized note directories, updates persisted body/header state, and deletes local note storage.

## Body Parsing Contract
- `applyBodyDocumentText(...)` is the read-side body decoder.
- The decoder still detects `<resource ...>` tags for thumbnail metadata, but the editor text now comes from `WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(...)`.
- This means empty paragraphs and whitespace-only paragraphs survive file reads instead of being normalized away.

## Update Contract
- `updateNote(...)` still serializes the edited plain text back into paragraph nodes when a real body write is requested.
- The important guard is that no-op saves now compare against a faithful plain-text reconstruction of the existing `.wsnbody`, so the store is not forced into an unsolicited rewrite simply because the read path trimmed the body differently.

## Regression Coverage
- `tests/app/test_whatson_local_note_file_store.cpp` now verifies that a body containing empty and whitespace-only paragraphs round-trips through `readNote(...)`.
