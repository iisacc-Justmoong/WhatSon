# `src/app/file/note/WhatSonLocalNoteFileStore.cpp`

## Role
`WhatSonLocalNoteFileStore` is the concrete filesystem adapter for `.wsnhead`, `.wsnbody`, `.wsnhistory`, and `.wsnversion`.

It creates notes, reads materialized note directories, updates persisted body/header state, and deletes local note storage.

## Body Parsing Contract
- `applyBodyDocumentText(...)` is the read-side body decoder.
- The decoder still detects `<resource ...>` tags for thumbnail metadata.
- It now projects both:
  - `bodyPlainText` (search/list summary text)
  - `bodySourceText` (editor-facing rich/source text from `.wsnbody`)
- The read path also derives `bodyFirstLine` from `WhatSon::NoteBodyPersistence::firstLineFromBodyDocument(...)` so inline titles before the first paragraph survive indexing and editor reads consistently.
- This means empty paragraphs and whitespace-only paragraphs survive file reads instead of being normalized away.
- `<resource ... resourcePath="...">` now accepts `.wsresource` package paths.
- Resource attribute parsing now also supports unquoted values with path separators (for example `path=PreviewHub.wsresources/preview.wsresource`) so note bodies can reference package paths without forcing quote normalization first.
- When the reference points to a package directory, the store resolves `resource.xml`, follows its `asset path`, and uses the real packaged asset file for preview thumbnail URLs.

## Update Contract
- `updateNote(...)` now canonicalizes body writes through `WhatSon::NoteBodyPersistence::serializeBodyDocument(...)`.
- This allows RichText editor payloads to be accepted while still writing canonical `.wsnbody` inline tags.
- During update, the store keeps `bodyPlainText` and `bodySourceText` synchronized from the serialized body so viewmodel/list binding and search/index projections do not drift.

## Regression Coverage
- `tests/app/test_whatson_local_note_file_store.cpp` verifies:
  - empty/whitespace paragraph round-trip
  - inline-tag source serialization
  - Qt Rich HTML source serialization into canonical `.wsnbody` tags
