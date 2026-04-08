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
- The bridge no longer owns the serialized persistence queue or stat-refresh helpers directly.
- Instead it now:
  - keeps note-list property wiring (`currentNoteId`, `currentBodyText`, `itemCount`)
  - forwards persistence/snapshot requests into `ContentsNoteManagementCoordinator`
  - forwards `editorTextPersistenceFinished(...)` from that coordinator back to QML
- `refreshNoteSelectionState()` now delegates note-id transitions into the coordinator through
  `bindSelectedNote(...)` / `clearSelectedNote()`, so open-count maintenance also left the bridge.
- `setContentViewModel(...)` now rebinds the same content view-model into the coordinator and re-seeds the currently
  selected note session there.

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

## Regression Checks

- Note-list selection changes must still update `selectedNoteId` / `selectedNoteBodyText`, but they must no longer run
  open-count/stat maintenance directly inside the bridge implementation.
- The bridge must continue forwarding coordinator persistence completion signals so `ContentsEditorSession.qml` behavior
  does not regress.
