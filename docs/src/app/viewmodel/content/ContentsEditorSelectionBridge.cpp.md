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
- `refreshNoteSelectionState()` now also treats a note-id transition as a real "open" event for
  `.wsnhead fileStat` tracking:
  - Resolves `noteDirectoryPathForNoteId(QString)` from the active content view-model when that
    contract exists.
  - Calls `WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNote(..., incrementOpenCount=true)`
    so `openCount` and incoming backlink counts are refreshed when the editor selection changes.
  - Queues `reloadNoteMetadataForNoteId(QString)` back into the active hierarchy viewmodel so note-list
    snapshots and the detail panel can re-read the just-updated header after the open-count write lands.

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
