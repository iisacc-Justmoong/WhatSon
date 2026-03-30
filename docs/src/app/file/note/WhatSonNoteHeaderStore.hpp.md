# `src/app/file/note/WhatSonNoteHeaderStore.hpp`

## Responsibility

`WhatSonNoteHeaderStore` is the normalized mutable container for `.wsnhead` metadata.

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

## Persistence Shape

At serialization time, folder bindings are written as:

```xml
<folders>
  <folder uuid="64-char-id">Research/Competitor</folder>
</folders>
```

The text node remains the readable path. The `uuid` attribute carries the stable semantic identity.

## Main Collaborators

- `WhatSonNoteHeaderParser.cpp`: populates the store from XML.
- `WhatSonNoteHeaderCreator.cpp`: serializes the store back to XML.
- `WhatSonHubNoteCreationService.cpp`: creates initial folder bindings for new notes.
- `WhatSonLibraryFolderHierarchyMutationService.cpp`: rewrites bindings when the folder tree changes.
