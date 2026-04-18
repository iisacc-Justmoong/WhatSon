# `src/app/viewmodel/detailPanel/DetailPanelViewModel.hpp`

## Responsibility
`DetailPanelViewModel` owns the active detail-panel page state, toolbar selection state, the dedicated `fileStat`
statistics object, and the three detail-local hierarchy selector copies used by the properties form.
It is now the reusable note-detail implementation base; the concrete runtime object mounted into the UI is
`NoteDetailPanelViewModel`, which inherits this contract unchanged.

## Owned Objects
- `DetailPropertiesViewModel` for `properties`
- `DetailFileStatViewModel` for `fileStat`
- `DetailContentSectionViewModel` instances for:
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
- `noteContextLinked`
- `fileStatViewModel`
- `toolbarItems`
- `projectSelectionViewModel`
- `bookmarkSelectionViewModel`
- `progressSelectionViewModel`
- `writeProjectSelection(int index)`
- `writeBookmarkSelection(int index)`
- `writeProgressSelection(int index)`
- `assignFolderByName(const QString& folderPath)`
- `assignTagByName(const QString& tag)`
- `removeActiveFolder()`
- `removeActiveTag()`
- `setCurrentNoteListModel(QObject*)`
- `setCurrentNoteDirectorySourceViewModel(QObject*)`
- `setTagsSourceViewModel(QObject*)`

## Dependency Direction
The detail panel no longer binds QML selectors directly to the sidebar hierarchy viewmodels.
Instead, C++ injects those hierarchy viewmodels as read-only option sources into the owned selector-copy objects while a separate current-note context bridge resolves the active note id and note directory path.
That bridge keeps the last valid note context when the active sidebar domain does not expose note-list or note-directory contracts, so the selectors can continue mirroring the open note header instead of dropping to `No ...`.
The same loaded header snapshot is also applied to the dedicated `DetailFileStatViewModel`, so the statistics tab reads
from the same persisted `.wsnhead` cache as the properties tab.
`noteContextLinked` is the gating contract for the panel surface: it flips to `true` only when the current note id and
directory are both resolved and the active `.wsnhead` snapshot loads successfully.
`setCurrentNoteListModel(QObject*)` also observes an optional `itemsChanged()` signal from the active note-list model so the same selected note can force-refresh its `.wsnhead` metadata snapshot after out-of-band folder or tag edits.
The private write path now also routes successful metadata edits back into the active hierarchy domain through `reloadNoteMetadataForNoteId(QString)`, keeping the note list and detail panel in lockstep.
It additionally tracks the canonical Tags hierarchy viewmodel as a secondary refresh target, so
tag writes performed while another sidebar domain is active still refresh the Tags note-list
projection.
Folder and tag add actions now share the same mutation shape from QML's point of view: the popup hands a canonical
string identifier to `assignFolderByName(...)` or `assignTagByName(...)`, and the viewmodel persists it before
re-applying the refreshed header snapshot.

## Selection Semantics
- The three selector-copy objects expose a synthetic `No ...` item at index `0`.
- Passing index `0` to `writeProjectSelection(...)`, `writeBookmarkSelection(...)`, or `writeProgressSelection(...)` clears the corresponding field in the current `.wsnhead` file.
- Passing the currently selected index to any `write...Selection(...)` API is a no-op success path and does not trigger metadata persistence or hierarchy reload callbacks.
