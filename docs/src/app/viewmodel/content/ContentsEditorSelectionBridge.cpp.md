# `src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp`
- Source kind: C++ implementation
- File name: `ContentsEditorSelectionBridge.cpp`
- Approximate line count: 317

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

## Current Implementation Notes
- `refreshSelectedNoteSnapshot()` now supports polling-style sync from QML editors:
  - Reads the currently selected note id from the bound note-list model.
  - Invokes `reloadNoteMetadataForNoteId(QString)` on the bound content view-model when available.
  - Refreshes exported selection/count state (`selectedNoteId`, `selectedNoteBodyText`, `visibleNoteCount`) after
    reload attempts.
- `persistEditorTextForNote(...)` now prefers a direct file-store lane:
  - Enqueues `{noteId, noteDirectoryPath, bodyText}` onto a serialized background queue instead of performing note-file
    IO on the editor/UI path.
  - Re-reads the current `.wsnote` package inside that worker before each write, so the async body save does not reuse
    stale header snapshots from an earlier enqueue turn.
  - Performs the `.wsnbody` / `.wsnhead` write entirely off the editor thread.
  - Uses `editorTextPersistenceFinished(...)` as the only completion path back to QML.
  - Mirrors normalized body text and `lastModifiedAt` back into the active editable hierarchy viewmodel only after the
    background write succeeds.
  - Queues `requestTrackedStatisticsRefreshForNote(...)` onto the content view-model so the expensive
    `backlinkByCount` / `openCount` scan still happens outside the editor bridge itself.
- Repeated autosaves for the same note now coalesce in the pending queue, so while one save is in flight the bridge
  keeps only the newest queued body for that note instead of scheduling every intermediate debounce payload.
- `refreshNoteSelectionState()` is now header-metadata-centered:
  - a note-id transition only seeds the bound `{noteId, noteDirectoryPath}` session
  - increments `openCount` through `WhatSonNoteFileStatSupport::incrementOpenCountForNoteHeader(...)`
  - does not ask the content view-model to run `requestTrackedStatisticsRefreshForNote(..., true)` anymore
  - therefore avoids the hub-wide `.wsnbody` backlink rescan on ordinary note selection changes

### Classes and Structs
- None detected during scaffold generation.

### Enums
- None detected during scaffold generation.

## Intended Detailed Sections
- Responsibility and business role
- Ownership and lifecycle
- Public API or externally observed bindings
- Collaborators and dependency direction
- Data flow and state transitions
- Error handling and recovery paths
- Threading, scheduling, or UI affinity constraints when relevant
- Extension points, invariants, and known complexity hotspots
- Test coverage and missing verification

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
