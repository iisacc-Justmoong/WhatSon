# `src/app/file/note/WhatSonLocalNoteFileStore.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/file/note/WhatSonLocalNoteFileStore.hpp`
- Source kind: C++ header
- File name: `WhatSonLocalNoteFileStore.hpp`
- Approximate line count: 73

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

### Classes and Structs
- `WhatSonLocalNoteFileStore`
- `ReadRequest`
- `CreateRequest`
- `UpdateRequest`
- `DeleteRequest`

## UpdateRequest Contract

- `UpdateRequest` carries the local note document plus persistence toggles for header/body writes.
- `touchLastModified` updates the header timestamp during the write transaction.
- `incrementModifiedCount` now independently controls whether that transaction advances the persisted `fileStat.modifiedCount`.
- The intended split is:
  - explicit metadata / structural note writes: keep `incrementModifiedCount == true`
  - debounced editor body autosave: set `incrementModifiedCount == false`

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - callers must be able to update `lastModifiedAt` without also incrementing `modifiedCount`
  - existing update callers that do not override `incrementModifiedCount` must keep the legacy increment behavior

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
