# `src/app/viewmodel/hierarchy/tags/TagsHierarchyModel.cpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/tags/TagsHierarchyModel.cpp`
- Source kind: C++ implementation
- File name: `TagsHierarchyModel.cpp`
- Approximate line count: 223

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

### Classes and Structs
- `ValidationIssue`

### Enums
- None detected during scaffold generation.

## Current Implementation Notes
- `data(...)` now returns a canonical `iconName` role for every tag row.
- The fallback glyph is `vcscurrentBranch`, so the tags hierarchy uses the same icon family as note-list tag metadata
  instead of inheriting a generic folder-like hierarchy icon.
- Sanitization trims any explicit `iconName` coming from upstream row state but still preserves the canonical fallback
  when no tag-specific icon was supplied.

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
