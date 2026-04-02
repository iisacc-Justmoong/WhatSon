# `src/app/viewmodel/detailPanel/DetailPanelViewModel.hpp`

## Responsibility
`DetailPanelViewModel` owns the active detail-panel page state, toolbar selection state, and the three detail-local hierarchy selector copies used by the properties form.

## Owned Objects
- `DetailContentSectionViewModel` instances for:
  - `properties`
  - `fileStat`
  - `insert`
  - `fileHistory`
  - `layer`
  - `help`
- `DetailHierarchySelectionViewModel` instances for:
  - `projectSelectionViewModel`
  - `bookmarkSelectionViewModel`
  - `progressSelectionViewModel`

## Public Wiring Surface
- `activeContentViewModel`
- `activeStateName`
- `toolbarItems`
- `projectSelectionViewModel`
- `bookmarkSelectionViewModel`
- `progressSelectionViewModel`
- `writeProjectSelection(int index)`
- `writeBookmarkSelection(int index)`
- `writeProgressSelection(int index)`
- `assignFolderByName(const QString& folderPath)`
- `removeActiveFolder()`
- `removeActiveTag()`
- `setCurrentNoteListModel(QObject*)`
- `setCurrentNoteDirectorySourceViewModel(QObject*)`

## Dependency Direction
The detail panel no longer binds QML selectors directly to the sidebar hierarchy viewmodels.
Instead, C++ injects those hierarchy viewmodels as read-only option sources into the owned selector-copy objects while a separate current-note context bridge resolves the active note id and note directory path.
That bridge keeps the last valid note context when the active sidebar domain does not expose note-list or note-directory contracts, so the selectors can continue mirroring the open note header instead of dropping to `No ...`.
`setCurrentNoteListModel(QObject*)` also observes an optional `itemsChanged()` signal from the active note-list model so the same selected note can force-refresh its `.wsnhead` metadata snapshot after out-of-band folder or tag edits.
The private write path now also routes successful metadata edits back into the active hierarchy domain through `reloadNoteMetadataForNoteId(QString)`, keeping the note list and detail panel in lockstep.

## Selection Semantics
- The three selector-copy objects expose a synthetic `No ...` item at index `0`.
- Passing index `0` to `writeProjectSelection(...)`, `writeBookmarkSelection(...)`, or `writeProgressSelection(...)` clears the corresponding field in the current `.wsnhead` file.
- Passing the currently selected index to any `write...Selection(...)` API is a no-op success path and does not trigger metadata persistence or hierarchy reload callbacks.
