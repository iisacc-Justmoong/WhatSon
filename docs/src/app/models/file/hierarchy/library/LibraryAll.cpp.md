# `src/app/models/file/hierarchy/library/LibraryAll.cpp`

## Responsibility

`LibraryAll` builds the runtime note bucket that backs the "All Library" list. It reads `.wsnindex`,
`.wsnhead`, and body content, merges those sources into `LibraryNoteRecord`, and exposes a stable
projection to the library hierarchy controller.

## Folder UUID Role

The record-merging pipeline now carries both:

- human-readable folder paths
- stable folder UUIDs

When the index is incomplete, `LibraryAll` falls back to `.wsnhead` and aligns folder paths with
their UUID counterparts so later filters can match notes by folder identity instead of by raw path.

## Compatibility Behavior

- Older note data that only contains folder paths still loads.
- Newer note headers that store `<folder uuid="...">path</folder>` keep the UUID values intact.
- Merging code pads or repairs UUID lists so a caller never receives mismatched folder path / UUID
  arrays.
- When `.wsnhead` is present, its `<project>` field is authoritative even when it is explicitly
  empty. This clears stale project labels that may still be cached in `index.wsnindex`, so project
  hierarchy filters do not accidentally surface "No project" notes under a named project bucket.
- Runtime body indexing now derives `bodyFirstLine` from `firstLineFromBodyDocument(...)`, which preserves inline titles that appear before the first paragraph block.
- Runtime body indexing now also carries a dedicated editor-source projection (`bodySourceText`) built from canonical
  `.wsnbody` inline tags, while keeping `bodyPlainText` for search/preview filtering.
- Resource thumbnail extraction also understands `.wsresource` package references in `.wsnbody`, including unquoted shorthand resource tags like `path=Hub.wsresources/image.wsresource`, so library note cards now preview packaged assets instead of requiring raw file paths.
- The canonical `all notes` bucket now also supports change-gated single-note `upsertNote(...)`, `removeNoteById(...)`,
  and `noteById(...)` operations. Those operations are the basis for partial library/calendar refreshes after local
  note edits.

## Why This Matters

The library sidebar no longer treats a parent rename as a semantic folder change. `LibraryAll`
therefore has to preserve the original folder UUIDs through load-time normalization, otherwise note
filters would break after a hierarchy rename even if the underlying data was migrated correctly.
