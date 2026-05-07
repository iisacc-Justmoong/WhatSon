# `src/app/models/file/note/WhatSonLocalNoteDocument.hpp`

## Responsibility

`WhatSonLocalNoteDocument` is the parsed in-memory representation of one local note file set.

## Package Paths

- The document tracks package paths for:
  - `.wsnhead`
  - `.wsnbody`
  - `.wsnversion`
  - `.wsnpaint`

## Library Projection

`toLibraryNoteRecord()` now exports folder UUIDs together with folder paths when it translates the
document into `LibraryNoteRecord`. This makes the local-file read path consistent with the rest of
the UUID-aware library pipeline.

`normalizeBodyFields()` keeps note-body ownership split:
- `bodySourceText` remains the editor-facing RAW source projection.
- `bodyPlainText` and `bodyFirstLine` are recomputed from that source through the canonical body serializer/parser
  projection, so note-list previews and search surfaces see rendered text instead of literal inline tags.

## Main Collaborators

- `WhatSonLocalNoteFileStore`: populates the document from `.wsnhead` and `.wsnbody`.
- `LibraryAll.cpp`: consumes the projected runtime record.
- `WhatSonHubNoteMutationSupport.cpp`: keeps the projected record aligned after writes.
