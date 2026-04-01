# `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp`
- Source kind: C++ header
- File name: `BookmarksHierarchyViewModel.hpp`
- Approximate line count: 118

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

### Classes and Structs
- `SystemCalendarStore`
- `BookmarksHierarchyViewModel`

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

## Current Public Contract Addendum
- The header now exposes `reloadNoteMetadataForNoteId(QString)` so the active bookmarks note list
  can refresh its current row after the detail panel mutates `.wsnhead` metadata.
- `requestViewModelHook()` is now a declared slot (not inline signal forwarding only) so the
  implementation can trigger a file-backed bookmarks projection refresh.
- The capability methods inherited from `IHierarchyRenameCapability` and
  `IHierarchyCrudCapability` are explicitly marked `override` so warning-clean builds and contract
  drift detection stay aligned.
