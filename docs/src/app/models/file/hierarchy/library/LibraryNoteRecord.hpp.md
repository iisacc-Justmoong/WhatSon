# `src/app/models/file/hierarchy/library/LibraryNoteRecord.hpp`

## Responsibility

`LibraryNoteRecord` is the normalized runtime payload for one note in the library domain. It is the
shared data shape used by note list models, hierarchy filters, note mutation helpers, and local file
stores.

## Relevant Folder Fields

- `folders`: visible folder paths used for presentation and backward compatibility.
- `folderUuids`: stable folder identities aligned by index with `folders`.

The two lists are intentionally stored together so the application can keep a readable path while
filtering and mutation logic rely on stable UUIDs.

## Invariants

- `folderUuids` should be normalized before the record is used for filtering or persistence.
- Callers should keep `folders` and `folderUuids` aligned in order and count.
- Missing UUIDs are tolerated only as legacy fallbacks during import; modern writes should preserve
  UUIDs.
- `bodySourceText` is the editor-facing RAW note-body source. It may contain inline formatting tags such as
  `<bold>` and `<italic>`.
- `bodyPlainText` and `bodyFirstLine` are display/search projections. `normalizeBodyFields()` must derive them from
  the RAW source through `WhatSon::NoteBodyPersistence::serializeBodyDocument(...)` and
  `plainTextFromBodyDocument(...)`, so note-list preview cards never expose raw body tags.
- `progress == -1` represents `No progress` and is the neutral default for new/cleared notes.
- Structural equality now covers the full record payload so incremental index layers can suppress no-op upserts and
  avoid emitting wide rebuild signals when a note record did not actually change.

## Main Collaborators

- `LibraryAll.cpp`: produces runtime records from hub files.
- `WhatSonHubNoteCreationService.cpp`: creates new records for freshly scaffolded notes.
- `WhatSonHubNoteMutationSupport.cpp`: keeps runtime records synchronized with edited note
  documents.
- `LibraryHierarchyController.cpp`: filters records by selected folder UUID.
