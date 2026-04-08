# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp`

## Role
`LibraryHierarchyViewModel` owns library hierarchy state, note list projection, and library note mutations.

## Interface Alignment
- Still implements the hierarchy capability set.
- Now also implements `ILibraryNoteMutationCapability` so mutation-only consumers can avoid the full concrete type.
- System calendar injection now depends on `ISystemCalendarStore`.
- Exposes an indexed-note snapshot accessor so adjacent runtime collaborators can project the current library note
  metadata without reparsing the hub from disk.
- Exposes `indexedNoteRecordById(...)` and emits `indexedNoteUpserted(...)` so collaborators such as
  `CalendarBoardStore` can react to one note mutation without waiting for a full snapshot replacement.
- Emits `indexedNotesSnapshotChanged()` whenever that runtime note snapshot changes, so other runtime collaborators such
  as `CalendarBoardStore` can stay synchronized when a real bulk snapshot replacement happens.
- Exposes `activateNoteById(...)` so cross-surface callers such as calendar overlays can force the library hierarchy
  back to a visible/selectable state for one note without reimplementing library bucket/search rules in QML.
- That invokable remains part of the public QML-facing surface because calendar note chips must be able to reopen the
  matching library note from both desktop and mobile editor routes.
