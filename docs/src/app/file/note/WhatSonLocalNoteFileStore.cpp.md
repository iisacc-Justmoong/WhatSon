# `src/app/file/note/WhatSonLocalNoteFileStore.cpp`

## Role
`WhatSonLocalNoteFileStore` is the concrete filesystem adapter for `.wsnhead`, `.wsnbody`, `.wsnhistory`, and `.wsnversion`.

It creates notes, reads materialized note directories, updates persisted body/header state, and deletes local note storage.

## Body Parsing Contract
- `applyBodyDocumentText(...)` is the read-side body decoder.
- The decoder still detects `<resource ...>` tags for thumbnail metadata, but the editor text now comes from `WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(...)`.
- The read path also derives `bodyFirstLine` from `WhatSon::NoteBodyPersistence::firstLineFromBodyDocument(...)` so inline titles before the first paragraph survive indexing and editor reads consistently.
- This means empty paragraphs and whitespace-only paragraphs survive file reads instead of being normalized away.
- `<resource ... resourcePath="...">` now accepts `.wsresource` package paths.
- Resource attribute parsing now also supports unquoted values with path separators (for example `path=PreviewHub.wsresources/preview.wsresource`) so note bodies can reference package paths without forcing quote normalization first.
- When the reference points to a package directory, the store resolves `resource.xml`, follows its `asset path`, and uses the real packaged asset file for preview thumbnail URLs.

## Update Contract
- `updateNote(...)` still serializes the edited plain text back into paragraph nodes when a real body write is requested.
- The important guard is that no-op saves now compare against a faithful plain-text reconstruction of the existing `.wsnbody`, so the store is not forced into an unsolicited rewrite simply because the read path trimmed the body differently.

## Regression Coverage
- `tests/app/test_whatson_local_note_file_store.cpp` now verifies that a body containing empty and whitespace-only paragraphs round-trips through `readNote(...)`.
