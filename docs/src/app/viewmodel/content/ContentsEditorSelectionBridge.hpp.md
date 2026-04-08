# `src/app/viewmodel/content/ContentsEditorSelectionBridge.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/content/ContentsEditorSelectionBridge.hpp`
- Source kind: C++ header
- File name: `ContentsEditorSelectionBridge.hpp`
- Approximate line count: 98

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

## Current Implementation Notes
- `ContentsEditorSelectionBridge` is now selection-facing only:
  - exports `selectedNoteId`, `selectedNoteBodyText`, and `visibleNoteCount`
  - keeps the note-list selection/count property wiring for QML
  - forwards persistence and note-management requests to `file/sync/ContentsEditorIdleSyncController`
- Public invokables remain:
  - `persistEditorTextForNote(noteId, text)`
  - `stageEditorTextForIdleSync(noteId, text)`
  - `flushEditorTextForNote(noteId, text)`
  - `refreshSelectedNoteSnapshot()`
  and they now delegate to the sync boundary instead of performing note-management work inside the bridge itself.
- The bridge still exposes `directPersistenceContractAvailable` and `contentPersistenceContractAvailable`, but those
  values now come from the sync-owned downstream management boundary.
- The bridge now also forwards `editorTextPersistenceQueued(...)` so QML sessions can distinguish
  "idle/flush gate accepted the snapshot" from "the write already finished".
- `editorTextPersistenceFinished(noteId, text, success, errorMessage)` is still the completion signal consumed by QML
  editor sessions, but the bridge now only forwards the sync/controller result.

### Classes and Structs
- `ContentsEditorSelectionBridge`

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

## Regression Checks

- Selection refresh must not directly execute note-file persistence, stat refresh, or open-count maintenance logic.
- `persistEditorTextForNote(...)` must stay as an enqueue-facing contract for QML even after the sync split.
- Binding a new content view-model must also rebind the current selected note into the sync controller so note-path
  resolution remains valid after model replacement.
- QML must still be able to request an explicit async flush through `flushEditorTextForNote(...)` when the editor leaves
  the current note.
