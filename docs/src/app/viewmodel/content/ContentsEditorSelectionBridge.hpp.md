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
- Public invokables include:
  - `persistEditorTextForNote(noteId, text)`
  - `refreshSelectedNoteSnapshot()`: triggers selected-note metadata/body refresh via connected content view-model
    contract (`reloadNoteMetadataForNoteId`) and re-synchronizes exposed selection/count properties.
- The bridge now also exposes `directPersistenceContractAvailable`, so QML can distinguish the async direct `.wsnote`
  save lane from the synchronous fallback `saveBodyTextForNote(...)` contracts.
- `editorTextPersistenceFinished(noteId, text, success, errorMessage)` is the completion signal for the editor save
  pipeline. QML sessions now clear or re-arm debounce state from that completion instead of assuming the write already
  finished when `persistEditorTextForNote(...)` returns.
- The bridge now also owns a serialized bound-note persistence session:
  - caches the selected note id / note directory path
  - reuses that path session for repeated editor autosaves instead of rediscovering the note package each time
  - also uses that metadata pair for note-selection open-count updates, so selection no longer depends on body scans
  - leaves the actual `.wsnote` read-modify-write to the background worker, so no note-file IO remains on the editor
    save enqueue path
  - treats direct `.wsnote` persistence as an implementation detail and leaves hub-wide stat refresh to the connected
    content view-model contract
  - keeps async save requests serialized so background `.wsnbody` writes do not race each other for one bridge

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
