# `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyModel.cpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Runtime Contract
- `BookmarksHierarchyModel` exposes a dedicated `iconName` role.
- Every bookmark color row resolves that role through the shared `bookmarksbookmark` token.
- The `iconSource` role now carries a color-specific SVG data URI generated in C++ for each
  bookmark row.
- This keeps the logical glyph token fixed while restoring per-label bookmark color symmetry
  without depending on QML-side label parsing to decide the actual icon tint.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyModel.cpp`
- Source kind: C++ implementation
- File name: `BookmarksHierarchyModel.cpp`
- Approximate line count: 195

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

### Classes and Structs
- `ValidationIssue`

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
