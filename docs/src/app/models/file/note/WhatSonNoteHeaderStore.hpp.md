# `src/app/models/file/note/WhatSonNoteHeaderStore.hpp`

## Responsibility

`WhatSonNoteHeaderStore` is the normalized mutable container for `.wsnhead` metadata.

## Activity Metadata

- `lastOpenedAt() / setLastOpenedAt(...)` stores the persisted RAW note-open timestamp.
- The value lives alongside the other top-level lifecycle metadata, not inside the numeric `fileStat` block.
- Empty values are valid for notes that have never been opened since this field was introduced.

## Folder Binding API

The store now exposes three levels of folder access:

- `folders() / setFolders(...)`: legacy path-only access
- `folderUuids() / setFolderUuids(...)`: raw stable folder identity access
- `setFolderBindings(folders, folderUuids)`: aligned write API for modern callers

Modern code should prefer `setFolderBindings(...)` so both arrays stay synchronized.

## Progress Metadata API

- `progressEnums() / setProgressEnums(...)` carries the exact enum labels declared in the current
  `.wsnhead` `<progress enums="{...}">` attribute.
- `progress() / setProgress(...)` continues to store only the selected integer value.
- The in-memory default is `-1`, which represents `No progress`.
- The enum-label array and the selected integer are intentionally separated so callers can preserve
  custom note-local progress taxonomies while still mutating the active selection.

## File Statistics API

`WhatSonNoteHeaderStore` now also owns the numeric `fileStat` metadata block that backs the Figma
detail statistics panel.

- Header-derived counters:
  - `totalFolders`
  - `totalTags`
- Body-derived counters:
  - `letterCount`
  - `wordCount`
  - `sentenceCount`
  - `paragraphCount`
  - `spaceCount`
  - `indentCount`
  - `lineCount`
  - `backlinkToCount`
  - `backlinkByCount`
  - `includedResourceCount`
- Runtime counters:
  - `openCount`
  - `modifiedCount`

All counters are clamped to non-negative integers. `incrementOpenCount()` and
`incrementModifiedCount()` are the intended mutation entrypoints for lifecycle tracking.

## Persistence Shape

At serialization time, folder bindings are written as:

```xml
<folders>
  <folder uuid="64-char-id">Research/Competitor</folder>
</folders>
```

The text node remains the readable path. The `uuid` attribute carries the stable semantic identity.

The lifecycle header metadata now also includes:

```xml
<lastModified>2026-04-18T09:00:00Z</lastModified>
<lastOpened>2026-04-18T09:03:15Z</lastOpened>
```

The statistics block is serialized as:

```xml
<fileStat>
  <totalFolders>0</totalFolders>
  <openCount>0</openCount>
</fileStat>
```

## Main Collaborators

- `WhatSonNoteHeaderParser.cpp`: populates the store from XML.
- `WhatSonNoteHeaderCreator.cpp`: serializes the store back to XML.
- `WhatSonHubNoteCreationService.cpp`: creates initial folder bindings for new notes.
- `WhatSonLibraryFolderHierarchyMutationService.cpp`: rewrites bindings when the folder tree changes.
