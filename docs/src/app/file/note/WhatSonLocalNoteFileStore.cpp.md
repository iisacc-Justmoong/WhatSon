# `src/app/file/note/WhatSonLocalNoteFileStore.cpp`

## Role
`WhatSonLocalNoteFileStore` is the concrete filesystem adapter for `.wsnhead`, `.wsnbody`, `.wsnversion`, and `.wsnpaint`.

It creates notes, reads materialized note directories, updates persisted body/header state, and deletes local note storage.

## Package Contract
- A note package now persists only these four files:
  - `<note-id>.wsnhead`
  - `<note-id>.wsnbody`
  - `<note-id>.wsnversion`
  - `<note-id>.wsnpaint`
- The store no longer creates or appends `.wsnhistory`.
- The store no longer creates `.meta` directories.
- `.wsnpaint` is treated as a dedicated paint-layer document, not as an attachment sidecar.

## Body Parsing Contract
- `applyBodyDocumentText(...)` is the read-side body decoder.
- The decoder still detects `<resource ...>` tags for thumbnail metadata.
- It now projects both:
  - `bodyPlainText` (search/list summary text)
  - `bodySourceText` (editor-facing canonical inline-tag source from `.wsnbody`)
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
  - empty/whitespace paragraph round-trip
  - inline-tag source serialization
  - Qt Rich HTML source serialization into canonical `.wsnbody` tags
