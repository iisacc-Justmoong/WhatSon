# `src/app/models/hierarchy/library/LibraryNoteRecord.hpp`

## Responsibility

`LibraryNoteRecord` is the normalized runtime payload for one note in the library domain. It is the
shared data shape used by note list models, hierarchy filters, and retained runtime projection code.

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
- The record does not carry note body source or derived body preview fields.
- `progress == -1` represents `No progress` and is the neutral default for new/cleared notes.
- Structural equality now covers the full record payload so incremental index layers can suppress no-op upserts and
  avoid emitting wide rebuild signals when a note record did not actually change.

## Main Collaborators

- `LibraryAll.cpp`: owns the current library indexing boundary.
- `LibraryHierarchyController.cpp`: filters records by selected folder UUID.
