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

## Selection Semantics
- The three selector-copy objects expose a synthetic `No ...` item at index `0`.
- Passing index `0` to `writeProjectSelection(...)`, `writeBookmarkSelection(...)`, or `writeProgressSelection(...)` clears the corresponding field in the current `.wsnhead` file.
