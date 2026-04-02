# `src/app/file/note/WhatSonLocalNoteDocument.hpp`

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

## Main Collaborators

- `WhatSonLocalNoteFileStore`: populates the document from `.wsnhead` and `.wsnbody`.
- `LibraryAll.cpp`: consumes the projected runtime record.
- `WhatSonHubNoteMutationSupport.cpp`: keeps the projected record aligned after writes.
