# `src/app/models/hierarchy/projects/ProjectsHierarchyController.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/models/hierarchy/projects/ProjectsHierarchyController.hpp`
- Source kind: C++ header
- File name: `ProjectsHierarchyController.hpp`
- Approximate line count: 111

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

### Classes and Structs
- `ProjectsHierarchyController`

## Current Public Surface Highlights

- Implements rename, create/delete, reorder, and expansion capabilities for the projects hierarchy.
- Exposes a `LibraryNoteListModel` so the projects domain can surface notes whose `.wsnhead`
  `project` label matches the active hierarchy selection.
- Exposes `noteDirectoryPathForNoteId(...)` for detail-panel note-header writes, with an expected
  canonical directory resolution contract (indexed path first, `.wsnhead` directory fallback).
- Body-state update and editor statistic refresh APIs were removed with the note editor/save boundary.
- Exposes both `setItemExpanded(int, bool)` and `setAllItemsExpanded(bool)` so any future expandable
  projects rows can share the same projects-owned expansion state as the sidebar footer context menu.
- Declares inherited capability methods with explicit `override` so the reorder/rename/crud
  contract stays compile-time checked and warning-clean.
- Declares `applyHierarchyMove(...)` as a targeted helper for explicit nested project-folder moves. The sidebar
  drag/drop surface uses full-node replay from the final LVRS model snapshot.

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
