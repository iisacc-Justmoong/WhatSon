# `src/app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp`
- Source kind: C++ header
- File name: `ProjectsHierarchyViewModel.hpp`
- Approximate line count: 111

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

### Classes and Structs
- `ProjectsHierarchyViewModel`

## Current Public Surface Highlights

- Implements rename, create/delete, reorder, and expansion capabilities for the projects hierarchy.
- Exposes a `LibraryNoteListModel` so the projects domain can surface notes whose `.wsnhead`
  `project` label matches the active hierarchy selection.
- Exposes `reloadNoteMetadataForNoteId(...)` so external metadata writers such as the detail panel
  can refresh one note's project membership without rebuilding the whole hub snapshot.
- Exposes `noteDirectoryPathForNoteId(...)` for detail-panel note-header writes, with an expected
  canonical directory resolution contract (indexed path first, `.wsnhead` directory fallback).
- Exposes `applyPersistedBodyStateForNote(...)` so body-only editor/validator writes can update the
  selected projects note projection in memory without forcing a second immediate metadata re-read.
- Exposes `requestTrackedStatisticsRefreshForNote(...)` so note-open stat refresh can move out of the editor bridge
  and into the projects-owned note projection path.
- Emits `hubFilesystemMutated()` on successful in-memory note-body projection updates so downstream mutation listeners
  observe the same contract already used by the library/bookmarks/progress hierarchy viewmodels.
- Exposes both `setItemExpanded(int, bool)` and `setAllItemsExpanded(bool)` so any future expandable
  projects rows can share the same projects-owned expansion state as the sidebar footer context menu.
- Declares inherited capability methods with explicit `override` so the reorder/rename/crud
  contract stays compile-time checked and warning-clean.

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
